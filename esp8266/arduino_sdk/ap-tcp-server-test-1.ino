
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>


const char* ssid = "ESPI2";
const char* pass = "12345678";

WiFiServer server(5000);

boolean alreadyConnected = false;
void setup() {
  Serial.begin(115200);
  //pinMode(D5,OUTPUT);
  pinMode(D4,OUTPUT);  
  //digitalWrite(D5,HIGH);  
  digitalWrite(D4,HIGH);  
  // put your setup code here, to run once:
  WiFi.mode(WIFI_AP);
  //uint8_t mac[WL_MAC_ADDR_LENGTH];
  //WiFi.softAPmacAddress(mac);
  WiFi.softAP(ssid, pass);
 
  // Wait for connection
//  while (WiFi.status() != WL_CONNECTED) {
    //WiFi.begin(ssid, pass);    
    //Serial.print(WiFi.status()); 
    //delay(500);        
  //}
  //digitalWrite(D5,LOW);
  
  //Serial.print(WiFi.status());
  //Serial.println(":status");
  WiFi.printDiag(Serial);
  Serial.println(WiFi.status());    
  
  //Serial.println(WiFi.localIP());
  Serial.println(WiFi.softAPIP());
  server.begin();
  
}
int aLow=999;
int lastAct=-1;
void loop() {  
  int act=-1;
  int a0;
  
  WiFiClient client = server.available();
  // when the client sends the first byte, say hello:
  if (client) {
    digitalWrite(D4,LOW);
     while (client.connected()) {
  
    a0 = analogRead(A0);
    if (aLow>a0)      
      aLow=a0;  
       
       
    if (!alreadyConnected) {
      // clead out the input buffer:
      //client.flush();    
      Serial.println("We have a new client");
      client.println("Hello, client!"); 
      alreadyConnected = true;
    } 

    if (client.available() > 0) {
      // read the bytes incoming from the client:
      char thisChar = client.read();
      // echo the bytes back to the client:
      //server.write(thisChar);
      client.print(thisChar);
      // echo the bytes to the server as well:
      Serial.write(thisChar);
    }

      if (a0>aLow+12) //forward      
        act=1;      
      else if (a0>aLow+9) //forward      
        act=2;      
      else if (a0>aLow+6) //forward      
        act=3;      
      else if (a0>aLow+3) //forward      
        act=4;      
      else
        act = 0;

      if (lastAct!=act)
      {
        Serial.println(a0);
        lastAct=act;
        client.println((String)act + "@");        
        Serial.println("Act: " + (String) act);
      }
      delay(100);
   }
  }
  else
  {
    if (alreadyConnected)
    {
        Serial.println("Client left.");
        alreadyConnected=false;
    }
    digitalWrite(D4,HIGH);
  }
  //delay(100);
}
