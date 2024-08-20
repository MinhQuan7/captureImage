#include <WiFi.h>
#include <HTTPClient.h>

// Thông tin kết nối WiFi
const char* ssid = "YOUR_SSID";          // Thay thế bằng SSID WiFi của bạn
const char* password = "YOUR_PASSWORD";  // Thay thế bằng mật khẩu WiFi của bạn

// Access Token từ Google OAuth Playground
const char* accessToken = "YOUR_ACCESS_TOKEN";  // Thay thế bằng Access Token bạn vừa lấy được

// Thông tin API của HIKVISION
const char* imageUrl = "http://14.241.233.207:28001/ISAPI/Streaming/channels/1/picture";
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
    
    String authHeader = "Basic " + String((char*)base64::encode((unsigned char*)(String(username) + ":" + String(userpassword)).c_str(), (String(username) + ":" + String(userpassword)).length()));
    
    http.begin(imageUrl);
    http.addHeader("Authorization", authHeader);

    int httpCode = http.GET();

    if (httpCode == HTTP_CODE_OK) {
      WiFiClient * stream = http.getStreamPtr();
      
      HTTPClient driveClient;
      driveClient.begin("https://www.googleapis.com/upload/drive/v3/files?uploadType=media");
      driveClient.addHeader("Authorization", String("Bearer ") + accessToken);
      driveClient.addHeader("Content-Type", "image/jpeg");

      int driveHttpCode = driveClient.POST(*stream, stream->available());
      
      if (driveHttpCode > 0) {
        Serial.printf("Google Drive Response Code: %d\n", driveHttpCode);
        String response = driveClient.getString();
        Serial.println("Google Drive Response: " + response);
      } else {
        Serial.printf("Error uploading to Google Drive: %s\n", driveClient.errorToString(driveHttpCode).c_str());
      }
      
      driveClient.end();
    } else {
      Serial.println("Lỗi khi tải ảnh từ HIKVISION: " + String(httpCode));
    }

    http.end();
  } else {
    Serial.println("Không thể kết nối WiFi");
  }
}

void loop() {
  // Không cần làm gì trong loop()
}
