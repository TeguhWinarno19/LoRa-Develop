//----------------------------------------------Setting library---------------------------------------------------//
#include <Ethernet.h>
#include "secrets.h"
#include "ThingSpeak.h"
#include <SoftwareSerial.h>
#include <Wire.h>
#include <RTClib.h>
#include <ArduinoHttpClient.h>

//----------------------------------------------RTC Setting-------------------------------------------------------//
RTC_DS3231 rtc;

//----------------------------------------------Setting HMI-------------------------------------------------------//
SoftwareSerial HMI (19,18);

//----------------------------------------------Setting Ethenet Module--------------------------------------------//
byte mac[] = SECRET_MAC;
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);
EthernetClient client;
char serverName[] = "api.callmebot.com";
HttpClient http(client, serverName, 80);

//----------------------------------------------Setting Write Thingspeak Variable---------------------------------//
unsigned long myChannelNumber = 1715266;
const char * myWriteAPIKey = "XB60J69TNLU5IT4U";

unsigned long myChannelNumber1 = 1798532;
const char * myWriteAPIKey1 = "UW71KDAAZ7PJFF2N";

//----------------------------------------------Setting Read Thingspeak Variabbe----------------------------------//

// Channel Value
unsigned long ValChannelNumber = 1715266;
unsigned int ValueFieldNumber = 1;
unsigned int HarianFieldNumber = 2;

// Channel Limits
unsigned long LimitsChannelNumber = 1715269;
unsigned int LimitsFieldNumber = 1; 
int statusCode = 0;

//----------------------------------------------Setting Value Nilai Sensor----------------------------------------//
int HarianMeter;
int ValueMeter;
int LastValueMeter;
int Limits;
int multiple;
int C1 = 0;

//----------------------------------------------Setting Timer Variable--------------------------------------------//
unsigned long previousMillis = 0;
const long interval = 60000;
unsigned long previousMillis1 = 0;
const long interval1 = 963;
int detik;
int menit;
int jam;
int Locking1 = 0;
long lastMsg=0;

//----------------------------------------------Setting Variable Output--------------------------------------------//
int Valve = 33;

//----------------------------------------------Setting input Variable--------------------------------------------//
int Pulse = 31;

//----------------------------------------------Setting Variable Bantuan------------------------------------------//
int ValveLock;
int TRIG;
int LimitsSementara;

//---------------------------------------------Setting TTGO DATA----------------------------
long LINIA;
long LINIB;
long SDM;
long MAKO;
long TASGANU;
long LOGAM;
long SICPA;
long LEVEL;
long SLINIA;
long SLINIB;
long SSDM;
long SMAKO;
long STASGANU;
long SLOGAM;
long SSICPA;
long SLEVEL;

String phoneNumber = "6289637890832";  // Ganti dengan nomor telepon tujuan
String apiKey = "9156515";             // Ganti dengan API key kamu
String message = "Be carefull! Bowl is low!";     // Pesan yang akan dikirim


void setup() {
  Wire.begin();
  rtc.begin();
  
  //Mulai komunikasi dengan HMI
  HMI.begin(9600);

  //Mulai Komunikasi dengan TTGO LORA32 ESP32
  Serial3.begin(9600);

  // inisiasi ethernet
  Ethernet.init(10); 
  // inisiasi komunikasi serial
  Serial.begin(9600);  
  //seting pinMode Output dan input
  pinMode(Valve,OUTPUT);
  pinMode(Pulse,INPUT_PULLUP);
  //inisialisasi nilai awal output
  digitalWrite(Valve,LOW);
  //ambil data waktu dari Rtc
  Serial.println("Mengambil data waktu");
  DateTime now = rtc.now();
  jam = now.hour(),DEC;
  menit = now.hour(),DEC;
  detik = now.hour(),DEC;
  Serial.print(jam);
  Serial.print(":");
  Serial.print(menit);
  Serial.print(":");
  Serial.print(detik);
  Serial.println("Selesai");
 
  
  Serial.println("Jam : " + String(jam) + "/ Menit : " + String(menit));
  ValveLock = LOW;

  //persiapan starting ethernet
  while (!Serial) {
    ; // tunggu untuk serial port dihubungkan.
  }  
  Serial.println("Preparation");
  startEthernet();
  ThingSpeak.begin(client);  // Periapan thingspeak
  delay(5000);
  //jalankan fungsi baca data di thingspeak
  BacaDataCloud();
  LastValueMeter = HarianMeter;
}
void loop(){
  //jika data berhasil di peroleh jalankan program utama
  if (C1 == 3){
    runProgram();
    ReceiveTTGO();
  }
  //jika data gagal diperoleh jalankan ulangi proses ambil data
  else {
    BacaDataCloud();  
    Serial.println("data tidak Valid! Proses di ulang");
  }
}

void ReceiveTTGO(){
  if (Serial3.available() > 0) {
    delay(10);
    String data = Serial3.readStringUntil('\n');
    sscanf(data.c_str(), "%ld,%ld,%ld,%ld,%ld,%ld,%ld,%ld", &SSDM, &SMAKO, &SLINIA, &SLINIB, &STASGANU, &SLOGAM, &SSICPA, &SLEVEL);
    
    Serial.print("SDM : ");
    Serial.println(SSDM);
    Serial.print("MAKO : ");
    Serial.println(SMAKO);    
    Serial.print("LINIA : ");
    Serial.println(SLINIA);    
    Serial.print("LINIB : ");
    Serial.println(SLINIB);    
    Serial.print("TASGANU : ");
    Serial.println(STASGANU);    
    Serial.print("LOGAM : ");
    Serial.println(SLOGAM);    
    Serial.print("SICPA : ");
    Serial.println(SSICPA);    
    Serial.print("LEVEL : ");
    Serial.println(SLEVEL);
  }

}

void HMI_Update(){
  HMI.print("detik.val=");HMI.print(detik);
  HMI.write(0xff);HMI.write(0xff);HMI.write(0xff);
  HMI.print("menit.val=");HMI.print(menit);
  HMI.write(0xff);HMI.write(0xff);HMI.write(0xff);
  HMI.print("jam.val=");HMI.print(jam);
  HMI.write(0xff);HMI.write(0xff);HMI.write(0xff);
  HMI.print("dailyusage.val=");HMI.print(HarianMeter);
  HMI.write(0xff);HMI.write(0xff);HMI.write(0xff);
  HMI.print("wmvalue.val=");HMI.print(ValueMeter);
  HMI.write(0xff);HMI.write(0xff);HMI.write(0xff);
  HMI.print("limit.val=");HMI.print(Limits);
  HMI.write(0xff);HMI.write(0xff);HMI.write(0xff);
  HMI.print("multiple.val=");HMI.print(multiple);
  HMI.write(0xff);HMI.write(0xff);HMI.write(0xff);
}

void runProgram() {
  //pewaktu
  unsigned long currentMillis1 = millis();
  if (currentMillis1 - previousMillis1 >= interval1) {
    DateTime now = rtc.now();
    jam = now.hour(),DEC;
    menit = now.minute(),DEC;
    detik = now.second(),DEC;    
    HMI_Update();    
    previousMillis1 = currentMillis1;
    //tampilkan data ke serial monitor
    Serial.println("Jam: " + String(jam) + "/Menit: " + String(menit) + "/Detik: " + String(detik));
    Serial.println("Trig : " + String(TRIG) + "Pulsa : " + String(digitalRead(Pulse))); 
    Serial.println("Value : " + String(ValueMeter) + "Harian : " + String(HarianMeter) + "multiple : " + String(multiple));
    }

  //jika pemakaian melebihi sama dengan limit
  
  if (HarianMeter >= Limits && ValveLock == LOW){
    digitalWrite(Valve,HIGH);
    ValveLock = HIGH;
    Serial.println("OK Gan");}
    
  //jika pemakaian lebih kecil dari limit
  if (HarianMeter < Limits && ValveLock == HIGH){
    digitalWrite(Valve,LOW);
    ValveLock = LOW;
    Serial.println("GAGAL GAN");}

    
  //saatv detik = 30 lakukan update data limit
  if (detik == 1){
    if (menit == 10 or menit == 30 or menit == 50){
      LimitsSementara = ThingSpeak.readLongField(LimitsChannelNumber, LimitsFieldNumber);
      statusCode = ThingSpeak.getLastReadStatus();
      //jika saat pengambilan tidak ada masalah maka update data limit
      if(statusCode == 200){
        Serial.println("Limits: " + String(Limits));
        Limits = LimitsSementara;}
        //jika bermasalah maka pertahankan nilai lama
        else{
          Limits = 100;
          Serial.println("Problem reading channel. HTTP error code " + String(statusCode));}}
          }
    
//  //        jika ada pemakaian maka lakukan update data harian dan wm value
//  if (LastValueMeter != HarianMeter){
//    updateData();
//    LastValueMeter = HarianMeter;}
  
  if (jam == 16 && menit == 5 && Locking1 == 0 ){
    //penyimpanan di nol kan
    HarianMeter = 0;
    updateData();
    Locking1 = 1;}
    
  if (jam == 7 && menit == 30 && Locking1 == 1){
    Locking1 = 0;}
    
  //jika pulse terbaca maka aktifkan safe increment variable 
  int Pulsa = digitalRead(Pulse);
  if (Pulsa == LOW && TRIG == LOW){
    TRIG = HIGH;
    delay(1000);}
  
  //jika sinyal sudah off maka increment nilai data 0,1 (metode flip on falling)
  if (Pulsa == HIGH && TRIG == HIGH){ 
    TRIG = LOW; 
    delay(1000);
    multiple++;}

  // saaat multiple = 10 maka lakukan increment data ke wm value & pemakaian harian
  if (multiple == 10){
    ValueMeter ++;
    HarianMeter ++;
    multiple = 0;
    updateData();
    }
  
  if (SSDM != SDM or SMAKO != MAKO or SLINIA != LINIA or SLINIB != LINIB or STASGANU != TASGANU or SLOGAM != LOGAM or SSICPA != SICPA or SLEVEL != LEVEL){
    SDM = SSDM;
    MAKO = SMAKO;
    LINIA = SLINIA;
    LINIB = SLINIB;
    TASGANU = STASGANU;
    LOGAM = SLOGAM;
    SICPA = SSICPA;
    LEVEL = SLEVEL;
    if (SSDM > 100){
      updateData1();
    }
  }
  // Periksa LEVEL dan kirim pesan WhatsApp jika LEVEL adalah 0 setiap 1 menit sekali
  if (detik == 37){
    if (LEVEL == 0) {
      sendWhatsAppMessage();
      }
    }
}

//-----------------------------fungsi update data ke cloud1------------------------------//
void updateData1(){
  ThingSpeak.setField(1, SDM);
  ThingSpeak.setField(2, MAKO);
  ThingSpeak.setField(3, LINIA);
  ThingSpeak.setField(4, LINIB);
  ThingSpeak.setField(5, TASGANU);
  ThingSpeak.setField(6, LOGAM);
  ThingSpeak.setField(7, SICPA);
  ThingSpeak.setField(8, LEVEL);
  //update data ke thingspeak
  int x = ThingSpeak.writeFields(myChannelNumber1, myWriteAPIKey1);
  delay(100);
  //jika data berhasil di update
  if(x == 200){
    Serial.println("Channel update successful.");}
  //jika gagal update data.
  else {Serial.println("Problem updating channel. HTTP error code " + String(x) + "Gagal Update");}
  }

//-----------------------------fungsi update data ke cloud------------------------------//
void updateData(){
  //set nilai ke field 1 = value wm, field 2 = pemakaian harian
  ThingSpeak.setField(1, ValueMeter);
  ThingSpeak.setField(2, HarianMeter);
  //update data ke thingspeak
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  delay(100);
  //jika data berhasil di update
  if(x == 200){
    Serial.println("Channel update successful.");}
  //jika gagal update data.
  else {Serial.println("Problem updating channel. HTTP error code " + String(x) + "Gagal Update");}}
  

//----------------------------------------------Start koneksi ethernet-------------------------------------//
void startEthernet(){
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1);}}
        
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");}
      
    // coba konfigurasi menggunakan IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);}
    else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());}
    
  // beri waktu untuk ethernet shiled persiapan
  delay(1000);
}

//-----------------------------Fungsi baca data cloud--------------------------------------//
void BacaDataCloud(){
  //inisialisasi nilai awal
  C1 = 0;
  //ambil data value meterean dari cloud
  HMI.print("detik.val=");HMI.print(1);
  HMI.write(0xff);HMI.write(0xff);HMI.write(0xff);
  delay(5000);
  ValueMeter= ThingSpeak.readFloatField(ValChannelNumber, ValueFieldNumber);  
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Value Meteran : " + String(ValueMeter) + " Kubik ");C1 = C1 + 1;}
  else{Serial.println("Problem reading channel. HTTP error code " + String(statusCode));ValueMeter=9018;}
  delay(10000);
  //ambil data pemakaian harian dari cloud
  HMI.print("detik.val=");HMI.print(2);
  HMI.write(0xff);HMI.write(0xff);HMI.write(0xff);
  delay(1000);
  HarianMeter= ThingSpeak.readFloatField(ValChannelNumber, HarianFieldNumber);  
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Pemakaian Harian : " + String(HarianMeter) + " Kubik ");C1 = C1 + 1;}
  else{Serial.println("Problem reading channel. HTTP error code " + String(statusCode));HarianMeter=9;}
  delay(10000);
  //ambil data limit dari cloud
  HMI.print("detik.val=");HMI.print(3);
  HMI.write(0xff);HMI.write(0xff);HMI.write(0xff);
  delay(5000);
  Limits = ThingSpeak.readLongField(LimitsChannelNumber, LimitsFieldNumber);
  statusCode = ThingSpeak.getLastReadStatus();
  if(statusCode == 200){
    Serial.println("Limits: " + String(Limits));C1 = C1 + 1;}
  else{Serial.println("Problem reading channel. HTTP error code " + String(statusCode)); Limits = 100;}
  delay(20000);
  LastValueMeter = ValueMeter;}

void sendWhatsAppMessage() { 
  // Encode URL
  String encodedMessage = "";
  for (int i = 0; i < message.length(); i++) {
    char c = message[i];
    if (isalnum(c)) {
      encodedMessage += c;
    } else {
      encodedMessage += '%';
      char hex[3];
      sprintf(hex, "%02X", c);
      encodedMessage += hex;
    }
  }

  // Membuat URL lengkap
  String url = "/whatsapp.php?phone=" + phoneNumber + "&text=" + encodedMessage + "&apikey=" + apiKey;

  // Membuat HTTP GET request
  http.get(url);

  // Mendapatkan HTTP response
  int statusCode = http.responseStatusCode();
  String response = http.responseBody();

  Serial.print("Status code: ");
  Serial.println(statusCode);
  Serial.print("Response: ");
  Serial.println(response);
}
