#include "agro.h"
/* =====================================================================
   AgroSense V2 — ESP32 Gateway / Main Node   (Sections A, C, E, F)
   Modules (F5):  this file = orchestration
                  sensors.ino = A1/A3/A12   (read, normalize, MA, faults)
                  logic.ino   = A4-A8       (thresholds, scoring, valve)
                  comms.ino   = A9-A11      (wifi, mqtt, buffer, replay)
                  lora_gw.ino = A13-A15,C   (LoRa rx, registry, binding)
   Libraries: PubSubClient, LoRa (sandeepmistry), Adafruit_SSD1306,
              Adafruit_GFX, DHT sensor library, Adafruit Unified Sensor,
              ESP32Servo.
   ===================================================================== */

// ---- shared globals (visible to all .ino tabs) ----
WiFiClient        espClient;
PubSubClient      mqtt(espClient);
Adafruit_SSD1306  oled(128, 64, &Wire, -1);
DHT               dht(PIN_DHT, DHTTYPE);
Servo             valveServo;

// live readings for the ESP32's own field
float  temp=0, hum=0, moist=0, light=0;
bool   rain=false, rainSoon=false;     // rainSoon = weather hint from dashboard
bool   valveOpen=false;
String selfMode="AUTO";                // AUTO / MANUAL (manual via dashboard)
int    score=0;
float  pMoist=0, pRain=0, pTime=0;     // explainable score parts
String partOfDay="day";

// adaptive thresholds (A4) — variables, not constants
int    thrMoist=40, thrTemp=35;

// fault state (A12)
bool   sensorFault=false;

void setup() {
  Serial.begin(115200);
  delay(200);
  
  // Wire.begin(OLED_SDA, OLED_SCL);
  // oledInit();                  // <-- COMMENTED OUT (No OLED wired yet)
  
  dht.begin();                    // <-- KEEP THIS (DHT11 is wired!)
  
  // valveServo.attach(PIN_SERVO); // <-- COMMENTED OUT
  // setValve(false);              // <-- COMMENTED OUT

  wifiConnect();
  configTime(19800, 0, "pool.ntp.org");   // UTC+5:30 (Sri Lanka)
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqtt);
  mqtt.setBufferSize(512);

  // loraInit();                  // <-- COMMENTED OUT (No LoRa wired yet)
  
  Serial.println("AgroSense ESP32 ready.");
}

unsigned long tSensor=0, tLastPub=0, tOled=0, tReg=0;

void loop() {
  if (WiFi.status()!=WL_CONNECTED) wifiConnect();
  if (!mqtt.connected()) mqttConnect();
  mqtt.loop();

  //loraReceive();                 // handle node packets (data/join/heartbeat)

  unsigned long now=millis();

  // sensor read: 2000 ms cadence (DHT11 is slow)
  if (now - tSensor >= PUBLISH_MS) {
    tSensor = now;
    readSensors();               // A1/A3 + fault check (A12)
    updateThresholds();          // A4/A7 time-based
    computeScore();              // A5/A6
    decideValve();               // A8
  }

  // adaptive publish: normal 2000 ms OR event-driven on moisture/valve/mode change (min 500 ms)
  static float         lastPublishedMoist = -1.0f;
  static unsigned long lastEventPublish   = 0;
  static bool          lastPubValve       = false;
  static String        lastPubMode        = "";
  bool normalPub = (now - tLastPub >= PUBLISH_MS);
  bool eventPub  = (now - lastEventPublish >= 500UL) &&
                   (fabs(moist - lastPublishedMoist) > 2.0f ||
                    valveOpen != lastPubValve                ||
                    selfMode  != lastPubMode);
  if (normalPub || eventPub) {
    tLastPub           = now;
    lastEventPublish   = now;
    lastPublishedMoist = moist;
    lastPubValve       = valveOpen;
    lastPubMode        = selfMode;
    publishSelfTelemetry();      // A9 (or buffer if offline, A10)
  }

  // if (now - tOled > OLED_MS) { tOled=now; oledShow(); }
  // if (now - tReg > 5000)     { tReg=now; checkNodeTimeouts(); publishRegistry(); }
}