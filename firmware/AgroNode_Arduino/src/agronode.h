// ===== agronode.h — shared declarations for the ESP32 sub-node build =====
#pragma once
#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <DHT.h>
#include <EEPROM.h>
#include <ESP32Servo.h>      // <-- ESP32 servo lib (NOT the AVR Servo.h)
#include "config.h"

// ---- shared globals (defined in main.cpp) ----
extern DHT    dht;
extern Servo  valve;
extern String UID, FIELD;
extern bool   bound, valveOpen;
extern float  soil, temp, hum;
extern unsigned long lastGateway;

// ---- node_sensors.cpp ----
void   readSensors();
String sensorPayload();

// ---- node_comms.cpp ----
void   bufPush(const String& payload);
void   bufFlush();
bool   linkUp();
void   loraInit();
void   txPacket(const char* type, const String& payload);
void   sendJoin();
void   sendHeartbeat();
void   sendData();
void   loraReceive();
void   loadIdentity();
void   saveIdentity();
void   checkRebindButton();
String nthField(const String& s, int idx);
String cmdVal(const String& p, const String& key);

// ---- node_logic.cpp ----
void setValve(bool open);
void emergencyCheck();