#include "agronode.h"
/* ===== node_sensors.cpp — B1: read soil + temp/humidity, prepare payload ===== */

void readSensors(){
  int raw = analogRead(PIN_SOIL);   // ESP32: 0..4095
  soil = map(constrain(raw, SOIL_WET_ADC, SOIL_DRY_ADC), SOIL_DRY_ADC, SOIL_WET_ADC, 0, 100);
  float t=dht.readTemperature(), h=dht.readHumidity();
  if(!isnan(t)) temp=t;
  if(!isnan(h)) hum=h;
  // LDR (light) reading on ADC1 pin — WiFi-safe
  analogSetPinAttenuation(PIN_LDR, ADC_11db); // full-range 0..3.3V
  int rawL = analogRead(PIN_LDR);
  light = map(constrain(rawL, 0, 4095), 0, 4095, 0, 100);
  // Rain sensor reading (analog threshold -> boolean)
  analogSetPinAttenuation(PIN_RAIN, ADC_11db);
  int rawR = analogRead(PIN_RAIN);
  rain = (rawR < RAIN_THRESHOLD_ADC); // adjust threshold per sensor wiring
}

// payload string used in D packets: moist=..;temp=..;hum=..
String sensorPayload(){
  String p="moist="; p+=String((int)soil);
  p+=";temp="; p+=String(temp,1);
  p+=";hum=";  p+=String((int)hum);
  p+=";light="; p+=String((int)light);
  p+=";rain="; p+=String(rain?1:0);
  return p;
}