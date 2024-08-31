#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_wifi.h>

#include "Arduino.h"
#include "esp_camera.h"
#include <WebSocketsClient.h>
#include <WiFiMulti.h>

// #define CAMERA_MODEL_XIAO_ESP32S3

#define WIFI_SSID "ESP32-SCOOTER"
#define WIFI_PASSWORD "UoAcYyo5FErnjXk"

#if defined(CAMERA_MODEL_XIAO_ESP32S3)
#define PWDN_GPIO_NUM     -1
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM     10
#define SIOD_GPIO_NUM     40
#define SIOC_GPIO_NUM     39
#define Y9_GPIO_NUM       48
#define Y8_GPIO_NUM       11
#define Y7_GPIO_NUM       12
#define Y6_GPIO_NUM       14
#define Y5_GPIO_NUM       16
#define Y4_GPIO_NUM       18
#define Y3_GPIO_NUM       17
#define Y2_GPIO_NUM       15
#define VSYNC_GPIO_NUM    38
#define HREF_GPIO_NUM     47
#define PCLK_GPIO_NUM     13
#define LED_GPIO_NUM      21
#else
#define PWDN_GPIO_NUM    32
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    0
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27
#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      21
#define Y4_GPIO_NUM      19
#define Y3_GPIO_NUM      18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22
#endif


WebSocketsClient webSocket;
WiFiMulti wifiMulti;
bool camEnabled = false;
bool camReady = false;

void setupCamera() {
  Serial.println("Camera setup started.");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QQVGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;


  const esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  Serial.println("Camera setup completed successfully.");
  camReady = true;
}

void webSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
  case WStype_DISCONNECTED:
    Serial.println("Disconnected");
    break;
  case WStype_CONNECTED:
    Serial.println("Connected");
    break;
  case WStype_TEXT:
    Serial.printf("Get text: %s\n", payload);
    if (strcmp(reinterpret_cast<const char*>(payload), "start") == 0) {
      if (!camReady) setupCamera();
      camEnabled = true;
    } else if (strcmp(reinterpret_cast<char*>(payload), "stop") == 0) {
      camEnabled = false;
      esp_camera_deinit();
      camReady = false;
    }
    break;
  default:
    break;
  }
}

void disableBluetooth() {
  if (esp_bluedroid_disable() == ESP_OK) {
    Serial.println("Bluetooth stack disabled");
  } else {
    Serial.println("Failed to disable Bluetooth stack");
  }

  if (esp_bt_controller_disable() == ESP_OK) {
    Serial.println("Bluetooth controller disabled");
  } else {
    Serial.println("Failed to disable Bluetooth controller");
  }
}

void setup() {
  Serial.begin(115200);

#if defined(CAMERA_MODEL_XIAO_ESP32S3)
  disableBluetooth();
#endif

  // setCpuFrequencyMhz(80);
  esp_wifi_set_max_tx_power(1);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  while (wifiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  webSocket.begin("192.168.4.1", 81, "/");
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(5000);
  Serial.println("Camera ready");
}

void loop() {
  webSocket.loop();

  if (camEnabled) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (fb) {
      webSocket.sendBIN(fb->buf, fb->len);
      esp_camera_fb_return(fb);
    }
  }
}
