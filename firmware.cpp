#include <Adafruit_Fingerprint.h>

//FirebaseESP8266.h must be included before ESP8266WiFi.h #include "FirebaseESP8266.h"
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display LiquidCrystal_I2C lcd(0x27, 16, 4);
#define FIREBASE_HOST "https://payme.firebaseio.com/" //Without http:// or https:// schemes
#define FIREBASE_AUTH "GN9******************************mpxa"
#define WIFI_SSID "Convenant University"
#define WIFI_PASSWORD "12345678"

SoftwareSerial mySerial(D2, D1); //(RX, TX) 
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// constants won't change. They're used here to set pin numbers: const int UP = D7; // the number of the pushbutton pin const int DW = D6;
const int SE = D5;
const int Led1 = D0; const int Led2 = D8;

// variables will change:
int UPstate = 0;
int DWstate = 0;
int SEstate = 0;

// variable for reading the pushbutton status
String name  = "";
String email = "";
int balance;
String details = "Purchase of N";
String acct_no = "";
bool bio = false;
int finger_count;
uint8_t id;
int Amount = 0;
int option = 0;
int Price = 0;

//Define Firebase Data object
FirebaseData firebaseData;
String path = "/Test/";
String path2 = "/Transactions";
String jsonStr = "";
FirebaseJson json;

void printResult(FirebaseData &data);

void setup(){
  lcd.begin(D3, D4); // sda=0, scl=2

  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.print("Welcome");

  pinMode(Led1, OUTPUT);
  pinMode(Led2, OUTPUT);
  pinMode(UP, INPUT);
  pinMode(DW, INPUT);
  pinMode(SE, INPUT);
  digitalWrite(Led2, LOW);
  digitalWrite(Led1, LOW);

  Serial.begin(9600);
  while (!Serial) { // For Yun/Leo/Micro/Zero/...
    delay(100);
  }
  Serial.println("\n\nAdafruit finger detect test");

  // Set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } 
  else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) {
      delay(1);
    }
  }

  finger.getTemplateCount();
  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  } 
  else {
    Serial.println("Waiting for valid finger...");
    Serial.print("Sensor contains ");
    Serial.print(finger.templateCount);
    Serial.println(" templates");
  }

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  // Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  firebaseData.setBSSLBufferSize(1024, 1024);

  // Set the size of HTTP response buffers in the case where we want to work with large data.
  firebaseData.setResponseSize(1024);

}

void loop() {
  Amount = 100;
  UPstate = digitalRead(UP);
  DWstate = digitalRead(DW);
  SEstate = digitalRead(SE);
  lcd.setCursor(0,0);
  lcd.print("BIOMETRIC P.O.S ");
  lcd.setCursor(0,1);
  lcd.print(" ");
  lcd.setCursor(-4,2);
  lcd.print("REG PAY BAL.");
  lcd.setCursor(-4,3);
  lcd.print(" V V V ");

  if(UPstate == HIGH){
    digitalWrite(Led1, HIGH);
    Enroll();
    delay(2000);
  } 
  else {
    digitalWrite(Led1, LOW);
  }

  if(DWstate == HIGH){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Place finger ");
    delay(1000);
    digitalWrite(Led2, HIGH);
    Serial.println("Place Finger");
    delay(2000);
    while(!getFingerprintID());
    delay(2000);
  } 
  else {
    // turn LED off:
    digitalWrite(Led2, LOW);
  }

  if(SEstate == HIGH){
    option++;
    delay(1500);
  }

  while(option > 0){
    option = constrain(option, 0, 3);
    lcd.clear();
    int pause = 100;

    if(digitalRead(SE) == HIGH){
      option++;
      delay(1500);
    }

    if(option >= 3){
      option = 0;
    }

    if(option == 1){
      lcd.setCursor(0,0);
      lcd.print(" Enter Amount ");
      lcd.setCursor(0,1);
      lcd.print("#");
      lcd.print(Amount);
      lcd.print(" ");

      if(digitalRead(DW) == HIGH){
        Amount -= 100;
        if(Amount < 0){
          Amount = 0;
        }
      }
      if(digitalRead(UP) == HIGH){
        Amount += 100;
      }
      delay(pause);
    }
    
    if(option == 2){
      lcd.setCursor(0,0);
      lcd.print("Place finger ");
      delay(2000);
      digitalWrite(Led2, HIGH);
      Serial.println("Place Finger");
      delay(2000);
      while(!getFingerprintID());
      delay(1000);

      if(bio == true){
        lcd.clear();
        
        if (Firebase.get(firebaseData, path + acct_no + "/name")) {
          name = firebaseData.stringData();
          lcd.setCursor(0,0);
          lcd.print(name);
          //lcd.print(" "); 
          Serial.println("------------------------------------"); 
          Serial.print("NAME IS >>>> ");
          Serial.println(name); 
          Serial.println("------------------------------------"); 
          Serial.println();
        } 
        else {
          lcd.setCursor(0,0); 
          lcd.print(firebaseData.errorReason());
          //lcd.print(" "); 
          Serial.println("REASON: " + firebaseData.errorReason()); 
          Serial.println("------------------------------------"); 
          Serial.println();
        }

        if (Firebase.get(firebaseData, path + acct_no + "/email")) {
          email = firebaseData.stringData();
          lcd.setCursor(0,1);
          lcd.print(email);
          //lcd.print(" "); 
          Serial.println("------------------------------------"); 
          Serial.print("EMAIL IS >>>> ");
          Serial.println(email); Serial.println("------------------------------------"); 
          Serial.println();
        } 
        else{
          lcd.setCursor(0,1); lcd.print(firebaseData.errorReason());
          //lcd.print(" "); 
          Serial.println("FAILED");
          Serial.println("REASON: " + firebaseData.errorReason()); 
          Serial.println("------------------------------------"); S
          erial.println();
        }
      
        if (Firebase.getInt(firebaseData, path + acct_no + "/balance")){
          balance = firebaseData.intData(); 
          Serial.println("------------------------------------"); 
          Serial.print("BALANCE IS >>>> "); 
          Serial.println(balance); 
          Serial.println("------------------------------------"); 
          Serial.println();
          lcd.setCursor(-4,2);
          lcd.print("Old bal. #");
          lcd.print(balance);
          //lcd.print(" ");

          if(balance >= Amount){
            balance -= Amount;
            lcd.setCursor(-4,3);
            lcd.print("New bal. #");
            lcd.print(balance);
            //lcd.print(" ");
            delay(3000);
            update_akant();
          } 
          else{
            delay(3000);
            lcd.setCursor(-4,2);
            lcd.print("Insuficient Fund");
            lcd.setCursor(-4,3);
            lcd.print("  Fund Account     ");
          }  
        }
    
        else {
          lcd.setCursor(-4,2);
          lcd.print(firebaseData.errorReason());
          lcd.setCursor(-4,3);
          lcd.print("Try again Later");
          Serial.println("FAILED");
          Serial.println("REASON: " + firebaseData.errorReason()); 
          Serial.println("------------------------------------"); 
          Serial.println();
        }
        delay(5000);
        bio = false;
      }
      option = 0; 
    }
  }

  if(bio == true){
    lcd.clear();
    if (Firebase.get(firebaseData, path + acct_no + "/name")) {
      name = firebaseData.stringData();
      lcd.setCursor(0,0);
      lcd.print(name);
      //lcd.print(" "); 
      Serial.println("------------------------------------"); 
      Serial.print("NAME IS >>>> ");
      Serial.println(name); Serial.println("------------------------------------"); 
      Serial.println();
      } 
      else{
          lcd.setCursor(0,0);
          lcd.print(firebaseData.errorReason());
          // lcd.print(" "); 
          Serial.println("FAILED");
          Serial.println("REASON: " + firebaseData.errorReason()); 
          Serial.println("------------------------------------"); 
          Serial.println();
          }
          
      if (Firebase.get(firebaseData, path + acct_no + "/email")){
        email = firebaseData.stringData();
        lcd.setCursor(0,1);
        lcd.print(email);
        //lcd.print(" "); 
        Serial.println("------------------------------------"); 
        Serial.print("EMAIL IS >>>> ");
        Serial.println(email); Serial.println("------------------------------------"); 
        Serial.println();
        } 
      else{
        lcd.setCursor(0,1); lcd.print(firebaseData.errorReason()); 
        Serial.println("FAILED");
        Serial.println("REASON: " + firebaseData.errorReason()); 
        Serial.println("------------------------------------"); 
        Serial.println();
        }
      
      if (Firebase.getInt(firebaseData, path + acct_no + "/balance")){
        balance = firebaseData.intData(); 
        Serial.println("------------------------------------");
        Serial.print("BALANCE IS >>>> "); 
        Serial.println(balance); 
        Serial.println("------------------------------------"); 
        Serial.println();
        lcd.setCursor(-4,2);
        lcd.print("Balance #");
        lcd.print(balance);
        //lcd.print(" ");
        delay(3000);
        lcd.setCursor(-4,3);
        lcd.print("******DONE******");
        } 
      else{
        lcd.setCursor(-4,2); 
        lcd.print(firebaseData.errorReason()); 
        lcd.print(" ");
        lcd.setCursor(-4,3);
        lcd.print("Try again Later");
        Serial.println("FAILED");
        Serial.println("REASON: " + firebaseData.errorReason()); 
        Serial.println("------------------------------------"); 
        Serial.println();
        }
        delay(7000);
        bio = false;
  }
  acct_no = "";
}


void update_akant(){
  lcd.clear();
  String trans = details + String(Amount);
  if (Firebase.setInt(firebaseData, path + acct_no + "/balance", balance)){
    lcd.setCursor(0,0);
    lcd.print(" Transaction ");
    delay(2000);
    lcd.setCursor(0,1);
    lcd.print(" Successful "); Serial.println("PASSED");
    Serial.println("PATH: " + firebaseData.dataPath()); 
    Serial.println("TYPE: " + firebaseData.dataType()); 
    Serial.println("ETag: " + firebaseData.ETag()); 
    //printResult(firebaseData); 
    Serial.println("------------------------------------"); 
    Serial.println();
    Serial.println("Push Json...");
    json.clear();
    json.add("name", name);
    json.add("email", email);
    json.add("acct_no", acct_no);
    json.add("balance", balance);
    json.add("Transxn", trans);
        
        if (Firebase.pushJSON(firebaseData, path2, json)){
          lcd.setCursor(-4,2);
          lcd.print(" Acount Updated ");
          delay(2000);
          lcd.setCursor(-4,3);
          lcd.print("******DONE****** "); Serial.println("PASSED");
          Serial.println("PATH: " + firebaseData.dataPath()); 
          Serial.print("PUSH NAME: "); 
          Serial.println(firebaseData.pushName()); 
          Serial.println("------------------------------------"); 
          Serial.println();
          } 
        else {
          lcd.setCursor(-4,2);
          lcd.print("Couldn't Update ");
          delay(3000);
          lcd.setCursor(-4,2); 
          lcd.print(firebaseData.errorReason());
          lcd.print(" ");
          delay(2000);
          lcd.setCursor(-4,3);
          lcd.print("******DONE****** "); 
          Serial.println("FAILED");
          Serial.println("REASON: " + firebaseData.errorReason()); 
          Serial.println("------------------------------------"); 
          Serial.println();
          }
    } 
    else {
      lcd.setCursor(0,0);
      lcd.print(" Transaction ");
      delay(1000);
      lcd.setCursor(0,1);
      lcd.print(" Failed ");
      lcd.setCursor(-4,2); 
      lcd.print(firebaseData.errorReason());
      delay(2000);
      lcd.setCursor(-4,3);
      lcd.print("Try again Later "); Serial.println("FAILED");
      Serial.println("REASON: " + firebaseData.errorReason()); 
      Serial.println("------------------------------------"); 
      Serial.println();
      } 
  }



uint8_t Enroll(void){
  lcd.clear();
  finger.getTemplateCount();
  Serial.println("Ready to enroll a fingerprint!");
  Serial.print("Generating ID..........   ");
  Serial.println(finger.templateCount);
  id = int(finger.templateCount) + 1;
  if (finger.templateCount == 0) {
    Serial.println("Welcome 1st User");
    lcd.setCursor(0,0);
    lcd.print("Welcome 1st User");
    }
    Serial.print("Enrolling ID #");
    Serial.println(id);
    lcd.setCursor(0,0);
    lcd.print("Enroll User #");
    lcd.print(id);
    lcd.print("     ");
  while (!  getFingerprintEnroll() );
}

uint8_t getFingerprintEnroll() {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); 
  Serial.println(id); 
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        lcd.setCursor(0,1);
        lcd.print("Image taken     ");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println(".");
        lcd.setCursor(0,1);
        lcd.print("  Place Finger  ");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        lcd.setCursor(0,1);
        lcd.print("Comm. Error!    ");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        lcd.setCursor(0,1);
        lcd.print("Imaging error   ");
        break;
      default:
        Serial.println("Unknown error");
        lcd.setCursor(0,1);
        lcd.print("Unknown error  ");
        break;
      }
    }

  // OK success!
  delay(2000);
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      lcd.setCursor(0,1);
      lcd.print("Image converted ");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      lcd.setCursor(0,1);
      lcd.print("Image too messy ");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      lcd.setCursor(0,1);
      lcd.print("Comm. Error!    ");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      lcd.setCursor(0,1);
      lcd.print("Not Found       ");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      lcd.setCursor(0,1);
      lcd.print("Not Found     ");
      return p;
    default:
      Serial.println("Unknown error");
      lcd.setCursor(0,1);
      lcd.print("Unknown error   ");
      return p;
    }
  
  delay(1000);
  Serial.println("Remove finger");
  lcd.setCursor(0,1);
  lcd.print("Remove finger   ");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); 
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
    
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();

    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        lcd.setCursor(-4,2);
        lcd.print("  Image taken  ");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        lcd.setCursor(-4,2);
        lcd.print("  Place Again ");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        lcd.setCursor(-4,2);
        lcd.print("Comm. Error!   ");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        lcd.setCursor(-4,2);
        lcd.print("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        lcd.setCursor(-4,2);
        lcd.print("Unknown error  ");
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
    Serial.print("Creating model for #");  
    Serial.println(id);
    delay(2000);
    p = finger.createModel();
    if (p == FINGERPRINT_OK) {
      Serial.println("Prints matched!");
      lcd.setCursor(-4,2);
      lcd.print("Prints matched!  ");
      } 
    else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      Serial.println("Communication error");
      lcd.setCursor(-4,2);
      lcd.print("Comm Error           ");
      return p;
      } 
    else if (p == FINGERPRINT_ENROLLMISMATCH) {
      Serial.println("Fingerprints did not match");
      lcd.setCursor(-4,2);
      lcd.print("Finger not match    ");
      return p;
    } 
    else {
      Serial.println("Unknown error");
      lcd.setCursor(-4,2);
      lcd.print("Unknown Error       ");
      return p; 
      }
    Serial.print("ID "); 
    Serial.println(id);
    p = finger.storeModel(id);
  
    if (p == FINGERPRINT_OK) {
      Serial.println("Stored!");
      lcd.setCursor(-4,3);
      lcd.print("Print Stored!   ");
      } 
    else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      Serial.println("Communication error");
      lcd.setCursor(-4,3);
      lcd.print("Comm. Error!     ");
      return p;
      } 
    else if (p == FINGERPRINT_BADLOCATION) {
      Serial.println("Could not store in that location");
      lcd.setCursor(-4,3);
      }
      lcd.print("Location Error! ");
      return p;
    else if (p == FINGERPRINT_FLASHERR) { 
      Serial.println("Error writing to flash");
      lcd.setCursor(-4,3);
      lcd.print("Error in flash ");
      return p;
      } 
    else {
        Serial.println("Unknown error");
        lcd.setCursor(-4,3);
        lcd.print("Unknown Error       ");
      return p; 
      }
  }

uint8_t getFingerprintID() {
  uint8_t p = -1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    Serial.println("waiting for a valid finger..");
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        lcd.setCursor(0,1);
        lcd.print("Image taken   ");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.println("No finger detected");
        lcd.setCursor(0,1);
        lcd.print("No Finger Detected    ");
        return p;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        lcd.setCursor(0,1);
        lcd.print("  Comm. Error!       ");
        return p;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        lcd.setCursor(0,1);
        lcd.print("Imaging error     ");
        return p;
      default:
        Serial.println("Unknown error");
        lcd.setCursor(0,1);
        lcd.print("Unknown error      ");
        return p;
    } 
  }
  
  // OK success!
  p = finger.image2Tz();
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
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    bio = true;
    acct_no = String(finger.fingerID);
    Serial.print("Found a print match!");
    lcd.setCursor(-4,2);
    lcd.print(" Found a Match   ");
    Serial.print("\t");
    Serial.print(bio);
    Serial.print("\t");
    Serial.println(acct_no);
  } 
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    lcd.setCursor(-4,2);
    lcd.print("  Comm. Error!       ");
    return p;
  } 
  else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    lcd.setCursor(-4,2);
    lcd.print("No Match Found!   ");
    return p;
  } 
  else {
    Serial.println("Unknown error");
    lcd.setCursor(-4,2);
    lcd.print("Unknown error   ");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); 
  Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); 
  Serial.println(finger.confidence); 
  lcd.setCursor(-4,3);
  lcd.print(" Found ID #");
  lcd.print(finger.fingerID);
  lcd.print(" ");
  return finger.fingerID;
}

int getFingerprintIDez() {
  Serial.println("Waiting for valid finger...");
  Serial.print("Sensor contains "); 
  Serial.print(finger.templateCount);
  Serial.println(" templates");
  uint8_t p = finger.getImage();
  
  if (p != FINGERPRINT_OK) 
    return -1;
  p = finger.image2Tz();
  if (p != FINGERPRINT_OK) 
    return -1;
  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK) 
    return -1;
  
  // found a match!
  bio = true;
  acct_no = String(finger.fingerID);
  Serial.print("Found a print match!");
  Serial.print("\t");
  Serial.print(bio);
  Serial.print("\t");
  Serial.println(acct_no);
  Serial.print("Found ID #"); 
  Serial.print(finger.fingerID); 
  Serial.print(" with confidence of "); 
  Serial.println(finger.confidence); 
  return finger.fingerID;
}