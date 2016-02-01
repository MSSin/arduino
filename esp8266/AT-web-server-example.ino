/* Testing esp8266 + arduino for web-server use..
  Polishing code later, currently it consumes quite lot of memory and is on brink of crashing due memory usage
*/

#include<stdlib.h>
#include <SoftwareSerial.h>

SoftwareSerial monitor(3, 4); // RX (aseta tx), TX

void setup()
{
  pinMode(13, OUTPUT);
//  pinMode(8, OUTPUT);
//  digitalWrite(8,HIGH);

  
  monitor.begin(9600);
  Serial.begin(9600);
  //sendCommand("AT+GSLP=5000",0); 
  //delay(5000);     
  //digitalWrite(8,LOW);
  //delay(1000);
  //digitalWrite(8,HIGH);
  //sensors.begin();
  msgEsp("+++",10000); //close CIPSEND, if its on...  
  msgEsp("AT+RST\r\n",10000);  
  delay(500);
  msgEsp("ATE1\r\n",10000);      
  msgEsp("AT\r\n",10000);      
  msgEsp("AT+CWMODE_CUR=2\r\n",1000);  
  msgEsp("AT+CIPAP_DEF=\"192.168.4.1\",\"192.168.4.1\",\"255.255.255.0\"\r\n",1000);    
  msgEsp("AT+CWSAP_DEF=\"ESPI\",\"12345678\",5,3\r\n",1000);      
  
  msgEsp("AT+CIPMODE=0\r\n",1000);      
  msgEsp("AT+CIPMUX=1\r\n",1000);      
  msgEsp("AT+CIPSERVER=1,80\r\n",1000);      
  msgEsp("AT+CIPCLOSE=5\r\n",1000);
  
  
}
int t=-60;

void loop(){
  String content = "";
  String importantContent = "";
  char character;
  
  //sensors.requestTemperatures();
  //float tempC = sensors.getTempCByIndex(0);
  //tempC = DallasTemperature::toFahrenheit(tempC);
  //char buffer[10];
  content="";
  //String tempF = dtostrf(t, 3, 1, buffer);
  //t++;
  //updateTemp(tempF);
  int connectionId = -1;
  bool readingData=true;
  
  long int receivedTime = 0;
  const int dataWaitTime = 1000; //ms
  bool IPDfound = false;
  bool HTTPfound = false;
  
  while (monitor.available() || receivedTime+dataWaitTime > millis())
  { 
               
    if (monitor.available())                                                     
    {
      character = monitor.read();                          
      content.concat(character);                              
      receivedTime = millis();   

      if (IPDfound == false && hasString(content, "+IPD"))
        IPDfound = true;      
      else if (IPDfound && content.charAt(content.length()) == 0 
     &&    content.charAt(content.length()-2) == 13
     &&    content.charAt(content.length()-3) == 10     
     &&    content.charAt(content.length()-4) == 13)
      {
        Serial.println("RN Breaker");
        break;
      }                       
      if (HTTPfound == false && IPDfound && hasString(content, "HTTP/1.1\r\n"))
      {        
        HTTPfound = true;
        importantContent = content;
      }
    }              
  }
  
  
  if (content != "") {
    //char k;
    //k=content.charAt(content.length()-2);    
    //Serial.println((int)(k));
    //k=content.charAt(content.length()-3);    
    //Serial.println((int)(k));
    if (receivedTime+dataWaitTime <= millis())
      Serial.println("TIMEOUT");
    
    //Serial.print("ESP8266: ");
    //Serial.println(content);
          
    connectionId= (int)importantContent[0]-48;
    Serial.print("Connection Id: ");
    Serial.println(connectionId); 
    
    processESP8266Message(importantContent, connectionId);
  }   
  //delay(10);

}

void processESP8266Message(String message, int connectionId)
{  
  //Serial.println("processESP8266Message");
  //Serial.println(message);
//  delay(1000);


  
  //int getPos = message.indexOf("GET");
  //Serial.println("GET pos: " + (String)getPos);

/*  if (state == STATE_UNKNOWN && hasString(message,"OK"))  
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
  }*/
  Serial.println(message);
  if (hasString(message,"GET / HTTP/1.1"))
  {  
    sendPage(1, connectionId);    
  }
  
  else if (hasString(message,"GET /ledon HTTP/1.1"))
  {
    Serial.println("LED ON");
    digitalWrite(13,HIGH);    
    sendPage(1, connectionId);
  }
  else if (hasString(message,"GET /ledoff HTTP/1.1"))
  {
    Serial.println("LED OFF");
    digitalWrite(13,LOW);
    sendPage(1, connectionId);
  }  
  else
  {
     //sendPage(1, connectionId);
     msgEsp("AT+CIPCLOSE=" + (String)connectionId + "\r\n",1000);     
  }
  
   //sendPage(1, connectionId);
  
}
void sendPage(int pageId, int connectionId)
{
  const char responseHeaders[50]="HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n\0";
  String responseData="";  
  
  responseData+="<html>";
  responseData+="<body>";
  responseData+=getPage(pageId);
  responseData+="</body>";
  responseData+="</html>\r\n";
   
  msgEsp("AT+CIPSENDEX=" + (String)connectionId + "," ,0); //no reply 
  msgEsp((String)(strlen(responseHeaders) + responseData.length()) ,0);  
  msgEsp("\r\n",1000);
    
  //waitSerialData();  
  msgEsp(responseHeaders,0);
    
  //waitSerialData();   
  msgEsp(responseData,0);    
  //waitSerialData();  
  msgEsp("+++",2000);
  //waitSerialData();
  //delay(50);
  msgEsp("AT+CIPCLOSE=" + (String) connectionId + "\r\n",2000);
  //if (connectionId < 0 || connectionId > 5)
    //msgEsp("5",1000);    
  //else
    //msgEsp((String)connectionId,1000);
    
//  msgEsp("\r\n",1000);
  //waitSerialData();

          
}

String msgEsp(String str, const int timeout)
{
  String content="";
  char character;
  
  //Serial.print("SEND ESP: " + str);   
  monitor.print(str);


  long int time = millis();
  long int receivedTime = 0;
  const int dataWaitTime = 20; //ms
    int n=0;
    bool receivedData=false;
    while( (time+timeout) > millis())
    {
        while(monitor.available())
        {
          character = monitor.read();
          content.concat(character);
          if (receivedData == false)                      
            receivedData = true;
          
          receivedTime = millis();
        }
        
        if (monitor.available()==false && receivedData == true && receivedTime+dataWaitTime < millis())      
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

    if (hasString(content,"busy s."))
    {
      delay(20);
      Serial.print("RETRYING command: ");
      Serial.println(str);
      return msgEsp(str, timeout);     
    }

    return content;
}


String getPage(int pageId)
{
  switch(pageId)
  {
    case 1:
      return "<a href='/ledon'>ON<br /><a href='/ledoff'>Off";
      break;
    case 2:
      return "page2";
      break;
    default:
      return "no data";
    break;    
  }
}
bool hasString(String haystack, String needle)
{
  if (haystack.indexOf(needle)!=-1)
    return true;
  else
    return false;  
}
