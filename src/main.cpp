#include "Arduino.h"
#include "esp_camera.h"


#define MIRROR_RX D1
#define MIRROR_TX D2
// #define MIRROR_UART_BAUD 115200
#define MIRROR_UART_BAUD   4000000
static HardwareSerial mirrorSerial(1);


#if defined(SEEED_XIAO_ESP32S3)
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
  config.frame_size = FRAMESIZE_QQVGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.jpeg_quality = 10;
  config.fb_count = 2;


  const esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  Serial.println("Camera setup completed successfully.");
}

void setup() {
  // int waitCount = 0;
  // while (!Serial.available() && waitCount++ < 100) {
  Serial.begin(115200);
  // delay(100);
  // }
  Serial.println("Starting");

  setupCamera();

  pinMode(LED_GPIO_NUM, OUTPUT);
  digitalWrite(LED_GPIO_NUM, HIGH);

  mirrorSerial.begin(MIRROR_UART_BAUD, SERIAL_8N1, MIRROR_RX, MIRROR_TX);
}

void loop() {
  if (mirrorSerial.available()) {
    const String command = mirrorSerial.readStringUntil('\r');

    if (command.equals("img")) {
      digitalWrite(LED_GPIO_NUM, LOW);
      camera_fb_t* fb = esp_camera_fb_get();
      if (fb) {
        uint16_t length = fb->len;
        mirrorSerial.write(reinterpret_cast<uint8_t*>(&length), sizeof(length));
        mirrorSerial.write(fb->buf, fb->len);
        esp_camera_fb_return(fb);
      } else {
        Serial.println("error");
      }
      digitalWrite(LED_GPIO_NUM, HIGH);
    }
  }
}
