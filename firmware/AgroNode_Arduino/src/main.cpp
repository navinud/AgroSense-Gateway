#include "agronode.h"
/* =====================================================================
   AgroNode — ESP32 + LoRa Remote Node   (Sections B, E, F)
   Converted from the Arduino Uno node. Same logic; ESP32 differences:
     - Serial @ 115200
     - EEPROM.begin(EEPROM_SIZE) + EEPROM.commit() after writes
     - ESP32Servo instead of AVR Servo
     - 12-bit ADC (0..4095)
     - esp_random() for UID seed (no analogRead(A1) on ESP32)
   Modules (F5):  this file        = orchestration
                  node_sensors.cpp = B1 (read)
                  node_comms.cpp   = B2/B3/B4/B5/B6/B8 (LoRa, join, bind, buffer)
                  node_logic.cpp   = B7 (emergency irrigation)
   ===================================================================== */

DHT    dht(PIN_DHT, DHTTYPE);
Servo  valve;

String UID="";          // unique device id (persisted)
String FIELD="";        // assigned id from gateway ("" = unbound)
bool   bound=false;
float  soil=0, temp=0, hum=0;
bool   valveOpen=false;
unsigned long lastGateway=0;   // last time we heard from gateway (link health, B8)

void setup(){
  Serial.begin(115200);
  delay(200);
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  EEPROM.begin(EEPROM_SIZE);          // <-- ESP32: must init emulated EEPROM
  dht.begin();
  valve.attach(PIN_SERVO); setValve(false);
  loadIdentity();          // UID + (maybe) FIELD from EEPROM
  loraInit();
  Serial.print("Node UID="); Serial.print(UID);
  Serial.print(" bound="); Serial.println(bound?FIELD:"NO");
  if(bound){ Serial.print("[LoRa] node binded as "); Serial.println(FIELD); }
}

unsigned long tData=0, tHb=0, tJoin=0;

void loop(){
  checkRebindButton();     // B5
  loraReceive();           // assign / commands
  readSensors();           // B1

  unsigned long now=millis();
  if(!bound){
    digitalWrite(PIN_LED, (now/250)%2);     // fast blink = JOIN mode
    if(now - tJoin > JOIN_MS){ tJoin=now; sendJoin(); }   // B3
    return;
  }
  digitalWrite(PIN_LED, linkUp()?HIGH:((now/500)%2));     // solid=linked, slow blink=offline

  if(now - tData > DATA_MS){ tData=now; sendData(); }        // B2
  if(now - tHb   > HEARTBEAT_MS){ tHb=now; sendHeartbeat(); } // B6
  emergencyCheck();        // B7
}