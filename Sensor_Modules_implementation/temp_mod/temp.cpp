#include <DHT.h>  // Including library for DHT
#include <ESP8266WiFi.h>

// ThingSpeak API key and WiFi credentials
String apiKey = "API KEY";     // Enter your Write API key from ThingSpeak
const char *ssid =  "WIFI_NAME";            // Replace with your WiFi SSID
const char *pass =  "12345678";        // Replace with your WiFi password
const char* server = "api.thingspeak.com";

// DHT sensor configuration
#define DHTPIN 2        // D4 pin on ESP8266 corresponds to GPIO 2
#define DHTTYPE DHT11  
DHT dht(DHTPIN, DHTTYPE);

// WiFi client
WiFiClient client;

void setup() 
{
    Serial.begin(115200);
    delay(10);
    dht.begin();

    Serial.println("Connecting to ");
    Serial.println(ssid);

    // Connect to WiFi
    WiFi.begin(ssid, pass);

    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
}

void loop() 
{
    delay(2000); // Wait for sensor to stabilize
    float h = dht.readHumidity();
    
    // Check if any reads failed and exit early (to try again later)
    if (isnan(h)) 
    {
        Serial.println("Failed to read from DHT sensor!");
        return;
    }

    // Connect to ThingSpeak and send data
    if (client.connect(server, 80))  
    {  
        Serial.println("Connected to ThingSpeak");

        String postStr = apiKey;
        postStr +="&field1=";
        postStr += String(h);
        postStr += "\r\n\r\n";

        client.print("POST /update HTTP/1.1\n");
        client.print("Host: api.thingspeak.com\n");
        client.print("Connection: close\n");
        client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
        client.print("Content-Type: application/x-www-form-urlencoded\n");
        client.print("Content-Length: ");
        client.print(postStr.length());
        client.print("\n\n");
        client.print(postStr);

        Serial.print("Humidity: ");
        Serial.print(h);
        Serial.println(" %. Sent to ThingSpeak.");
    } 
    else 
    {
        Serial.println("Connection to ThingSpeak failed");
    }
    
    client.stop();
    Serial.println("Waiting...");
  
    // ThingSpeak requires a minimum 15 seconds delay between updates
    delay(15000); 
}
