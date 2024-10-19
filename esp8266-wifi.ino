#define BLYNK_TEMPLATE_ID "TMPL3ZcRJzz-E"
#define BLYNK_TEMPLATE_NAME "visitors counter "

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Replace with your network credentials
char ssid[] = "oneplus";
char pass[] = "oneplus12";

// Auth Token from the Blynk App
char auth[] = "LT6PDMxuXGdS-Sfp0PpGyhgIoS5bbaLN";

int messageCount = 0; // Variable to keep track of message count

void setup()
{
  // Debug console
  Serial.begin(115200);

  // Connect to Blynk server
  Blynk.begin(auth, ssid, pass);
}

void loop()
{
  Blynk.run();
  sendData();
  delay(5000); // Send data every 5 seconds
}

void sendData() {
  // Create a unique message using the counter
  String message = "msg" + String(messageCount);
  
  // Print the message to the serial monitor
  Serial.println(message);
  
  // Send the message to the Blynk app on Virtual Pin V0
  Blynk.virtualWrite(V0, message);
  
  // Increment the message counter for the next message
  messageCount++;
}