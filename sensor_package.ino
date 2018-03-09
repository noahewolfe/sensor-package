/**************************************************************
 *
 * This sketch connects to a website and downloads a page.
 * It can be used to perform HTTP/RESTful API calls.
 *
 * For this example, you need to install ArduinoHttpClient library:
 *   https://github.com/arduino-libraries/ArduinoHttpClient
 *   or from http://librarymanager/all#ArduinoHttpClient
 *
 * TinyGSM Getting Started guide:
 *   http://tiny.cc/tiny-gsm-readme
 *
 **************************************************************/

// Select your modem:
#define TINY_GSM_MODEM_SIM800
// #define TINY_GSM_MODEM_SIM808
// #define TINY_GSM_MODEM_SIM900
// #define TINY_GSM_MODEM_A6
// #define TINY_GSM_MODEM_A7
// #define TINY_GSM_MODEM_M590
// #define TINY_GSM_MODEM_ESP8266

// Increase RX buffer
#define TINY_GSM_RX_BUFFER 200

// Use Hardware Serial on Mega, Leonardo, Micro
//#define atSerial Serial1

// DGS-NO2 stuff
#include "DGS.h"

// or Software Serial on Uno, Nano
#include <SoftwareSerial.h>
// ===== Setup variables for DGS-NO2 =====
SoftwareSerial mySerial(3, 4); // !!! RX, TX Must be on 3.3 volt communication, or using level shifters to get to 3.3V UART!!!
DGS mySensor(mySerial);
SoftwareSerial atSerial(12, 13); // RX, TX

//#define DUMP_AT_COMMANDS
//#define TINY_GSM_DEBUG Serial


// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "wholesale";
//const char user[] = "";
//const char pass[] = "";

// Name of the server we want to connect to
//const char server[] = "54.227.187.108";
//const int  port     = 80;
// Path to download (this is the bit after the hostname in the URL)
const char resource[] = "/data"; //"/ip";

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>

TinyGsm modem(atSerial);
TinyGsmClient client(modem);
HttpClient http(client, "54.227.187.108", 80);

// LED's
int GREEN = 7;
int BLUE = 6;
int RED = 5;

//String imei = "";

void setup() {
  // Set console baud rate
  Serial.begin(9600);
  delay(10);

  // ===== LED initialization =====
  pinMode(GREEN, OUTPUT);
  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);
  // Flash all on to test
  digitalWrite(GREEN, HIGH);
  digitalWrite(RED, HIGH);
  digitalWrite(BLUE, HIGH);

  // set dgs baud rate
  mySerial.begin(9600);
  delay(5000);

  // Setup DGS-NO2
  //mySensor.DEBUG = false; //Use true/false for verbose output of setup

  // Set GSM module baud rate
  atSerial.begin(9600);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println(F("Initializing modem..."));
  modem.restart();

  // Then turn off everything except power light, to signal that the startup cycle is done
  delay(1000);
  digitalWrite(RED, LOW);
  digitalWrite(BLUE, LOW);

  // Unlock your SIM card with a PIN
  //modem.simUnlock("1234");
}

//int num = 0;

void blueFlash(int flashes) {
  for (int i = 0; i < flashes; i++) {
    digitalWrite(BLUE, HIGH);
    delay(250);
    digitalWrite(BLUE, LOW);
    delay(250);
  }
}

void errorFlash() {
  // comes out to total wait time of 5 seconds
  for (int i = 0; i < 5; i++) {
    digitalWrite(RED, HIGH);
    delay(500);
    digitalWrite(RED, LOW);
    delay(500);
  }
}

void loop() {

  // ===== Grabbing Data =====
  blueFlash(1);
  
  Serial.print(F("Grabbing data..."));
  // get a piece of no2 data
  mySerial.listen(); // make sure to listen to the port with the sensor!
  mySensor.getData('\r');
  delay(100);

  // ===== Sending Data =====
  // then, switch over to listening for the modem port 
  atSerial.listen();

  blueFlash(2);
  
  Serial.print(F("Waiting for network..."));
  if (!modem.waitForNetwork()) {
    //Serial.println(F(" fail"));
    errorFlash();
    //delay(10000);
    return;
  }
  Serial.println(F(" OK"));

  blueFlash(3);

  Serial.print(F("Connecting to "));
  //Serial.print(apn);
  if (!modem.gprsConnect(apn, "", "")) { // apn, user, password
    //Serial.println(F(" fail"));
    errorFlash();
    //delay(10000);
    return;
  }
  Serial.println(F(" OK"));

  blueFlash(4);

  Serial.print(F("Performing HTTP POST request... "));
  //"i=" + modem.getIMEI() + "&q=" + csq + "&c="+ conc + "&t="+temp + "&r="+rh // + F("_") + temp + F("_") + rh
  int err = http.post(resource, F("application/x-www-form-urlencoded"), 
    "i=" + modem.getIMEI() + 
    "&q=" + modem.getSignalQuality() + 
    "&c=" + mySensor.getConc() + 
    "&t=" + mySensor.getTemp() + 
    "&r=" + mySensor.getRh() 
  );
  if (err != 0) {
    Serial.println(F("failed to connect"));
    errorFlash();
    //delay(10000);
    return;
  }
}

