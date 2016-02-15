#include<stdlib.h>
#include <SoftwareSerial.h>

/* Testing esp8266 + arduino for remote control
 This is tcp server, which has physical buttons for remote controlling client which connects to this server.
*/

SoftwareSerial esp8266(3, 4); // RX (aseta tx), TX
byte FORWARD=2;
byte RIGHT=9;
byte LEFT=8;
byte BACKWARD=7;

void setup()
{
  pinMode(FORWARD,INPUT_PULLUP);  
  pinMode(RIGHT,INPUT_PULLUP);
  pinMode(LEFT,INPUT_PULLUP);
  pinMode(BACKWARD,INPUT_PULLUP);
  pinMode(13, OUTPUT);
      
  esp8266.begin(9600);
  Serial.begin(9600);
  Serial.println("Starting up..");  
  
  msgEsp("+++",10000); //close CIPSEND, if its on...  
  msgEsp("AT+RST\r\n",10000);  
  delay(500);
  msgEsp("ATE1\r\n",10000);      
  msgEsp("AT\r\n",10000);      
  msgEsp("AT+CWMODE_CUR=2\r\n",1000);  
  //msgEsp("AT+CIPAP_DEF=\"192.168.4.1\",\"192.168.4.1\",\"255.255.255.0\"\r\n",5000);      
  msgEsp("AT+CWDHCP_DEF=0,1\r\n",1000); 
  msgEsp("AT+CWSAP_DEF=\"ESPI2\",\"12345678\",5,3\r\n",1000);      
  msgEsp("AT+CIFSR\r\n",10000);  
  msgEsp("AT+CIPMODE=0\r\n",1000);      
  msgEsp("AT+CIPMUX=1\r\n",1000);      
  msgEsp("AT+CIPSERVER=1,5000\r\n",1000);      
  msgEsp("AT+CIPCLOSE=5\r\n",1000);   
}

int connectionId = -1;
int lastAct=-1;
void loop(){
  String content = "";
  String reply="";
  String importantContent = "";
  char character;   
  content="";  
  
  bool readingData=true;
  
  long int receivedTime = 0;
  const int dataWaitTime = 1000; //ms
  bool IPDfound = false;  

  if (connectionId != -1)
  {
    if (!digitalRead(FORWARD))
      reply=sendAct(1);
    else if (!digitalRead(LEFT))
      reply=sendAct(4);
    else if (!digitalRead(RIGHT))
      reply=sendAct(2);
    else if (!digitalRead(BACKWARD))
      reply=sendAct(3);
    else
      reply=sendAct(0);
    
    if (hasString(reply,"link is") || 
    hasString(reply,"ERROR"))
    {
      connectionId=-1;
      msgEsp("AT+CIPCLOSE=5\r\n",1000);  
    }
    
    delay(100);
  }
    
  while (esp8266.available())
  {   
    if (esp8266.available())                                                     
    {
      character = esp8266.read();                          
      content.concat(character);                              
      receivedTime = millis();   

      if (IPDfound == false && hasString(content, "+IPD"))
        IPDfound = true;      
      else if (IPDfound && content.charAt(content.length()) == 0 
     &&    content.charAt(content.length()-2) == 13
     &&    content.charAt(content.length()-3) == 10     
     &&    content.charAt(content.length()-4) == 13)
      {
        //Serial.println("RN Breaker");
        break;
      }                       
      if (IPDfound)
      {                
        importantContent = content;
        connectionId= 0;
        Serial.print("ConnectionId:");
        Serial.println(connectionId);
        content="!"; //reset so doesnt consume memory;
      }                   
    }              
  }
  
  
  if (content != "") {
    if (receivedTime+dataWaitTime <= millis())
    {
      Serial.println("DATA WAIT TIMEOUT");             
    }              
    //Serial.println("Connection Id: " + (String)connectionId);           
    //processESP8266Message(importantContent, connectionId);
  }     
  delay(10);
}

String sendAct(int action)
{   
  String message="";
   
  if (lastAct==action)  
    return "";
  else
    lastAct=action;
     
  Serial.print("SendAct:");
  Serial.println(action);
  int dataSize = 0;
  String actionStr=(String)action + "@";
    dataSize += actionStr.length();   
   
  message+=msgEsp("AT+CIPSENDEX=" + (String)connectionId + "," ,0); //no reply 
  message+=msgEsp((String)dataSize + "\r\n" ,1000);    
         
  message+=msgEsp(actionStr,0);

  return message;
}

String msgEsp(String str, const int timeout)
{
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

    return content;
}


bool hasString(String haystack, String needle)
{
  if (haystack.indexOf(needle)!=-1)
    return true;
  else
    return false;  
}
