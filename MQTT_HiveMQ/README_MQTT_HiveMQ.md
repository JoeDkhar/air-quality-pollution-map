

# **MQTT HiveMQ Setup Guide**

*For Air Quality Monitoring with Node-RED and ESP32*

---

## **1. What is MQTT and HiveMQ?**

**MQTT** (Message Queuing Telemetry Transport) is a lightweight protocol used for IoT communication.
**HiveMQ** offers a free **public MQTT broker** you can use for testing without needing your own server.

---

## **2. HiveMQ Public Broker Details**

* **Broker URL:** `broker.hivemq.com`
* **Port:** `1883` (non-secure)
* **Secure Port (TLS):** `8883` (optional, requires certificate)
* **Authentication:** None (public)
* **QoS Levels:** 0, 1, 2 (you can use QoS 0 for faster updates)
* **Restrictions:**

  * Data is **not private** (anyone can subscribe to your topic)
  * No message persistence — data disappears if no one is connected

---

## **3. MQTT Topic Structure for This Project**

For your Air Quality Monitoring system, we will publish each location’s data to:

```
city/airquality/<area>
```

**Examples:**

```
city/airquality/area1   // Delhi
city/airquality/area2   // Mumbai
city/airquality/area3   // Bengaluru
```

---

## **4. Testing MQTT Connection with HiveMQ Web Client**

You can test publishing and subscribing without hardware using the **HiveMQ WebSocket Client**:

1. Go to: [https://www.hivemq.com/demos/websocket-client/](https://www.hivemq.com/demos/websocket-client/)
2. Click **Connect** (leave default broker as `broker.hivemq.com` and port `8000` for WebSocket).
3. In the **Subscribe to topic** box, enter:

   ```
   city/airquality/#
   ```

   The `#` wildcard means you’ll receive data from **all areas**.
4. When you run your ESP32, you should see messages appear in real time.

---

## **5. Setting Up ESP32 to Publish to HiveMQ**

In your ESP32 code:

```cpp
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

snprintf(mqtt_topic, sizeof(mqtt_topic), "city/airquality/%s", NODE_ID);
client.publish(mqtt_topic, payload);
```

Where `NODE_ID` is your location (e.g., `"area1"`).

---

## **6. Setting Up Node-RED to Subscribe to HiveMQ**

### Step 1: Install Required Nodes

* `node-red-dashboard` (for UI widgets)
* `node-red-contrib-web-worldmap` (for map visualization)

### Step 2: Add MQTT Input Node

* **Server:** `broker.hivemq.com`
* **Port:** `1883`
* **Topic:**

  ```
  city/airquality/#
  ```

### Step 3: Connect to JSON Parser & Dashboard Widgets

* Use a **JSON node** to parse incoming payloads.
* Send data to **gauges** for temperature, humidity, CO₂, PM2.5.
* Send data to **World Map node** with coordinates of the city.
* Alternatively, open the left-hand side menu in Node-RED, select Import, and choose the air_quality_monitoring_dashboard_flow.json file. This will instantly preload the entire Air Quality Monitoring dashboard, saving you from manually wiring all the nodes. 
---

## **7. Troubleshooting Tips**

* **No data in Node-RED?**

  * Make sure topic names match exactly in ESP32 and Node-RED.
  * Ensure you’re connected to the same broker (`broker.hivemq.com`).
* **Dashboard not updating?**

  * Confirm the **JSON parser** is working (check debug output).
  * Verify gauge `Value Format` matches the payload structure (e.g., `{{payload.temperature}}`).
* **Multiple devices?**

  * Give each device a unique `NODE_ID` and topic.

---

## **8. Security Note**

The HiveMQ public broker is open. For production use:

* Run a **private broker** (e.g., Mosquitto, EMQX, or private HiveMQ instance).
* Enable **authentication** and **TLS encryption**.

---

