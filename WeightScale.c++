/*
 * ESP32-C3 Weight Scale with MQTT
 * Sends weight data to Raspberry Pi via MQTT
 * 
 * Hardware Connections:
 * HX711: DOUT->GPIO4, SCK->GPIO5
 * LCD I2C: SDA->GPIO8, SCL->GPIO9
 * Load Cell: Connect to HX711
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// Pin definitions
#define HX711_DOUT_PIN 4
#define HX711_SCK_PIN  5
#define SDA_PIN        8
#define SCL_PIN        9

// WiFi credentials - UPDATE THESE
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// MQTT settings - UPDATE RASPBERRY PI IP
const char* mqtt_server = "192.168.1.100";  // Your Raspberry Pi IP
const int mqtt_port = 1883;
const char* mqtt_topic = "scale/weight";
const char* mqtt_status_topic = "scale/status";
const char* device_id = "ESP32_Scale_01";

// Hardware setup
LiquidCrystal_I2C lcd(0x27, 20, 4);
HX711 scale;
WiFiClient espClient;
PubSubClient client(espClient);

// Settings
float calibration_factor = -7050.0;
float weight = 0.0;
float last_sent_weight = 0.0;
unsigned long last_mqtt_send = 0;
const unsigned long mqtt_interval = 2000; // Send every 2 seconds
bool wifi_connected = false;
bool mqtt_connected = false;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // Initialize I2C and LCD
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
  
  // Welcome message
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("SMART SCALE");
  lcd.setCursor(3, 1);
  lcd.print("Connecting WiFi");
  
  // Connect to WiFi
  connectWiFi();
  
  // Setup MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(mqttCallback);
  
  // Initialize scale
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare();
  
  // Ready message
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("SCALE READY");
  lcd.setCursor(4, 1);
  lcd.print("MQTT: ");
  lcd.print(mqtt_connected ? "OK" : "FAIL");
  delay(2000);
  lcd.clear();
  
  Serial.println("Scale ready with MQTT!");
}

void loop() {
  // Maintain MQTT connection
  if (!client.connected()) {
    reconnectMQTT();
  }
  client.loop();
  
  // Check for serial commands
  if (Serial.available()) {
    String command = Serial.readString();
    command.trim();
    
    if (command == "tare") {
      scale.tare();
      lcd.clear();
      lcd.setCursor(3, 1);
      lcd.print("SCALE ZEROED");
      delay(1000);
      lcd.clear();
      
      // Send tare event via MQTT
      sendStatusMessage("tared");
    }
  }
  
  // Read weight
  if (scale.is_ready()) {
    weight = scale.get_units(3);
    
    // Display on LCD
    updateDisplay();
    
    // Send weight via MQTT (every 2 seconds or significant change)
    if (millis() - last_mqtt_send > mqtt_interval || 
        abs(weight - last_sent_weight) > 0.1) {
      
      sendWeightData();
      last_mqtt_send = millis();
      last_sent_weight = weight;
    }
    
    // Print to serial
    Serial.print("Weight: ");
    Serial.print(weight, 2);
    Serial.println(" kg");
  }
  
  delay(100);
}

void connectWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    wifi_connected = true;
    Serial.println();
    Serial.print("WiFi connected! IP: ");
    Serial.println(WiFi.localIP());
    
    lcd.setCursor(0, 2);
    lcd.print("WiFi: ");
    lcd.print(WiFi.localIP());
  } else {
    wifi_connected = false;
    Serial.println("WiFi connection failed!");
    lcd.setCursor(0, 2);
    lcd.print("WiFi: FAILED");
  }
}

void reconnectMQTT() {
  while (!client.connected() && wifi_connected) {
    Serial.print("Connecting to MQTT...");
    
    if (client.connect(device_id)) {
      Serial.println("connected");
      mqtt_connected = true;
      
      // Send startup message
      sendStatusMessage("online");
      
      // Subscribe to commands
      client.subscribe("scale/commands");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      mqtt_connected = false;
      delay(5000);
    }
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("MQTT received: ");
  Serial.println(message);
  
  // Handle commands from Raspberry Pi
  if (String(topic) == "scale/commands") {
    if (message == "tare") {
      scale.tare();
      lcd.clear();
      lcd.setCursor(2, 1);
      lcd.print("REMOTE TARE");
      delay(1000);
      lcd.clear();
      sendStatusMessage("tared");
    }
  }
}

void sendWeightData() {
  if (!mqtt_connected) return;
  
  // Create JSON payload
  StaticJsonDocument<200> doc;
  doc["device_id"] = device_id;
  doc["weight"] = weight;
  doc["unit"] = "kg";
  doc["timestamp"] = millis();
  doc["status"] = scale.is_ready() ? "ready" : "error";
  
  char buffer[256];
  serializeJson(doc, buffer);
  
  // Publish to MQTT
  if (client.publish(mqtt_topic, buffer)) {
    Serial.println("Weight data sent via MQTT");
  } else {
    Serial.println("Failed to send MQTT data");
  }
}

void sendStatusMessage(String status) {
  if (!mqtt_connected) return;
  
  StaticJsonDocument<150> doc;
  doc["device_id"] = device_id;
  doc["status"] = status;
  doc["timestamp"] = millis();
  
  char buffer[200];
  serializeJson(doc, buffer);
  
  client.publish(mqtt_status_topic, buffer);
}

void updateDisplay() {
  // Line 1: Title with connection status
  lcd.setCursor(0, 0);
  lcd.print("SMART SCALE ");
  if (mqtt_connected) {
    lcd.print("MQTT");
  } else if (wifi_connected) {
    lcd.print("WiFi");
  } else {
    lcd.print("OFF ");
  }
  
  // Line 2: Weight
  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.setCursor(0, 1);
  
  if (abs(weight) < 0.01) {
    lcd.setCursor(6, 1);
    lcd.print("0.00 kg");
  } else {
    lcd.setCursor(3, 1);
    lcd.print(weight, 2);
    lcd.print(" kg");
  }
  
  // Line 3: Status
  lcd.setCursor(0, 2);
  if (mqtt_connected) {
    lcd.print("Sending to Pi...   ");
  } else {
    lcd.print("Local mode only    ");
  }
  
  // Line 4: Controls
  lcd.setCursor(0, 3);
  lcd.print("'tare' to zero     ");
}

