// ===================== AgroNode (Arduino Uno) — config.h =====================
#pragma once

// ---- pins ----
#define PIN_DHT       3
#define PIN_SOIL      A0
#define LORA_SS       10
#define LORA_RST      9
#define LORA_DIO0     2
#define PIN_SERVO     6      // local emergency valve (or an LED)
#define PIN_BTN       4      // re-bind button to GND (INPUT_PULLUP)
#define PIN_LED       5      // status LED
#define LORA_FREQ     433E6  // match the ESP32 gateway
#define DHTTYPE       DHT11

// ---- timing ----
#define DATA_MS       3000   // send sensor data
#define HEARTBEAT_MS  5000   // B6 heartbeat
#define JOIN_MS       2000   // JOIN retry while unbound
#define LINK_TIMEOUT  12000  // no gateway downlink => link considered down (B8)
#define EMERGENCY_MS  20000  // disconnected this long + dry => emergency irrigation (B7)

// ---- calibration ----
#define SOIL_DRY_ADC  620
#define SOIL_WET_ADC  280
#define DRY_TRIGGER   30     // % moisture below which emergency irrigation kicks in

// ---- EEPROM layout ----
#define EE_MAGIC_ADDR 0
#define EE_MAGIC_VAL  0xA6
#define EE_BOUND_ADDR 1
#define EE_UID_ADDR   2      // 6 chars
#define EE_FIELD_ADDR 10     // up to 12 chars

#define BUF_SIZE      8      // offline buffer depth (B8)