#include <Ticker.h> // Ticker can periodically call a function
Ticker blinker; // Ticker object called blinker.

int ledPin = 5; // LED is attached to ESP8266 pin 5.
uint8_t ledStatus = 0; // Flag to keep track of LED on/off status
int counter = 0; // Count how many times we've blinked
#include "ESP8266WiFi.h"
#include <ESP8266WebServer.h>
const char* host = "api.thingspeak.com"; // Your domain  
String ApiKey = "Y2VHLZJC7RP0KUQS&test123";
String path = "/update?key=" + ApiKey + "&field1="; 

const char* ssid = "***";
const char* pass = "****";

void setup() 
{
  Serial.begin(115200);
  Serial.println("Starting..");
  pinMode(ledPin, OUTPUT); // Set LED pin (5) as an output
  blinker.attach(0.25, blink); // Ticker on blink function, call every 250ms
  WiFi.begin(ssid, pass);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

   Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}
char temperatureString[6];
float temp=24.00;
void loop() 
{
  temp+=counter;
  dtostrf(24.00, 2, 2, temperatureString);
  // send temperature to the serial console
  Serial.println(temperatureString);
  
WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
   client.print(String("GET ") + path + temperatureString + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: keep-alive\r\n\r\n");
  delay(1000);
  ESP.deepSleep(60000000, WAKE_RF_DEFAULT); // Sleep for 60 min
  
  
  
}

void blink()
{
  Serial.print("Blink: ");
  Serial.println(counter);
  
  if (ledStatus)
    digitalWrite(ledPin, HIGH);
  else
    digitalWrite(ledPin, LOW);
  ledStatus = (ledStatus + 1) % 2; // Flip ledStatus

  
  if (counter++ > 60) // If we've blinked 60 times
  {
    Serial.println("Going sleep");
    ESP.deepSleep(20000000, WAKE_RF_DEFAULT); // Sleep for 5min
  }
}
