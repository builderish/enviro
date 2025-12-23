#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_SHT31.h>
#include "secrets.h"  // <-- contains WIFI_SSID, WIFI_PASS, MQTT_USER, MQTT_PASS

#define DEVICE_ID "esp32-001"

// HiveMQ Cloud broker config
const int   MQTT_PORT   = 8883;  // TLS port

// MQTT topics
const char* TOPIC_TELEMETRY = "devices/esp32-001/telemetry";
const char* TOPIC_STATUS    = "devices/esp32-001/status";

// Timing
const unsigned long PUBLISH_INTERVAL = 30000; // 30 seconds
unsigned long lastPublish = 0;

// Objects
WiFiClientSecure espClient;
PubSubClient mqtt(espClient);
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// ==========================
// Helper: publish JSON
// ==========================
void publishJson(const char* topic, JsonDocument& doc) {
  char buffer[256];
  size_t len = serializeJson(doc, buffer);
  mqtt.publish(topic, buffer, len);
  Serial.println(buffer); // mirror MQTT payload to serial
}

// ==========================
// Connect Wi-Fi
// ==========================
void connectWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");

  StaticJsonDocument<128> doc;
  doc["deviceId"] = DEVICE_ID;
  doc["ts"] = millis();
  doc["status"] = "wifi_connected";
  publishJson(TOPIC_STATUS, doc);
}

// ==========================
// Connect MQTT
// ==========================
void connectMQTT() {
  mqtt.setServer(MQTT_BROKER, MQTT_PORT);
  espClient.setInsecure(); // Accept self-signed certificate (HiveMQ Cloud)

  while (!mqtt.connected()) {
    Serial.print("Connecting to MQTT Cloud... ");
    if (mqtt.connect(DEVICE_ID, MQTT_USER, MQTT_PASS)) {
      Serial.println("connected");

      StaticJsonDocument<128> doc;
      doc["deviceId"] = DEVICE_ID;
      doc["ts"] = millis();
      doc["status"] = "mqtt_connected";
      publishJson(TOPIC_STATUS, doc);

    } else {
      Serial.print("failed, rc=");
      Serial.println(mqtt.state());
      delay(3000);
    }
  }
}

// ==========================
// SETUP
// ==========================
void setup() {
  Serial.begin(115200);
  delay(1000);

  if (!sht31.begin(0x44)) {
    Serial.println("SHT31 not found!");
    while (1) delay(1);
  }

  connectWiFi();
  connectMQTT();
}

// ==========================
// LOOP
// ==========================
void loop() {
  if (!mqtt.connected()) {
    connectMQTT();
  }

  mqtt.loop();

  unsigned long now = millis();
  if (now - lastPublish >= PUBLISH_INTERVAL) {
    lastPublish = now;

    float temp = sht31.readTemperature();
    float humidity = sht31.readHumidity();

    if (!isnan(temp) && !isnan(humidity)) {
      StaticJsonDocument<256> doc;
      doc["deviceId"] = DEVICE_ID;
      doc["ts"] = millis();
      doc["temp"] = temp;
      doc["humidity"] = humidity;

      publishJson(TOPIC_TELEMETRY, doc);
    } else {
      Serial.println("Sensor read failed");
    }
  }
}
