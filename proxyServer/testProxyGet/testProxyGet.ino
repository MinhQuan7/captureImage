#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>

// Thông tin kết nối WiFi
const char* ssid = "eoh.io";          
const char* password = "Eoh@2020";    

WebServer server(80); // HTTP Server trên cổng 80

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
  Serial.println("Đã nhận được yêu cầu chụp ảnh!");

  HTTPClient http;

  // URL của camera HKVISION
  String url = "http://14.241.233.207:28001/ISAPI/Streaming/channels/1/picture";

  // Chuỗi xác thực Base64
  String authHeader = "Basic YWRtaW46RW9oNTQzMjE="; // "admin:Eoh54321" mã hóa thành Base64

  http.begin(url);
  http.addHeader("Authorization", authHeader);  // Thêm Header xác thực

  int httpCode = http.GET();

  Serial.print("HTTP Code từ HIKVISION: ");
  Serial.println(httpCode);

  if (httpCode == HTTP_CODE_OK) {
    Serial.println("Chụp ảnh thành công, đang gửi ảnh...");
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
