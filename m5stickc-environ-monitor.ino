#include <M5StickC.h>
#include <MHZ19.h>
#include <SHT3x.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>

#define UPDATE_PERIOD 1000    // このmsecごとにセンサの値取得と表示更新
#define SLEEP_TIMEOUT 600000  // このmsec以上無操作でsleepする

#define POSE_TOP 0    // 通常姿勢
#define POSE_LEFT 1   // 左が上
#define POSE_BOTTOM 2 // 下が上
#define POSE_RIGHT 3  // 右が上
uint8_t pose = POSE_TOP;
uint8_t last_pose = POSE_TOP;

// 加速度
float accX_g = 0;
float accY_g = 0;
float accZ_g = 0;
float accX_mpss = 0;
float accY_mpss = 0;
float accZ_mpss = 0;

#define RX_PIN 26  // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 0   // Tx pin which the MHZ19 Rx pin is attached to

long start_t, last_t;
double vbat;
float ibat;
uint8_t warning;
RTC_DATA_ATTR int co2;
RTC_DATA_ATTR float temp, humid, press;

MHZ19 mhz19;
SHT3x sht30;
Adafruit_BMP280 bme;

// +1G なら true
boolean near_p_g(float value) {
  if (8.0 < value && value < 12.0) {
    return true;
  } else {
    return false;
  }
}
// -1G なら true
boolean near_m_g(float value) {
  if (-12.0 < value && value < -8.0) {
    return true;
  } else {
    return false;
  }
}
// 0G なら true
boolean near_zero(float value) {
  if (-2.0 < value && value < 2.0) {
    return true;
  } else {
    return false;
  }
}
// ボタンで解除可能なディープスリープ開始
void enter_deep_sleep() {
  M5.Axp.SetLDO2(false);  // 画面表示をOFF
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH,ESP_PD_OPTION_AUTO);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_37, 0);  // BtnA:37 BtnB:39
  esp_deep_sleep_start();
}
// センサの値取得
void read_sensor() {
  vbat = M5.Axp.GetVbatData() * 1.1 / 1000;
  ibat = M5.Axp.GetBatCurrent();
  warning = M5.Axp.GetWarningLeve();

  co2 = mhz19.getCO2();

  sht30.UpdateData();
  temp = sht30.GetTemperature();
  humid = sht30.GetRelHumidity();
  press = bme.readPressure();
}
// 画面更新
void update_display(uint8_t pose) {
  switch (pose) {
    case POSE_TOP:
      M5.Lcd.setRotation(POSE_TOP);
      render_portrait();
      break;
    case POSE_BOTTOM:
      M5.Lcd.setRotation(POSE_BOTTOM);
      render_portrait();
      break;
    case POSE_LEFT:
      M5.Lcd.setRotation(POSE_LEFT);
      render_landscape();
      break;
    case POSE_RIGHT:
      M5.Lcd.setRotation(POSE_RIGHT);
      render_landscape();
      break;
    default:
      ;
  }
}
// 横画面表示
void render_landscape() {
  // battery
  uint16_t color = WHITE;
  if (warning) {
    color = RED;
  }
  if (ibat > 0) {
    color = GREEN;
  }
  M5.Lcd.fillRect(140, 0, 18, 9, BLACK);
  if (vbat >= 4) {
    M5.Lcd.fillRect(140, 0, 18, 9, color);
  } else if (vbat >= 3.7) {
    M5.Lcd.fillRect(140, 0, 12, 9, color);
  } else {
    M5.Lcd.fillRect(140, 0, 6, 9, color);
  }
  M5.Lcd.drawRect(140, 0, 18, 9, color);

  M5.Lcd.setCursor(100, 1);
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(color, BLACK);
  M5.Lcd.printf("%.3fV", vbat);

  // sleep timer
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(DARKGREY,BLACK);
  M5.Lcd.printf("%3i (%3isec)", (millis() - start_t) / 1000, SLEEP_TIMEOUT / 1000);

  // sensor
  M5.Lcd.setCursor(0, 15);
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setTextColor(WHITE,BLACK);
  M5.Lcd.printf("CO2   %4ippm\n", co2);
  M5.Lcd.setTextColor(ORANGE,BLACK);
  M5.Lcd.printf("Temp  %4.1f'C\n", temp);
  M5.Lcd.setTextColor(CYAN,BLACK);
  M5.Lcd.printf("Humid %4.1f%%\n", humid);
  M5.Lcd.setTextColor(LIGHTGREY,BLACK);
  M5.Lcd.printf("Press %4.0fhPa\n", press / 100);
}
// 縦画面表示
void render_portrait() {
  // battery
  uint16_t color = WHITE;
  if (warning) {
    color = RED;
  }
  if (ibat > 0) {
    color = GREEN;
  }
  M5.Lcd.fillRect(60, 10, 18, 9, BLACK);
  if (vbat >= 4) {
    M5.Lcd.fillRect(60, 10, 18, 9, color);
  } else if (vbat >= 3.7) {
    M5.Lcd.fillRect(60, 10, 12, 9, color);
  } else {
    M5.Lcd.fillRect(60, 10, 6, 9, color);
  }
  M5.Lcd.drawRect(60, 10, 18, 9, color);

  M5.Lcd.setCursor(20, 10);
  M5.Lcd.setTextFont(1);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(color, BLACK);
  M5.Lcd.printf("%.3fV", vbat);

  // sensor
  M5.Lcd.setCursor(0, 22);
  M5.Lcd.setTextFont(2);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(WHITE, BLACK);
  M5.Lcd.printf("CO2\n%4i ppm\n", co2);
  M5.Lcd.setTextColor(ORANGE, BLACK);
  M5.Lcd.printf("Temp\n%4.1f 'C\n", temp);
  M5.Lcd.setTextColor(CYAN, BLACK);
  M5.Lcd.printf("Humid\n%4.1f %%\n", humid);
  M5.Lcd.setTextColor(LIGHTGREY, BLACK);
  M5.Lcd.printf("Press\n%4.0f hPa\n", press / 100);
}

// main
void setup() {
  M5.begin();
  setCpuFrequencyMhz(40);   // 無線使用する場合は80以上
  M5.Axp.ScreenBreath(9);   // 画面の明るさ(7-12)
  M5.Lcd.setTextFont(1);    // フォント選択(1-8)
  M5.Lcd.fillScreen(BLACK); // 画面塗り潰し

  M5.MPU6886.Init();

  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  mhz19.begin(Serial1);
  mhz19.autoCalibration();

  Wire.begin();
  while (!bme.begin(0x76)) {
    M5.Lcd.println("BMP280 fail");
  }
  sht30.Begin();

  start_t = last_t = millis();
}

void loop() {
  long t = millis();

  // ボタン状態取得
  M5.update();
  if (M5.BtnA.wasReleased()) {
    start_t = t;  // スリープタイマーをリセット
  }
  if (M5.BtnB.wasReleased() || t - start_t > SLEEP_TIMEOUT) {
    enter_deep_sleep();
  }

  // 加速度取得
  M5.MPU6886.getAccelData(&accX_g, &accY_g, &accZ_g);
  accX_mpss = accX_g * 9.8;
  accY_mpss = accY_g * 9.8;
  accZ_mpss = accZ_g * 9.8;

  // 姿勢を判定する
  if (near_zero(accX_mpss) && near_p_g(accY_mpss)) {
    pose = POSE_TOP;
  } else if (near_p_g(accX_mpss) && near_zero(accY_mpss)) {
    pose = POSE_LEFT;
  } else if (near_zero(accX_mpss) && near_m_g(accY_mpss)) {
    pose = POSE_BOTTOM;
  } else if (near_m_g(accX_mpss) && near_zero(accY_mpss)) {
    pose = POSE_RIGHT;
  }

  // 姿勢が変更された場合画面を塗り潰してから更新
  if (last_pose != pose) {
    M5.Lcd.fillScreen(BLACK);
    update_display(pose);
    last_pose = pose;
  }

  // 更新間隔経過していたらセンサの値を取得し画面を更新
  if (t - last_t > UPDATE_PERIOD) {
    read_sensor();
    update_display(pose);
    last_t = t;
  }

  delay(1);
}
