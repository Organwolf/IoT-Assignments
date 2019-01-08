/*
 * Send a message to lamp_1
 * if the message is of lenght 2 you can toggle the 5th lamp
 * if the meggafge is of length 3 a  request is printed to the console
 * 
 * Authors: Aron P, Filip N, Jesper A
 */

 // create MQTT labels 1, 2, 3 and use the "light" int to publish

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <TimerOne.h>

// Timer
TimerOne timer;
// Ethernet
byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0xC1, 0xDA };
IPAddress ip(192,168,20,222);
EthernetClient ethClient;
// PubSubClient
PubSubClient mqttClient(ethClient);
IPAddress server(54,75,8,165);
int port = 16982;
char* myClientID = "manuels";
char* myUsername = "gsbbbjxh";
char* myPassword = "98Flmcp5Y9ul";
char* Lamp_1 = "Lamp_1";
char* Lamp_2 = "Lamp_2";
char* Lamp_3 = "Lamp_3";
//  Hue constants
const char hueHubIP[] = "192.168.20.107";
const char hueUsername[] = "Emt5PLjKK1eKet8S3LBy9YFm0tFyuKO1Qqw5oEMH"; 
const int hueHubPort = 80;
// Hue variables
unsigned int hueLight; 
String hueOn =""; 
String hueBri ="";  
String hueHue =""; 
String hueCmd = "{\"on\": false}"; 
char charBuf[100];
String hueLightInfo = "";

// Setup

void setup()
{  
  Serial.begin(9600);
  Serial.println("Connecting w. ethernet shield");
  
  Ethernet.begin(mac,ip);
  
  mqttClient.setServer(server, port);
  mqttClient.setCallback(callback);

  //timer.initialize(2000000);
  //timer.attachInterrupt(updateInfo);
  
  delay(2000);
  Serial.println("Ready.");
}

// Main Loop

void loop() 
{
  if (!mqttClient.connected()) 
  {
    reconnect();
  } 
  mqttClient.loop();
}

boolean GET(int light)
{
  if (ethClient.connect(hueHubIP, hueHubPort))
  {
    ethClient.print("GET /api/");
    ethClient.print(hueUsername);
    ethClient.print("/lights/");
    ethClient.print(light);
    ethClient.println(" HTTP/1.1");
    ethClient.print("Host: ");
    ethClient.println(hueHubIP);
    ethClient.println("Content-type: application/json");
    ethClient.println("keep-alive");
    ethClient.println();
    while (ethClient.connected())
    {
      Serial.println("Do we get here?");
      if(ethClient.available())
      {       
//      Serial.println(ethClient.readStringUntil('\0'));
        ethClient.findUntil("\"on\":", "\"effect\":");
        hueOn = ethClient.readStringUntil(','); // if light is on, set variable to true
        ethClient.findUntil("\"bri\":", "\"effect\":");
        hueBri = ethClient.readStringUntil(','); // set variable to brightness value
        ethClient.findUntil("\"hue\":", "\"effect\":");
        hueHue = ethClient.readStringUntil(','); // set variable to hue value
        PublishToBroker(light);
        break; 
        
//      Serial.println("HueOn: " + hueOn);
//      Serial.println("HueBri: " + hueBri);
//      Serial.println("HueHue: " + hueHue);
      }
    }
    ethClient.stop();
    return true;
  } else { return false; }
}

// Set hue -rename to somevalue or setcmd

boolean SetHue(int light)
{
  if (ethClient.connect(hueHubIP, hueHubPort))
  {
    while (ethClient.connected())
    {
      ethClient.print("PUT /api/");
      ethClient.print(hueUsername);
      ethClient.print("/lights/");
      ethClient.print(light);
      ethClient.println("/state HTTP/1.1");
      ethClient.println("keep-alive");
      ethClient.print("Host: ");
      ethClient.println(hueHubIP);
      ethClient.print("Content-Length: ");
      ethClient.println(hueCmd.length());
      ethClient.println("Content-Type: text/plain;charset=UTF-8");
      ethClient.println();
      ethClient.println(hueCmd);
    }
    ethClient.stop();
    return true;  // command executed
  }
  else
    return false;  // command failed
}

// Callback - called when message is recieved
// not used for 

void callback(char* topic, byte* payload, unsigned int length) 
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  delay(10);
//  mqttClient.disconnect();
  hueCmd = "{\"on\": true}";
  //SetHue();
  //reconnect();
  GET(1);
  Serial.println("hueOn: " + hueOn + " hueBri: " + hueBri + " hueHue: " + hueHue);
  hueOn.toCharArray(charBuf, 100);
  Serial.println(charBuf);
  mqttClient.publish(Lamp_1, charBuf, true);
  //mqttClient.publish(Lamp_1, "Communication baby");
  mqttClient.disconnect();
}

// Reconnect

void reconnect() 
{
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(myClientID, myUsername, myPassword)) {
      Serial.println("connected");      
      // ... and resubscribe
      mqttClient.subscribe(Lamp_1);
      mqttClient.subscribe(Lamp_2);
      mqttClient.subscribe(Lamp_3);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 1 second");
      delay(1000);
    }
  }
}

void updateInfo() 
{
  if(mqttClient.connected()){
    GET(1);
    GET(2);
    GET(3);
  }
  else {
    Serial.println("Nothing published");
  }
   
   /*
   GET("1");
   // skicka till broker
   GET("2");
   // skicka till broker
   GET("3");
   // skicka till broker
   */
}

void PublishToBroker(int light)
{
    Serial.println("hueOn: " + hueOn + " hueBri: " + hueBri + " hueHue: " + hueHue);
    // --- hueLightInfo --- publish this?
    // int to char *
    mqttClient.publish(light, charBuf, true);
}


