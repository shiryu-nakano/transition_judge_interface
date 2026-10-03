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
  std::optional<std::string> current_state_id;

  // ロボットの状態量（world frame）。
  // x[0] = x [m], x[1] = y [m], x[2] = yaw [rad],
  // x[3] = v [m/s], x[4] = omega [rad/s].
  std::optional<std::array<double, 5>> x;

  // 現在の状態に滞在し続けている経過時間 [s]。
  std::optional<double> time_span{0.0};

  // 検知された障害物群（base_link 座標系）。
  // 各要素 obstacles[i] = {x, y}（ロボット正面が +x、左が +y）。
  std::optional<std::vector<std::array<double, 2>>> obstacles;
};

// 判定の閾値。Node 側で config/params.yaml（ROS パラメータ）から読み込んで渡す。
// 既定値は yaml が無い場合（シミュレーション launch など）に使われる。
struct TransitionJudgeParams
{
  // pp -> dwa: 前方 ±pp_to_dwa_half_angle_deg、距離 pp_to_dwa_dist_m 以内に障害物があれば dwa へ
  double pp_to_dwa_dist_m{2.0};
  double pp_to_dwa_half_angle_deg{30.0};

  // dwa -> pp: dwa に min_dwa_duration_sec 以上滞在し、
  // 前方 ±dwa_to_pp_half_angle_deg、距離 dwa_to_pp_dist_m 以内が空いていれば pp へ。
  // dwa_to_pp_dist_m を pp_to_dwa_dist_m より短くすると、その間に障害物がある場合 pp <-> dwa を往復する。
  double dwa_to_pp_dist_m{2.0};
  double dwa_to_pp_half_angle_deg{30.0};
  double min_dwa_duration_sec{7.0};
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
  static std::optional<TransitionDecision> Judge(
    const TransitionInput & in, const TransitionJudgeParams & params = TransitionJudgeParams{});
};

}  // namespace transition_judge_interface
