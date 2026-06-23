#pragma once
#include <Arduino.h>

// Calculate desired valve servo angle (0..180) from moisture (0..100), light (0..100), and raining flag
int calculateValveAngle(float moisture, float light, bool raining);

// Update servo position using current sensor globals. Call from main loop.
void updateValveControl();
