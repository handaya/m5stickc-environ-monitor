# m5stickc-environ-monitor

[M5StickC](https://m5stack-store.myshopify.com/collections/m5-core/products/stick-c) に、
[M5StickC/Plus用「Co2 HAT（ソケット版）」](https://kitto-yakudatsu.com/archives/7766) と
[ENV II Unit with Temperature Humidity Environment Sensor (SHT30+BMP280)](https://m5stack-store.myshopify.com/products/env-ii-unit)
を接続して、以下を測定し画面表示します。

* CO2濃度(ppm)
* 気温(℃)
* 湿度(%)
* 気圧(hPa)

以下の機能があります。

* 本体を回転させると、その向きに応じて画面を回転します。
* 本体向かって右のボタン(電源ボタンではない方)を押すと、画面OFFになりディープスリープします。
* 10分無操作で、画面OFFになりディープスリープします。
  * `M5` ボタンを押すとタイマーの残り時間をリセットします。
* ディープスリープ状態で `M5` ボタンを押すと復帰します。
