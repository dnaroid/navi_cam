#include <Arduino.h>
#include <Camera.h>
#include <HTTPCamServer.h>

IPAddress local_IP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Serial initialized");
  setupCamera();
  setupHttpServer();
}

void loop() {
  loopHttpServer();
}
