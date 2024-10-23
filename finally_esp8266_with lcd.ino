#define BLYNK_TEMPLATE_ID "TMPL3ZcRJzz-E"
#define BLYNK_TEMPLATE_NAME "Visitors Counter"

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // Include the LiquidCrystal library

// Replace with your network credentials
char ssid[] = "oneplus";
char pass[] = "oneplus12";

// Auth Token from the Blynk App
char auth[] = "LT6PDMxuXGdS-Sfp0PpGyhgIoS5bbaLN";

// Define IR sensor pins
const int irPin1 = 5;   // Entry IR sensor output (D1)
const int irPin2 = 4;   // Exit IR sensor output (D2)

// Counters
int peopleCount = 0;        // Current count of people inside
int totalIn = 0;            // Total entries
int totalOut = 0;           // Total exits

// Flags and timing
bool sensor1Triggered = false;
bool sensor2Triggered = false;
unsigned long sensor1TriggerTime = 0;
unsigned long sensor2TriggerTime = 0;
const unsigned long maxPassingTime = 10000; // 10 seconds max to pass through the gate
const unsigned long cooldownTime = 1000;    // 1 second cooldown period
const unsigned long wifiTimeout = 1000;    // 10 seconds max to connect to Wi-Fi
const unsigned long wifiRetryInterval = 30000; // Retry Wi-Fi every 60 seconds

unsigned long lastActionTime = 0;      // Time of the last entry/exit action
unsigned long lastWiFiCheckTime = 0;   // Time of the last Wi-Fi check

// Track Wi-Fi and Blynk connection states
bool wifiConnected = false;
bool blynkConnected = false;

// LCD object
LiquidCrystal_I2C lcd(0x27, 16, 2); // Adjust the address if necessary

void setup() {
  Serial.begin(9600);    // Initialize Serial communication
  pinMode(irPin1, INPUT);
  pinMode(irPin2, INPUT);

  // Initialize I2C (LCD) on D3 (GPIO 0) and D4 (GPIO 2)
  Wire.begin(0, 2); // SDA on D3 (GPIO 0), SCL on D4 (GPIO 2)
  lcd.begin();
  lcd.backlight();
  lcd.print("Visitor Counter");
  delay(2000);
  lcd.clear();

  connectToWiFi();
}

void loop() {
  unsigned long currentTime = millis();
  
  // Handle Wi-Fi reconnection attempts every wifiRetryInterval (e.g., 60 seconds)
  if (!wifiConnected && currentTime - lastWiFiCheckTime >= wifiRetryInterval) {
    connectToWiFi();
    lastWiFiCheckTime = currentTime;
  }

  // Handle sensors and counting logic
  int sensor1State = digitalRead(irPin1);
  int sensor2State = digitalRead(irPin2);

  // Check if enough time has passed since the last action
  if (currentTime - lastActionTime < cooldownTime) {
    return; // Skip processing if within cooldown period
  }

  // Check for Sensor 1 (Entry) trigger
  if (sensor1State == LOW && !sensor1Triggered) {
    sensor1Triggered = true;
    sensor1TriggerTime = currentTime;
  }

  // Check for Sensor 2 (Exit) trigger
  if (sensor2State == LOW && !sensor2Triggered) {
    sensor2Triggered = true;
    sensor2TriggerTime = currentTime;
  }

  // Check if both sensors have been triggered
  if (sensor1Triggered && sensor2Triggered) {
    // Determine direction based on which sensor was triggered first
    if (sensor1TriggerTime < sensor2TriggerTime) {
      // Entry detected
      peopleCount++;
      totalIn++;
      Serial.println("Entry detected");
    } else {
      // Exit detected
      peopleCount--;
      totalOut++;
      Serial.println("Exit detected");
    }

    // Update last action time and reset triggers
    lastActionTime = currentTime;
    sensor1Triggered = false;
    sensor2Triggered = false;

    // Update Blynk and LCD if count changes
    sendData();
    updateLCD();
  }

  // Reset triggers if too much time has passed
  if (sensor1Triggered && (currentTime - sensor1TriggerTime > maxPassingTime)) {
    Serial.println("Sensor 1 timeout");
    sensor1Triggered = false;
  }
  if (sensor2Triggered && (currentTime - sensor2TriggerTime > maxPassingTime)) {
    Serial.println("Sensor 2 timeout");
    sensor2Triggered = false;
  }

  // Run Blynk only if Wi-Fi and Blynk are connected
  if (wifiConnected && blynkConnected) {
    Blynk.run();
  }
}

void connectToWiFi() {
  // Attempt to connect to Wi-Fi
  Serial.println("Attempting to connect to Wi-Fi...");
  WiFi.begin(ssid, pass);
  unsigned long startAttemptTime = millis(); // Record the start time
  lcd.clear();
  lcd.print("Connecting WiFi");
  
  // Attempt to connect for wifiTimeout duration
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
    delay(1000);
    Serial.print(".");
  }

  // Check Wi-Fi status
  if (WiFi.status() == WL_CONNECTED) {
    wifiConnected = true;
    Serial.println("Wi-Fi Connected!");
    lcd.clear();
//    lcd.print("WiFi Connected");
    delay(500);
    updateLCD();

    // Attempt to connect to Blynk
    Blynk.config(auth);
    if (Blynk.connect()) {
      blynkConnected = true;
      Serial.println("Blynk Connected!");
    } else {
      Serial.println("Failed to connect to Blynk");
    }
  } else {
    wifiConnected = false;
    Serial.println("Failed to connect to Wi-Fi. Entering standalone mode.");
    lcd.clear();
//    lcd.print("No WiFi.");
    delay(500);
    updateLCD();
  }
}

void sendData() {
  // Create a unique message using the counter
  String message = "In: " + String(totalIn) + " Out: " + String(totalOut)+ " Total: " + String(peopleCount);
  
  // Print the message to the serial monitor
  Serial.println(message);
  
  // Send the message to the Blynk app on Virtual Pin V0 only if connected
  if (wifiConnected && blynkConnected) {
    Blynk.virtualWrite(V0, message.c_str());
  }
}

void updateLCD() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("In: ");
  lcd.print(totalIn);
  lcd.print(" Out: ");
  lcd.print(totalOut);

  lcd.setCursor(0, 1);
  lcd.print("Total: ");
  lcd.print(peopleCount);
}
