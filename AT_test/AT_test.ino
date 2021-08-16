#include <SoftwareSerial.h>
SoftwareSerial sim808(11, 10);
long int waitTimeout = 60000;
#define DEBUG true
String message, tin_nhan;
String state, timegps, latitude, longitude;

void setup() {
  // put your setup code here, to run once:
  sim808.begin(9600);
  Serial.begin(9600);
  //  Serial.write(0x1a);
  //  Serial.println("");

  //  waitForSignal();
  //  send2Sim808("AT+CMGF=1", 1000, DEBUG);

  //  sendSMS("+8468299258", "Lew lew");
  sendData("AT+CGNSPWR=1", 2000, DEBUG); //Turn on GPS(GNSS - Global Navigation Satellite System)
  delay(2000);
  sendData("AT+CGNSSEQ=RMC", 2000, DEBUG);
  delay(2000);
  sendData("AT+CMGF=1", 2000, DEBUG);
  delay(2000);

//  Serial.println("innit gps");
//  while (!getGPSinfo()) {
//    Serial.println("...");
//  }
//  Serial.println("innit gps successfully");
//  getGPSinfo();

  Serial.println("READ sms");
//  delay(10000);
  RecieveMessage();
}

void loop()
{
//  //  getGPSinfo();
//  if (Serial.available())
//
//    sim808.print((char)Serial.read());
//
//  if (sim808.available())
//
//    Serial.print((char)sim808.read());

}

String sendData (String command , const int timeout , boolean debug) {
  String response = "";
  sim808.println(command);
  long int times = millis();
  while ( (times + timeout ) > millis()) {
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
void sendSMS(String sdt, String message) {
  //  String command1 = "AT+CMGS=\"";
  //  command1 += sdt;
  //  command1 += "\"";
  sim808.println("at");
  delay(200);
  sim808.println("AT+CMGF=1");
  delay(200);
  //  send2Sim808("AT+CMEE=1");
  delay(1000);
  sim808.println("AT+CMGS=\"+84372403968\"");
  delay(5000);
  sim808.println("I am SMS from GSM Module");
  delay(1000);
  //  sim808.println(0x1a);
  sim808.println((char)26);
  delay(1000);
}

//String send2Sim808 (String command) {
//  String response = "";
//  sim808.println(command);
//  long int time = millis();
//  while ((time + 1000) > millis()) {
//    while (sim808.available()) {
//      char c = sim808.read();
//      response += c;
//    }
//  }
//  if (true) {
//    Serial.print(response);
//  }
//  return response;
//}

//boolean waitForSignal(){
//  sim808.println("AT");
//  delay(200);
//  String response = "";
//  long int time = millis();
//  while ((time + waitTimeout) > millis()) {
//    while (sim808.available()) {
//      char c = sim808.read();
//      if (c!='R' && c!='I' && c!='N' && c!='G'){
//        break;
//      }
//      Serial.println(c);
//      response += c;
//
//    }
//    if (response == "RING"){
//      Serial.println("Confirmed, stop waiting.");
//      break;
//    }
//
//  }
//
//  if (response == "RING"){
//    Serial.println("Call Confirmed.");
//    return true;
//  }
//  return false;
//}

boolean getGPSinfo() {
  String data[5];
  sim808.println("AT+CGNSINF");
  delay(150);
  long int times = millis();
  int i = 0;
  while ((times + 2000) > millis()) {
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
  //  Serial.println(debug);
  if (true) {
    state = data[1];
    timegps = data[2];
    latitude = data[3];
    longitude = data[4];
    //    Wire.onRequest(requestEvent);
  }
  if (state != "0") {
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

void RecieveMessage() {

//  sim808.println("AT+CNMI=2,2");
  // AT Command to recieve a live SMS
  sendData("AT+CNMI=2,2,0,0,0", 2000, DEBUG);
  
  delay(1000);
  Serial.println("run");
  readMessages();
  delay(1000);

}

void readMessages() {
  long int times = millis();
  tin_nhan="";
  Serial.println("waiting");
  while ((times + 10000) > millis()) {
    Serial.println(".");
    while (sim808.available()) {
      char c = (char) sim808.read();
      Serial.println(c);
//      if (c == '#') break;
      tin_nhan += c;
    }
    if (tin_nhan.length() > 0) {
      Serial.println(tin_nhan);
      Serial.println(tin_nhan+" nhan duoc");
      message = tin_nhan;
      tin_nhan = "";
    }
  }

}
