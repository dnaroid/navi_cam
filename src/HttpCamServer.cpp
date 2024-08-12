#include <Camera.h>
#include <HttpCamServer.h>
#include <Secrets.h>

WebServer httpCamServer(80);

void handle_jpg_stream() {
  WiFiClient client = httpCamServer.client();
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
  httpCamServer.sendContent(response);

  while (client.connected()) {
    cam.run();
    if (!client.connected()) break;

    response = "--frame\r\n";
    response += "Content-Type: image/jpeg\r\n\r\n";
    httpCamServer.sendContent(response);

    client.write((char*)cam.getfb(), cam.getSize()); // Отправляем данные кадра
    httpCamServer.sendContent("\r\n");
  }
}

void handle_jpg() {
  WiFiClient client = httpCamServer.client();

  cam.run();
  if (!client.connected()) return;
  String response = "HTTP/1.1 200 OK\r\n";
  response += "Content-disposition: inline; filename=capture.jpg\r\n";
  response += "Content-type: image/jpeg\r\n\r\n";
  httpCamServer.sendContent(response);
  client.write((char*)cam.getfb(), cam.getSize());
}

void handleNotFound() {
  String message = "Server is running!\n\n";
  message += "URI: ";
  message += httpCamServer.uri();
  message += "\nMethod: ";
  message += (httpCamServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpCamServer.args();
  message += "\n";
  httpCamServer.send(200, "text/plain", message);
}

void setupHttpServer() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid_name, ssid_password);
  IPAddress ip = WiFi.softAPIP();
  Serial.println(F("Access Point started"));
  Serial.println("");
  Serial.println(ip);

  httpCamServer.on("/", HTTP_GET, handle_jpg_stream);
  httpCamServer.on("/jpg", HTTP_GET, handle_jpg);
  httpCamServer.onNotFound(handleNotFound);
  httpCamServer.begin();

  Serial.println("HTTP server started");
}

void loopHttpServer() {
  httpCamServer.handleClient();
}
