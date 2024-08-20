#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const char* imageUrl = "http://14.241.233.207:28001/ISAPI/Streaming/channels/1/picture";
const char* username = "admin";
const char* userpassword = "Eoh54321";

void setup() {
  Serial.begin(115200);
  
  // Kết nối WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Đang kết nối WiFi...");
  }
  Serial.println("Đã kết nối WiFi!");

  // Tải hình ảnh từ HIKVISION
  downloadImage();
}

void downloadImage() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    
    // Thêm thông tin xác thực Basic
    String authHeader = "Basic " + base64::encode(String(username) + ":" + String(userpassword));

    http.begin(imageUrl);
    http.addHeader("Authorization", authHeader);

    int httpCode = http.GET();
    
    if (httpCode == HTTP_CODE_OK) {
      WiFiClient * stream = http.getStreamPtr();
      
      // Gửi ảnh về qua Serial hoặc lưu trữ
      // Ở đây ta chỉ gửi dữ liệu ảnh về qua Serial để theo dõi
      while (http.connected()) {
        size_t size = stream->available();
        if (size) {
          uint8_t buffer[128] = { 0 };
          int c = stream->readBytes(buffer, std::min((int)size, (int)sizeof(buffer)));
          Serial.write(buffer, c);
        }
      }
    } else {
      Serial.println("Lỗi khi tải ảnh: " + String(httpCode));
    }
    
    http.end();
  } else {
    Serial.println("Không thể kết nối WiFi");
  }
}

void loop() {
  // Chương trình chính sẽ nằm ở đây nếu cần thiết
}
