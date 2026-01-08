# Simple ESP32 code for monitoring environment
----
- Reads temp and RH from SHT31 sensor
- Rolls data into JSON and publishes readings every 30 seconds.
- HIVEMQ recieves data and allows other devices or endpoints access.



#### WIFI CONFIG
**all sensative data is excpected in a secrets.h ie. credentials and wifi access**




-----
Used  on espressif ESP32-wroom and adafruit SHT31 Sensor




