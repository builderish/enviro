#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include "Adafruit_SHT31.h"

// ================= CONFIG =================

// WiFi
const char *ssid = "TDB";
const char *password = "Tdb8954$";

// MQTT (NON-TLS)
const char *mqtt_server = "broker.hivemq.com"; // public broker
const int mqtt_port = 1883;
const char *mqtt_user = nullptr;
const char *mqtt_pass = nullptr;

// ================= OBJECTS =================

WiFiClient espClient;
PubSubClient mqtt(espClient);
Adafruit_SHT31 sht31 = Adafruit_SHT31();

// ================= FUNCTIONS =================

void connectWiFi()
{
    Serial.print("Connecting WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi connected");
}

void connectMQTT()
{
    while (!mqtt.connected())
    {
        Serial.print("Connecting MQTT...");
        if (mqtt.connect("esp32-enviro"))
        {
            Serial.println("connected");
        }
        else
        {
            Serial.print("failed rc=");
            Serial.println(mqtt.state());
            delay(2000);
        }
    }
}

// ================= SETUP =================

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    connectWiFi();

    mqtt.setServer(mqtt_server, mqtt_port);

    if (!sht31.begin(0x44))
    {
        Serial.println("SHT31 NOT FOUND");
        while (1)
            delay(10);
    }

    Serial.println("SHT31 OK");
}

// ================= LOOP =================

void loop()
{
    if (!mqtt.connected())
        connectMQTT();
    mqtt.loop();

    float t = sht31.readTemperature();
    float h = sht31.readHumidity();

    if (!isnan(t) && !isnan(h))
    {
        char payload[64];
        snprintf(payload, sizeof(payload),
                 "{\"temperature\":%.2f,\"humidity\":%.2f}", t, h);

        mqtt.publish("esp32/sensors", payload);

        Serial.println(payload);
    }
    else
    {
        Serial.println("Sensor read failed");
    }

    delay(2000);
}
