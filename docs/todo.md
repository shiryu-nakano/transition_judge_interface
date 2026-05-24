
**依存関係の解消**
* [ ] transition_recipe_testが用意している
  * [ ] msg/TransiitonRequestを利用できること

**入力**
- string: current_state_id
- T : 外部センサ情報がたくさん
- 


**出力**
- transition_request
  - 必要なときにのみpublishする

**やること(0524)**
- [ ] 最低限のnodeの実装
  - [ ] 規約を守ること
    - 規約については後述する
- 動作確認
  - [ ] transition_recipe_testのノード群を起動するだけのlaunchを書く
  - [ ] p.p のシミュレーション用のlaunch
  - [ ] dwaのシミュレーション用のlaunch
  
  - dwaをlifecycleで
    - build pass
    - run pass
  - [ ] pp＋dwaを両方とも起動できるlaunch
  

---
- メンバ未初期化（#3） — scanCallback が最初に飛んできた瞬間に未初期化の x_[0], x_[1] でゴミ座標を世界座標に push してしまう
  - →対処しましょう
- 死コードの削除（#7） — 単純清掃
  - 対処しました
- 二重上書き整理（#1, #2） — 設計判断（どちら使うか）が必要
  - これはまだこのままでいいです

- scanCallback の yaw（#4） — 並進＋回転の正しい変換
  - これはどういう問題なのかがわかっていないので、理解してから対処を考えたいです
- トピック名不一致（#5, #6） — transition_recipe_test 側を見て名前合わせ
  - state_timespan_secは直しました
  - global, localは一旦このままでいいです
- 未使用 include 整理（#10）
  - これは？
- TransitionInput 構造体化（#8） — Judge の中身を書き始める前にやっておきたい
  - オーバーロードするとだめでしょうか？
    - むしろ、多様な入力に合わせて、それを構造体に
- edge 検出（#9） — Judge の中身が出てから
  - これは重要ですね
