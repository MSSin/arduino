#include<stdlib.h>
#include <SoftwareSerial.h>

/* Testing esp8266 + arduino for web-server use..
  Polishing code more later, currently it consumes quite lot of memory and is on brink of crashing due memory usage
*/

SoftwareSerial esp8266(3, 4); // RX (aseta tx), TX

void setup()
{
  pinMode(13, OUTPUT);
//  pinMode(8, OUTPUT);
//  digitalWrite(8,HIGH);

  
  esp8266.begin(9600);
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
  const int dataWaitTime = 2000; //ms
  bool IPDfound = false;
  bool HTTPfound = false;
  
  while (esp8266.available() || receivedTime+dataWaitTime > millis())
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
      if (HTTPfound == false && IPDfound && hasString(content, "HTTP/1.1\r\n"))
      {        
        HTTPfound = true;
        importantContent = content;
        connectionId= (int)importantContent[0]-48;
        content="!"; //reset so doesnt consume memory;
      }
      //else if(HTTPfound && importantContent!="")
        
      
    }              
  }
  
  
  if (content != "") {
    if (receivedTime+dataWaitTime <= millis())
    {
      Serial.println("DATA WAIT TIMEOUT");       
      return closeAllConnections();
    }              
    //Serial.println("Connection Id: " + (String)connectionId);           
    processESP8266Message(importantContent, connectionId);
  }     
}

void processESP8266Message(String message, int connectionId)
{   
  Serial.print(message);
  if (hasString(message,"GET / HTTP/1.1"))  
    sendPage(1, connectionId);       
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
     msgEsp("AT+CIPCLOSE=" + (String)connectionId + "\r\n",1000);      
}
void sendPage(int pageId, int connectionId)
{   
  int pageDatas[10];
  int dataSize = 0;
  int i=0,n=0;
  
  pageDatas[i]=20; i++; //headers
  pageDatas[i]=10; i++; //html>
  
  pageDatas[i]=13; i++; //>head<, body>  
  pageDatas[i]=pageId; i++;
  pageDatas[i]=3; i++;
  pageDatas[i]=2; i++;
  pageDatas[i]=12; i++; //>style<
  pageDatas[i]=11; i++; //<body<html
  
  for (n=0;n<i;n++)
    dataSize += getHtmlData(pageDatas[n]).length();   
   
  msgEsp("AT+CIPSENDEX=" + (String)connectionId + "," ,0); //no reply 
  msgEsp((String)dataSize + "\r\n" ,1000);    
    
  for (n=0;n<i;n++)    
    msgEsp(getHtmlData(pageDatas[n]),0);
       
  msgEsp("+++",2000);  
  
  msgEsp("AT+CIPCLOSE=" + (String) connectionId + "\r\n",2000);       
}

String msgEsp(String str, const int timeout)
{
  String content="";
  char character;
  
  //Serial.print("SEND ESP: " + str);   
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


String getHtmlData(int dataId)
{
  switch(dataId)
  {
    case 1: return "<a href='/ledon'>ON</a>";      
    case 2: return "<a href='/ledoff'>Off</a>";
    case 3: return "<hr />";
    case 10: return "<html>";      
    case 11: return "</body></html>\r\n";      
    case 12: return "<style>a{display:inline-block;height:50px;width:200px;font-size:30px;}</style>";
    case 13: return "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, maximum-scale=1\"></head><body>";
    case 20: return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n\0";
    default: return "no data";    
  }
}
void closeAllConnections()
{  
  msgEsp("AT+CIPCLOSE=5\r\n",5000);
}
bool hasString(String haystack, String needle)
{
  if (haystack.indexOf(needle)!=-1)
    return true;
  else
    return false;  
}
