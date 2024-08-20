#include <WiFi.h>
#include <HTTPClient.h>
#include "mbedtls/base64.h"  // Sử dụng thư viện mbedtls để mã hóa Base64

// Thông tin kết nối WiFi
const char* ssid = "eoh.io";          // Thay thế bằng SSID WiFi của bạn
const char* password = "Eoh@2020";  // Thay thế bằng mật khẩu WiFi của bạn

// Access Token từ Google OAuth Playground
const char* accessToken = "ya29.a0AcM612wfgMK6c0NM7hlRKLi2IsCjPKLaQiYTN2zQVgxg_79-MfztxwOzatgKvGZndqljwFec1rnNh-dIlYmot4JvTx8B1MdhyvWjXgrEqbB507i_TdQ4I5RnazBB7aZvGJBhvbTGctNZuheIqDYYWogVpoH3gKzP110IhqL4aCgYKAVsSARMSFQHGX2MiYb1XCP-p_RiUzfKMMP1DRg0175";  // Thay thế bằng Access Token bạn vừa lấy được

// Thông tin API của HIKVISION
const char* imageUrl = "http://admin:Eoh54321@14.241.233.207:28001/ISAPI/Streaming/channels/1/picture";
const char* username = "admin";         // Thay thế bằng tên đăng nhập HIKVISION của bạn
const char* userpassword = "Eoh54321";  // Thay thế bằng mật khẩu HIKVISION của bạn

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Đang kết nối WiFi...");
  }
  
  Serial.println("Đã kết nối WiFi!");
  
  uploadImageToGoogleDrive();
}

void uploadImageToGoogleDrive() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Mã hóa Base64 thủ công
    String authString = String(username) + ":" + String(userpassword);
    size_t toEncodeLen = authString.length();
    size_t encodedLen = 0;
    unsigned char encodedData[128];
    mbedtls_base64_encode(encodedData, sizeof(encodedData), &encodedLen, (const unsigned char*)authString.c_str(), toEncodeLen);
    String authHeader = "Basic " + String((char*)encodedData);

    // In ra chuỗi Authorization để kiểm tra
    Serial.println("Authorization Header: " + authHeader);

    http.begin(imageUrl);
    http.addHeader("Authorization", authHeader);

    int httpCode = http.GET();

    // Kiểm tra mã phản hồi từ HIKVISION
    Serial.printf("HTTP Code from HIKVISION: %d\n", httpCode);

    if (httpCode == HTTP_CODE_OK) {
      WiFiClient * stream = http.getStreamPtr();
      
      // Đọc dữ liệu từ stream vào buffer
      int contentLength = http.getSize();
      uint8_t * buffer = (uint8_t *) malloc(contentLength);
      int bytesRead = stream->readBytes(buffer, contentLength);

      HTTPClient driveClient;
      driveClient.begin("https://www.googleapis.com/upload/drive/v3/files?uploadType=media");
      driveClient.addHeader("Authorization", String("Bearer ") + accessToken);
      driveClient.addHeader("Content-Type", "image/jpeg");

      // Gửi dữ liệu từ buffer
      int driveHttpCode = driveClient.POST(buffer, bytesRead);

      // Kiểm tra mã phản hồi từ Google Drive
      Serial.printf("Google Drive Response Code: %d\n", driveHttpCode);
      String response = driveClient.getString();
      Serial.println("Google Drive Response: " + response);

      free(buffer);  // Giải phóng bộ nhớ sau khi dùng xong
      driveClient.end();
    } else {
      Serial.println("Lỗi khi tải ảnh từ HIKVISION: " + String(httpCode));
      String payload = http.getString();
      Serial.println("Phản hồi từ HIKVISION: " + payload);
    }

    http.end();
  } else {
    Serial.println("Không thể kết nối WiFi");
  }
}

void loop() {
  // Không cần làm gì trong loop()
}
