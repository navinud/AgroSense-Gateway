// ===================== AgroSense ESP32 — config.h =====================
// Edit Wi-Fi, MQTT broker, and (if needed) pins to match your wiring.
#pragma once

// ---- Wi-Fi ----
#define WIFI_SSID      "iPhone"
#define WIFI_PASS      "aaaaaaaa"

// ---- MQTT broker (the PC running Node-RED/Mosquitto) ----
#define MQTT_HOST "test.mosquitto.org"   // <-- your broker IP (run `ipconfig`/`hostname -I`)
#define MQTT_PORT      1883
#define MQTT_CLIENT    "agrosense-esp32"

// This ESP32 is also a field node. Its own field id:
#define SELF_FIELD     "field_1"
#define SELF_NAME      "Field A (ESP32)"

// ---- Pins (ESP32 DevKit v1). All analog pins are ADC1 (Wi-Fi safe). ----
#define PIN_DHT        4
#define PIN_SOIL       34   // analog, input-only
#define PIN_RAIN       35   // analog (AO of rain board), input-only
#define PIN_LDR        32   // analog
#define PIN_SERVO      13
#define OLED_SDA       21
#define OLED_SCL       22

// ---- LoRa (SPI) ----
#define LORA_SCK       18
#define LORA_MISO      19
#define LORA_MOSI      23
#define LORA_SS        5
#define LORA_RST       14
#define LORA_DIO0      2
#define LORA_FREQ      433E6   // Ra-02 = 433 MHz. Use 868E6/915E6 per region.

// ---- Behaviour ----
#define MA_N           3      // moving-average window (A3, configurable)
#define PUBLISH_MS     2000   // telemetry period
#define OLED_MS        1000
#define HEARTBEAT_TIMEOUT 15000  // node OFFLINE if silent this long (A13)
#define BUF_SIZE       20     // circular/offline buffer depth (A10/A11)
#define DHTTYPE        DHT11

// ---- Sensor calibration (tune to your probes) ----
#define SOIL_DRY_ADC   4095   // ADC reading in dry air
#define SOIL_WET_ADC   2100   // ADC reading in water
#define RAIN_WET_ADC   1500   // below this = raining
#define LDR_DARK_ADC   200
#define LDR_BRIGHT_ADC 3000

// ---- MQTT topics (must match the Node-RED dashboard) ----
#define T_TELEM_FMT    "smartfarm/%s/telemetry"
#define T_REG_NODES    "smartfarm/registry/nodes"
#define T_REG_UNREG    "smartfarm/registry/unregistered"
#define T_ALERTS       "smartfarm/alerts"
#define T_CMD_VALVE    "smartfarm/cmd/valve"
#define T_CMD_MODE     "smartfarm/cmd/mode"
#define T_CMD_THR      "smartfarm/cmd/threshold"
#define T_CMD_BIND     "smartfarm/cmd/bind"
#define T_CMD_WX       "smartfarm/cmd/weather"
#define T_STATUS_GW    "smartfarm/status/gateway"