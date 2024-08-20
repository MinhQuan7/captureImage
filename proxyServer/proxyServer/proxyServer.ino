#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include "mbedtls/base64.h"  // Sử dụng mbedtls cho Base64

// Thông tin kết nối WiFi
const char* ssid = " eoh.io";
const char* password = "Eoh@2020";

// Thông tin API của HIKVISION
const char* imageUrl = "http://14.241.233.207:28001/ISAPI/Streaming/channels/1/picture";
const char* username = "admin";
const char* userpassword = "Eoh54321";

// Khởi tạo server chạy trên ESP32 tại cổng 80
WebServer server(80);

void handleCaptureImage() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Tạo chuỗi xác thực
    String authString = String(username) + ":" + String(userpassword);
    size_t toencodeLength = authString.length();
    size_t encodedLength = 0;

    // Mã hóa bằng Base64
    unsigned char encodedData[128];
    mbedtls_base64_encode(encodedData, sizeof(encodedData), &encodedLength, (const unsigned char*)authString.c_str(), toencodeLength);
    
    String authHeader = "Basic " + String((char*)encodedData);

    http.begin(imageUrl);
    http.addHeader("Authorization", authHeader);

    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
      WiFiClient * stream = http.getStreamPtr();
      server.sendHeader("Content-Type", "image/jpeg");
      server.send(200, "image/jpeg", "");
      
      size_t size = stream->available();
      while (size > 0) {
        uint8_t buffer[128] = { 0 };
        int bytesRead = stream->readBytes(buffer, std::min((int)size, (int)sizeof(buffer)));
        server.client().write(buffer, bytesRead);
        size = stream->available();
      }
    } else {
      server.send(500, "text/plain", "Lỗi khi tải ảnh từ HIKVISION");
    }

    http.end();
  } else {
    server.send(500, "text/plain", "Không thể kết nối WiFi");
  }
}

void setup() {
  Serial.begin(115200);

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Đang kết nối WiFi...");
  }
  Serial.println("Đã kết nối WiFi!");

  // Cấu hình ESP32 để xử lý yêu cầu từ IoT Platform
  server.on("/capture_image", handleCaptureImage);
  server.begin();
}

void loop() {
  server.handleClient();
}
