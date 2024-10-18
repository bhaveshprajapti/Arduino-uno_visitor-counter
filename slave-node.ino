#include <Wire.h>

const int irPin1 = 7;   // Entry IR sensor output
const int irPin2 = 8;   // Exit IR sensor output

int slaveCount = 0;           // Counter for the number of people inside
bool sensor1Triggered = false;
bool sensor2Triggered = false;
unsigned long sensor1TriggerTime = 0;
unsigned long sensor2TriggerTime = 0;
const unsigned long maxPassingTime = 10000; // 10 seconds max to pass through the gate
const unsigned long cooldownTime = 1000;    // 1 second cooldown period

unsigned long lastActionTime = 0; // Time of the last entry/exit action
const int slaveAddress = 8;        // I2C address for this slave
int lastActionResponse = 0;         // Store last action response for the master

void setup() {
  Serial.begin(9600);                // Initialize Serial communication
  Wire.begin(slaveAddress);          // Join I2C bus with the slave address
  Wire.onRequest(requestEvent);      // Register event to be called when requested by master
  pinMode(irPin1, INPUT);
  pinMode(irPin2, INPUT);
}

void loop() {
  int sensor1State = digitalRead(irPin1);
  int sensor2State = digitalRead(irPin2);
  
  unsigned long currentTime = millis();

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
      slaveCount++;
      lastActionResponse = 1; // Send signal for entry completed
    } else {
      slaveCount--;
      lastActionResponse = 2; // Send signal for exit completed
      
    }
    
    // Update last action time and reset triggers
    lastActionTime = currentTime;
    sensor1Triggered = false;
    sensor2Triggered = false;

    // Print the current count to Serial Monitor
    Serial.print("Current Count: ");
    Serial.println(slaveCount);
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

// This function is called when the master requests data
void requestEvent() {
  Wire.write(lastActionResponse);  // Send the last action response to the master
  lastActionResponse = 0;           // Reset response after sending
}