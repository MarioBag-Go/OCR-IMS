/*
 * Simple ESP32-C3 Weight Scale
 * 
 * Hardware Connections:
 * HX711: DOUT->GPIO4, SCK->GPIO5
 * LCD I2C: SDA->GPIO8, SCL->GPIO9
 * Load Cell: Connect to HX711
 */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <HX711.h>

// Pin definitions
#define HX711_DOUT_PIN 4
#define HX711_SCK_PIN  5
#define SDA_PIN        8
#define SCL_PIN        9

// Hardware setup
LiquidCrystal_I2C lcd(0x27, 20, 4);
HX711 scale;

// Settings
float calibration_factor = -7050.0;
float weight = 0.0;

void setup() {
  Serial.begin(115200);
  
  // Initialize I2C and LCD
  Wire.begin(SDA_PIN, SCL_PIN);
  lcd.init();
  lcd.backlight();
  
  // Initialize scale
  scale.begin(HX711_DOUT_PIN, HX711_SCK_PIN);
  scale.set_scale(calibration_factor);
  scale.tare();
  
  // Welcome message
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("SCALE");
  lcd.setCursor(5, 1);
  lcd.print("READY");
  delay(2000);
  lcd.clear();
  
  Serial.println("Scale ready!");
}

void loop() {
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
    }
  }
  
  // Read weight
  if (scale.is_ready()) {
    weight = scale.get_units(3);
    
    // Display on LCD
    lcd.setCursor(0, 0);
    lcd.print("    WEIGHT SCALE    ");
    
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
    
    lcd.setCursor(0, 3);
    lcd.print("Type 'tare' to zero");
    
    // Print to serial
    Serial.print("Weight: ");
    Serial.print(weight, 2);
    Serial.println(" kg");
  }
  
  delay(500);
}

