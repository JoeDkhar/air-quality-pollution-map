#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

// ---------- CONFIGURATION ----------
#define NODE_ID "area1"       // Unique ID for MQTT topic: area1, area2, etc.
#define LOCATION_NAME "Delhi"  // Human-friendly name for dashboard/map
#define LATITUDE 28.6139     // Change to your node's latitude
#define LONGITUDE 77.2090   // Change to your node's longitude
#define ALERT_LED 2           // GPIO for alert LED

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

  // Simulated readings for Wokwi
  float temperature = isnan(dht.readTemperature()) ? random(200, 350) / 10.0 : dht.readTemperature();
  float humidity = isnan(dht.readHumidity()) ? random(400, 700) / 10.0 : dht.readHumidity();
  int co_ppm = map(analogRead(CO_PIN), 0, 4095, 0, 100);
  int co2_ppm = map(analogRead(CO2_PIN), 0, 4095, 400, 5000);
  int pm25_ug = map(analogRead(PM25_PIN), 0, 4095, 0, 500);

  // ALERT logic
  bool alert = (co_ppm > 50 || co2_ppm > 2000 || pm25_ug > 150);
  digitalWrite(ALERT_LED, alert ? HIGH : LOW);

  // Serial output
  Serial.printf("[%s] %s | Temp: %.1f°C  Hum: %.1f%%  CO: %dppm  CO2: %dppm  PM2.5: %dµg/m³ %s\n",
                NODE_ID, LOCATION_NAME, temperature, humidity, co_ppm, co2_ppm, pm25_ug,
                alert ? "!! ALERT !!" : "");

  // JSON payload (matches Node-RED map + gauges)
  char payload[256];
  snprintf(payload, sizeof(payload),
  "{\"node\":\"%s\",\"location\":\"Delhi\",\"lat\":28.6139,\"lon\":77.2090,"
  "\"temperature\":%.1f,\"humidity\":%.1f,\"co\":%d,\"co2\":%d,\"pm25\":%d}",
  NODE_ID, temperature, humidity, co_ppm, co2_ppm, pm25_ug);


  // Publish to MQTT
  client.publish(mqtt_topic, payload);

  delay(5000); // Publish every 5 seconds
}
