#include "transition_judge_interface/transition_judge_interface_component.hpp"

namespace transition_judge_interface
{

std::optional<TransitionDecision> TransitionJudge::Judge(const TransitionInput & input)
{

  // current stateがnullの場合は判定しない
  if (!input.current_state_id.has_value())
  {
    return std::nullopt;
  }


  return std::nullopt;
}

}  // namespace transition_judge_interface

