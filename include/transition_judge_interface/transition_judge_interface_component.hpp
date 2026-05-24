#pragma once

#include <optional>
#include <string>

namespace transition_judge_interface
{

struct TransitionDecision
{
  std::string from_state_id;
  std::string target_state_id;
};

// 状態遷移判定ロジック（静的メソッドのみ）
class TransitionJudge
{
public:
  // current_state_id と外部センサ情報から遷移要否を判定する。
  // 遷移不要なら std::nullopt を返す。
  static std::optional<TransitionDecision> Judge(
    const std::string & current_state_id);
  
  // 関数のオーバーロードが必要かもしれない→　引数が変化する可能性があるため
};

}  // namespace transition_judge_interface
