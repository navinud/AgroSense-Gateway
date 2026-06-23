// ===================== AgroNode (ESP32) — config.h =====================
#pragma once

// ---- pins (plain ESP32 DevKit + SX1278 module) ----
// SX1278 wiring:  SCK->18  MISO->19  MOSI->23  NSS->5  RST->14  DIO0->26  3V3->3V3  GND->GND
// (If using a Heltec WiFi LoRa 32 V2: SS=18, RST=14, DIO0=26 are internal — same here by luck,
//  but on Heltec also set SCK=5, MISO=19, MOSI=27 and DO NOT wire an external module.)
#define LORA_SCK      18
#define LORA_MISO     19
#define LORA_MOSI     23
#define LORA_SS        5
#define LORA_RST      14
#define LORA_DIO0     26

#define PIN_DHT       17     // DHT11 data
#define PIN_SOIL      34     // soil AO -> ADC1 (input-only pin, Wi-Fi safe)
#define PIN_SERVO     25     // local emergency valve (or an LED)
#define PIN_BTN        4     // re-bind button to GND (INPUT_PULLUP) — must be a pin WITH pullup
#define PIN_LED        2     // onboard LED on most DevKits
#define LORA_FREQ     433E6  // MUST match the ESP32 gateway
#define DHTTYPE       DHT11

// ---- timing ----
#define DATA_MS       3000   // send sensor data
#define HEARTBEAT_MS  5000   // B6 heartbeat
#define JOIN_MS       2000   // JOIN retry while unbound
#define LINK_TIMEOUT  12000  // no gateway downlink => link considered down (B8)
#define EMERGENCY_MS  20000  // disconnected this long + dry => emergency irrigation (B7)

// ---- calibration (ESP32 is 12-bit: 0..4095) — RECALIBRATE to your probe ----
#define SOIL_DRY_ADC  4095   // ADC reading in dry air
#define SOIL_WET_ADC  2100   // ADC reading in water
#define DRY_TRIGGER   30     // % moisture below which emergency irrigation kicks in

// ---- EEPROM (ESP32 uses flash-emulated EEPROM: must EEPROM.begin(EEPROM_SIZE)) ----
#define EEPROM_SIZE   64
#define EE_MAGIC_ADDR 0
#define EE_MAGIC_VAL  0xA6
#define EE_BOUND_ADDR 1
#define EE_UID_ADDR   2      // 6 chars
#define EE_FIELD_ADDR 10     // up to 12 chars

#define BUF_SIZE      16     // offline buffer depth (B8) — bigger than Uno, ESP32 has RAM