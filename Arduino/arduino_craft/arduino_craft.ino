#include <Wire.h>
#include <SoftwareSerial.h>

#define DEBUG true

const int LED = 13;
const unsigned long waitTimeout = 60000;

// const String masterPhoneNum = "84396119521";
const String masterPhoneNum =    "84967237101";
const String modulePhoneNum =    "84396119521";

SoftwareSerial sim808(11, 10);

String stateWifi = "0"; // not_connect

// Debounce tránh kích hoạt ngắt quá nhiều
unsigned long lastSwitchDetectedMIllis = 0;
unsigned long debounceInterval = 10000;
unsigned long interval;
boolean isSensorActive = false;

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
  Wire.onReceive(recieveEventfromWifi);
  Wire.onRequest(requestEventfromWifi);

  // Cài đặt mode cho pin của cảm biến là pullup
  pinMode(2, INPUT_PULLUP);
  attachInterrupt(0, eventPIRsensor,  RISING);
  pinMode(LED, OUTPUT);

  // Bật GPS trên sim808
  command2sim808("AT+CGNSPWR=1", 2000, DEBUG);
  delay(200);

  // Đổi mode text NMEA sang RMC
  command2sim808("AT+CGNSSEQ=RMC", 2000, DEBUG);
  delay(200);

  // Đổi chế độ text cho chức năng nhắn tin
  command2sim808("AT+CMGF=1", 2000, DEBUG);
  delay(200);

  tryingGPS();

  Serial.print("Is wifi connected to trusted: ");
  Serial.println(isWifiConnected() ? "true" : "false");

}

void loop() {

  Runner();

}

void Runner() {

  // dieu kien dung isWifiConnected() == false, dang set dieu kien gia test he thong
  if (isSensorActive == true && isWifiConnected() == false && isOnGuarded == true) {

    // First Alert!
    tryingGPS();
    SendWarningMessage(masterPhoneNum);
    delay(2000);

    // read all return
    readAllResponse(1000);
    delay(50);

    // Wait for cancel signal!
    if (waitForCancelCallSignal(waitTimeout)) {
      isOnGuarded = false;
      Serial.println("Sequence cancel!");
    }
    else {     
      // bat canh bao
      
      // wait for relocate command
      while (true) {
        if (ListenToRELOmessage()){
          tryingGPS();
          SendLocationReport(masterPhoneNum);
        }
      }
    }
  }
  else {
    isSensorActive == false;
  }
}

// SENSOR SECTION: THINH

void eventPIRsensor() {

  if (isOnGuarded == true && isSensorActive == false) {

    // Thêm hệ điều kiện để hoạt động chính xác hơn.

    interval = millis() - lastSwitchDetectedMIllis;
    if (interval > debounceInterval) {
      lastSwitchDetectedMIllis = millis();
      isSensorActive = true;
      Serial.println("Ngat kich hoat.");
    }
    else {
      Serial.println("Skip ngat (time).");
    }
  }
  else {
      Serial.println("Skip ngat (condition).");
  }
}

// WIFI SECTION: HUNG

boolean isWifiConnected() {

  // todo: return true false hop ly > DONE
  
  if (stateWifi == "0") return false;
  else return true;
}

void recieveEventfromWifi(int howMany) {

  // Hàm được gọi tương tự khi gọi ngắt mà nhận được sự kiện từ esp

  stateWifi = "";
  while (0 < Wire.available()) {
    char c = Wire.read();
    stateWifi += c;
  }

}

void requestEventfromWifi() {
  x += latitude;
  x += "-";
  x += longitude;
  x += ",";
  int str_len = x.length() + 1;
  char location[str_len];
  x.toCharArray(location, str_len);
  Wire.write(location);
  x = "";
}

// GPS SECTION: LAM

void tryingGPS() {
  Serial.println("Trying to get gps...");
  while (!getGPSinfo(true)) {
    Serial.println("...");
  }
  Serial.println("Get gps successfully!");
}

boolean getGPSinfo(boolean debug) {
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
  if (debug) {
    state = data[1];
    timegps = data[2];
    latitude = data[3];
    longitude = data[4];
    //    Wire.onRequest(requestEventfromWifi);
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

String createMapsLinkWithLocation() {
  String link = "http://maps.google.com/maps?q=";
  link += latitude;
  link += ",";
  link += longitude;
  return link;
}

// SMS send, receive and CALL SECTION: TIEN

String command2sim808 (String command , const unsigned long timeout , boolean debug) {
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



void SendWarningMessage(String masterPhoneNum) {
  String message = "Xe co the da bi mat trom, goi " + modulePhoneNum + " de huy canh bao! Vi tri xe la " + createMapsLinkWithLocation();
  sendSMS(masterPhoneNum, message);
}

void SendLocationReport(String masterPhoneNum) {
  String message = "Vi tri hien tai cua xe la: "+createMapsLinkWithLocation();
  sendSMS(masterPhoneNum, message);
}

void sendSMS(String masterPhoneNum, String message) {
  String command1 = "AT+CMGS=\"";
  command1 += masterPhoneNum;
  command1 += "\"";
  sim808.println("at");
  delay(200);
  sim808.println("AT+CMGF=1");
  delay(200);
  sim808.println(command1);
  delay(1000);
  sim808.println(message);
  delay(1000);
  sim808.println((char)26);
  delay(1000);
  Serial.println("SMS sent successfully.");
}

void readAllResponse(unsigned long waitTime){
  String response = "";
  unsigned long times = millis();
  while ((times + waitTime) > millis()) {
    while (sim808.available()) {
      char c = (char)sim808.read();
      Serial.println(c);
      response += c;
    }
  }
}

boolean waitForCancelCallSignal(unsigned long waitTime) {
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


boolean ListenToRELOmessage() {
  command2sim808("AT+CNMI=2,2,0,0,0", 2000, true);
  delay(1000);
  Serial.println("Listening to RELO sequence!");
  if (readRELO()) return true;
  else return false;
}

boolean readRELO() {
  String response = "";
  Serial.println("waiting");
  while (true) {
    while (sim808.available()) {
      char c = (char) sim808.read();
      if (c != 'R' && c != 'E' && c != 'L' && c != 'O') {
        break;
      }
      Serial.println(c);
      response += c;
    }
    if (response == "RELO") {
      Serial.println("RELOCATE Confirmed, stop waiting.");
      break;
    }
  }
  if (response == "RELO") {
    return true;
  }
  return false;
}
