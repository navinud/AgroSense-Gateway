#include "agro.h"
/* ===== comms.ino — A9 (wifi+mqtt), A10 (offline buffer), A11 (circular log/replay) ===== */

// circular / offline buffer of telemetry payloads (A10/A11)
String  buf[BUF_SIZE]; String bufTopic[BUF_SIZE];
int     bufHead=0, bufCount=0;

void bufPush(const String& topic, const String& payload){
  buf[bufHead]=payload; bufTopic[bufHead]=topic;
  bufHead=(bufHead+1)%BUF_SIZE;
  if(bufCount<BUF_SIZE) bufCount++;
}
void bufReplay(){                       // A11: send buffered data on reconnect (feeds D8)
  int idx=(bufHead - bufCount + BUF_SIZE)%BUF_SIZE;
  for(int i=0;i<bufCount;i++){
    mqtt.publish(bufTopic[idx].c_str(), buf[idx].c_str());
    idx=(idx+1)%BUF_SIZE;
    delay(20);
  }
  bufCount=0;
  publishAlert("info", SELF_FIELD, "Replayed buffered data after reconnect");
}

void wifiConnect(){
  if(WiFi.status()==WL_CONNECTED) return;
  WiFi.mode(WIFI_STA); WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi");
  unsigned long t0=millis();
  while(WiFi.status()!=WL_CONNECTED && millis()-t0<8000){ delay(300); Serial.print("."); }
  if(WiFi.status()==WL_CONNECTED) {
    Serial.println(" ok (Waiting for internet routing...)");
    delay(2000); // Give the hotspot 2 seconds to establish internet/DNS
  } else {
    Serial.println(" FAILED (buffering)");
  }
}

void mqttConnect(){
  if(WiFi.status()!=WL_CONNECTED) return;
  if(mqtt.connect(MQTT_CLIENT, NULL, NULL, T_STATUS_GW, 1, true, "OFFLINE")){
    mqtt.publish(T_STATUS_GW, "ONLINE", true);
    // ... subscriptions ...
    Serial.println("MQTT connected");
    if(bufCount > 0) {
        Serial.print("Attempting to replay ");
        Serial.print(bufCount);
        Serial.println(" buffered packets.");
        bufReplay(); 
    }
    mqtt.subscribe(T_CMD_VALVE); mqtt.subscribe(T_CMD_MODE);
    mqtt.subscribe(T_CMD_THR);   mqtt.subscribe(T_CMD_BIND);
    mqtt.subscribe(T_CMD_WX);
    Serial.println("MQTT connected");
    if(bufCount>0) bufReplay();         // flush offline buffer
  }
}

void publishJSON(const char* topic, const String& json){
  if(mqtt.connected()) mqtt.publish(topic, json.c_str());
  else                 bufPush(topic, json);   // A10: store if offline
}

// build + publish this ESP32's own field telemetry (A9)
void publishSelfTelemetry(){
  char topic[48]; snprintf(topic,sizeof(topic),T_TELEM_FMT,SELF_FIELD);
  String j = "{";
  j += "\"uid\":\"ESP32-SELF\",";
  j += "\"name\":\""+String(SELF_NAME)+"\",";
  j += "\"temp\":"+String(temp,1)+",";
  j += "\"hum\":"+String((int)hum)+",";
  j += "\"moist\":"+String((int)moist)+",";
  j += "\"light\":"+String((int)light)+",";
  j += "\"rain\":"+String(rain?"true":"false")+",";
  j += "\"valve\":\""+String(valveOpen?"ON":"OFF")+"\",";
  j += "\"mode\":\""+selfMode+"\",";
  j += "\"score\":"+String(score)+",";
  j += "\"scoreparts\":{\"moisture\":"+String(pMoist,0)+",\"rain\":"+String(pRain,0)+",\"time\":"+String(pTime,0)+"}";
  j += "}";
  publishJSON(topic, j);
}

void publishAlert(const char* level, const char* node, const char* msgtxt){
  String j="{\"level\":\""+String(level)+"\",\"node\":\""+String(node)+"\",\"msg\":\""+String(msgtxt)+"\"}";
  if(mqtt.connected()) mqtt.publish(T_ALERTS, j.c_str());
}

// ---- command handler (A9 subscribe; E3) ----
String jsonStr(const String& s, const String& key){
  int k=s.indexOf("\""+key+"\""); if(k<0) return "";
  int c=s.indexOf(':',k); int q1=s.indexOf('"',c+1);
  if(q1<0) return "";
  int q2=s.indexOf('"',q1+1); return s.substring(q1+1,q2);
}

void onMqtt(char* topic, byte* payload, unsigned int len){
  String t=topic, p; for(unsigned i=0;i<len;i++) p+=(char)payload[i];
  String field=jsonStr(p,"field");

  if(t==T_CMD_VALVE){
    String st=jsonStr(p,"state");
    if(field==SELF_FIELD){ selfMode="MANUAL"; setValve(st=="ON"); }
    else loraSendCommand(field, "valve="+st);          // forward to remote node
  }
  else if(t==T_CMD_MODE){
    String m=jsonStr(p,"mode");
    if(field==SELF_FIELD) selfMode=m;
    else loraSendCommand(field, "mode="+m);
  }
  else if(t==T_CMD_THR){
    int mi=p.indexOf("\"moist\""); if(mi>=0){ thrMoist=p.substring(p.indexOf(':',mi)+1).toInt(); }
  }
  else if(t==T_CMD_BIND){                                // C3 -> assign + push to node
    String uid=jsonStr(p,"uid"), name=jsonStr(p,"name");
    bindNode(uid, name);
  }
  else if(t==T_CMD_WX){                                  // weather hint from dashboard (A5/D3)
    rainSoon = (p.indexOf("true")>=0);
  }
}