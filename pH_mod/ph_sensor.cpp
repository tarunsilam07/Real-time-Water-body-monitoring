#include <WiFi.h> // For ESP32/ESP8266, use <ESP8266WiFi.h> or <WiFi.h> based on your board
#include "ThingSpeak.h"

#define SensorPin A0          // the pH meter Analog output is connected with the Arduino’s Analog

// Wi-Fi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// ThingSpeak settings
unsigned long myChannelNumber = YOUR_CHANNEL_NUMBER; // Replace with your ThingSpeak Channel number
const char * myWriteAPIKey = "YOUR_WRITE_API_KEY";   // Replace with your ThingSpeak Write API Key

WiFiClient client;

unsigned long int avgValue;  // Store the average value of the sensor feedback
float b;
int buf[10], temp;

void setup()
{
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi");
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak
  Serial.println("Ready");
}

void loop()
{
  for (int i = 0; i < 10; i++)       // Get 10 sample values from the sensor to smooth the value
  {
    buf[i] = analogRead(SensorPin);
    delay(10);
  }
  for (int i = 0; i < 9; i++)        // Sort the analog from small to large
  {
    for (int j = i + 1; j < 10; j++)
    {
      if (buf[i] > buf[j])
      {
        temp = buf[i];
        buf[i] = buf[j];
        buf[j] = temp;
      }
    }
  }
  avgValue = 0;
  for (int i = 2; i < 8; i++)                      // Take the average value of 6 center samples
    avgValue += buf[i];
  float phValue = (float)avgValue * 5.0 / 1024 / 6; // Convert the analog into millivolt
  phValue = 3.5 * phValue;                         // Convert the millivolt into pH value

  Serial.print("pH: ");
  Serial.print(phValue, 2);
  Serial.println(" ");

  // Write data to ThingSpeak
  ThingSpeak.setField(1, phValue);  // Send pH value to Field 1
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  
  if (x == 200) {
    Serial.println("Data sent to ThingSpeak");
  } else {
    Serial.println("Problem sending data to ThingSpeak");
  }

  digitalWrite(13, HIGH);
  delay(800);
  digitalWrite(13, LOW);
  delay(20000);  // Wait 20 seconds to match ThingSpeak update rate limit
}