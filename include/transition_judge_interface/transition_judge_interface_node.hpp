#pragma once

#include <memory>
#include <string>
#include <array>
#include <vector>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/bool.hpp"
#include <std_msgs/msg/float64.hpp>

#include "transition_recipe_test/msg/transition_request.hpp"

#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "visualization_msgs/msg/marker_array.hpp"
#include "tf2_ros/static_transform_broadcaster.h"
#include <nav_msgs/msg/path.hpp>
#include <sensor_msgs/msg/laser_scan.hpp>


namespace transition_judge_interface
{

class TransitionJudgeInterfaceNode : public rclcpp::Node
{
public:
  TransitionJudgeInterfaceNode();
  ~TransitionJudgeInterfaceNode() = default;

private:
  // Callback
  void timerCalback();
  void currentStateIdCallback(const std_msgs::msg::String::SharedPtr msg); //current state_id; state_id is id of state graph
  void timeSpanCallback(const std_msgs::smg::Fload64::SharedPtr msg); // elapsed time of current state
  void scanCallback(const sensor_msgs::msg::LaserScan::SharedPtr msg);
  void localObstacleCallback(const visualization_msgs::msg::MarkerArray::SharedPtr msg);
  void posecCallback(const geometry_msgs::msg::PoseStamped::SharedPtr msg);
  void odomCallback(const nav_msgs::msg::Odometry::SharedPtr msg); // odom info can be extracted from pose info

  // Subscriber
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr current_state_id_sub_;
  rclcpp::Subscription<nav_msgs::msg::Odometry>::SharedPtr odom_sub_;
  rclcpp::Subscription<visualization_msgs::msg::MarkerArray>::SharedPtr local_obstacle_sub_;
  rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr scan_sub_;
  rclcpp::Subscription<geometry_msgs::msg::PoseStamped>::SharedPtr pose_sub_;

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
  std::string current_state_id_; // current node_id in state graph defined in transitision_recipe_node
  std::array<double, 5> x_;
  std::array<double, 1> time_span_;
  std::vector<std::array<double, 2>> obstacle_;
  bool received_obstacles_;
};

}  // namespace transition_judge_interface
