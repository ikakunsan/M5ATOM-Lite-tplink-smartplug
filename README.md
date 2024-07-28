# M5ATOM Lite スマートプラグスイッチ

## 概要
M5ATOMを、TP-Linkのスマートプラグ (HS105) のスイッチにしてOn/Offのコントロールをします。

スマートプラグを直接制御するので、TP-Linkのクラウドに接続する必要はなく、レスポンスも良好です。
タイマーを内蔵していて、プログラム時にあらかじめ設定した時刻にOn/Offをします。内部時計の時刻はNTPで取得します。
また、MQTTのsubscriberの機能も持たせているので、組み込みタイマー以外にも外部のプログラムから簡単にプラグのOn/Offコントロールが可能です。

## ハードウェア
- M5 ATOM Lite
- TP-Link Smart Plug HS105

## ソフトウェア
- 開発環境: PlatformIO on VSCode
- ライブラリ
  + M5Atom
  + FastLED
  + PubSubClient
- ヘッダファイル
  + https://github.com/mnakai3/WioTerminal_CO2_Controller/tree/master の TPLinkSmartPlug.h を使わせていただいています。
   
## 使い方
- HS105を開封したら、ACに繋いで、スマホのTP-Linkのアプリを使ってWiFi接続します。アプリはこれ以降は不要です。
- スマートプラグのIPアドレスを確認します。ルーターでIPアドレスを固定しておくのがおすすめです。
- 上記IPアドレス他の情報を自分のものに合わせてコンパイル・書き込みします。
- 組み込みタイマーが不要であれば、timerEnableをfalseにしてください。
- MQTTでコントロールする際は、thisMqttTopic で設定したトピックに、"ON" もしくは "OFF" （いずれも大文字）を投げる形です。
