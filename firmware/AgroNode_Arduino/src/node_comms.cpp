#include "agronode.h"
/* ===== node_comms.ino — B2/B3/B4/B5/B6/B8 + E1 packet =====
   Packet:  AG,<type>,<uid>,<id>,<payload>
     uplink:   J join | D data | H heartbeat
     downlink: A assign | C command                                    */

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
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);
  if(!LoRa.begin(LORA_FREQ)){ Serial.println(F("LoRa FAIL")); while(1){ digitalWrite(PIN_LED,(millis()/100)%2); } }
  LoRa.setSyncWord(0x12);
}

void txPacket(const char* type, const String& payload){
  String pkt="AG,"; pkt+=type; pkt+=","; pkt+=UID; pkt+=","; pkt+=FIELD; pkt+=","; pkt+=payload;
  LoRa.beginPacket(); LoRa.print(pkt); LoRa.endPacket();
  LoRa.receive();   // back to rx mode
}

void sendJoin(){ txPacket("J", "hello"); }                 // B3
void sendHeartbeat(){ txPacket("H", "alive"); }            // B6

void sendData(){                                           // B2 + B8
  String p=sensorPayload();
  if(linkUp()) txPacket("D", p);
  else         bufPush(p);                                 // store if link down
}

void loraReceive(){
  int sz=LoRa.parsePacket(); if(sz==0) return;
  String s=""; while(LoRa.available()) s+=(char)LoRa.read();
  if(nthField(s,0)!="AG") return;
  String type=nthField(s,1), uid=nthField(s,2), id=nthField(s,3), payload=nthField(s,4);
  if(uid!=UID) return;                                      // not addressed to us
  lastGateway=millis();                                     // link is alive

  if(type=="A"){                                            // B4: assignment
    FIELD=id; bound=true; saveIdentity();
    if(oCount>0) bufFlush();                                // B8: flush after (re)link
  }
  else if(type=="C"){                                       // command from gateway (E3)
    String v=cmdVal(payload,"valve");
    if(v.length()) setValve(v=="ON");
  }
}

// ---- EEPROM identity (B3 UID, B4 stored id) ----
void loadIdentity(){
  if(EEPROM.read(EE_MAGIC_ADDR)!=EE_MAGIC_VAL){            // first boot -> make a UID
    randomSeed(analogRead(A1)^micros());
    const char* hx="0123456789ABCDEF"; UID="";
    for(int i=0;i<6;i++) UID+=hx[random(16)];
    for(int i=0;i<6;i++) EEPROM.write(EE_UID_ADDR+i, UID[i]);
    EEPROM.write(EE_BOUND_ADDR,0);
    EEPROM.write(EE_MAGIC_ADDR,EE_MAGIC_VAL);
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
  for(int i=0;i<12;i++) EEPROM.write(EE_FIELD_ADDR+i, i<FIELD.length()?FIELD[i]:0);
}

// ---- B5: re-bind button clears EEPROM + re-enters JOIN ----
void checkRebindButton(){
  static unsigned long t=0;
  if(digitalRead(PIN_BTN)==LOW){
    if(t==0) t=millis();
    else if(millis()-t>1500){                              // hold ~1.5s
      EEPROM.write(EE_BOUND_ADDR,0);
      for(int i=0;i<12;i++) EEPROM.write(EE_FIELD_ADDR+i,0);
      bound=false; FIELD=""; oCount=0;
      Serial.println(F("Re-bind: cleared, entering JOIN"));
      delay(300);
    }
  } else t=0;
}

// ---- helpers ----
String nthField(const String& s,int idx){
  int start=0,count=0;
  for(int i=0;i<=s.length();i++){
    if(i==s.length()||s[i]==','){ if(count==idx) return s.substring(start,i); count++; start=i+1; }
  } return "";
}
String cmdVal(const String& p,const String& key){
  int k=p.indexOf(key+"="); if(k<0) return "";
  int e=p.indexOf(';',k); if(e<0) e=p.length();
  return p.substring(k+key.length()+1,e);
}