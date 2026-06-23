#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// --- MATCH THESE PINS EXACTLY TO YOUR WIRING ---
// SPI Bus (Hardware defined on Uno, do not change these on the board)

#define MISO  19
#define MOSI  23
#define SCK   18

// Control Pins (Match these to your specific jumper wires)
#define LORA_SS    5  // NSS / CS
#define LORA_RST   14   // Reset
#define LORA_DIO0  26   // Interrupt

// 433 MHz is standard for SX1278 in Asia/Europe. 
// Change to 915E6 if you bought a 915MHz US version.
#define LORA_FREQ 915E6 

void setup() {
  // Start the serial monitor
  Serial.begin(115200);
  while (!Serial); // Wait for Serial to be ready (needed for some boards)
  
  Serial.println("\n--- LoRa Hardware Diagnostic Test ---");
  Serial.println("Attempting to initialize LoRa radio...");

  // Tell the LoRa library which pins we are using
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DIO0);

  // Attempt to start the radio
  if (!LoRa.begin(LORA_FREQ)) {
    Serial.println("");
    Serial.println("❌ FAILED! LoRa radio not found.");
    
    // Stop the program completely
    while (1); 
  }

  // If we get past the 'begin' function, it works!
  Serial.println("");
  Serial.println("✅ SUCCESS! LoRa Gateway is UP.");
  Serial.println("The Arduino is successfully communicating with the SX1278 chip.");
  
  // Set a sync word just to prove we can write to its memory
  LoRa.setSyncWord(0x12);
}

void loop() {
  // Nothing to do in the loop for a hardware test
  delay(1000);
}