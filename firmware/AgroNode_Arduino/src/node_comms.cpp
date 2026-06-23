#include "agronode.h"
/* ===== node_comms.cpp — JOIN/BIND/telemetry workflow =====
   Node packets:
     JOIN:<UID>
     HEARTBEAT:<UID>
     {json telemetry}
   Gateway packets:
     ASSIGN:<UID>:<field>

   ESP32 notes:
     - SPI.begin(SCK,MISO,MOSI,SS) before LoRa.setPins
     - EEPROM.commit() after every EEPROM.write (flash-emulated)
*/

// ---- offline buffer (B8) ----
String obuf[BUF_SIZE]; int oHead=0, oCount=0;
void bufPush(const String& payload){ obuf[oHead]=payload; oHead=(oHead+1)%BUF_SIZE; if(oCount<BUF_SIZE)oCount++; }
void bufFlush(){
  int idx=(oHead-oCount+BUF_SIZE)%BUF_SIZE;
  for(int i=0;i<oCount;i++){ txPacket("D", obuf[idx]); idx=(idx+1)%BUF_SIZE; delay(40); }
  oCount=0;
}

bool linkUp(){ return (millis()-lastGateway) < LINK_TIMEOUT; }

void loraInit(){
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);   // ESP32: bind the VSPI bus first
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if(!LoRa.begin(LORA_FREQ)){
    while(1){ digitalWrite(PIN_LED,(millis()/100)%2); }
  }

  LoRa.setSyncWord(LORA_SYNC_WORD);
  LoRa.setSignalBandwidth(LORA_BANDWIDTH);
  LoRa.setSpreadingFactor(LORA_SPREADING);
  LoRa.setCodingRate4(LORA_CODING_RATE);
  LoRa.receive();
}

void txRaw(const String& payload){
  LoRa.beginPacket();
  LoRa.print(payload);
  LoRa.endPacket();
  LoRa.receive();
}

// void txPacket(const char* type, const String& payload){
//   String pkt = String(type) + ":" + payload;
//   txRaw(pkt);
// }
void txPacket(const char* type, const String& payload){
  String pkt="AG7X9K,"; pkt+=type; pkt+=","; pkt+=UID; pkt+=","; pkt+=FIELD; pkt+=","; pkt+=payload;
  Serial.print("[LoRa] TX raw: "); Serial.println(pkt);
  LoRa.beginPacket(); LoRa.print(pkt); LoRa.endPacket();
  LoRa.receive();
}

// void sendJoin(){ txPacket("J", "hello"); }

void sendJoin(){
  txPacket("J", "hello");
}

// void sendJoin(){
//   String payload = UID;
//   Serial.print("[LoRa] Sending JOIN UID="); Serial.println(UID);
//   txPacket("JOIN", payload);
// }

void sendHeartbeat(){
  String payload = UID;
  txPacket("H", payload);
}


void sendData(){
  String p = sensorPayload();   // builds "moist=..;temp=..;hum=.."
  if(true){
    Serial.print("[LoRa] data sending  "); Serial.println(p);
    txPacket("D", p);           // -> AG,D,DBB846,field_2,moist=..;temp=..;hum=..
  } else {
    bufPush(p);
  }
}
// void sendData(){
//   String json = "{";
//   json += "\"uid\":\"" + UID + "\",";
//   json += "\"field\":\"" + FIELD + "\",";
//   json += "\"temp\":" + String(temp, 1) + ",";
//   json += "\"hum\":" + String(hum, 1) + ",";
//   json += "\"moist\":" + String((int)soil) + ",";
//   json += "\"timestamp\":" + String(millis());
//   json += "}";

//   Serial.println("[LoRa] Sending telemetry");
//   txRaw(json);
// }

// void loraReceive(){
//   int sz = LoRa.parsePacket();
//   if(sz == 0) return;

//   String packet = "";
//   while(LoRa.available()) packet += (char)LoRa.read();
//   packet.trim();

//   Serial.print("[LoRa] RX=");
//   Serial.print(packet);
//   Serial.print(" RSSI=");
//   Serial.println(LoRa.packetRssi());

//   if(packet.startsWith("ASSIGN:")){
//     int firstSep = packet.indexOf(':');
//     int secondSep = packet.indexOf(':', firstSep + 1);
//     if(firstSep < 0 || secondSep < 0){
//       Serial.println("[LoRa] ASSIGN packet malformed");
//       return;
//     }

//     String packetUid = packet.substring(firstSep + 1, secondSep);
//     String packetField = packet.substring(secondSep + 1);
//     packetUid.trim();
//     packetField.trim();

//     if(packetUid != UID){
//       Serial.println("[LoRa] ASSIGN packet for different UID");
//       return;
//     }

//     FIELD = packetField;
//     bound = true;
//     saveIdentity();
//     lastGateway = millis();

//     Serial.print("[LoRa] Assigned ");
//     Serial.println(FIELD);
//     return;
//   }
// }

void loraReceive(){
  int sz = LoRa.parsePacket();
  if(sz == 0) return;
  String s = "";
  while(LoRa.available()) s += (char)LoRa.read();
  s.trim();

  if(nthField(s,0) != "AG7X9K") return;
  String type = nthField(s,1), uid = nthField(s,2), id = nthField(s,3), payload = nthField(s,4);
  if(uid != UID) return;            // not addressed to us
  lastGateway = millis();

  if(type == "A"){                  // assignment from gateway
    FIELD = id;
    bound = true;
    saveIdentity();
    if(oCount > 0) bufFlush();
  }
  else if(type == "C"){             // valve command
    String v = cmdVal(payload, "valve");
    if(v.length()) setValve(v == "ON");
  }
}

// ---- EEPROM identity (B3 UID, B4 stored id) ----
void loadIdentity(){
  if(EEPROM.read(EE_MAGIC_ADDR)!=EE_MAGIC_VAL){            // first boot -> make a UID
    randomSeed(esp_random());                              // <-- ESP32 hardware RNG (no analogRead(A1))
    const char* hx="0123456789ABCDEF"; UID="";
    for(int i=0;i<6;i++) UID+=hx[random(16)];
    for(int i=0;i<6;i++) EEPROM.write(EE_UID_ADDR+i, UID[i]);
    EEPROM.write(EE_BOUND_ADDR,0);
    EEPROM.write(EE_MAGIC_ADDR,EE_MAGIC_VAL);
    EEPROM.commit();                                        // <-- ESP32: persist
  } else {
    UID=""; for(int i=0;i<6;i++) UID+=(char)EEPROM.read(EE_UID_ADDR+i);
    if(EEPROM.read(EE_BOUND_ADDR)==1){
      bound=true; FIELD="";
      for(int i=0;i<12;i++){ char c=EEPROM.read(EE_FIELD_ADDR+i); if(c==0||c==255)break; FIELD+=c; }
    }
  }
}
void saveIdentity(){
  EEPROM.write(EE_BOUND_ADDR, bound?1:0);
  for(int i=0;i<12;i++) EEPROM.write(EE_FIELD_ADDR+i, i<(int)FIELD.length()?FIELD[i]:0);
  EEPROM.commit();                                          // <-- ESP32: persist
}

// ---- B5: re-bind button clears EEPROM + re-enters JOIN ----
void checkRebindButton(){
  static unsigned long t=0;
  if(digitalRead(PIN_BTN)==LOW){
    if(t==0) t=millis();
    else if(millis()-t>1500){                              // hold ~1.5s
      EEPROM.write(EE_BOUND_ADDR,0);
      for(int i=0;i<12;i++) EEPROM.write(EE_FIELD_ADDR+i,0);
      EEPROM.commit();                                      // <-- ESP32: persist
      bound=false; FIELD=""; oCount=0;
      delay(300);
    }
  } else t=0;
}

// ---- helpers ----
String nthField(const String& s,int idx){
  int start=0,count=0;
  for(int i=0;i<=(int)s.length();i++){
    if(i==(int)s.length()||s[i]==','){ if(count==idx) return s.substring(start,i); count++; start=i+1; }
  } return "";
}
String cmdVal(const String& p,const String& key){
  int k=p.indexOf(key+"="); if(k<0) return "";
  int e=p.indexOf(';',k); if(e<0) e=p.length();
  return p.substring(k+key.length()+1,e);
}