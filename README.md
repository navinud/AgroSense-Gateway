# AgroSense V2

AgroSense V2 is a distributed IoT system for agricultural field monitoring. Arduino Uno sensor nodes
collect soil-moisture and environmental readings and transmit them over LoRa to an ESP32 gateway,
which relays the data via Wi-Fi and MQTT to a local Mosquitto broker. A Node-RED dashboard running
on the same host subscribes to those topics and visualises the readings in real time, giving farmers
a low-cost, low-power solution for remote crop monitoring.

## Architecture

```
[Arduino Uno Node]
  sensors → LoRa TX
      │
      │  LoRa (915 / 868 MHz)
      ▼
[ESP32 Gateway]
  LoRa RX → Wi-Fi → MQTT publish
      │
      │  MQTT (TCP 1883)
      ▼
[Mosquitto Broker]  ←→  [Node-RED Dashboard]
```

## Repo Layout

```
AgroSenseV2/
├── README.md
├── .gitignore
├── agrosense.code-workspace      # opens both PlatformIO projects in VS Code
├── node-red/
│   └── flows.json                # Node-RED dashboard flows
├── docs/
│   ├── HARDWARE_GUIDE.md
│   └── PLATFORMIO_README.md
└── firmware/
    ├── AgroSense_ESP32/          # ESP32 LoRa-to-MQTT gateway
    │   ├── platformio.ini
    │   └── src/
    └── AgroNode_Arduino/         # Arduino Uno field sensor node
        ├── platformio.ini
        └── src/
```

## Build, Upload & Monitor

### ESP32 Gateway (`firmware/AgroSense_ESP32/`)

```bash
# Build
pio run -e esp32dev

# Upload (ensure the board is on the correct COM port in platformio.ini)
pio run -e esp32dev --target upload

# Serial monitor
pio device monitor -e esp32dev
```

### Arduino Uno Node (`firmware/AgroNode_Arduino/`)

```bash
# Build
pio run -e uno

# Upload
pio run -e uno --target upload

# Serial monitor
pio device monitor -e uno
```

> Run all commands from inside each project's own directory, or use
> `pio run -d firmware/AgroSense_ESP32` from the repo root.

## Dashboard Setup

1. Install [Node-RED](https://nodered.org/docs/getting-started/) and the
   `node-red-dashboard` palette.
2. Start Node-RED: `node-red`
3. Open <http://localhost:1880> → **Menu → Import → select file**
4. Import `node-red/flows.json` from this repository.
5. Deploy the flow — the dashboard will be available at <http://localhost:1880/ui>.

Make sure Mosquitto is running and the MQTT broker address in the Node-RED MQTT
node matches the one configured in `firmware/AgroSense_ESP32/src/config.h`.
