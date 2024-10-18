#include <Wire.h>
#include <LiquidCrystal_I2C.h>

const int numSlaves = 3;                    // Number of slave devices
const int slaveAddresses[numSlaves] = {8, 9, 10}; // Addresses of your slaves
int totalCount = 0;                         // Variable to store the final total count
int prevTotalCount = -1;                    // Variable to track the previous total count

// Create an instance of the LCD (address 0x27 is common; change if needed)
LiquidCrystal_I2C lcd(0x27, 16, 2); // 16 columns and 2 rows

void setup() {
  Wire.begin();                            // Join I2C bus as master
  lcd.begin();                             // Initialize the LCD
  lcd.backlight();                         // Turn on the backlight
  lcd.print("Total Count:");               // Initial message
}

void loop() {
  // Loop through each slave and request their status
  for (int i = 0; i < numSlaves; i++) {
    Wire.requestFrom(slaveAddresses[i], 1);  // Request 1 byte from each slave

    if (Wire.available()) {
      int responseFromSlave = Wire.read();    // Read the response from the slave

      // Update totalCount based on the response
      if (responseFromSlave == 1) {
        totalCount++;  // Entry completed
      } else if (responseFromSlave == 2) {
        if (totalCount > 0) {
          totalCount--;  // Exit completed, only if totalCount is greater than 0
        }
      }
      // Response 0 indicates no action, so we do nothing
    }
  }

  // Only update the display if the totalCount has changed
  if (totalCount != prevTotalCount) {
    lcd.setCursor(0, 1);                     // Set cursor to the second line
    lcd.print("Count: ");                     // Print label
    lcd.print(totalCount);                    // Print the current count
    prevTotalCount = totalCount;              // Update the previous total count to the new one
  }

  delay(1000);  // Wait for 1 second before requesting again
}
