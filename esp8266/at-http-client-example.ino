/*
Simple http-client example using AT, connects to http-server and requests /ledon, /ledoff
*/
#include<stdlib.h>
#include <SoftwareSerial.h>
//#include <OneWire.h>
//#include <DallasTemperature.h>
//#define ONE_WIRE_BUS 8
//OneWire oneWire(ONE_WIRE_BUS);
//DallasTemperature sensors(&oneWire);

#define SSID "ESPI"
#define PASS "12345678"
#define IP "192.168.4.1" 
String GET = "GET /ledon HTTP/1.1\r\n";
SoftwareSerial esp8266(3, 4); // RX (aseta tx), TX

int state=-1;
const int STATE_UNKNOWN=-1;
const int STATE_NOT_CONNECTED=0;
const int STATE_CONNECTED=1;
const int STATE_WAITING_FOR_DATA=2;
const int STATE_ESP8266_AVAILABLE=9;




void setup()
{
  pinMode(13, OUTPUT);
    
 
  esp8266.begin(9600);
  Serial.begin(9600);
 
  msgEsp("+++",10000); //close CIPSEND, if its on...  
 
  //msgEsp("AT+RST\r\n",10000);  
  //delay(500);
  msgEsp("ATE1\r\n",10000);      
  msgEsp("AT\r\n",10000);     
  msgEsp("AT+CWMODE_DEF=1\r\n",1000);  
  msgEsp("AT+CIPMUX=0\r\n",1000);  

  
  connectWiFi();

  
   msgEsp("AT+CWJAP?\r\n",10000);  
   msgEsp("AT+CIFSR\r\n",10000);  
   msgEsp("AT+CIPSTATUS\r\n",10000);  
   
   delay(2000);
  
}
int t=-60;
void loop(){
  //sensors.requestTemperatures();
  //float tempC = sensors.getTempCByIndex(0);
  //tempC = DallasTemperature::toFahrenheit(tempC);
  char buffer[10];
  String tempF = dtostrf(t, 3, 1, buffer);
  t++;
  updateTemp(tempF);
  delay(1);
}
int n=0;
void updateTemp(String tenmpF){
  String cmd = "AT+CIPSTART=\"TCP\",\"";   
  cmd += IP;
  cmd += "\",80\r\n";
  msgEsp(cmd,10000);
  if (n==0)
  {
    GET ="GET /ledon HTTP/1.1\r\n";
    n=1;
  }
  else
  {
    GET ="GET /ledoff HTTP/1.1\r\n";
    n=0;
  }
  
  
  cmd = GET;  
  cmd += "\r\n\r\n\0";  
  
  //esp8266.print("AT+CIPSEND=");
  //sprintf(sendcmd,"AT+CIPSEND=%d",cmd.length());
  //esp8266.println(cmd.length());
  //reply = sendCommand(sendcmd,0);
  //sendCommand()
  //msgEsp("AT+CIPMODE=1\r\n",1000);
  //sprintf(sendcmd,"%d",cmd.length());
  //sendCommand(sendcmd,0);
  msgEsp("AT+CIPSEND=" + (String)cmd.length() + "\r\n",5000);
  msgEsp(cmd,5000);  
  msgEsp("+++",5000);
  //msgEsp("AT+CIPCLOSE\r\n",2000);
  delay(1);
}

void processESP8266Message(String message)
{  
  int oldState=state;

  if (state == STATE_UNKNOWN && hasString(message,"OK"))  
    state = STATE_ESP8266_AVAILABLE;
  
    
  if (hasString(message,"no ip") || hasString(message,"WIFI DISCONNECT"))
    state = STATE_NOT_CONNECTED;
      
  if (hasString(message,"GOT IP") || hasString(message,">"))
    state = STATE_CONNECTED;

  if (STATE_CONNECTED && hasString(message,">"))
    state = STATE_WAITING_FOR_DATA;
      

  if (state!=oldState)
  {
    Serial.print("State ");
    Serial.print(oldState);
    Serial.print(" > ");
    Serial.println(state);
  }
  
}
bool hasString(String haystack, String needle)
{
  if (haystack.indexOf(needle)!=-1)
    return true;
  else
    return false;  
}
 
boolean connectWiFi(){  
  msgEsp("AT+CWMODE=1\r\n",1000);  
  msgEsp("AT+CIPSTA_DEF=\"192.168.4.10\",",0);
  msgEsp("\"192.168.4.1\",",0);
  msgEsp("\"255.255.255.0\"\r\n",5000);  

  
  //msgEsp("AT+CWQAP\r\n",5000);
    
  String cmd="AT+CWJAP_DEF=\"";
  cmd+=SSID;
  cmd+="\",\"";
  cmd+=PASS;
  cmd+="\"\r\n";
  msgEsp(cmd,0);      
  delay(15000);
  
  
  //GET IP/status
  msgEsp("AT+CIFSR\r\n",10000);  
}


String msgEsp(String str, const int timeout)
{
  delay(500);
  String content="";
  char character;
  
  Serial.print("SEND ESP: " + str);   
  esp8266.print(str);


  long int time = millis();
  long int receivedTime = 0;
  const int dataWaitTime = 20; //ms
    int n=0;
    bool receivedData=false;
    while( (time+timeout) > millis())
    {
        while(esp8266.available())
        {
          character = esp8266.read();
          content.concat(character);
          if (receivedData == false)                      
            receivedData = true;
          
          receivedTime = millis();
        }
        
        if (esp8266.available()==false && receivedData == true && receivedTime+dataWaitTime < millis())      
        {  
          if (receivedTime+dataWaitTime >= millis())
          {
            Serial.print("ESP8266 TIMEOUT! cmd: ");
            Serial.println(str);
          }
            
            
          break;
        }
        
    }

  if (content != "")
    Serial.println("ESP8266: " + content);

    if (hasString(content,"busy "))
    {
      delay(20);
      Serial.print("RETRYING command: ");
      Serial.println(str);
      return msgEsp(str, timeout);     
    }
   // else if (hasString(content,"ERROR"))
    //  connectWiFi();

    

    return content;
}
