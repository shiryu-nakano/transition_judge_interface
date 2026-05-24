#include "rclcpp/rclcpp.hpp"

#include "transition_judge_interface/transition_judge_interface_node.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<transition_judge_interface::TransitionJudgeInterfaceNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
