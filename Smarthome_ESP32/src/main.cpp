#include <Arduino.h>
#include <SoftwareSerial.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

// #define BLYNK_TEMPLATE_ID "TMPL6IifWIeqH"
// #define BLYNK_TEMPLATE_NAME "SmartHomeESP32withFingerprint"
// #define BLYNK_AUTH_TOKEN "fRR5_rtgyTGoXBI7HYV4s4mOsvY8-czJ"

#define BLYNK_TEMPLATE_ID "TMPL6ilB9Vbg2"
#define BLYNK_TEMPLATE_NAME "SmartRoom"
#define BLYNK_AUTH_TOKEN "01xHS4rygamcT64pT8aeET_N8gKsgIl-"

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>


#define Button 15
#define DHT21_Pin 13
#define DHT_Type DHT21
#define infraredSensor1_Pin 4
#define infraredSensor2_Pin 0
#define LED_Room1_Pin 2
#define LED_Room2_Pin 14
#define SoftUART_RX_Pin 16
#define SoftUART_TX_Pin 5
#define Relay 12

DHT dht(DHT21_Pin, DHT_Type);
SoftwareSerial SoftUART(SoftUART_RX_Pin, SoftUART_TX_Pin);

//Variable:
char ssid[] = "Akaito";       
char pass[] = "thang1310"; 
boolean StateButon = 0;
String stateRoom1 = "OFF";
String stateRoom2 = "OFF";
uint32_t countValue_1 = 0;
uint32_t countValue_2 = 0;
uint8_t stateLED_1 = 0;
uint8_t stateLED_2 = 0;
uint8_t manual = 0;
void sendDatatoBlynk(){   

  //Read sensor
  if((digitalRead(infraredSensor1_Pin) == 0) && (manual == 0)){
    stateLED_1 = 1;
  }
  if((digitalRead(infraredSensor2_Pin) == 0) && (manual == 0)){
    stateLED_2 = 1;
  }

  //LED Room 1:
  if(stateLED_1 == 0){
    digitalWrite(LED_Room1_Pin, 0);
    stateRoom1 = "Room 1: OFF";
  }
  if(stateLED_1 == 1){
    digitalWrite(LED_Room1_Pin, 1);
    stateRoom1 = "Room 1: ON";
    countValue_1++;
    delay(1);
    if(countValue_1 > 20){
      countValue_1 = 0;
      stateLED_1 = 0;
    }
  }
  if(stateLED_1 == 2){
    digitalWrite(LED_Room1_Pin, 1);
    stateRoom1 = "Room 1: ON";
  }

  //LED Room 2:
  if(stateLED_2 == 0){
    digitalWrite(LED_Room2_Pin, 0);
    stateRoom2 = "Room 2: OFF";
  }
  if(stateLED_2 == 1){
    digitalWrite(LED_Room2_Pin, 1);
    stateRoom2 = "Room 2: ON";
    countValue_2++;
    delay(1);
    if(countValue_2 > 20){
      countValue_2 = 0;
      stateLED_2 = 0;
    }
  }
  if(stateLED_2 == 2){
    digitalWrite(LED_Room2_Pin, 1);
    stateRoom2 = "Room 2: ON";
  }

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float f = dht.readTemperature(true);
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read data from DHT sensor!");
    return;
  }
  float hif = dht.computeHeatIndex(f, h);
  float hic = dht.computeHeatIndex(t, h, false);

  Blynk.virtualWrite(V0, stateRoom1);
  Blynk.virtualWrite(V1, stateRoom2);
  Blynk.virtualWrite(V2, t);
  Blynk.virtualWrite(V3, h);

} 

void setup() {
  Serial.begin(9600);
  SoftUART.begin(9600);
  dht.begin();

  //Configurate I/O Pin:
  pinMode(Button, INPUT);
  pinMode(DHT21_Pin, INPUT);
  pinMode(infraredSensor1_Pin, INPUT);
  pinMode(infraredSensor2_Pin, INPUT);
  pinMode(LED_Room1_Pin, OUTPUT);
  pinMode(LED_Room2_Pin, OUTPUT);
  pinMode(Relay, OUTPUT);

  // Setup Wifi:
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.println("Connected to Wifi");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  if (Blynk.connected()){
    Serial.println("Connected to Blynk");
  }

}

void loop() {
  Blynk.run();
  sendDatatoBlynk();
  if(SoftUART.available()){
    String receiveData = SoftUART.readString();
    Blynk.virtualWrite(V5, receiveData);
    if(receiveData == "5"){
      digitalWrite(Relay, 1);
    }
    if(receiveData == "6"){
      digitalWrite(Relay, 0);
    }
  }
}

BLYNK_WRITE(V5)
{
  String receiveData = param.asString();
  SoftUART.print(receiveData);
  Serial.print(receiveData);
//
 }

BLYNK_WRITE(V4)
{
  boolean receiveData = param.asInt();
  if(receiveData == 1){
    SoftUART.print("4");
  }
}

BLYNK_WRITE(V6)
{
  boolean receiveData = param.asInt();
  Blynk.virtualWrite(V8, 1);
  manual = 1;
  if(receiveData == 1){
    stateLED_1 = 2;
  }
  else{
    stateLED_1 = 0;
  }
}
BLYNK_WRITE(V7)
{
  boolean receiveData = param.asInt();
  manual = 1;
  Blynk.virtualWrite(V8, 1);
  if(receiveData == 1){
    stateLED_2 = 2;
  }
  else{
    stateLED_2 = 0;
  }
}
BLYNK_WRITE(V8)
{
  boolean receiveData = param.asInt();
  if(receiveData == 1){
    manual = 1;
  }
  else{
    Blynk.virtualWrite(V6, 0);
    stateLED_1 = 0;
    Blynk.virtualWrite(V7, 0);
    stateLED_2 = 0;
    manual = 0;
  }
}