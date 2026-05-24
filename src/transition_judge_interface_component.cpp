#include "transition_judge_interface/transition_judge_interface_component.hpp"

namespace transition_judge_interface
{

std::optional<TransitionDecision> TransitionJudge::Judge(const TransitionInput & /*in*/)
{
  return std::nullopt;
}

}  // namespace transition_judge_interface
