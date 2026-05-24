
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
  - 