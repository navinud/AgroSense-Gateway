#include "agro.h"
/* ===== storage.cpp — persistent EEPROM queue (power-cut safe) =====
   Layout:
     Addr 0    : magic byte 0  (0xAB)
     Addr 1    : magic byte 1  (0xCD)
     Addr 2    : head  — next write slot (0 .. MAX_EE_RECORDS-1)
     Addr 3    : count — valid records   (0 .. MAX_EE_RECORDS)
     Addr 4..N : packed TelemetryRecord array
*/

#define EE_MAGIC0  0xAB
#define EE_MAGIC1  0xCD
#define ADDR_M0    0
#define ADDR_M1    1
#define ADDR_HEAD  2
#define ADDR_COUNT 3
#define ADDR_DATA  4

static uint8_t ee_head  = 0;
static uint8_t ee_count = 0;

void eeInit() {
  EEPROM.begin(EEPROM_SIZE);
  if (EEPROM.read(ADDR_M0) != EE_MAGIC0 || EEPROM.read(ADDR_M1) != EE_MAGIC1) {
    ee_head = 0; ee_count = 0;
    EEPROM.write(ADDR_M0, EE_MAGIC0);
    EEPROM.write(ADDR_M1, EE_MAGIC1);
    EEPROM.write(ADDR_HEAD, 0);
    EEPROM.write(ADDR_COUNT, 0);
    EEPROM.commit();
    Serial.println("EE: first boot, storage reset");
  } else {
    ee_head  = EEPROM.read(ADDR_HEAD);
    ee_count = EEPROM.read(ADDR_COUNT);
    Serial.printf("EE: restored  head=%d  count=%d\n", ee_head, ee_count);
  }
}

void eePush(const TelemetryRecord& r) {
  int addr = ADDR_DATA + ee_head * (int)sizeof(TelemetryRecord);
  EEPROM.put(addr, r);
  ee_head = (ee_head + 1) % MAX_EE_RECORDS;
  if (ee_count < MAX_EE_RECORDS) ee_count++;
  EEPROM.write(ADDR_HEAD, ee_head);
  EEPROM.write(ADDR_COUNT, ee_count);
  EEPROM.commit();  // flush to flash — safe on power cut
  //Serial.printf("EE push  head=%d  count=%d\n", ee_head, ee_count);
}

bool eePop(TelemetryRecord& r) {
  if (ee_count == 0) return false;
  uint8_t tail = (ee_head - ee_count + MAX_EE_RECORDS) % MAX_EE_RECORDS;
  int addr = ADDR_DATA + tail * (int)sizeof(TelemetryRecord);
  EEPROM.get(addr, r);
  ee_count--;
  EEPROM.write(ADDR_COUNT, ee_count);
  // No commit here — count update is persisted by the next eePush commit.
  // Worst case on power cut: replays already-sent records (acceptable for telemetry).
  return true;
}

bool eePeek(TelemetryRecord& r) {
  if (ee_count == 0) return false;
  uint8_t tail = (ee_head - ee_count + MAX_EE_RECORDS) % MAX_EE_RECORDS;
  int addr = ADDR_DATA + tail * (int)sizeof(TelemetryRecord);
  EEPROM.get(addr, r);
  return true;
}

void eeConfirmPop() {
  if (ee_count == 0) return;
  ee_count--;
  EEPROM.write(ADDR_COUNT, ee_count);
  // no EEPROM.commit() — same strategy as eePop; next eePush commits
}

int eeCount() {
  return (int)ee_count;
}
