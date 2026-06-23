#include "agronode.h"
/* ===== node_logic.cpp — B7: local edge logic / emergency irrigation ===== */

void setValve(bool open){
  valveOpen=open;
  valve.write(open?90:0);
}

// If the node loses the gateway for too long AND soil is dry,
// irrigate locally so the crop is protected without the network.
void emergencyCheck(){
  static bool emergency=false;
  bool disconnected = (millis()-lastGateway) > EMERGENCY_MS;
  if(disconnected && soil < DRY_TRIGGER){
    if(!emergency){ emergency=true; setValve(true); Serial.println("[NODE] EMERGENCY irrigation ON"); }
  } else if(emergency){
    emergency=false; setValve(false); Serial.println("[NODE] EMERGENCY irrigation OFF");
  }
}