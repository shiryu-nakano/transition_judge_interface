#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace transition_judge_interface
{

// Judge への入力をまとめた構造体。
// Node が保持している状態を 1 つのスナップショットとして Judge に渡す。
// フィールドを増やす場合は、CSV ベースのテストフィクスチャも合わせて更新する。
struct TransitionInput
{
  // 現在の状態 ID（state graph のノード ID）。
  std::string current_state_id;

  // ロボットの状態量（world frame）。
  // x[0] = x [m], x[1] = y [m], x[2] = yaw [rad],
  // x[3] = v [m/s], x[4] = omega [rad/s].
  std::array<double, 5> x{{0.0, 0.0, 0.0, 0.0, 0.0}};

  // 現在の状態に滞在し続けている経過時間 [s]。
  double time_span{0.0};

  // 検知された障害物群（base_link 座標系）。
  // 各要素 obstacles[i] = {x, y}（ロボット正面が +x、左が +y）。
  std::vector<std::array<double, 2>> obstacles;
};

struct TransitionDecision
{
  std::string from_state_id;
  std::string target_state_id;
};

// 状態遷移判定ロジック（静的メソッドのみ）。
//
// 入力は TransitionInput 構造体に統一する。フィールドが増えてもシグネチャは
// 変わらないので、呼び出し側・テスト側の追従コストが小さい。
// 判定ロジックの種類が増えたら、Judge ではなく JudgeByStateId / JudgeByTimeSpan
// のような static メソッドを追加し、本 Judge はそれらを集約する形にする。
class TransitionJudge
{
public:
  // 統合判定。遷移不要なら std::nullopt を返す。
  static std::optional<TransitionDecision> Judge(const TransitionInput & in);
};

}  // namespace transition_judge_interface
