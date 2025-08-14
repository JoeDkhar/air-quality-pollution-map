
---

# **IoT Air Quality Monitoring System — **

## **1. Overview**

This project monitors **temperature, humidity, CO, CO₂, and PM2.5** levels using ESP32-based sensor nodes, publishes readings via **MQTT**, and visualizes them in **Node-RED Dashboard** with:

* **Live updating gauges**
* **Interactive pollution map** showing each city’s location
* **Alert system** via LED if pollution is hazardous

---

## **2. System Architecture**

**Components:**

* **ESP32** microcontroller with DHT22 + simulated CO, CO₂, PM2.5 sensors (Wokwi or real hardware)
* **MQTT Broker** — HiveMQ Public Broker (`broker.hivemq.com`)
* **Node-RED Dashboard** (UI Gauges, UI Map, Charts)
* **Wi-Fi Network** — Wokwi default or your local Wi-Fi

**Data Flow:**

```
Sensors (ESP32) → MQTT publish (topic: city/airquality/<area>)
                → Node-RED MQTT In → Dashboard (gauges, map)
```

---

## **3. MQTT Topic Structure**

Each node publishes to:

```
city/airquality/<node_id>
```

Example:

```
city/airquality/area1
city/airquality/area2
```

Payload format (JSON with coordinates):

```json
{
  "node": "area1",
  "location": "Delhi",
  "lat": 28.6139,
  "lon": 77.2090,
  "temperature": 29.5,
  "humidity": 56.2,
  "co": 35,
  "co2": 800,
  "pm25": 42
}
```

---

## **4. ESP32 Code — Example for Delhi (`area1`)**

```cpp
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define NODE_ID "area1"
#define ALERT_LED 2
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

char mqtt_topic[50];
#define DHTPIN 14
#define DHTTYPE DHT22
#define CO_PIN 34
#define CO2_PIN 35
#define PM25_PIN 32

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  Serial.printf("[%s] Connecting to WiFi", NODE_ID);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi connected!");
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect(NODE_ID)) {
      Serial.println("MQTT connected");
    } else {
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
  if (!client.connected()) reconnect();
  client.loop();

  float temperature = isnan(dht.readTemperature()) ? random(20, 35) : dht.readTemperature();
  float humidity = isnan(dht.readHumidity()) ? random(40, 70) : dht.readHumidity();
  int co_ppm = map(analogRead(CO_PIN), 0, 4095, 0, 100);
  int co2_ppm = map(analogRead(CO2_PIN), 0, 4095, 400, 5000);
  int pm25_ug = map(analogRead(PM25_PIN), 0, 4095, 0, 500);

  bool alert = (co_ppm > 50 || co2_ppm > 2000 || pm25_ug > 150);
  digitalWrite(ALERT_LED, alert ? HIGH : LOW);

  char payload[200];
  snprintf(payload, sizeof(payload),
    "{\"node\":\"%s\",\"location\":\"Delhi\",\"lat\":28.6139,\"lon\":77.2090,"
    "\"temperature\":%.1f,\"humidity\":%.1f,\"co\":%d,\"co2\":%d,\"pm25\":%d}",
    NODE_ID, temperature, humidity, co_ppm, co2_ppm, pm25_ug);

  client.publish(mqtt_topic, payload);
  delay(5000);
}
```

---

## **5. ESP32 Code — Example for Mumbai (`area2`)**

Change:

```cpp
#define NODE_ID "area2"
...
"location\":\"Mumbai\",\"lat\":19.0760,\"lon\":72.8777,"
```

---

## **6. Node-RED Flow Setup**

1. **Install** Node-RED Dashboard:

   ```bash
   npm install node-red-dashboard
   ```

2. **MQTT In Node**

   * Server: `broker.hivemq.com`
   * Port: `1883`
   * Topic: `city/airquality/#`

3. **JSON Node**

   * Converts payload to JSON

4. **UI Gauges**

   * Temperature (°C), Humidity (%), CO (ppm), CO₂ (ppm), PM2.5 (µg/m³)
   * Bind each gauge to its respective property (`msg.payload.temperature`, etc.)

5. **UI Map Node**

   * Add marker: use `msg.payload.lat` and `msg.payload.lon`
   * Popup text: `{{payload.location}} - Temp: {{payload.temperature}}°C`

6. **UI Chart**

   * Show trends over time for temperature, humidity, pollutants

---

## **7. Dashboard Example Layout**

* **Tab:** "Air Quality Monitor"
* **Group 1:** Air Quality Data

  * Gauges: Temperature, Humidity, CO, CO₂, PM2.5
* **Group 2:** Pollution Map

  * UI Map showing markers for each city
* **Group 3:** Historical Trends

  * Charts for pollutants over time

---

## **8. Alerts**

* LED on ESP32 turns ON if:

  * CO > 50 ppm
  * CO₂ > 2000 ppm
  * PM2.5 > 150 µg/m³

---

## **9. Testing**

* **Wokwi Simulation:** Works with virtual DHT22 + analog readings
* **Real Hardware:** Replace simulated pins with actual sensors

---

## **10. Possible Extensions**

* Add **GPS module** for dynamic coordinates
* Store readings in **InfluxDB** for long-term trends
* Add **email/SMS alerts** when levels exceed thresholds

---

