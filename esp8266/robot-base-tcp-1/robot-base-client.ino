/*************************************************************
Motor Shield 1-Channel DC Motor Demo
by Randy Sarafan

For more information see:
http://www.instructables.com/id/Arduino-Motor-Shield-Tutorial/


*************************************************************/

#include <SoftwareSerial.h>
SoftwareSerial esp8266(4, 5); // RX (aseta tx), TX

int motorA = 12;
int motorAbrake=9;
int motorAspeed=3;
int motorB = 13;
int motorBbrake = 8;
int motorBspeed=11;

#define SSID "###"
#define PASS "##"
#define IP "192.168.0.100" //server ip

bool connectedToServer = false;
bool debug=false;

void setup() {
delay(500); //wait for esp8266 power-up
 
  esp8266.begin(9600);
  if (debug)
  {
    Serial.begin(9600);
    Serial.print("Starting...\n");
  }
 
  msgEsp("+++",10000); //close CIPSEND, if its on...  
  //msgEsp("AT+CIPCLOSE\r\n",10000);
  //msgEsp("AT+RST\r\n",10000);  
  //delay(500);
  msgEsp("ATE0\r\n",10000);        
  msgEsp("AT+CWMODE_DEF=1\r\n",1000);  

    //Setup Channel A
  pinMode(motorA, OUTPUT); //Initiates Motor Channel A pin
  pinMode(motorAbrake, OUTPUT); //Initiates Brake Channel A pin

  pinMode(motorB, OUTPUT); //Initiates Motor Channel B pin
  pinMode(motorBbrake, OUTPUT); //Initiates Brake Channel B pin


  turnOffEngines();
}


void loop(){
  String cmd="";
  if (!connectedToServer)
    connectServer();
  else
  {    
    cmd = "!\r\n";
    msgEsp("AT+CIPSEND=" + (String)cmd.length() + "\r\n",5000);
    cmd = msgEsp(cmd,5000);  
    if (hasString(cmd,"0@"))
      turnOffEngines();
    else if (hasString(cmd,"1@"))
      moveForward(205);
    else if (hasString(cmd,"2@"))
      turnRight(120);
    else if (hasString(cmd,"3@"))
      moveBackward(140);
    else if (hasString(cmd,"4@"))
      turnLeft(120);
    
    //msgEsp("+++",5000);
  }
  //delay(500);
  //turnOffEngines();
}

void connectServer(){
  if (debug)
    Serial.println("Connecting to server");
  
  String cmd = "AT+CIPSTART=\"TCP\",\"";   
  cmd += IP;
  cmd += "\",5000\r\n";
  msgEsp(cmd,10000); 
  connectedToServer=true;

    /*
  cmd = GET;  
  cmd += "\r\n\r\n\0";  

  msgEsp("AT+CIPSEND=" + (String)cmd.length() + "\r\n",5000);
  msgEsp(cmd,5000);  
  msgEsp("+++",5000);
  //msgEsp("AT+CIPCLOSE\r\n",2000);
  delay(1000);
  */
}

boolean connectWiFi(){  
  msgEsp("AT+CWMODE=1\r\n",1000);  //might need changing!
  msgEsp("AT+CIPSTA_DEF=\"192.168.0.110\",",0);
  msgEsp("\"192.168.0.254\",",0);
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
      turnOffEngines();
      connectedToServer = false;
      connectWiFi();
    }
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
  analogWrite(motorAspeed, velocity);   
}

void turnLeft(int velocity)
{
  Serial.println("turnLeft()");
  digitalWrite(motorA, LOW); //Establishes backward direction of Channel A
  digitalWrite(motorB, HIGH); //Establishes forward direction of Channel B

  digitalWrite(motorAbrake, LOW); //Disengage the Brake for Channel A
  digitalWrite(motorBbrake, LOW); //Disengage the Brake for Channel B
  
  
  analogWrite(motorBspeed, velocity);   
  analogWrite(motorAspeed, velocity);   
}
