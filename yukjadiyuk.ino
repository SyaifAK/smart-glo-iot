#include "DHT.h"
#define DHTPIN 5
#define DHTTYPE DHT11 // pin dht 11

#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

#include <Adafruit_NeoPixel.h>

#define PIN_PIR       D2   // Pin GPIO yang terhubung dengan sensor PIR
#define PIN_NEOPIXEL  D4   // Pin GPIO yang terhubung dengan data lampu Neopixel Ring
#define PIN_BUZZER    D5   // Pin GPIO yang terhubung dengan sensor buzzer
#define NUM_PIXELS    12   // Jumlah pixel pada lampu Neopixel Ring


DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel pixels(NUM_PIXELS, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "abc"
#define WIFI_PASSWORD "12345678"

// Insert Firebase project API Key
#define API_KEY "AIzaSyAQSCsQJHfCiQnUcokkrTImhYQGCCrYthQ"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://yeeejadi-default-rtdb.firebaseio.com/" 

//Define Firebase Data object
FirebaseData fbdo;


FirebaseAuth auth;
FirebaseConfig config;

//unsigned long sendDataPrevMillis = 0;
//int count = 0;
bool signupOK = false;

// Fungsi untuk mengatur warna pada semua pixel lampu Neopixel Ring
void setColor(uint8_t red, uint8_t green, uint8_t blue) {
  for (int i = 0; i < NUM_PIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(red, green, blue));
  }
  pixels.show();
}

void setup() {
  pinMode(DHTPIN, INPUT);
  dht.begin();
  Serial.begin(9600);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
  
  pinMode(PIN_PIR, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pixels.begin();  // Inisialisasi lampu Neopixel Ring


}


void loop() {

// Membaca status sensor PIR
  int pirStatus = digitalRead(PIN_PIR);
  // Jika sensor PIR mendeteksi gerakan
  if (pirStatus == HIGH) {
    // Mengatur lampu Neopixel Ring menjadi warna putih
    setColor(255, 255, 255);
  // Menyalakan sensor buzzer
    tone(PIN_BUZZER, 1000, 200); // Frekuensi 1000Hz, durasi 200ms
  } else {
    // Mematikan lampu Neopixel Ring
    setColor(0, 0, 0);
     // Mematikan sensor buzzer
    noTone(PIN_BUZZER);
  }

 delay(1000);
 
  float h = dht.readHumidity();

  float t = dht.readTemperature();
  
  if (Firebase.ready() && signupOK ) {
     
     if (Firebase.RTDB.getString(&fbdo, "DHT/L1")) {
      if (fbdo.stringData() == "0") {
          // Mengatur lampu Neopixel Ring menjadi warna-warni
          // Mengubah semua pixel menjadi warna merah
        setColor(255, 0, 0);
        delay(1000);

          // Mengubah semua pixel menjadi warna hijau
        setColor(0, 255, 0);
        delay(1000);

          // Mengubah semua pixel menjadi warna biru
        setColor(0, 0, 255);
        delay(1000);

          // Mengubah semua pixel menjadi warna ungu
        setColor(197, 42, 215);
        delay(1000);
          
       
      } else if(fbdo.stringData() == "1") {
          // Mematikan lampu Neopixel Ring
          setColor(0, 0, 0);
      }
    }

    if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
      // Kirim data kelembaban dan suhu ke Firebase
    Firebase.RTDB.setFloat(&fbdo, "DHT/kelembapan", h);
    Firebase.RTDB.setFloat(&fbdo, "DHT/suhu", t);

    Serial.print("Kelembapan: ");
    Serial.println(h);
    Serial.print("Suhu: ");
    Serial.println(t);

    delay(5000); // Kirim data setiap 5 detik
  
  }
    
  Serial.println("______________________________");


  

}


