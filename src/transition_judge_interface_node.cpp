#include "transition_judge_interface/transition_judge_interface_node.hpp"

#include "transition_judge_interface/transition_judge_interface_component.hpp"

namespace transition_judge_interface
{

TransitionJudgeInterfaceNode::TransitionJudgeInterfaceNode()
: rclcpp::Node("transition_judge_interface")
{
  current_state_id_sub_ = this->create_subscription<std_msgs::msg::String>("/current_state_id", 10, std::bind(&TransitionJudgeInterfaceNode::currentStateIdCallback, this, std::placeholders::_1));


  transition_request_pub_ = this->create_publisher<transition_recipe_test::msg::TransitionRequest>("/transition_request", 10);

  RCLCPP_INFO(this->get_logger(), "TransitionJudgeInterfaceNode started");
}

void TransitionJudgeInterfaceNode::currentStateIdCallback(
  const std_msgs::msg::String::SharedPtr msg)
{
  current_state_id_ = msg->data;

  const auto decision = TransitionJudge::Judge(current_state_id_);
  if (!decision.has_value()) {
    return;
  }

  transition_recipe_test::msg::TransitionRequest out;
  out.from_state_id = decision->from_state_id;
  out.target_state_id = decision->target_state_id;
  transition_request_pub_->publish(out);
}

}  // namespace transition_judge_interface
