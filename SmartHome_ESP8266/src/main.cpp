#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <Adafruit_Fingerprint.h>

#define ChipSelect_Pin D8
#define RST_Pin D0
#define RX_Pin_fingerprintSensor D2
#define TX_Pin_fingerprintSensor D3
#define Relay D1

// Config Pin Comunication:
MFRC522 mfrc522(ChipSelect_Pin, RST_Pin); 
SoftwareSerial COMfingerprintSensor(RX_Pin_fingerprintSensor, TX_Pin_fingerprintSensor);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&COMfingerprintSensor);


//Variable:
byte uidBytes[4];
uint8_t sysState = 0; 


//-------------------------------------------Function Read UID-------------------------------------------
byte UID_1[4] = {145, 62, 133, 47};
byte UID_2[4] = {19, 3, 135, 169};
byte UID_3[4] = {163, 6, 85, 169};
byte UID_4[4] = {68, 152, 44, 131};
uint8_t UID_Correct = 0;
void readUID(){
  while (!(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()));
    // Lưu trữ UID của thẻ RFID vào một mảng
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidBytes[i] = mfrc522.uid.uidByte[i];
    Serial.print("...");
    delay(10);
  }
  Serial.println("...");

  for(uint8_t i = 0; i < 4; i++){
    if((uidBytes[i] == UID_1[i]) | (uidBytes[i] == UID_2[i]) | (uidBytes[i] == UID_3[i]) | (uidBytes[i] == UID_4[i])){
      UID_Correct = 1;
    }
    else{
      UID_Correct = 0;
    }
  }
}
//-------------------------------------------------------------------------------------------------------


//----------------------------------------Function show information Fingerprint_Sensor------------------------------------
void showInformationSensor(){
    if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
}
//-----------------------------------------------Compare ID Fingerprint_Sensor------------------------------------------------------------------
// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  return finger.fingerID;
}

//-----------------------------------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------Func Enroll-------------------------------------------------------------------------------
uint8_t id;
uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}

uint8_t getFingerprintEnroll() {

  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}


void enrollFingerprint(){
  Serial.println("Ready to enroll a fingerprint!");
  Serial.println("Please type in the ID # (from 1 to 127) you want to save this finger as...");
  id = readnumber();
  if (id == 0) {// ID #0 not allowed, try again!
     return;
  }
  Serial.print("Enrolling ID #");
  Serial.println(id);

  while (!  getFingerprintEnroll() );
}


//-----------------------------------------------------------------------------------------------------------------------------------------------


void setup() {
  Serial.begin(9600);
  finger.begin(57600);
  delay(10);

  //Initilize MFRC522:
  SPI.begin();   
  mfrc522.PCD_Init();
  Serial.println("Hello");

  //Config I/O:
  pinMode(Relay, OUTPUT);
}


void loop(){

  //State wating:
  if(sysState == 0){
    Serial.println("Enter function: ");
    while(!(Serial.available()));
    String receiveData = Serial.readString();
    if(receiveData == "1"){
      sysState = 1;
    }
    else if(receiveData == "2"){
      sysState = 2;
    }
    else if(receiveData == "3"){
      sysState = 3;
    }
    else if(receiveData == "4"){
      sysState = 4;
    }
  }

  // Read UID:
  if(sysState == 1){
    Serial.println("...Touch RFID Card...");
    readUID();
    if(UID_Correct == 1){
      digitalWrite(Relay, 1);
    }
    else{
      digitalWrite(Relay, 0);
    }
    sysState = 0;

  }

  // Read ID Fingerprint:
  if(sysState == 2){
    showInformationSensor();
    Serial.println("...Touch finger...");
    int Temp1 = getFingerprintIDez();
    while(Temp1 == -1){
      getFingerprintIDez();
      Temp1 = getFingerprintIDez();
    }
    digitalWrite(Relay, 1);
    sysState = 0;
  }

  // Enroll:
  if(sysState == 3){
    enrollFingerprint();
    sysState = 0;
  }
  
  if(sysState == 4){
    digitalWrite(Relay, 0);
    sysState = 0;
  }

}