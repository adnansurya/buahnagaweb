#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define BUZZER_PIN 1  //D1
#define pinIR_1 13    //D7
#define pinIR_2 0     //D8
#define pinIR_3 12    //D6
#define pinIR_4 15    //D10
#define pinIR_5 16     //D0
#define pinIR_6 2     //D9
#define pinRelay 14   //D5

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Week Days
String weekDays[7] = { "Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu" };

//Month names
String months[12] = { "Januari", "Februari", "Maret", "April", "Mei", "Juni", "Juli", "Agustus", "September", "Oktober", "November", "Desember" };

#define FIREBASE_HOST "buahnagaweb-default-rtdb.asia-southeast1.firebasedatabase.app"
#define FIREBASE_AUTH "vuXQqB3gQ3nSy3zBCxJTXmDwuPiPhPrrDijKNS55"
#define WIFI_SSID "Cafe Tulus"
#define WIFI_PASSWORD "PesanDulu"

#define GMT_OFFSET 8

FirebaseData firebaseData;

time_t epochTime;
String weekDay, formattedTime, currentDate;

int deteksi1 = 0;
int deteksi2 = 0;
int deteksi3 = 0;
int deteksi4 = 0;
int deteksi5 = 0;
int deteksi6 = 0;

int last_deteksi1 = 1;
int last_deteksi2 = 1;
int last_deteksi3 = 1;
int last_deteksi4 = 1;
int last_deteksi5 = 1;
int last_deteksi6 = 1;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(pinIR_1, INPUT);
  pinMode(pinIR_2, INPUT);
  pinMode(pinIR_3, INPUT);
  pinMode(pinIR_4, INPUT);
  pinMode(pinIR_5, INPUT);
  pinMode(pinIR_6, INPUT);
  pinMode(pinRelay, OUTPUT);

  beep(1, 0.5);
  Serial.begin(115200);

  konekWifi();
  setWaktu();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  beep(2, 0.5);
}

void loop() {
  timeClient.update();

  epochTime = timeClient.getEpochTime();

  String tStamp = String(epochTime);
  getWaktu();

  deteksi1 = digitalRead(pinIR_1);
  deteksi2 = digitalRead(pinIR_2);
  deteksi3 = digitalRead(pinIR_3);
  deteksi4 = digitalRead(pinIR_4);
  deteksi5 = digitalRead(pinIR_5);
  deteksi6 = digitalRead(pinIR_6);

  if (deteksi1 == 1 && last_deteksi1 == 0) {
    tambahBuah(1);
  }

  if (deteksi2 == 1 && last_deteksi2 == 0) {
    tambahBuah(2);
  }
  if (deteksi3 == 1 && last_deteksi3 == 0) {
    tambahBuah(3);
  }
  if (deteksi4 == 1 && last_deteksi4 == 0) {
    tambahBuah(4);
  }
  if (deteksi5 == 1 && last_deteksi5 == 0) {
    tambahBuah(5);
  }
  if (deteksi6 == 1 && last_deteksi6 == 0) {
    tambahBuah(6);
  }

  last_deteksi1 = deteksi1;
  last_deteksi2 = deteksi2;
  last_deteksi3 = deteksi3;
  last_deteksi4 = deteksi4;
  last_deteksi5 = deteksi5;
  last_deteksi6 = deteksi6;
  
}

void tambahBuah(int nomor) {
  beep(1, 0.2);
  int getBuah = getJumlahBuah(nomor);
  if (getBuah < 0) {
    Serial.println("Error getBuah");
    beep(5, 0.8);
  } else {
    Serial.print("Total Buah ");
    Serial.print(nomor);
    Serial.print(" Saat Ini: ");
    Serial.println(getBuah);
    
    String dirStr = "/counter/sensor" + String(nomor) + "/count";
    int newTotal = getBuah + 1;
    if (Firebase.setInt(firebaseData, dirStr, newTotal)) {
      beep(2, 0.2);
      Serial.println("Data Total Berhasil Diubah");
      addHistori(nomor, newTotal);
      
    } else {
      beep(5, 0.8);
      Serial.print("Error : ");
      Serial.printf(firebaseData.errorReason().c_str());
      Serial.println();
    }

  }
}


int getJumlahBuah(int nomor) {
  String dirStr = "/counter/sensor" + String(nomor) + "/count";
  Serial.println(dirStr);
  int jmlBuah = -2;
  if (Firebase.getString(firebaseData, dirStr)) {

    if (firebaseData.dataType() == "int") {
      jmlBuah = firebaseData.intData();
    } else {
      jmlBuah = -1;
    }
  } else {
    jmlBuah = -1;
  }

  return jmlBuah;
}


void addHistori(int nomor, int total) {
  FirebaseJson json;

  String labelBuah = getLabel(nomor);

  json.set("nomor", nomor);
  json.set("label", labelBuah);
  json.set("total", total);
  json.set("tanggal", currentDate);
  json.set("waktu", formattedTime);

  if (Firebase.pushJSON(firebaseData, "/histori", json)) {

    Serial.println(firebaseData.dataPath());

    Serial.println(firebaseData.pushName());

    Serial.println(firebaseData.dataPath() + "/" + firebaseData.pushName());

  } else {
    Serial.println(firebaseData.errorReason());
  }
}


String getLabel(int nomor){
  String dirStr = "/counter/sensor" + String(nomor) + "/label";
  Serial.println(dirStr);
  String label = "-";
  if (Firebase.getString(firebaseData, dirStr)) {
    Serial.print("TYPE : ");
    Serial.println(firebaseData.dataType());

    if (firebaseData.dataType() == "string") {
      label = firebaseData.stringData();
    } else {
      label = "Error";
    }
  } else {
    label = "Error";
  }

  return label;  
}


void konekWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //memulai menghubungkan ke wifi router
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  //status saat mengkoneksikan
  }
  Serial.println("Sukses terkoneksi wifi!");
  Serial.println("IP Address:");  //alamat ip lokal
  Serial.println(WiFi.localIP());
}

void setWaktu() {

  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0

  timeClient.setTimeOffset(GMT_OFFSET * 3600);
}

void beep(int ulang, float detik) {

  for (int i = 0; i < ulang; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(detik * 1000);
    digitalWrite(BUZZER_PIN, LOW);
    delay(detik * 1000);
  }
}

void getWaktu() {

  formattedTime = timeClient.getFormattedTime();


  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();



  weekDay = weekDays[timeClient.getDay()];

  //Get a time structure
  struct tm *ptm = gmtime((time_t *)&epochTime);

  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon + 1;
  String currentMonthName = months[currentMonth - 1];

  int currentYear = ptm->tm_year + 1900;

  //Print complete date:
  currentDate = String(monthDay) + "-" + String(currentMonth) + "-" + String(currentYear);

  String waktu = weekDay + ", " + currentDate + " " + formattedTime + " WITA";
}
