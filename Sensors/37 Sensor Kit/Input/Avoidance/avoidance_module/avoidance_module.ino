/*
 * ESP32 WiFi Bridge for Arduino LED Controller
 * 
 * This ESP32 code demonstrates IoT gateway functionality:
 * 1. Connects to WiFi network
 * 2. Communicates with Arduino via Serial
 * 3. Sends data to web server via HTTP
 * 4. Receives commands from web server
 * 5. Acts as a bridge between local device and internet
 * 
 * Educational Focus:
 * - WiFi connectivity and management
 * - HTTP client/server communication
 * - Inter-device serial communication
 * - IoT gateway architecture
 * - Real-time data transmission
 * 
 * Author: Educational Demo
 * Purpose: Teaching distributed IoT systems
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>
#include <WebServer.h>

// ===== NETWORK CONFIGURATION =====
// Replace with your WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Replace with your server URL (Node.js server)
const char* serverURL = "http://192.168.1.100:3000/api/data";

// ===== DEVICE IDENTIFICATION =====
const char* deviceID = "ESP32_Arduino_LED_001";

// ===== ARDUINO COMMUNICATION =====
HardwareSerial ArduinoSerial(1); // Use Serial1
const int ARDUINO_RX_PIN = 16;   // ESP32 RX pin (connects to Arduino TX pin 3)
const int ARDUINO_TX_PIN = 17;   // ESP32 TX pin (connects to Arduino RX pin 2)

// ===== TIMING CONFIGURATION =====
unsigned long lastDataSend = 0;
const unsigned long DATA_SEND_INTERVAL = 10000; // Send data every 10 seconds
unsigned long lastArduinoCheck = 0;
const unsigned long ARDUINO_CHECK_INTERVAL = 3000; // Check Arduino every 3 seconds

// ===== LED STATUS TRACKING =====
struct LEDStatus {
  bool ledState = false;
  bool arduinoConnected = false;
  unsigned long lastArduinoUpdate = 0;
  String lastArduinoMessage = "";
  
  // ESP32 status
  long wifiRSSI = 0;
  unsigned long uptime = 0;
  bool serverConnected = false;
} ledStatus;

// ===== WEB SERVER FOR LOCAL CONTROL =====
WebServer localServer(80);

// ===== SETUP FUNCTION =====
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println("\n=== ESP32 WiFi Bridge for Arduino LED ===");
  Serial.println("Starting up...");
  
  // Initialize Arduino communication
  ArduinoSerial.begin(9600, SERIAL_8N1, ARDUINO_RX_PIN, ARDUINO_TX_PIN);
  Serial.println("🔗 Arduino Serial initialized on pins RX:16, TX:17");
  
  // Connect to WiFi
  connectToWiFi();
  
  // Setup local web server
  setupLocalWebServer();
  
  // Send connection signal to Arduino
  sendToArduino("ESP32_CONNECTED");
  delay(1000);
  
  // Request initial status from Arduino
  sendToArduino("STATUS");
  
  Serial.println("🚀 ESP32 Bridge Setup completed!");
  Serial.print("🌐 Server URL: ");
  Serial.println(serverURL);
  Serial.print("📱 Local control: http://");
  Serial.println(WiFi.localIP());
  Serial.println("📡 Monitoring Arduino and bridging to server...\n");
}

// ===== MAIN LOOP =====
void loop() {
  // Handle local web server requests
  localServer.handleClient();
  
  // Check WiFi connection
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Attempting to reconnect...");
    sendToArduino("ESP32_DISCONNECTED");
    connectToWiFi();
    if (WiFi.status() == WL_CONNECTED) {
      sendToArduino("ESP32_CONNECTED");
    }
    return;
  }
  
  // Process Arduino messages
  processArduinoMessages();
  
  // Periodic Arduino status check
  if (millis() - lastArduinoCheck >= ARDUINO_CHECK_INTERVAL) {
    sendToArduino("PING");
    lastArduinoCheck = millis();
  }
  
  // Send data to server at regular intervals
  if (millis() - lastDataSend >= DATA_SEND_INTERVAL) {
    sendDataToServer();
    lastDataSend = millis();
  }
  
  // Check if Arduino connection is stale
  if (millis() - ledStatus.lastArduinoUpdate > 15000) {
    ledStatus.arduinoConnected = false;
  }
  
  delay(100);
}

// ===== WIFI CONNECTION =====
void connectToWiFi() {
  Serial.print("🔗 Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(1000);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi connected successfully!");
    Serial.print("📍 IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("📶 Signal strength (RSSI): ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");
  } else {
    Serial.println("\n❌ Failed to connect to WiFi!");
    Serial.println("Please check your credentials and try again.");
  }
}

// ===== ARDUINO COMMUNICATION =====
void processArduinoMessages() {
  if (ArduinoSerial.available()) {
    String message = ArduinoSerial.readStringUntil('\n');
    message.trim();
    
    if (message.length() > 0) {
      Serial.println("📨 Arduino: " + message);
      parseArduinoMessage(message);
      ledStatus.lastArduinoUpdate = millis();
      ledStatus.arduinoConnected = true;
      ledStatus.lastArduinoMessage = message;
    }
  }
}

void parseArduinoMessage(String message) {
  if (message == "ARDUINO_READY") {
    Serial.println("🤖 Arduino is ready");
    sendToArduino("ESP32_CONNECTED");
    
  } else if (message == "ARDUINO_CONNECTED") {
    Serial.println("✅ Arduino connection confirmed");
    
  } else if (message == "LED_ON_CONFIRMED") {
    ledStatus.ledState = true;
    Serial.println("💡 LED ON confirmed by Arduino");
    
  } else if (message == "LED_OFF_CONFIRMED") {
    ledStatus.ledState = false;
    Serial.println("🔌 LED OFF confirmed by Arduino");
    
  } else if (message.startsWith("LED_TOGGLED_")) {
    String state = message.substring(12); // After "LED_TOGGLED_"
    ledStatus.ledState = (state == "ON");
    Serial.println("🔄 LED toggled to " + state);
    
  } else if (message.startsWith("STATUS:")) {
    parseArduinoStatus(message.substring(7)); // After "STATUS:"
    
  } else if (message == "PONG") {
    Serial.println("🏓 Arduino responded to ping");
    
  } else if (message.startsWith("UNKNOWN_COMMAND:")) {
    Serial.println("❓ Arduino reports unknown command: " + message.substring(16));
    
  } else {
    Serial.println("💬 Arduino message: " + message);
  }
}

void parseArduinoStatus(String statusData) {
  // Parse status like "led=ON,uptime=123,memory=1500"
  Serial.println("📊 Arduino status: " + statusData);
  
  // Simple parsing for educational purposes
  if (statusData.indexOf("led=ON") >= 0) {
    ledStatus.ledState = true;
  } else if (statusData.indexOf("led=OFF") >= 0) {
    ledStatus.ledState = false;
  }
}

void sendToArduino(String command) {
  ArduinoSerial.println(command);
  Serial.println("➡️  Command to Arduino: " + command);
}

// ===== SERVER COMMUNICATION =====
void sendDataToServer() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Cannot send data: WiFi not connected");
    ledStatus.serverConnected = false;
    return;
  }
  
  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");
  
  // Update ESP32 status
  ledStatus.wifiRSSI = WiFi.RSSI();
  ledStatus.uptime = millis();
  
  // Create JSON payload
  DynamicJsonDocument doc(400);
  
  // Device information
  doc["deviceID"] = deviceID;
  doc["timestamp"] = millis();
  doc["uptime"] = ledStatus.uptime;
  
  // ESP32 data
  doc["esp32"]["wifiRSSI"] = ledStatus.wifiRSSI;
  doc["esp32"]["freeHeap"] = ESP.getFreeHeap();
  doc["esp32"]["chipModel"] = ESP.getChipModel();
  doc["esp32"]["localIP"] = WiFi.localIP().toString();
  
  // Arduino data
  doc["arduino"]["connected"] = ledStatus.arduinoConnected;
  doc["arduino"]["ledState"] = ledStatus.ledState;
  doc["arduino"]["lastUpdate"] = ledStatus.lastArduinoUpdate;
  doc["arduino"]["lastMessage"] = ledStatus.lastArduinoMessage;
  
  // System status
  doc["system"]["wifiConnected"] = true;
  doc["system"]["serverConnected"] = ledStatus.serverConnected;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  Serial.println("\n--- Sending Data to Server ---");
  Serial.print("📤 Payload: ");
  Serial.println(jsonString);
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("📡 HTTP Response: ");
    Serial.print(httpResponseCode);
    Serial.print(" - ");
    Serial.println(response);
    
    if (httpResponseCode == 200) {
      Serial.println("✅ Data sent successfully!");
      ledStatus.serverConnected = true;
    } else {
      ledStatus.serverConnected = false;
    }
  } else {
    Serial.print("❌ HTTP Error: ");
    Serial.println(httpResponseCode);
    ledStatus.serverConnected = false;
  }
  
  http.end();
  Serial.println("--- End Transmission ---\n");
}

// ===== LOCAL WEB SERVER =====
void setupLocalWebServer() {
  // Home page with local control
  localServer.on("/", handleLocalHomePage);
  
  // LED control endpoints
  localServer.on("/led/on", handleLocalLEDOn);
  localServer.on("/led/off", handleLocalLEDOff);
  localServer.on("/led/toggle", handleLocalLEDToggle);
  
  // Status endpoint
  localServer.on("/status", handleLocalStatus);
  
  // Start server
  localServer.begin();
  Serial.println("🌐 Local web server started");
}

void handleLocalHomePage() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>ESP32 Bridge Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial, sans-serif; max-width: 500px; margin: 50px auto; padding: 20px; text-align: center; }
        .status { padding: 20px; margin: 20px 0; border-radius: 10px; }
        .online { background-color: #d4edda; color: #155724; }
        .offline { background-color: #f8d7da; color: #721c24; }
        button { padding: 15px 30px; margin: 10px; font-size: 16px; border: none; border-radius: 5px; cursor: pointer; }
        .btn-on { background-color: #28a745; color: white; }
        .btn-off { background-color: #dc3545; color: white; }
        .btn-toggle { background-color: #007bff; color: white; }
    </style>
</head>
<body>
    <h1>🌉 ESP32 Bridge Control</h1>
    
    <div class="status )" + (ledStatus.arduinoConnected ? "online" : "offline") + R"(">
        Arduino: )" + (ledStatus.arduinoConnected ? "Connected ✅" : "Disconnected ❌") + R"(
    </div>
    
    <div class="status )" + (ledStatus.serverConnected ? "online" : "offline") + R"(">
        Server: )" + (ledStatus.serverConnected ? "Connected ✅" : "Disconnected ❌") + R"(
    </div>
    
    <div class="status">
        LED Status: )" + (ledStatus.ledState ? "ON 💡" : "OFF 🔌") + R"(
    </div>
    
    <div>
        <button class="btn-on" onclick="controlLED('on')">Turn ON</button>
        <button class="btn-off" onclick="controlLED('off')">Turn OFF</button>
        <button class="btn-toggle" onclick="controlLED('toggle')">Toggle</button>
    </div>
    
    <p><strong>Bridge Functions:</strong><br>
    • WiFi to Arduino communication<br>
    • Data forwarding to server<br>
    • Local device control</p>
    
    <script>
        function controlLED(action) {
            fetch('/led/' + action)
                .then(response => response.text())
                .then(data => {
                    alert(data);
                    setTimeout(() => location.reload(), 1000);
                })
                .catch(error => alert('Error: ' + error));
        }
    </script>
</body>
</html>
)";
  
  localServer.send(200, "text/html", html);
}

void handleLocalLEDOn() {
  sendToArduino("LED_ON");
  localServer.send(200, "text/plain", "LED ON command sent to Arduino");
  Serial.println("🌐 Local web request: LED ON");
}

void handleLocalLEDOff() {
  sendToArduino("LED_OFF");
  localServer.send(200, "text/plain", "LED OFF command sent to Arduino");
  Serial.println("🌐 Local web request: LED OFF");
}

void handleLocalLEDToggle() {
  sendToArduino("LED_TOGGLE");
  localServer.send(200, "text/plain", "LED TOGGLE command sent to Arduino");
  Serial.println("🌐 Local web request: LED TOGGLE");
}

void handleLocalStatus() {
  DynamicJsonDocument doc(300);
  doc["ledState"] = ledStatus.ledState;
  doc["arduinoConnected"] = ledStatus.arduinoConnected;
  doc["serverConnected"] = ledStatus.serverConnected;
  doc["wifiRSSI"] = ledStatus.wifiRSSI;
  doc["uptime"] = ledStatus.uptime;
  doc["lastArduinoMessage"] = ledStatus.lastArduinoMessage;
  
  String jsonString;
  serializeJson(doc, jsonString);
  
  localServer.send(200, "application/json", jsonString);
}

// ===== UTILITY FUNCTIONS =====
void handleRemoteCommand(String command) {
  Serial.println("🌐 Remote command received: " + command);
  
  if (command == "LED_ON" || command == "LED_OFF" || command == "LED_TOGGLE" || 
      command == "STATUS" || command == "PING") {
    sendToArduino(command);
  } else if (command == "RESTART_ESP32") {
    Serial.println("🔄 Restarting ESP32...");
    ESP.restart();
  } else if (command == "RECONNECT_WIFI") {
    Serial.println("🔄 Reconnecting WiFi...");
    WiFi.disconnect();
    connectToWiFi();
    sendToArduino(WiFi.status() == WL_CONNECTED ? "ESP32_CONNECTED" : "ESP32_DISCONNECTED");
  } else {
    Serial.println("❓ Unknown remote command: " + command);
  }
}

void printSystemStatus() {
  Serial.println("\n=== System Status ===");
  Serial.println("ESP32 Bridge:");
  Serial.print("  WiFi: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  Serial.print("  IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("  Free Heap: ");
  Serial.println(ESP.getFreeHeap());
  
  Serial.println("Arduino Connection:");
  Serial.print("  Status: ");
  Serial.println(ledStatus.arduinoConnected ? "Connected" : "Disconnected");
  Serial.print("  LED State: ");
  Serial.println(ledStatus.ledState ? "ON" : "OFF");
  Serial.print("  Last Update: ");
  Serial.println(millis() - ledStatus.lastArduinoUpdate);
  
  Serial.println("Server Connection:");
  Serial.print("  Status: ");
  Serial.println(ledStatus.serverConnected ? "Connected" : "Disconnected");
  Serial.println("==================\n");
}

/*
 * ===== EDUCATIONAL NOTES =====
 * 
 * Key IoT Gateway Concepts Demonstrated:
 * 
 * 1. BRIDGING FUNCTION:
 *    - ESP32 acts as a bridge between Arduino (local device) and internet
 *    - Translates between serial communication and HTTP protocols
 *    - Maintains connection status with both endpoints
 * 
 * 2. DUAL COMMUNICATION:
 *    - Serial communication with Arduino (device-to-device)
 *    - HTTP communication with server (device-to-cloud)
 *    - Local web server for direct access
 * 
 * 3. STATE SYNCHRONIZATION:
 *    - Tracks and synchronizes LED state across all components
 *    - Provides real-time status updates
 *    - Handles connection failures gracefully
 * 
 * 4. GATEWAY ARCHITECTURE:
 *    - Demonstrates typical IoT gateway patterns
 *    - Shows how local devices can be internet-enabled
 *    - Illustrates protocol translation and data forwarding
 * 
 * Communication Flow:
 * 
 * Web Browser → ESP32 Local Server → Arduino → LED
 *              ↓
 * Internet Server ← ESP32 HTTP Client ← Status Updates
 * 
 * Extensions for Advanced Learning:
 * - Add command queuing and retry mechanisms
 * - Implement WebSocket for real-time updates
 * - Add security features (authentication, encryption)
 * - Support multiple Arduino devices
 * - Include data buffering and offline operation
 */