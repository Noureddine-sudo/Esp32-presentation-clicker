# ESP32 Presentation Clicker

A wireless presentation clicker built with ESP32. It connects to a local presentation server via WiFi and mDNS and controls Next / Previous / Action buttons.

## Features
- WiFi connection using WiFiManager (captive portal)
- Automatic server discovery via mDNS (`serveur-pres.local`)
- Saves server IP in non-volatile memory
- Debounced buttons
- Long press on Action button to force rediscovery

## Hardware
- ESP32 development board
- 3 buttons connected to GPIO 18 (Next), 19 (Prev), 21 (Action)
- Internal pull-up resistors used

## How to Use
1. Upload the code to your ESP32
2. Connect to the WiFi network created by the ESP32 (`ESP32-Clicker-Setup`) and configure your WiFi
3. Make sure your presentation server is running and advertising as `serveur-pres.local` better use the server.py in a linux vm cause of the problems of mDns in windows 11
4. Press the buttons to control the presentation

## Server Side
Your presentation server must respond to:
- `GET /next`
- `GET /prev`
- `GET /action`

And should advertise itself via mDNS as `serveur-pres.local`.
