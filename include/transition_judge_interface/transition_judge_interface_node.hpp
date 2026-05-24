#pragma once

#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "transition_recipe_test/msg/transition_request.hpp"

namespace transition_judge_interface
{

class TransitionJudgeInterfaceNode : public rclcpp::Node
{
public:
  TransitionJudgeInterfaceNode();
  ~TransitionJudgeInterfaceNode() = default;

private:
  void currentStateIdCallback(const std_msgs::msg::String::SharedPtr msg);

  // Subscriber
  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr current_state_id_sub_;

  // Publisher
  rclcpp::Publisher<transition_recipe_test::msg::TransitionRequest>::SharedPtr
    transition_request_pub_;

  // States
  std::string current_state_id_;
};

}  // namespace transition_judge_interface
