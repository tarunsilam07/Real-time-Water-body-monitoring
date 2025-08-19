#include <SoftwareSerial.h>

// Create serial connections for ESP8266 (RX=2, TX=3)
SoftwareSerial esp8266(2, 3);

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Server details
const char* host = "44.207.212.117";
const int port = 3000;

// Example sensor pins
#define TEMP_PIN A0
#define PH_PIN A1
#define COND_PIN A2
#define DO_PIN A3
#define NITRATE_PIN A4

void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);  // ESP8266 default baud may be 115200, lower if needed

  Serial.println("Resetting ESP8266...");
  sendCommand("AT+RST", 2000);

  // Set ESP to station mode
  sendCommand("AT+CWMODE=1", 1000);

  // Connect to WiFi
  String cmd = "AT+CWJAP=\"" + String(ssid) + "\",\"" + String(password) + "\"";
  sendCommand(cmd.c_str(), 6000);
}

void loop() {
  // === Read sensor values (replace with your actual calibration equations) ===
  int rawTemp = analogRead(TEMP_PIN);
  int rawPH = analogRead(PH_PIN);
  int rawCond = analogRead(COND_PIN);
  int rawDO = analogRead(DO_PIN);
  int rawNitrate = analogRead(NITRATE_PIN);

  // Convert raw values (dummy formulas, replace with calibrated conversions)
  float temperature = (rawTemp / 1023.0) * 50.0;   // Example: 0-50 °C
  float pH = (rawPH / 1023.0) * 14.0;              // Example: 0-14 pH
  int conductivity = map(rawCond, 0, 1023, 0, 2000); // µS/cm
  float oxygen = (rawDO / 1023.0) * 10.0;          // Example: 0-10 mg/L
  int nitrate = map(rawNitrate, 0, 1023, 0, 100);  // Example: 0-100 ppm

  // === Build GET request ===
  String url = "/api/data?temperature=" + String(temperature, 1) +
               "&pH=" + String(pH, 1) +
               "&conductivity=" + String(conductivity) +
               "&oxygen=" + String(oxygen, 1) +
               "&nitrate=" + String(nitrate);

  Serial.println("Requesting URL: " + url);

  // Start TCP connection
  String cmd = "AT+CIPSTART=\"TCP\",\"" + String(host) + "\"," + String(port);
  if (sendCommand(cmd.c_str(), 3000).indexOf("OK") != -1) {
    String httpRequest = "GET " + url + " HTTP/1.1\r\nHost: " + String(host) + "\r\nConnection: close\r\n\r\n";
    String sendCmd = "AT+CIPSEND=" + String(httpRequest.length());

    if (sendCommand(sendCmd.c_str(), 2000).indexOf(">") != -1) {
      esp8266.print(httpRequest);
      Serial.println("Data sent!");
    }
  }

  delay(10000); // send every 10 sec
}

// Utility function to send AT commands
String sendCommand(const char* cmd, const int timeout) {
  String response = "";
  esp8266.println(cmd);
  long int time = millis();
  while ((millis() - time) < timeout) {
    while (esp8266.available()) {
      char c = esp8266.read();
      response += c;
    }
  }
  Serial.println(response);
  return response;
}
