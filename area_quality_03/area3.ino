#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// ---------- CONFIGURATION ----------
#define NODE_ID "area3"         // Unique ID for MQTT topic
#define LOCATION_NAME "Bangalore"
#define LATITUDE 12.9716
#define LONGITUDE 77.5946
#define ALERT_LED 2             // GPIO for alert LED

// WiFi
const char* ssid = "Wokwi-GUEST";
const char* password = "";

// MQTT Broker
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;
char mqtt_topic[50];

// Sensor pins
#define DHTPIN 14
#define DHTTYPE DHT22
#define CO_PIN 34
#define CO2_PIN 35
#define PM25_PIN 32

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

// ---------- FUNCTIONS ----------
void setup_wifi() {
  Serial.printf("[%s] Connecting to WiFi", NODE_ID);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.printf("\n[%s] WiFi connected!\n", NODE_ID);
}

void reconnect() {
  while (!client.connected()) {
    Serial.printf("[%s] Attempting MQTT connection...\n", NODE_ID);
    if (client.connect(NODE_ID)) {
      Serial.printf("[%s] Connected to MQTT broker!\n", NODE_ID);
    } else {
      Serial.printf("[%s] Failed, rc=%d. Retrying...\n", NODE_ID, client.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(ALERT_LED, OUTPUT);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  snprintf(mqtt_topic, sizeof(mqtt_topic), "city/airquality/%s", NODE_ID);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Simulated "good" readings
  float temperature = 25.5; // °C
  float humidity = 55.0;    // %
  int co_ppm = 20;          // ppm
  int co2_ppm = 800;        // ppm
  int pm25_ug = 30;         // µg/m³

  // ALERT logic - should remain false
  bool alert = (co_ppm > 50 || co2_ppm > 2000 || pm25_ug > 150);
  digitalWrite(ALERT_LED, alert ? HIGH : LOW);

  // Serial output
  Serial.printf("[%s] %s | Temp: %.1f°C  Hum: %.1f%%  CO: %dppm  CO2: %dppm  PM2.5: %dµg/m³ %s\n",
                NODE_ID, LOCATION_NAME, temperature, humidity, co_ppm, co2_ppm, pm25_ug,
                alert ? "!! ALERT !!" : "(Good)");

  // JSON payload
  char payload[256];
  snprintf(payload, sizeof(payload),
    "{\"node\":\"%s\",\"location\":\"%s\",\"lat\":%.4f,\"lon\":%.4f,"
    "\"temperature\":%.1f,\"humidity\":%.1f,\"co\":%d,\"co2\":%d,\"pm25\":%d}",
    NODE_ID, LOCATION_NAME, LATITUDE, LONGITUDE,
    temperature, humidity, co_ppm, co2_ppm, pm25_ug);

  // Publish to MQTT
  client.publish(mqtt_topic, payload);

  delay(5000); // Publish every 5 seconds
}
