#include "transition_judge_interface/transition_judge_interface_component.hpp"

#include <cmath>

namespace transition_judge_interface
{

std::optional<TransitionDecision> TransitionJudge::Judge(const TransitionInput & input)
{

  // current stateがnullの場合は判定しない
  if (!input.current_state_id.has_value())
  {
    return std::nullopt;
  }


  // ALL_UNCONFIGUREDの場合→ALL_CONFIGUREDに変更する
  if (input.current_state_id.value() == "ALL_UNCONFIGURED") {
    return TransitionDecision{input.current_state_id.value(), "ALL_CONFIGURED"};
  }

  // ALL_CONFIGUREDの場合→ひとまず実験としてppにする
  if (input.current_state_id.value() == "ALL_CONFIGURED") {
    return TransitionDecision{input.current_state_id.value(), "pure_pursuit_planner"};
  }



  // current == pure_pursuit_nodeの場合→dwaに遷移する条件を書く
  // 前方 ±30deg、距離 3m 以内に障害物が 1 つでもあれば dwa_node へ遷移する。
  if (input.current_state_id.value() == "pure_pursuit_planner") {
    if (input.obstacles.has_value()) {
      
      constexpr double kFrontHalfAngleRad = M_PI / 12.0;  // ±30deg
      constexpr double kDistThreshM = 2.0;

      for (const auto & o : input.obstacles.value()) {
        const double angle = std::atan2(o[1], o[0]);
        const double dist  = std::hypot(o[0], o[1]);
        if (std::abs(angle) <= kFrontHalfAngleRad && dist <= kDistThreshM) {
          return TransitionDecision{input.current_state_id.value(), "dwa_planner"};
        }
      }
    }
  }

  // [実験用] dwa_planner に 2 秒以上滞在したら無条件で pp に戻す。
  if (input.current_state_id.value() == "dwa_planner") {
    if (input.time_span.has_value() && input.time_span.value() >= 2.0) {
      return TransitionDecision{input.current_state_id.value(), "pure_pursuit_planner"};
    }
  }

  // current == dwa_nodeの場合→ppに遷移する条件を書く
  // dwa に滞在し続けて 7 秒以上経過しており、かつ前方 ±30deg / 3m 以内に障害物が無ければ
  // pure_pursuit_node に戻す。境界近辺での pp↔dwa 振動を抑えるための時間ヒステリシス。
  /*
  if (input.current_state_id.value() == "dwa_planner") {
    // 経過時間が無いと「7 秒以上滞在した」ことを判定できないため遷移しない。
    if (!input.time_span.has_value()) {
      return std::nullopt;
    }
    constexpr double kMinDwaDurationSec = 7.0;
    if (input.time_span.value() < kMinDwaDurationSec) {
      return std::nullopt;
    }

    // 障害物未受信の状態で pp に戻すのは危険なため、ここでは何もしない（dwa を維持）。
    //if (!input.obstacles.has_value()) {
    //  return std::nullopt;
    //}

    constexpr double kFrontHalfAngleRad = M_PI / 6.0;  // ±30deg
    constexpr double kDistThreshM = 3.0;

    for (const auto & o : input.obstacles.value()) {
      const double angle = std::atan2(o[1], o[0]);
      const double dist  = std::hypot(o[0], o[1]);
      if (std::abs(angle) <= kFrontHalfAngleRad && dist <= kDistThreshM) {
        return std::nullopt;  // 前方に障害物が残っているので dwa を維持
      }
    }

    // 前方クリア & 時間経過済み → pp に戻す
    return TransitionDecision{input.current_state_id.value(), "pure_pursuit_planner"};
  }
  */


  return std::nullopt;
}

}  // namespace transition_judge_interface


// 将来的にはここを、COR, Strategyで実装するが、現在実験中なのでベタ書きでOK
/*
launchを書いてほしいです。一つ改装を戻ると、pure_pursuit_planner
のディレクトリがあって、そこのlaunchにはobstacle simulatorがありますし、もしくはdwa_plannerのlaunchにも同様な呼び出しを行っているlaunch
が存在します。それを参考にして、このディレクトリに障害物ありで実験をスタートするラウンチを書いてください

ここでは状態遷移自体を担当するものはありませんが、一旦どうさすればOKとします。
*/
//TODO
/*
実機で動くようにする→別のcomponentでもいいかも、オーバーライドすれば

実機のlidarの場合はlidar側の前処理でロボットから一番近い障害物との距離が出るのでその距離だけで判断すればOK


*/