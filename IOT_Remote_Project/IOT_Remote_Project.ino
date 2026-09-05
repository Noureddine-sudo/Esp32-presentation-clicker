#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <WiFiManager.h>
#include <Preferences.h>

// ================== BUTTON PINS ==================
const int BUTTON_NEXT   = 18;
const int BUTTON_PREV   = 19;
const int BUTTON_ACTION = 21;

// Optional: Built-in LED for status feedback
const int LED_PIN = 2;

IPAddress serverIP;
bool serverFound = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 250;

Preferences preferences;

void setup() {
  Serial.begin(115200);
  
  pinMode(BUTTON_NEXT,   INPUT_PULLUP);
  pinMode(BUTTON_PREV,   INPUT_PULLUP);
  pinMode(BUTTON_ACTION, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Load previously saved server IP
  preferences.begin("clicker", false);
  String savedIP = preferences.getString("server_ip", "");
  if (savedIP.length() > 0) {
    serverIP.fromString(savedIP);
    serverFound = true;
    Serial.print("📂 Loaded saved server IP: ");
    Serial.println(serverIP);
  }

  // WiFiManager - Captive Portal
  WiFiManager wm;
  if (!wm.autoConnect("ESP32-Clicker-Setup")) {
    Serial.println("Failed to connect. Restarting...");
    delay(3000);
    ESP.restart();
  }

  Serial.println("✅ WiFi Connected!");
  Serial.print("ESP32 IP: "); Serial.println(WiFi.localIP());

  // Start mDNS
  if (!MDNS.begin("esp32-client")) {
    Serial.println("❌ mDNS responder failed to start");
  }

  // Try to discover server if no saved IP
  if (!serverFound) {
    discoverServer();
  } else {
    Serial.println("✅ Using saved server IP - Ready!");
    digitalWrite(LED_PIN, HIGH);
  }
}

void discoverServer() {
  Serial.println("🔍 Searching for serveur-pres.local via mDNS...");
  int n = MDNS.queryService("http", "tcp");
  
  if (n > 0) {
    serverIP = MDNS.address(0);
    serverFound = true;
    preferences.putString("server_ip", serverIP.toString());
    Serial.print("✅ Server found & SAVED: ");
    Serial.println(serverIP);
    digitalWrite(LED_PIN, HIGH);   // Solid ON = good connection
  } else {
    Serial.println("❌ mDNS not found - Will retry on button press");
    serverFound = false;
  }
}

void sendRequest(String path) {
  if (WiFi.status() != WL_CONNECTED) return;

  if (!serverFound) {
    discoverServer();
    if (!serverFound) return;
  }

  HTTPClient http;
  String url = "http://" + serverIP.toString() + ":5000" + path;
  Serial.print("→ Sending: "); Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();
  
  if (httpCode > 0) {
    Serial.printf("✅ HTTP %d\n", httpCode);
    // Visual feedback
    digitalWrite(LED_PIN, HIGH);
    delay(80);
    digitalWrite(LED_PIN, LOW);
  } else {
    Serial.printf("❌ HTTP Error: %s\n", http.errorToString(httpCode).c_str());
    if (httpCode == -1) serverFound = false; // Force rediscovery next time
  }
  http.end();
}

void loop() {
  unsigned long now = millis();

  // NEXT
  if (digitalRead(BUTTON_NEXT) == LOW && (now - lastDebounceTime > debounceDelay)) {
    lastDebounceTime = now;
    sendRequest("/next");
    while(digitalRead(BUTTON_NEXT) == LOW) delay(10);
  }

  // PREV
  if (digitalRead(BUTTON_PREV) == LOW && (now - lastDebounceTime > debounceDelay)) {
    lastDebounceTime = now;
    sendRequest("/prev");
    while(digitalRead(BUTTON_PREV) == LOW) delay(10);
  }

  // ACTION
  if (digitalRead(BUTTON_ACTION) == LOW && (now - lastDebounceTime > debounceDelay)) {
    lastDebounceTime = now;
    sendRequest("/action");
    while(digitalRead(BUTTON_ACTION) == LOW) delay(10);
  }
}