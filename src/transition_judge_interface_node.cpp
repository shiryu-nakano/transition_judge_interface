#include "transition_judge_interface/transition_judge_interface_node.hpp"

#include <cmath>

#include "tf2/utils.hpp"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

#include "transition_judge_interface/transition_judge_interface_component.hpp"

namespace transition_judge_interface
{
// constructor
TransitionJudgeInterfaceNode::TransitionJudgeInterfaceNode()
: rclcpp::Node("transition_judge_interface")
{
  // 判定の閾値（config/params.yaml）。既定値は TransitionJudgeParams のもの
  const TransitionJudgeParams defaults;
  this->declare_parameter<double>("pp_to_dwa_dist_m", defaults.pp_to_dwa_dist_m);
  this->declare_parameter<double>("pp_to_dwa_half_angle_deg", defaults.pp_to_dwa_half_angle_deg);
  this->declare_parameter<double>("dwa_to_pp_dist_m", defaults.dwa_to_pp_dist_m);
  this->declare_parameter<double>("dwa_to_pp_half_angle_deg", defaults.dwa_to_pp_half_angle_deg);
  this->declare_parameter<double>("min_dwa_duration_sec", defaults.min_dwa_duration_sec);

  const TransitionJudgeParams p = loadJudgeParams();
  RCLCPP_INFO(
    this->get_logger(),
    "Judge params: pp->dwa %.2fm / ±%.1fdeg, dwa->pp %.2fm / ±%.1fdeg after %.1fs",
    p.pp_to_dwa_dist_m, p.pp_to_dwa_half_angle_deg,
    p.dwa_to_pp_dist_m, p.dwa_to_pp_half_angle_deg, p.min_dwa_duration_sec);
  if (p.dwa_to_pp_dist_m < p.pp_to_dwa_dist_m) {
    RCLCPP_WARN(
      this->get_logger(),
      "dwa_to_pp_dist_m (%.2f) < pp_to_dwa_dist_m (%.2f): pp <-> dwa may oscillate",
      p.dwa_to_pp_dist_m, p.pp_to_dwa_dist_m);
  }

  // subscriber
  current_state_id_sub_ = this->create_subscription<std_msgs::msg::String>(
    "/current_state_id", 10, 
    std::bind(&TransitionJudgeInterfaceNode::currentStateIdCallback, this, std::placeholders::_1));
  
  odom_sub_ = this->create_subscription<nav_msgs::msg::Odometry>(
    "/odom", 10, 
    std::bind(&TransitionJudgeInterfaceNode::odomCallback, this, std::placeholders::_1));
  
  local_obstacle_sub_ = this->create_subscription<visualization_msgs::msg::MarkerArray>(
    "global_obstacle_markers", 10,
    std::bind(&TransitionJudgeInterfaceNode::localObstacleCallback, this, std::placeholders::_1));

  scan_sub_ = this->create_subscription<sensor_msgs::msg::LaserScan>(
    "/scan", 10,
    std::bind(&TransitionJudgeInterfaceNode::scanCallback, this, std::placeholders::_1));

  pose_sub_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
    "/pose", 10,
    std::bind(&TransitionJudgeInterfaceNode::poseCallback, this, std::placeholders::_1));
  
  time_span_sub_ = this->create_subscription<std_msgs::msg::Float64>(
    "/state_timespan_sec", 10,
    std::bind(&TransitionJudgeInterfaceNode::timeSpanCallback, this, std::placeholders::_1));

  // timer
  timer_ = this->create_wall_timer(
    std::chrono::milliseconds(100),
    std::bind(&TransitionJudgeInterfaceNode::timerCallback, this)
  );

  // publisher
  transition_request_pub_ = this->create_publisher<transition_recipe_test::msg::TransitionRequest>(
    "/transition_request", 10);

  RCLCPP_INFO(this->get_logger(), "TransitionJudgeInterfaceNode started");
}

// ----- Callback methods -----
void TransitionJudgeInterfaceNode::timerCallback()
{
  // Node が保持している最新の状態を TransitionInput に詰めて Judge に渡す。
  // 受信フラグが立っていないフィールドは詰めない（std::nullopt のまま）。
  TransitionInput in;
  if (received_state_id_)  { in.current_state_id = current_state_id_; }
  if (received_x_)         { in.x                = x_; }
  if (received_time_span_) { in.time_span        = time_span_[0]; }
  if (received_obstacles_) { in.obstacles        = obstacle_; }

  const std::optional<TransitionDecision> decision = TransitionJudge::Judge(in, loadJudgeParams());
  if (!decision.has_value()) {
    return;
  }

  // edge 検出: 直近 publish した内容と一致する場合は同じ要求の連投を避けるため skip する。
  if (last_published_request_.has_value() &&
      last_published_request_->from_state_id == decision->from_state_id &&
      last_published_request_->target_state_id == decision->target_state_id)
  {
    return;
  }

  transition_recipe_test::msg::TransitionRequest out;
  out.from_state_id = decision->from_state_id;
  out.target_state_id = decision->target_state_id;
  transition_request_pub_->publish(out);

  last_published_request_ = decision;

  RCLCPP_INFO(
    this->get_logger(),
    "Published TransitionRequest: %s -> %s",
    decision->from_state_id.c_str(),
    decision->target_state_id.c_str());
}


TransitionJudgeParams TransitionJudgeInterfaceNode::loadJudgeParams()
{
  TransitionJudgeParams p;
  p.pp_to_dwa_dist_m = this->get_parameter("pp_to_dwa_dist_m").as_double();
  p.pp_to_dwa_half_angle_deg = this->get_parameter("pp_to_dwa_half_angle_deg").as_double();
  p.dwa_to_pp_dist_m = this->get_parameter("dwa_to_pp_dist_m").as_double();
  p.dwa_to_pp_half_angle_deg = this->get_parameter("dwa_to_pp_half_angle_deg").as_double();
  p.min_dwa_duration_sec = this->get_parameter("min_dwa_duration_sec").as_double();
  return p;
}


void TransitionJudgeInterfaceNode::odomCallback(const nav_msgs::msg::Odometry::SharedPtr msg)
{
  // Odom callback implementation
  x_[0] = msg->pose.pose.position.x;
  x_[1] = msg->pose.pose.position.y;
  x_[2] = tf2::getYaw(msg->pose.pose.orientation);
  x_[3] = msg->twist.twist.linear.x;
  x_[4] = msg->twist.twist.angular.z;
  received_x_ = true;
}


void TransitionJudgeInterfaceNode::poseCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
{
  // Pose callback implementation
  x_[0] = msg->pose.position.x;
  x_[1] = msg->pose.position.y;
  x_[2] = tf2::getYaw(msg->pose.orientation);
  // these can be extracted from odom message
  received_x_ = true;
}


void TransitionJudgeInterfaceNode::scanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg)
{
  // Scan callback implementation
  obstacle_.clear();
  double angle = msg->angle_min;
  for (size_t i = 0; i < msg->ranges.size(); ++i)
  {
      float r = msg->ranges[i];
      if (std::isfinite(r) && (r >= msg->range_min && r <= msg->range_max))
      {
          // obstacle_ はロボット中心座標系（base_link）で保持する。
          // Judge は「ロボットから見た角度・距離」で判定するため世界座標系への変換は不要。
          double ox = r * std::cos(angle);
          double oy = r * std::sin(angle);
          obstacle_.push_back({ox, oy});
      }
      angle += msg->angle_increment;
  }

  // scan message を受信した時点で「障害物データを受信した」とみなす。
  // 障害物が検知されていない（obstacle_ が空）状態と未受信を区別するため、空でも true にする。
  received_obstacles_ = true;
}


void TransitionJudgeInterfaceNode::timeSpanCallback(const std_msgs::msg::Float64::SharedPtr msg)
{
  // Time span callback implementation
  time_span_[0] = msg->data;
  received_time_span_ = true;
}


void TransitionJudgeInterfaceNode::localObstacleCallback(const visualization_msgs::msg::MarkerArray::SharedPtr msg)
{
  // global_obstacle_markers は map (world) 座標系で publish されてくる。
  // Judge はロボット中心座標系（base_link）で角度・距離判定するため、ここで world → robot 変換を行う。
  // 変換にはロボットの (x, y, yaw) が必要なので、odom/pose が一度も来ていない状態では
  // 変換できない → obstacle_ を更新せず、受信フラグも立てない。
  if (!received_x_) {
    return;
  }

  const double rx = x_[0];
  const double ry = x_[1];
  const double cos_yaw = std::cos(x_[2]);
  const double sin_yaw = std::sin(x_[2]);

  obstacle_.clear();
  for (const auto & marker : msg->markers) {
    const double dx = marker.pose.position.x - rx;
    const double dy = marker.pose.position.y - ry;
    // 並進: ロボット位置を原点に
    // 回転: ロボットの yaw 分だけ逆回転して robot frame に揃える
    const double bx =  cos_yaw * dx + sin_yaw * dy;
    const double by = -sin_yaw * dx + cos_yaw * dy;
    obstacle_.push_back({bx, by});
  }
  received_obstacles_ = true;
}


void TransitionJudgeInterfaceNode::currentStateIdCallback(const std_msgs::msg::String::SharedPtr msg)
{
  current_state_id_ = msg->data;
  received_state_id_ = true;
}
}  // namespace transition_judge_interface
