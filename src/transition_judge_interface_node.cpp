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
  TransitionInput in;
  in.current_state_id = current_state_id_;
  in.x = x_;
  in.time_span = time_span_[0];
  in.obstacles = obstacle_;

  const std::optional<TransitionDecision> decision = TransitionJudge::Judge(in);
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


void TransitionJudgeInterfaceNode::odomCallback(const nav_msgs::msg::Odometry::SharedPtr msg)
{
  // Odom callback implementation
  x_[0] = msg->pose.pose.position.x;
  x_[1] = msg->pose.pose.position.y;
  x_[2] = tf2::getYaw(msg->pose.pose.orientation);
  x_[3] = msg->twist.twist.linear.x;
  x_[4] = msg->twist.twist.angular.z;
}


void TransitionJudgeInterfaceNode::poseCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
{
  // Pose callback implementation
  x_[0] = msg->pose.position.x;
  x_[1] = msg->pose.position.y;
  x_[2] = tf2::getYaw(msg->pose.orientation);
  // these can be extracted from odom message
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

  if (!obstacle_.empty()) {
    received_obstacles_ = true;
  }
}


void TransitionJudgeInterfaceNode::timeSpanCallback(const std_msgs::msg::Float64::SharedPtr msg)
{
  // Time span callback implementation
  time_span_[0] = msg->data;
}


void TransitionJudgeInterfaceNode::localObstacleCallback(const visualization_msgs::msg::MarkerArray::SharedPtr msg)
{
  // Local obstacle callback implementation
  obstacle_.clear();
  for (const auto & marker : msg->markers) {
    obstacle_.push_back({marker.pose.position.x, marker.pose.position.y});
  }
  if (!obstacle_.empty()) {
    received_obstacles_ = true;
  }
}


void TransitionJudgeInterfaceNode::currentStateIdCallback(const std_msgs::msg::String::SharedPtr msg)
{
  current_state_id_ = msg->data;
}
}  // namespace transition_judge_interface
