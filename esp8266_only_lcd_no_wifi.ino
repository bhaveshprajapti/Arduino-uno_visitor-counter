#include <Wire.h>
#include <LiquidCrystal_I2C.h>  // Include the LiquidCrystal library

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

unsigned long lastActionTime = 0;      // Time of the last entry/exit action

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
  delay(1000);
  lcd.clear();
  updateLCD();
}

void loop() {
  unsigned long currentTime = millis();

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

    // Update LCD if count changes
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