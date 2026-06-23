#include "agronode.h"
#include "valve_control.h"

// Local static controller state (keeps changes local to this module)
static String vc_mode = "AUTO";       // "AUTO" or "REMOTE"
static int vc_currentAngle = 0;        // last commanded servo angle
static int vc_remoteAngle = 0;         // used when mode==REMOTE

int calculateValveAngle(float moisture, float light, bool raining){
  if(raining) return 0;
  float dryness = 100.0f - moisture;
  float demand = 0.7f * dryness + 0.3f * light; // weighted demand
  demand = constrain(demand, 0.0f, 100.0f);
  int angle = constrain((int)(demand * 1.8f), 0, 180); // map 0..100 -> 0..180
  return angle;
}

void updateValveControl(){
  int newAngle = vc_currentAngle;
  if(vc_mode == "REMOTE"){
    newAngle = constrain(vc_remoteAngle, 0, 180);
  } else {
    newAngle = calculateValveAngle(soil, light, rain);
    if(soil > 90.0f) newAngle = 0;                // fully closed if saturated
    else if(soil < 15.0f && !rain) newAngle = 180; // emergency open if very dry
  }

  // Simple hysteresis to avoid jitter: move only if change > 5 degrees
  if(abs(newAngle - vc_currentAngle) > 5){
    vc_currentAngle = newAngle;
    if(&valve){ // servo attached
      valve.write(vc_currentAngle);
      valveOpen = (vc_currentAngle > 0);
    }
    Serial.print("[VALVE] Moved to "); Serial.print(vc_currentAngle);
    Serial.println(" degrees");
  }
}
