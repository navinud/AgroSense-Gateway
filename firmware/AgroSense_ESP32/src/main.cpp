#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

// ==========================================
// 1. YOUR NETWORK CONFIGURATION
// ==========================================
const char* ssid = "iPhone";           // Change this
const char* password = "aaaaaaaa";   // Change this
const char* mqtt_server = "broker.hivemq.com";       // Change to your PC's local IP

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;

// ==========================================
// 2. WI-FI & MQTT SETUP
// ==========================================
void setup_wifi() {
  delay(10);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP32_Gateway")) {
      Serial.println("connected");
    } else {
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883); // Standard MQTT port
}

// ==========================================
// 3. MAIN LOOP
// ==========================================
void loop() {
  if (client.connect("ESP32Client")) {
    Serial.println("Connected to MQTT Broker!");

  } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state()); // This is the error code
      Serial.println(" - Retrying in 5 seconds...");
      delay(50000);
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) { // Publish every 5 seconds
    lastMsg = now;

    // Simulating moisture data for the dashboard
    int moisture = random(30, 70);
    char payload[8];
    itoa(moisture, payload, 10);

    Serial.print("Publishing to agrosense/field1/moisture: ");
    Serial.println(payload);

    client.publish("agrosense/field1/moisture", payload);
  }
}
