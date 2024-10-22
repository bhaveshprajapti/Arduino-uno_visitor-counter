// #testing remaining

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

const int numSlaves = 3;                           // Number of slave devices
const int slaveAddresses[numSlaves] = {8, 9, 10}; // Addresses of your slaves

long totalIn = 0;                                  // Total number of entries
long totalOut = 0;                                 // Total number of exits
long currentCount = 0;                             // Current number of people in the room
long prevCurrentCount = -1;                        // Variable to track the previous count

const int totalInAddress = 0;                      // EEPROM address for totalIn
const int totalOutAddress = sizeof(long);          // EEPROM address for totalOut (after totalIn)

// Create an instance of the LCD (address 0x27 is common; change if needed)
LiquidCrystal_I2C lcd(0x27, 16, 2); // 16 columns and 2 rows

void setup() {
  Wire.begin();                            // Join I2C bus as master
  lcd.begin();                             // Initialize the LCD
  lcd.backlight();                         // Turn on the backlight
  lcd.print("Visitor Counter");             // Initial message

  // Read saved counts from EEPROM
  EEPROM.get(totalInAddress, totalIn);
  EEPROM.get(totalOutAddress, totalOut);
}

void loop() {
  // Loop through each slave and request their status
  for (int i = 0; i < numSlaves; i++) {
    Wire.requestFrom(slaveAddresses[i], 1);  // Request 1 byte from each slave

    if (Wire.available()) {
      int responseFromSlave = Wire.read();    // Read the response from the slave

      // Update counts based on the response
      if (responseFromSlave == 1) {
        currentCount++;  // Entry completed
        totalIn++;       // Increment total entries
        EEPROM.put(totalInAddress, totalIn); // Save to EEPROM
      } else if (responseFromSlave == 2) {
        if (currentCount > 0) {
          currentCount--;  // Exit completed, only if currentCount is greater than 0
          totalOut++;      // Increment total exits
          EEPROM.put(totalOutAddress, totalOut); // Save to EEPROM
        }
      }
      // Response 0 indicates no action, so we do nothing
    }
  }

  // Only update the display if the currentCount has changed
  if (currentCount != prevCurrentCount) {
    lcd.clear();                             // Clear previous display
    lcd.setCursor(0, 0);                    // Set cursor to the first line
    lcd.print("In: ");                       // Print "In" label
    lcd.print(totalIn);                      // Print total entries
    lcd.print(" Out: ");                     // Print "Out" label
    lcd.print(totalOut);                     // Print total exits

    lcd.setCursor(0, 1);                    // Set cursor to the second line
    lcd.print("Cur: ");                      // Print "Cur" label for current count
    lcd.print(currentCount);                 // Print current count
    
    prevCurrentCount = currentCount;         // Update the previous current count
  }

  delay(1000);  // Wait for 1 second before requesting again
}
