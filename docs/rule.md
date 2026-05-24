Skip to:
Top Bar
Sidebar
Main Content


検索

作成

アップグレード





先頭へ戻る
タイトルで検索

ユーザーを招待

02.実装-リポジトリ構成規約
5月 11, 2025 に更新
nkn4ryu
編集

共有


02.実装-リポジトリ構成規約



作成者: Keigo Miyama

閲覧数を表示

リアクションを追加
1. 目的
2. リポジトリ構成
3. 各レイヤの役割
4. 設計思想
1. 目的
構造の一貫性の明確化

可読性・保守性の向上

テストの自動化・品質ゲートの安定化

2. リポジトリ構成


***（リポジトリ名）/
├── include/
│   └── ***/
│       ├── ***_node.hpp        ← Nodeクラス（ROSインターフェース部分のみでロジックとは切り離す）
│       └── ***_component.hpp   ← ロジッククラス（純粋な演算部分でROS特有の処理とは完全に切り離す）
├── src/
│   ├── ***.cpp            　　 ← main実装（Node呼び出しのみ）
│   ├── ***_node.cpp            ← Node実装（Publisher、Subscriber、パラメータ設定、ロジック呼び出し）
│   └── ***_component.cpp       ← ロジックの実装（ロジックの各関数がテスト対象）
└── test/
│   └── test_data/
│       ├── 関数名.csv    　　　　　  ← 関数毎のテストケース（csvファイルで時系列データを定義）
│       └── test_results_関数名.csv   ← 関数毎のテスト結果（期待値、演算値、OK or NG判定結果が出力されている）
│   ├── test_***_component.cpp  ← GoogleTestで***_component.cppの単体テストを実施するテストコード
│   └── test_utils.hpp          ← テストコード内の共通処理を実装（テストデータ読み込み、結果出力など）
├── config/
│   └── params.yaml             ← ロジック内で使用するパラメータと設定値
├── launch/
│   └── ***_launch.py           ← 複数ノードやパラメータをまとめて起動する
├── CMakeLists.txt
└── package.xml
（参考）https://github.com/Arcanain/dwa_planner/tree/feature/unit_test

3. 各レイヤの役割
パス

種類

役割

詳細説明

include/your_package/your_node.hpp

ヘッダー

Node クラス定義

・Publisher／Subscriber とパラメータ取得のインターフェースだけを宣言
・ロジックには依存させない

include/your_package/your_component.hpp

ヘッダー

ロジッククラス定義

・制御計算など純粋演算メソッドのシグネチャだけを定義
・ROS型を使わず単体テスト可能な状態

src/your_package.cpp

ソース

main()

・rclcpp::init()→your_nodeを生成→rclcpp::spin() だけ
・アプリケーションのエントリーポイント

src/your_node.cpp

ソース

Node 実装

Publisher／Subscriber の初期化、パラメータ読み込み、コールバック内で your_component を呼び出し、結果を publishするのみ

src/your_component.cpp

ソース

ロジック実装

・アルゴリズムなどのコア演算部分
・ROS依存なしで記述し、再利用・テストしやすく記述する

test/test_data/function_a.csv

データ

入力データ定義

・CSV で入力時系列や期待値を定義
・関数毎にファイルを分けて管理
・関数 a() に与える時系列入力（例：時間, センサ値…）をテキストで管理

test/test_data/test_results_function_a.csv

データ

出力比較結果ファイル

テスト実行後に自動で出力される「実際の演算値」「期待値」「OK/NG 判定」を格納

test/test_your_component.cpp

ソース

単体テストコード

・GoogleTest で your_component の各メソッドを検証
・CSV から読み込んだ入力で演算→期待値と比較

test/test_utils.hpp

ヘッダー

テスト共通ヘルパー

CSV 読み込み・結果出力などの共通処理をまとめて重複を排除

config/params.yaml

設定ファイル

パラメータ定義

・ロジックや Node が使用するゲイン値やトピック名などを YAML で外部化
・ソース修正不要でチューニング可能

launch/your_launch.py

Launch

起動設定

複数ノードの同時起動、パラメータファイル指定、トピックのリマップなどを一元管理

CMakeLists.txt

ビルド設定

ビルドスクリプト

include パス追加、ライブラリリンク、テスト有効化（ament_add_gtest）などを定義

package.xml

メタ情報

パッケージマニフェスト

名前／バージョン／依存関係／ライセンスなどを宣言

4. 設計思想
純粋ロジックの切り出し  
***_component.hpp/.cpp は ROSの型（rclcpp::Node や geometry_msgs::msg::Pose）に依存させず、
標準型（std::vector や自作の構造体）だけで書くことで、  

単体テストが可能

他プロジェクトへの流用が容易

Node クラスの責務を明確化  
***_node はあくまで「ROSのInput/Output」を担うだけ。  

パラメータ読み込み → *** にコンストラクタ引数で渡す  

Subscriber コールバック → 入力を整形して ***::compute() を呼ぶ  

Publisher → ***::compute() から返ってきた結果をそのまま publish  

テストの自動化  
test/ 以下に GoogleTest（gtest）で「***の出力が期待値と合うか」を細かく書いておけば、
CICDでの組み込みが容易

関連コンテンツ

04.テスト工程規約
Keigo Miyama
05.テスト設計規約
Keigo Miyama
03.実装-コーディング規約
Keigo Miyama
03.詳細設計規約
Keigo Miyama
2025-05-11 ミーティング議事録
nkn4ryu
2025-10-26 ミーティング議事録
nkn4ryu





コメントを追加

ラベルを追加

リアクションを追加