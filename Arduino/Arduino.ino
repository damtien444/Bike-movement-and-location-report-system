#include <Wire.h>
#include <SoftwareSerial.h>
SoftwareSerial sim808(11, 10); //Arduino(RX), Arduino(TX)
//Arduino(RX) to SIM808(TX)
//Arduino(TX) to SIM808(RX)

String x;
#define DEBUG true
String state, timegps, latitude, longitude;
String message, tin_nhan;


void setup() {
  Wire.begin(8);                /* join i2c bus with address 8 */
  sim808.begin(9600);
  Serial.begin(9600);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);
  sendData("AT+CGNSPWR=1", 2000, DEBUG); //Turn on GPS(GNSS - Global Navigation Satellite System)
  delay(2000);
  sendData("AT+CGNSSEQ=RMC", 2000, DEBUG);
  delay(2000);
  sendData("AT+CMGF=1",2000,DEBUG);
  delay(2000);
  
}
void loop() {
  sendTabData("AT+CGNSINF", 2000, DEBUG); //Get GPS info(location)
//  Wire.onReceive(receiveEvent);
//  requestEvent();
}
void sendTabData(String command , const int timeout , boolean debug) {
  String data[5];
  sim808.println(command);
  long int time = millis();
  int i = 0;
  while ((time + timeout) > millis()) {
    while (sim808.available()) {
      char c = sim808.read();
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
  Serial.println(debug);
  if (debug) {
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
  } else {
    Serial.println("GPS Initializing…");
  }
  Serial.println("------------------------------------------------");
}
String sendData (String command , const int timeout , boolean debug) {
  String response = "";
  sim808.println(command);
  long int time = millis();
  while ( (time + timeout ) > millis()) {
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

void SendMessage() {

  sim808.println("AT+CMGF=1");    //Sets the GSM Module in Text Mode

  delay(1000);  // Delay of 1000 milli seconds or 1 second

  sim808.println("AT+CMGS=\"+8468299258\"\r"); // Replace x with mobile number

  delay(1000);

  sim808.println("I am SMS from GSM Module");// The SMS text you want to send

  delay(100);

  sim808.println((char)26);// ASCII code of CTRL+Z

  delay(1000);
}

void RecieveMessage() {

  sim808.println("AT+CNMI=2,2"); // AT Command to recieve a live SMS
  readMessages();
  delay(1000);

}

void readMessages() {
  ​while (sim808.available() > 0) {
    ​char c = sim808.read(); //Đọc từng ký tự gửi v
    ​if (c == '#') {
      ​break;
      ​
    } //Nếu phát hiện ký tự "#" thì thoát vòng lặp
    ​tin_nhan += c; //Ghép các ký tự thành 1 xâu tin_nha
    ​
  }
  ​if (tin_nhan.length() > 0) {
    ​Serial.println(tin_nhan);x
//    ​if (tin_nhan == "bat_den") {
//      ​digitalWrite(13, HIGH)
//      ​
//    }
//    ​else if (tin_nhan == "tat_den") {
//      ​digitalWrite(13, LOW)
//      ​
//    }
    message = tin_nhan;
    ​tin_nhan = "";
  }
  
}

void receiveEvent(int howMany){
  String recibido;
  while (0 < Wire.available()) {
    char c = Wire.read();
    recibido += c;
  }
  Serial.print(recibido);
}
