#include <Wire.h>
#include <SoftwareSerial.h>

#define DEBUG true

const int LED = 13;
const unsigned long waitTimeout = 60000;

const String SDT = "84396119521";

SoftwareSerial sim808(11, 10);
String stateWifi = "0"; // not_connect

// Debounce tránh kích hoạt ngắt quá nhiều
unsigned long lastSwitchDetectedMIllis = 0;
unsigned long debounceInterval = 60000;
unsigned long interval;
boolean mode = false;
//boolean isStolen = false;

String state, timegps, latitude, longitude, x;
String message, tin_nhan;

// TODO: viết hàm set riêng biến này sau
boolean isOnGuarded = true;

void setup() {

  // Kết nối i2c bus tại địa chỉ số 8
  Wire.begin(8);

  // Giao tiếp uart với Sim 808
  sim808.begin(9600);

  Serial.begin(9600);

  // Bắt đầu các ngắt sự kiện i2c
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  // Cài đặt mode cho pin của cảm biến là pullup
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(0, eventSensor,  RISING);
  pinMode(LED, OUTPUT);

  // Bật GPS trên sim808
  send2Sim808("AT+CGNSPWR=1", 2000, DEBUG);
  delay(200);

  // Đổi mode text NMEA sang RMC
  send2Sim808("AT+CGNSSEQ=RMC", 2000, DEBUG);
  delay(200);

  // Đổi chế độ text cho chức năng nhắn tin
  send2Sim808("AT+CMGF=1", 2000, DEBUG);
  delay(200);

  Serial.println("innit gps");
  while (!getGPSinfo()) {
    delay(200);
    Serial.println("...");
  }
  Serial.println("innit gps successfully");

}

void loop() {
  if (mode) {
    Runner();
  }
}

void Runner() {

  if (isWifiConnected()) {
    tryingGPS();
    SendWarningMessage(SDT);
    delay(2000);
    if (waitForSignal(500000)) {
      isOnGuarded = false;
    }
    else {
      
      // bat canh bao
      
      tryingGPS();
      sendSMS(SDT, createSMScontainLocation());
      while (true) {
        tryingGPS();
        if (RecieveMessage()){
          sendSMS(SDT, createSMScontainLocation());
        }
      }
    }
  }
  else {
    mode == false;
  }
}

void tryingGPS() {
  Serial.println("get gps");
  while (!getGPSinfo()) {
    Serial.println("...");
  }
  Serial.println("get gps successfully");
}

String createSMScontainLocation() {
  String link = "http://maps.google.com/maps?q=";
  link += latitude;
  link += ",";
  link += longitude;
  return link;
}

String send2Sim808 (String command , const unsigned long timeout , boolean debug) {
  String response = "";
  sim808.println(command);
  unsigned long time = millis();
  while ((time + timeout) > millis()) {
    while (sim808.available()) {
      char c = sim808.read();
      response += c;
    }
  }
  if (debug) {
    Serial.print(response);
  }
  return response;
}

boolean isWifiConnected() {

  // todo: return true false hop ly#
  
  if (stateWifi == "0") return false;
  else return true;
}

void receiveEvent(int howMany) {

  // Hàm được gọi tương tự khi gọi ngắt mà nhận được sự kiện từ esp

  stateWifi = "";
  while (0 < Wire.available()) {
    char c = Wire.read();
    stateWifi += c;
  }
    Serial.print(stateWifi);
}

void requestEvent() {
  x += latitude;
  x += "-";
  x += longitude;
  x += ",";
  int str_len = x.length() + 1;
  char location[str_len];
  x.toCharArray(location, str_len);
  Serial.println(x);
  Wire.write(location);
  /*send string on request */
  x = "";
}

void eventSensor() {

  if (isOnGuarded && mode == false) {

    // Thêm hệ điều kiện để hoạt động chính xác hơn.

    interval = millis() - lastSwitchDetectedMIllis;
    if (interval > debounceInterval) {
      lastSwitchDetectedMIllis = millis();
      mode = !mode;
      Serial.println("Ngat kich hoat.");
    }
    else {
      Serial.println("Skip ngat.");
    }
  }
}

void SendWarningMessage(String sdt) {
  String message = "xe co the da bi mat trom, goi 0967237101 de bat canh bao.\n Vi tri xe la "+createSMScontainLocation();
  sendSMS(sdt, message);
}

void sendSMS(String sdt, String message) {
  String command1 = "AT+CMGS=\"";
  command1 += sdt;
  command1 += "\"";
  sim808.println("at");
  delay(200);
  sim808.println("AT+CMGF=1");
  delay(200);
  sim808.println(command1);
  delay(5000);
  sim808.println(message);
  delay(1000);
  sim808.println((char)26);
  delay(1000);
  Serial.println("SMS sent successfully.");
}


boolean waitForSignal(unsigned long waitTime) {
//  sim808.println("AT");
  delay(200);
  String response = "";
  unsigned long times = millis();
  while ((times + waitTime) > millis()) {
    while (sim808.available()) {
      char c = (char)sim808.read();
      if (c != 'R' && c != 'I' && c != 'N' && c != 'G') {
        break;
      }
      Serial.println(c);
      response += c;
    }
    if (response == "RING") {
      Serial.println("Confirmed, stop waiting.");
      break;
    }
  }
  if (response == "RING") {
    Serial.println("Call Confirmed.");
    return true;
  }
  return false;
}

boolean getGPSinfo() {
  String data[5];
  sim808.println("AT+CGNSINF");
  delay(150);
  unsigned long time = millis();
  int i = 0;
  while ((time + 2000) > millis()) {
    while (sim808.available()) {
      char c = (char)sim808.read();
      if (c != ',') {
        data[i] += c;
        delay(100);
      }
      else {
        i++;
      }
      if (i == 5) {
        delay(100);
        break;
      }
    }
  }
  //  Serial.println(debug);
  if (true) {
    state = data[1];
    timegps = data[2];
    latitude = data[3];
    longitude = data[4];
    //    Wire.onRequest(requestEvent);
  }
  if (state != 0) {
    Serial.println("State: " + state);
    Serial.println("Time: " + timegps);
    Serial.println("Latitude: " + latitude);
    Serial.println("Longitude: " + longitude);
    return true;
  } else {
    Serial.println("GPS Initializing…");
    return false;
  }
  Serial.println("------------------------------------------------");
}

boolean RecieveMessage() {

  //  sim808.println("AT+CNMI=2,2");
  // AT Command to recieve a live SMS
  send2Sim808("AT+CNMI=2,2,0,0,0", 2000, DEBUG);

  delay(1000);
  Serial.println("run");
  if (readMessages()) return true;
  else return false;
  delay(1000);

}

boolean readMessages() {
  unsigned long times = millis();
  String response = "";
  Serial.println("waiting");
  while (true) {
    //    Serial.println(".");
    while (sim808.available()) {
      char c = (char) sim808.read();
      if (c != 'R' && c != 'E' && c != 'L' && c != 'O') {
        break;
      }
      Serial.println(c);
      response += c;
    }
    if (response == "RELO") {
      Serial.println("Confirmed, stop waiting.");
      break;
    }
  }
  if (response == "RELO") {
    Serial.println("Call Confirmed.");
    return true;
  }
  return false;
}
