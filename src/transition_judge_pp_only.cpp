// src/transition_judge_pp_only.cpp
//
// pure_pursuit_planner のみを管理対象とする構成（transition_recipe_test の config/pp_only.yaml,
// manager の recipe_set:=pp_only）用の判定ノード。
//
// 判定は起動時の 2 段だけ：
//   ALL_UNCONFIGURED -> ALL_CONFIGURED -> pure_pursuit_planner
// DWA が居ないので、障害物による pp -> dwa の判定は行わない（障害物 / odom も購読しない）。

#include <optional>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"

#include "transition_recipe_test/msg/transition_request.hpp"

namespace transition_judge_interface
{

class TransitionJudgePpOnlyNode : public rclcpp::Node
{
public:
  TransitionJudgePpOnlyNode()
  : rclcpp::Node("transition_judge_pp_only")
  {
    current_state_id_sub_ = this->create_subscription<std_msgs::msg::String>(
      "/current_state_id", 10,
      [this](const std_msgs::msg::String::SharedPtr msg) {
        current_state_id_ = msg->data;
      });

    transition_request_pub_ =
      this->create_publisher<transition_recipe_test::msg::TransitionRequest>(
      "/transition_request", 10);

    timer_ = this->create_wall_timer(
      std::chrono::milliseconds(100),
      std::bind(&TransitionJudgePpOnlyNode::timerCallback, this));

    RCLCPP_INFO(this->get_logger(), "TransitionJudgePpOnlyNode started");
  }

private:
  void timerCallback()
  {
    if (!current_state_id_.has_value()) {
      return;
    }

    std::string target;
    if (current_state_id_.value() == "ALL_UNCONFIGURED") {
      target = "ALL_CONFIGURED";
    } else if (current_state_id_.value() == "ALL_CONFIGURED") {
      target = "pure_pursuit_planner";
    } else {
      return;  // pure_pursuit_planner / UNKNOWN などでは何もしない
    }

    // edge 検出: 直近 publish した内容と一致する場合は同じ要求の連投を避けるため skip する。
    if (last_from_ == current_state_id_.value() && last_target_ == target) {
      return;
    }

    transition_recipe_test::msg::TransitionRequest out;
    out.from_state_id = current_state_id_.value();
    out.target_state_id = target;
    transition_request_pub_->publish(out);

    last_from_ = out.from_state_id;
    last_target_ = out.target_state_id;

    RCLCPP_INFO(
      this->get_logger(), "Published TransitionRequest: %s -> %s",
      out.from_state_id.c_str(), out.target_state_id.c_str());
  }

  rclcpp::Subscription<std_msgs::msg::String>::SharedPtr current_state_id_sub_;
  rclcpp::Publisher<transition_recipe_test::msg::TransitionRequest>::SharedPtr
    transition_request_pub_;
  rclcpp::TimerBase::SharedPtr timer_;

  std::optional<std::string> current_state_id_;
  std::string last_from_;
  std::string last_target_;
};

}  // namespace transition_judge_interface

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<transition_judge_interface::TransitionJudgePpOnlyNode>());
  rclcpp::shutdown();
  return 0;
}
