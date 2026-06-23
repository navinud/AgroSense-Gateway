#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

#define LORA_SCK      18
#define LORA_MISO     19
#define LORA_MOSI     23
#define LORA_SS        5
#define LORA_RST      14
#define LORA_DIO0     26

#define LORA_FREQ     915E6
#define LORA_SYNC_WORD    0xF3
#define LORA_BANDWIDTH    125E3
#define LORA_SPREADING    7
#define LORA_CODING_RATE  5

struct Assignment { String uid; String field; };
Assignment assignment = {"", "field_1"};

void setup() {
  Serial.begin(115200);
  while (!Serial) {}

  Serial.println("[GATEWAY] Starting LoRa gateway...");
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_SS);
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("[GATEWAY] LoRa init failed!");
    while (1) {
      delay(1000);
    }
  }

  LoRa.setSyncWord(LORA_SYNC_WORD);
  LoRa.setSignalBandwidth(LORA_BANDWIDTH);
  LoRa.setSpreadingFactor(LORA_SPREADING);
  LoRa.setCodingRate4(LORA_CODING_RATE);
  LoRa.receive();

  Serial.println("[GATEWAY] LoRa initialized");
}

void loop() {
  int packetSize = LoRa.parsePacket();
  if (packetSize == 0) return;

  String packet = "";
  while (LoRa.available()) {
    packet += (char)LoRa.read();
  }
  packet.trim();

  int rssi = LoRa.packetRssi();
  Serial.print("[GATEWAY] RX=");
  Serial.print(packet);
  Serial.print(" RSSI=");
  Serial.println(rssi);

  if (packet.startsWith("JOIN:")) {
    String uid = packet.substring(5);
    uid.trim();
    if (uid.length() == 0) {
      Serial.println("[GATEWAY] Invalid JOIN packet: empty UID");
      return;
    }

    if (assignment.uid.length() == 0) {
      assignment.uid = uid;
      Serial.print("[GATEWAY] Assigned UID ");
      Serial.print(assignment.uid);
      Serial.print(" to ");
      Serial.println(assignment.field);
    }

    if (uid == assignment.uid) {
      String assignPacket = "ASSIGN:" + uid + ":" + assignment.field;
      LoRa.beginPacket();
      LoRa.print(assignPacket);
      LoRa.endPacket();
      LoRa.receive();
      Serial.print("[GATEWAY] Sent ");
      Serial.println(assignPacket);
    } else {
      Serial.print("[GATEWAY] JOIN received for unknown UID ");
      Serial.println(uid);
    }
    return;
  }

  if (packet.startsWith("{")) {
    Serial.println("[GATEWAY] Telemetry JSON received:");
    Serial.println(packet);
    return;
  }

  Serial.println("[GATEWAY] Unknown packet format");
}
