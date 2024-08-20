#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// Thông tin kết nối WiFi
const char* ssid = "YOUR_SSID";          // Thay thế bằng SSID WiFi của bạn
const char* password = "YOUR_PASSWORD";  // Thay thế bằng mật khẩu WiFi của bạn

WebServer server(443); // HTTPS Server trên cổng 443

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Đang kết nối WiFi...");
  }
  
  Serial.println("Đã kết nối WiFi!");

  // Cấu hình URL xử lý yêu cầu từ IoT Platform
  server.on("/get_image", handleCaptureImage);
  server.begin();
  Serial.println("HTTPS Server đã bắt đầu");
}

void handleCaptureImage() {
  HTTPClient http;
  
  String url = "http://admin:Eoh54321@14.241.233.207:28001/ISAPI/Streaming/channels/1/picture";
  
  http.begin(url);
  
  int httpCode = http.GET();

  if (httpCode == HTTP_CODE_OK) {
    WiFiClient *stream = http.getStreamPtr();
    server.sendHeader("Content-Type", "image/jpeg");
    server.send(200, "image/jpeg", "");
    
    uint8_t buffer[128] = { 0 };
    while (stream->available()) {
      size_t len = stream->read(buffer, sizeof(buffer));
      server.client().write(buffer, len);
    }
  } else {
    server.send(500, "text/plain", "Lỗi khi lấy ảnh từ HIKVISION");
    Serial.printf("Lỗi HTTP từ HIKVISION: %d\n", httpCode);
  }

  http.end();
}

void loop() {
  server.handleClient();
}
