#pragma once

#include <array>
#include <string>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/float64.hpp"

#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "visualization_msgs/msg/marker_array.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"

#include "transition_recipe_test/msg/transition_request.hpp"


namespace transition_judge_interface
{

class TransitionJudgeInterfaceNode : public rclcpp::Node
{
public:
  TransitionJudgeInterfaceNode();
  ~TransitionJudgeInterfaceNode() = default;

private:
  // Callback
  void timerCallback();
  void currentStateIdCallback(const std_msgs::msg::String::SharedPtr msg); //current state_id; state_id is id of state graph
  void timeSpanCallback(const std_msgs::msg::Float64::SharedPtr msg); // elapsed time of current state
  void scanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg);
  void localObstacleCallback(const visualization_msgs::msg::MarkerArray::SharedPtr msg);
  void poseCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg);
  void odomCallback(const nav_msgs::msg::Odometry::SharedPtr msg); // odom info can be extracted from pose info

  // Subscriber
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr current_state_id_sub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<visualization_msgs::msg::MarkerArray>::SharedPtr local_obstacle_sub_;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr pose_sub_;
  rclcpp::Subscription<std_msgs::msg::Float64>::SharedPtr time_span_sub_;

  // Timer
  rclcpp::TimerBase::SharedPtr timer_;

  // Publisher
  rclcpp::Publisher<transition_recipe_test::msg::TransitionRequest>::SharedPtr transition_request_pub_;

  // States
  /* x_
  x_[0] = x         // position x in world frame [m]
  x_[1] = y         // position y in world frame [m]
  x_[2] = yaw       // heading angle [rad]
  x_[3] = v         // linear velocity [m/s]
  x_[4] = omega     // angular velocity [rad/s]
  */
  std::string current_state_id_; // current node_id in state graph defined in transition_recipe_node
  std::array<double, 5> x_{{0.0, 0.0, 0.0, 0.0, 0.0}};
  std::array<double, 1> time_span_{{0.0}};
  std::vector<std::array<double, 2>> obstacle_;
  bool received_obstacles_{false};
};

}  // namespace transition_judge_interface
