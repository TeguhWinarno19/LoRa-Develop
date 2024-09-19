// Load Library
#include <SPI.h>
#include <SD.h>
#include <LoRa.h>
#include <Wire.h>
#include "SSD1306Wire.h"
// Definisi pin untuk LoRa
#define LORA_SCK 5    // GPIO5  -- lora SCK
#define LORA_MISO 19  // GPIO19 -- lora MISO
#define LORA_MOSI 27  // GPIO27 -- lora MOSI
#define LORA_SS 18    // GPIO18 -- lora CS
#define LORA_RST 12   // GPIO12 -- RESET
#define LORA_DI0 26   // GPIO26 -- IRQ(Interrupt Request)
#define LORA_BAND 923E6

// Definisi pin untuk OLED
#define OLED_SDA 21
#define OLED_SCL 22
#define OLED_RST 23

#define LED_BUILTIN 25

String induk;
long nilai;

// Initialize the OLED display using Wire library
SSD1306Wire display(0x3c, OLED_SDA, OLED_SCL);

// SPI untuk SD card
SPIClass sdSPI(VSPI);

// SPI untuk LoRa
SPIClass loraSPI(HSPI);

// Receive Package Variable
int RxDataRSSI = 0;
float RxDataSNR;
char Str1[15];
// Parsing Data
String myString;
int i;
String dt[10];
boolean parsing = false;
int simpanOn = 0;
long SDM;
long MAKO;
long LINIA;
long LINIB;
long TASGANU;
long LOGAM;
long SICPA;
long LEVEL;
long SSDM;
long SMAKO;
long SLINIA;
long SLINIB;
long SLOGAM;
long STASGANU;
long SSICPA;
long SLEVEL;
long lastMsg=0;
long lastMsg1=0;
int grafik = 0;
String activity = "";
// Definisi pin untuk antarmuka SPI
#define SD_CS   13   // CS pin untuk SD card module
#define SD_MOSI 15   // MOSI pin untuk SD card module
#define SD_SCK  14   // SCK pin untuk SD card module
#define SD_MISO 2    // MISO pin untuk SD card module

void setup() {
  // START aktivas Oled
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);    // set GPIO16 low to reset OLED
  delay(50);
  digitalWrite(OLED_RST, HIGH);   // while OLED is running, must set GPIO16 in high、

  // INISIALISASI DISPLAY
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  // clear the display
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "SERVER ON");
  display.display();
  delay(1000);
  display.clear();
  // aktivasi Oled END

  // Nyalakan serial monitor
  Serial.begin(9600);
  while (!Serial);

  // Inisialisasi SPI untuk SD card
  sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);

  // Inisialisasi SPI untuk LoRa
  loraSPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);

  // Inisialisasi LoRa
  LoRa.setSPI(loraSPI);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
  if (!LoRa.begin(LORA_BAND)) {
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 15, "> LoRa Initial fail!");
  display.display();

    while (1);
  }
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 15, "> LoRa Initial Ok!");
  display.display();

  // Inisialisasi SD card
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 25, "> Initial SD card");
  display.display();
  if (!SD.begin(SD_CS, sdSPI)) {
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 25, "> Initial SD card fail!");
    display.display();
    while (1);
  }
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 25, "> Initial SD card ok!");
  display.display();

  // Put the radio into receive mode
  LoRa.receive();
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 35, "> Get Data from SD card");
  display.display();
  bacaDariSD(); 
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 35, "> Get Data from SD card ok!");
  display.display();
}

void loop() {
  
  if((SSDM != SDM) || (SMAKO != MAKO) || (SLINIA != LINIA) || (SLINIB != LINIB)|| (STASGANU != TASGANU) || (SLOGAM != LOGAM)|| (SSICPA != SICPA)|| (SLEVEL != LEVEL)){
    SDM = SSDM;
    MAKO = SMAKO;
    LINIA = SLINIA;
    LINIB = SLINIB;
    TASGANU = STASGANU;
    LOGAM = SLOGAM;
    SICPA = SSICPA;
    LEVEL = SLEVEL;
    simpanData();
  }  
  perbaruiScreen();
  receiveData();
  long now = millis();
  if (now - lastMsg > 3000){
    lastMsg = now;
    grafik++;
    if (grafik == 4){
      grafik = 0;
    }
    }
  long now1 = millis();
  if (now - lastMsg1 > 10000){
    lastMsg1 = now1;
    activity = "Send Data to Serial";
        Serial.print(SDM);
        Serial.print(",");
        Serial.print(MAKO);
        Serial.print(",");
        Serial.print(LINIA);
        Serial.print(",");
        Serial.print(LINIB);
        Serial.print(",");
        Serial.print(TASGANU);
        Serial.print(",");
        Serial.print(LOGAM);
        Serial.print(",");
        Serial.print(SICPA);
        Serial.print(",");
        Serial.println(LEVEL);
        activity = "Send Data to Serial done";
  } 
}

void receiveData() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // baca packet
    String receivedData = "";
    while (LoRa.available()) {
      activity = "Receive Data";
      receivedData += (char)LoRa.read();
    }
    // Parse the received data
    if (receivedData.startsWith("Concentrator|")) {
      int firstPipe = receivedData.indexOf('|');
      int secondPipe = receivedData.indexOf('|', firstPipe + 1);
      int thirdPipe = receivedData.indexOf('|', secondPipe + 1);
      int fourthPipe = receivedData.indexOf('|', thirdPipe + 1);
      int fivePipe = receivedData.indexOf('|', fourthPipe + 1);
      int sixPipe = receivedData.indexOf('|', fivePipe + 1);
      int sevenPipe = receivedData.indexOf('|', sixPipe + 1);
      int eightPipe = receivedData.indexOf('|', sevenPipe + 1);

      if (secondPipe != -1 && thirdPipe != -1 && fourthPipe != -1) {
        SSDM    = receivedData.substring(firstPipe + 1, secondPipe).toInt();
        SMAKO   = receivedData.substring(secondPipe + 1, thirdPipe).toInt();
        SLINIA  = receivedData.substring(thirdPipe + 1, fourthPipe).toInt();
        SLINIB  = receivedData.substring(fourthPipe + 1, fivePipe).toInt();
        STASGANU  = receivedData.substring(fivePipe + 1, sixPipe).toInt();
        SLOGAM  = receivedData.substring(sixPipe + 1, sevenPipe).toInt();
        SSICPA  = receivedData.substring(sevenPipe + 1, eightPipe).toInt();
        SLEVEL  = receivedData.substring(eightPipe + 1).toInt();
      }
    }
  }
}

void perbaruiScreen() {
  display.clear();
  // Line 1
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, "LORA IOT SERVER");
  // Line 2
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 3, "___________________________________________");
  // Line 3
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 16, "Status : " + activity);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 28, "RSSI: " + String(LoRa.packetRssi()) + " | SNR:" + String(LoRa.packetSnr()));
  
  if (grafik == 0){
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 40, "LINI A : " + String(LINIA));
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 52, "LINI B : " + String(LINIB));
  }
  if (grafik == 1){
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 40, "SDM   : " + String(SDM));
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 52, "MAKO : " + String(MAKO));
  }
    if (grafik == 2){
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 40, "TASGANU: " + String(TASGANU));
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 52, "LOGAM : " + String(LOGAM));
  }
    if (grafik == 3){
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 40, "SICPA   : " + String(SICPA));
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 52, "LEVEL : " + String(LEVEL));
  }
  display.display();
}

void simpanData() {
  activity = "Simpan data";
  // Rekam data kedalam file excel "SDM.csv"
  File dataFile = SD.open("/DATABASE.csv", FILE_APPEND);
  if (dataFile) {
    dataFile.print(SDM);
    dataFile.print(",");
    dataFile.print(MAKO);
    dataFile.print(",");
    dataFile.print(LINIA);
    dataFile.print(",");
    dataFile.print(LINIB);
    dataFile.print(",");
    dataFile.print(TASGANU);
    dataFile.print(",");
    dataFile.print(LOGAM);
    dataFile.print(",");
    dataFile.print(SICPA);
    dataFile.print(",");
    dataFile.println(LEVEL);
    dataFile.close();
    
  } else {
  }
  simpanOn = 0;
  activity = "Stand by";
}

void bacaDariSD() {
  File dataFile = SD.open("/DATABASE.csv", FILE_READ);
  if (dataFile) {
    String lastLine = "";
    // Baca seluruh isi file dan simpan baris terakhir
    while (dataFile.available()) {
      lastLine = dataFile.readStringUntil('\n');
    }
    dataFile.close();
    
    if (lastLine.length() > 0) {      
      // Pisahkan data menggunakan koma sebagai pemisah
      int firstComma = lastLine.indexOf(',');
      int secondComma = lastLine.indexOf(',', firstComma + 1);
      int thirdComma = lastLine.indexOf(',', secondComma + 1);
      int fourthComma = lastLine.indexOf(',', thirdComma + 1);
      int fiveComma = lastLine.indexOf(',', fourthComma + 1);
      int sixComma = lastLine.indexOf(',', fiveComma + 1);
      int sevenComma = lastLine.indexOf(',', sixComma + 1);      
      
      if (firstComma != -1 && secondComma != -1 && thirdComma != -1) {
        SDM = lastLine.substring(0, firstComma).toInt();
        SSDM = lastLine.substring(0, firstComma).toInt();
        MAKO = lastLine.substring(firstComma + 1, secondComma).toInt();
        SMAKO = lastLine.substring(firstComma + 1, secondComma).toInt();
        LINIA = lastLine.substring(secondComma + 1, thirdComma).toInt();
        SLINIA = lastLine.substring(secondComma + 1, thirdComma).toInt();
//        LINIB = lastLine.substring(thirdComma + 1).toInt();
//        SLINIB = lastLine.substring(thirdComma + 1).toInt();
        LINIB = lastLine.substring(thirdComma + 1, fourthComma).toInt();
        SLINIB = lastLine.substring(thirdComma + 1, fourthComma).toInt();
        TASGANU = lastLine.substring(fourthComma + 1, fiveComma).toInt();
        STASGANU = lastLine.substring(fourthComma + 1, fiveComma).toInt();
        LOGAM = lastLine.substring(fiveComma + 1, sixComma).toInt();
        SLOGAM = lastLine.substring(fiveComma + 1, sixComma).toInt();
        SICPA = lastLine.substring(sixComma + 1, sevenComma).toInt();
        SSICPA = lastLine.substring(sixComma + 1, sevenComma).toInt();
        LEVEL = lastLine.substring(sevenComma + 1).toInt();
        SLEVEL = lastLine.substring(sevenComma + 1).toInt();
        
      } else {
      }
    } else {
    }
  } else {
  }
}
