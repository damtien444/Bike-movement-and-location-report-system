#include <FirebaseESP8266.h>
String recibido;
#include <Wire.h>
#include <ESP8266WiFi.h>
// Them thu vien
String location;
String latitude,longtitude;
#define FIREBASE_HOST "proven-answer-309312-default-rtdb.firebaseio.com" //Thay bằng địa chỉ firebase của bạn
#define FIREBASE_AUTH "T34NQy9tSmV8LEuZHkLMf7rE3FhXgwgMpkz38Dpj"   //Không dùng xác thực nên không đổi
#define WIFI_SSID "The Ancient Wifi"   //Thay wifi và mật khẩu
#define WIFI_PASSWORD "0967237101"
FirebaseData f;
void setup() {
  Serial.begin(9600);
  Wire.begin(D1, D2);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}
void loop() {
//WiFi.status() != WL_CONNECTED
  if (WiFi.status() != WL_CONNECTED){
    Wire.beginTransmission(8);//0x08 = 8;
    Wire.write("0");
    Wire.endTransmission();
  } 
  else {
    Wire.beginTransmission(8);//0x08 = 8;
    Wire.write("connected \n");
    Wire.endTransmission();
  }
  Wire.requestFrom(8,30); /* request & read data of size 13 from slave */
  while (Wire.available()) {
    char c = Wire.read();
    if (c == '-' || c == '.'||c==','||(c >= '0' && c <= '9'))
    {
      Serial.print(c);
      location += c;
      if(c==',')
        break;
    }
   
  }
  Serial.println();
//  delay(1000);
  latitude = location.substring(0,location.indexOf('-'));
  longtitude=location.substring(location.indexOf('-')+1,location.indexOf(','));
  Serial.println(latitude);
  Serial.println(longtitude);
  Firebase.setFloat(f, "Location/latitude", latitude.toFloat());
  Firebase.setFloat(f, "Location/longtitude", longtitude.toFloat());

}
