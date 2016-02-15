#include <SoftwareSerial.h>
SoftwareSerial esp8266(4, 5); // RX (aseta tx), TX

int motorA = 12;
int motorAbrake=9;
int motorAspeed=3;
int motorB = 13;
int motorBbrake = 8;
int motorBspeed=11;


#define SSID "ESPI2"
#define PASS "12345678"
#define IP "192.168.4.1" 



bool connectedToServer = false;
bool debug=true;

void setup() {   
    
  delay(500); //wait for esp8266 power-up
 
  esp8266.begin(9600);
  if (debug)
  {
    Serial.begin(9600);
    Serial.print("Starting...\n");
  }
 
  //msgEsp("+++",10000); //close CIPSEND, if its on...  
  //msgEsp("AT+CIPCLOSE\r\n",10000);
  //msgEsp("AT+RST\r\n",10000);  
  //delay(500);
  //msgEsp("ATE0\r\n",10000);          
  
  msgEsp("AT+CWMODE_DEF=1\r\n",1000);  
  msgEsp("AT+CIPMUX=0\r\n",1000);  
  

    //Setup Channel A
  pinMode(motorA, OUTPUT); //Initiates Motor Channel A pin
  pinMode(motorAbrake, OUTPUT); //Initiates Brake Channel A pin

  pinMode(motorB, OUTPUT); //Initiates Motor Channel B pin
  pinMode(motorBbrake, OUTPUT); //Initiates Brake Channel B pin


  turnOffEngines();  
  checkConnection();
}
void checkConnection()
{
  if (hasString(msgEsp("AT+CIPSTATUS\r\n",500),"+CIPSTATUS:0,"))
    connectedToServer=true;
  else
    connectedToServer=false;
}
void loop(){
  String data="";  
  
  if (!connectedToServer)
    connectServer();
  else
  {     
    data = readEsp(2000);
    if (hasString(data,"0@"))
      turnOffEngines();
    else if (hasString(data,"1@"))
      moveForward(140);
    else if (hasString(data,"2@"))
      turnRight(110);
    else if (hasString(data,"3@"))
      moveBackward(120);
    else if (hasString(data,"4@"))
      turnLeft(110);  
    else
      checkConnection();
  }
  
}

void connectServer(){
  if (debug)
    Serial.println("Connecting to server");

  turnOffEngines();
  connectWiFi();
  
  String cmd = "AT+CIPSTART=\"TCP\",\"";   
  cmd += IP;
  cmd += "\",5000\r\n";
  msgEsp(cmd,10000); 
  connectedToServer=true;
     
 int dataSize = 0;
 String actionStr=F("hello@");
 dataSize += actionStr.length();   
   
  msgEsp("AT+CIPSENDEX=" ,0); //no reply 
  msgEsp((String)dataSize + "\r\n" ,1000);    
         
  msgEsp(actionStr,0);

  
}

boolean connectWiFi(){  
  msgEsp("AT+CWMODE=1\r\n",1000);  
  msgEsp("AT+CWDHCP_DEF=1,1\r\n",1000);  
  //msgEsp("AT+CIPSTA_DEF=\"192.168.4.5\",\"192.168.4.1\",\"255.255.255.0\"",1000);
  
  //msgEsp("AT+CIPSTA_DEF=\"192.168.4.110\",\"192.168.4.1\",\"255.255.255.0\"\r\n",5000);
  //msgEsp("\"192.168.4.1\",",100);
  //msgEsp("\"255.255.255.0\"\r\n",5000);  

  
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
  String content="";
  char character;

  if (debug)
    Serial.print("SEND ESP: " + str);   
    
  esp8266.print(str);


  long int time = millis();
  long int receivedTime = 0;
  const int dataWaitTime = 40; //ms
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
            if (debug)
            {
              Serial.print("ESP8266 TIMEOUT! cmd: ");
              Serial.println(str);
            }
          }
            
            
          break;
        }
        
    }

  if (content != "")
  {
    if (debug)
      Serial.println("ESP8266: " + content);

    if (hasString(content,"busy "))
    {
      delay(20);
      
      if (debug)
      {
        Serial.print("RETRYING command: ");
        Serial.println(str);
      }
      
      return msgEsp(str, timeout);     
    }
    else if (hasString(content,"ERROR")
    && hasString(content,"ALREADY CONNECTED") == false
    && hasString(content, "no tail") == false
    )
    {      
      connectedToServer = false;      
    }
  } 

    return content;
}
String readEsp(const int timeout)
{  
  String content="";
  char character;
  
  long int time = millis();
  long int receivedTime = 0;
  const int dataWaitTime = 40; //ms
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
          break;                
    }

  if (content != "")  
  {
    if (debug)
      Serial.println("ESP8266: " + content);         

      if (hasString(content,"CLOSED"))     
        connectedToServer = false;       
      else if (hasString(content,"ERROR"))     
        connectedToServer = false;       
      else if (hasString(content,"UNLINK"))     
        connectedToServer = false;      
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

void turnOffEngines()
{
  Serial.println("turnOffEngines()");
  digitalWrite(motorA, HIGH); //Establishes forward direction of Channel A
  digitalWrite(motorB, HIGH); //Establishes forward direction of Channel A
  
  analogWrite(motorAspeed, 0);   //Spins the motor on Channel A at 0 speed
  analogWrite(motorBspeed, 0);   //Spins the motor on Channel B at 0 speed
    
  
  digitalWrite(motorAbrake, HIGH); //Engage the Brake for Channel A
  digitalWrite(motorBbrake, HIGH); //Engage the Brake for Channel B
     
}

void moveForward(int velocity)
{  
  Serial.println("MoveForward()");
  digitalWrite(motorA, HIGH); //Establishes forward direction of Channel A
  digitalWrite(motorB, HIGH); //Establishes forward direction of Channel B

  digitalWrite(motorAbrake, LOW); //Eengage the Brake for Channel A
  digitalWrite(motorBbrake, LOW); //Eengage the Brake for Channel B
  
  
  analogWrite(motorBspeed, velocity);   
  analogWrite(motorAspeed, velocity);   
  
}

void moveBackward(int velocity)
{  
  Serial.println("moveBackward()");
  digitalWrite(motorA, LOW); //Establishes forward direction of Channel A
  digitalWrite(motorB, LOW); //Establishes forward direction of Channel B

  digitalWrite(motorAbrake, LOW); //Eengage the Brake for Channel A
  digitalWrite(motorBbrake, LOW); //Eengage the Brake for Channel B
  
  
  analogWrite(motorBspeed, velocity);   
  analogWrite(motorAspeed, velocity);   
  
}

void turnRight(int velocity)
{
  Serial.println("turnRight()");
  digitalWrite(motorA, HIGH); //Establishes forward direction of Channel A
  digitalWrite(motorB, LOW); //Establishes backward direction of Channel B

  digitalWrite(motorAbrake, LOW); //Disengage the Brake for Channel A
  digitalWrite(motorBbrake, LOW); //Disengage the Brake for Channel B
  
  
  analogWrite(motorBspeed, velocity);   
  analogWrite(motorAspeed, velocity*0.8);   
}

void turnLeft(int velocity)
{
  Serial.println("turnLeft()");
  digitalWrite(motorA, LOW); //Establishes backward direction of Channel A
  digitalWrite(motorB, HIGH); //Establishes forward direction of Channel B

  digitalWrite(motorAbrake, LOW); //Disengage the Brake for Channel A
  digitalWrite(motorBbrake, LOW); //Disengage the Brake for Channel B
  
  
  analogWrite(motorBspeed, velocity*0.8);   
  analogWrite(motorAspeed, velocity);   
}
