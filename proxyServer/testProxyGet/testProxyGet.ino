#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <mbedtls/base64.h>  // Thư viện để mã hóa Base64

// Thông tin kết nối WiFi
const char* ssid = "eoh.io";
const char* password = "Eoh@2020";

// Thông tin xác thực HIKVISION
const char* hikvision_username = "admin";
const char* hikvision_password = "Eoh54321";

WebServer server(80); // HTTP Server trên cổng 80

String base64_encode(String input) {
  size_t output_length;
  unsigned char output[128];
  mbedtls_base64_encode(output, sizeof(output), &output_length, (const unsigned char *)input.c_str(), input.length());
  return String((char*)output);
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Đang kết nối WiFi...");
  }
  
  Serial.println("Đã kết nối WiFi!");
  Serial.print("Địa chỉ IP của ESP32: ");
  Serial.println(WiFi.localIP());

  server.on("/get_image", handleCaptureImage);
  server.begin();
  Serial.println("HTTP Server đã bắt đầu");
}

void handleCaptureImage() {
  HTTPClient http;

  String url = "http://14.241.233.207:28001/ISAPI/Streaming/channels/1/picture";
  
  // Tạo chuỗi xác thực Basic
  String auth = String(hikvision_username) + ":" + String(hikvision_password);
  auth = base64_encode(auth);
  String authHeader = "Basic " + auth;

  http.begin(url);
  http.addHeader("Authorization", authHeader);
  
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
    Serial.println("Gửi ảnh thành công!");
  } else {
    String payload = http.getString();
    server.send(500, "text/plain", "Lỗi khi lấy ảnh từ HIKVISION");
    Serial.printf("Lỗi HTTP từ HIKVISION: %d\n", httpCode);
    Serial.println("Nội dung phản hồi từ HIKVISION: " + payload);
  }

  http.end();
}

void loop() {
  server.handleClient();
}
