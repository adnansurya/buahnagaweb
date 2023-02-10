#include "HX711.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

//bagian coding ambil data
bool ambilDataWarna = true;  //ubah ke true jika alat sedang digunakan untuk mengambil data warna
int loopData = 30;
int count = 0;

//definisikan pin sensor warna
#define S0 13
#define S1 12
#define S2 8
#define S3 4
#define sensorOut 9

//definisikan pin servo
#define pinServo1 11
#define pinServo2 10
#define pinServo3 5
#define pinServo4 2
#define pinServo5 6
#define pinServo6 3
#define pinServo7 7

// definisikan pin loadcell
const int LOADCELL_DOUT_PIN = A1;
const int LOADCELL_SCK_PIN = A0;


//variabel pembanding batas berat dan ringan (gram)
#define batasBerat 300

#define jedaTahan 12

LiquidCrystal_I2C lcd(0x27, 16, 2);                            //deklarasi objek lcd
HX711 scale;                                                   //deklaasi objek loadcell
Servo servo1, servo2, servo7, servo4, servo6, servo3, servo5;  //deklarasi objek servo

void setup() {
  Serial.begin(9600);
  Serial.println("MULAI");

  //inisalisasi LCD
  lcd.begin();
  lcd.backlight();

  tampilkanTeks("Pastikan Tidak", "Ada Beban");
  delay(2000);

  //inisialisasi sensor berat
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(210550.0);                        // nilai pembanding setelah kalibrasi
  scale.tare();
  delay(100);

  //inisialisasi Sensor Warna
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(sensorOut, INPUT);

  digitalWrite(S0, HIGH);
  digitalWrite(S1, LOW);

  //inisialisasi servo
  servo1.attach(pinServo1);
  servo2.attach(pinServo2);
  servo7.attach(pinServo7);
  servo4.attach(pinServo4);
  servo6.attach(pinServo6);
  servo3.attach(pinServo3);
  servo5.attach(pinServo5);
  servo1.write(0);
  delay(300);
  servo2.write(0);
  delay(300);
  servo3.write(0);
  delay(300);
  servo4.write(0);
  delay(300);
  servo5.write(0);
  delay(300);
  servo6.write(0);
  delay(300);
  servo7.write(0);

  delay(2000);
  servo1.write(90);
  delay(300);
  servo2.write(90);
  delay(300);
  servo3.write(90);
  delay(300);
  servo4.write(90);
  delay(300);
  servo5.write(90);
  delay(300);
  servo6.write(90);
  delay(300);
  servo7.write(90);
  delay(300);
  Serial.println("SELESAI");
}

String kategori = "";

void loop() {


  //membaca data serial monitor (untuk keperluan testing)
  if (Serial.available() > 0) {

    kategori = Serial.readStringUntil('\n');
    kategori.trim();
  }

  float beratBuah = ukurBerat();                                         //mengukur berat buah dengan loadcell
  Serial.print("Berat : ");
  Serial.print(beratBuah, 3);
  Serial.println(" gram");

  if (beratBuah > 0) {                                                    //mendeteksi jika ada buah pada loadcell

    if (count == 0 && ambilDataWarna) {
      delay(3000);
    }
    String kategori = getKategori(beratBuah);                            //mendapatkan kategori dengan parameter input berat buah

    tampilkanTeks("Berat:" + String(beratBuah) + " g", kategori);      //tampilkan berat dan kategori buah pada lcd
    delay(2000);
    if (kategori != "" && !ambilDataWarna) {                         //jika kategori sudah didapatkan (tidak kosong)
      dorongBuah(kategori);                                          //maka buah akan didorong ke konveyor dan diarahkan sesuai kategori
    }
  } else {
    tampilkanTeks("Letakkan", "Buah");                                //tampilkan instruksi jika tidak ada buah yang diletakkan pada loadcell
    count = 0;
  }
  delay(200);
}



String getKategori(float inputBerat) {
  //fungsi ini menentukan kategori buah dengan parameter input berat buah

  //deklarasikan variabel nilai frekuensi untuk masing2 intensitas warna dasar (RGB)
  int frequencyr = 0;
  int frequencyg = 0;
  int frequencyb = 0;

  //deklarasikan variabel teks kategori buah
  String kategori1 = "";
  String kategori2 = "";
  String kategoriBuah = "";


  if (inputBerat >= batasBerat) {  //jika berat buah lebih besar dari variabel batas
    kategori1 = "Berat";           //maka dimasukkan kedalam kategori berat
  } else {
    kategori1 = "Ringan";  //jika tidak, maka termasuk kategori ringan
  }

  //baca frekuensi warna merah
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  frequencyr = pulseIn(sensorOut, LOW);
  Serial.print("R = ");
  Serial.print(frequencyr);

  delay(10);

  //baca frekuensi warna hijau
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  frequencyg = pulseIn(sensorOut, LOW);
  Serial.print(" G = ");
  Serial.print(frequencyg);

  delay(10);

  //baca frekuensi warna biru
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  frequencyb = pulseIn(sensorOut, LOW);
  Serial.print(" B = ");
  Serial.println(frequencyb);

  delay(10);



  if (ambilDataWarna) {  //jika variabel ambil data dalam kondisi true, maka mode ambil data aktif
    Serial.println("AmbilData");
    count++;
  }
  //menentukan kategori buah berdasarkan batas dari masing-masing warna
  if (frequencyr < frequencyg && frequencyg < frequencyb && frequencyr <= 400 && frequencyg <= 400 && frequencyb <= 400) {

    kategori2 = "Mentah";

  } else if (frequencyr < frequencyb && frequencyb < frequencyg && frequencyr <= 200 && frequencyg <= 400 && frequencyb <= 400) {
    kategori2 = "Masak";

  } else if (frequencyr < frequencyb && frequencyb < frequencyg && frequencyr > 200 && frequencyr <= 400 && frequencyg <= 400 && frequencyb <= 400) {
    kategori2 = "Busuk";

  } else {
    //jika buah tidak termasuk ke dalam salah satu kategori warna, maka ditampilkan tulisan error
    kategori2 = "Error";
  }

  kategoriBuah = kategori1 + " " + kategori2;

  Serial.println("Kategori Buah : " + kategoriBuah);
  return kategoriBuah;
}

//fungsi untuk mengukur berat
float ukurBerat() {

  float berat = scale.get_units();      //berat pada 0.1 gram ke bawah tidak di anggap

  if (berat <= 0.1) {
    berat = 0.0;
  }
  berat = berat * 1000.0;

  return berat;
}

//fungsi menampilkan teks pada lcd, dengan memasukkan teks pada baris pertama dan kedua
void tampilkanTeks(String baris1, String baris2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(baris1);
  lcd.setCursor(0, 1);
  lcd.print(baris2);
}

//fungsi mendorong buah ke arah konveyor, dan menggerakkan servo untuk berdasarkan kategori buah untuk diarahkan ke dalam keranjang yang sesuai
void dorongBuah(String kategori) {
  Serial.println("dorong buah");

  //menggerakkan servo sesuai kategori
  if (kategori == "Ringan Busuk") {
    servo7.write(10);
  } else if (kategori == "Ringan Masak") {
    servo6.write(10);
  } else if (kategori == "Ringan Mentah") {
    servo5.write(10);
  } else if (kategori == "Berat Busuk") {
    servo4.write(10);
  } else if (kategori == "Berat Masak") {
    servo3.write(10);
  } else if (kategori == "Berat Mentah") {
    servo2.write(10);
  }



  delay(1000);

  //servo mendorong buah masuk ke konveyor
  servo1.write(30);
  delay(2000);
//servo kembali ke kondisi semula
  servo1.write(90);
  kategori = "";
  delay(jedaTahan * 1000);
  servo1.write(90);
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
  servo5.write(90);
  servo6.write(90);
  servo7.write(90);

  // Serial.flush();
}
