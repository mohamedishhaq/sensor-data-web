#include <WiFi.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <Firebase_ESP_Client.h>
#include <time.h>  // For NTP time

// WiFi credentials
const char* ssid = "ISHHAQ";
const char* password = "23456789";

// DHT Sensor setup
#define DHTPIN 27
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Firebase credentials
#define API_KEY "AIzaSyCZAwUYLExU7mmJ1r_9D85rkCJzk82mVGQ"
#define DATABASE_URL "https://temphumdata-1023b-default-rtdb.firebaseio.com/"
#define USER_EMAIL "mimishaq010@gmail.com"
#define USER_PASSWORD "MimI$##@q@010"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

float currentTemperature = 0.0;
float currentHumidity = 0.0;
unsigned long lastSendTime = 0;
const unsigned long interval = 1000; // 1 second

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\n✅ Connected! IP: ");
  Serial.println(WiFi.localIP());

  // Initialize Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Set up NTP time
 configTime(18000, 0, "pool.ntp.org", "time.nist.gov"); // 18000 seconds = 5 hours
  
  Serial.print("⏳ Waiting for time sync");
  while (time(nullptr) < 100000) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\n✅ Time synchronized");
}

void loop() {
  unsigned long now = millis();
  if (now - lastSendTime >= interval) {
    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (!isnan(temp) && !isnan(hum)) {
      currentTemperature = temp;
      currentHumidity = hum;
      Serial.printf("📡 Sending: %.2f°C, %.2f%%\n", temp, hum);
      sendToFirebase(temp, hum);
    } else {
      Serial.println("❌ DHT Read Failed");
    }
    lastSendTime = now;
  }
}

void sendToFirebase(float temperature, float humidity) {
  String path = "/dhtSensor";

  FirebaseJson json;
  json.set("temperature", temperature);
  json.set("humidity", humidity);
  json.set("timestamp", getFormattedTime());  // Use real time from NTP

  if (Firebase.RTDB.pushJSON(&fbdo, path, &json)) {
    Serial.println("✅ Firebase: Data pushed with time.");
  } else {
    Serial.print("❌ Firebase error: ");
    Serial.println(fbdo.errorReason());
  }
}

// Helper function to get current timestamp
String getFormattedTime() {

  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  char buffer[30];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
  return String(buffer);
}
