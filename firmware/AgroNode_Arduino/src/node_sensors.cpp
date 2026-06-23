// #include "agronode.h"
// /* ===== node_sensors.ino — B1: read soil + temp/humidity, prepare payload ===== */

// void readSensors(){
//   int raw = analogRead(PIN_SOIL);
//   soil = map(constrain(raw, SOIL_WET_ADC, SOIL_DRY_ADC), SOIL_DRY_ADC, SOIL_WET_ADC, 0, 100);
//   float t=dht.readTemperature(), h=dht.readHumidity();
//   if(!isnan(t)) temp=t;
//   if(!isnan(h)) hum=h;
// }

// // payload string used in D packets: moist=..;temp=..;hum=..
// String sensorPayload(){
//   String p="moist="; p+=String((int)soil);
//   p+=";temp="; p+=String(temp,1);
//   p+=";hum=";  p+=String((int)hum);
//   return p;
// }