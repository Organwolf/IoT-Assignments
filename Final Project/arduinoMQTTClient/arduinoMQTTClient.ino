/*
 * Send a message to lamp_1
 * if the message is of lenght 2 you can toggle the 5th lamp
 * if the meggafge is of length 3 a  request is printed to the console
 * 
 * Authors: Aron P, Filip N, Jesper A
 */

 // Cloud MQTT
 // Topics: Lamp_1, Lamp_2, Lamp_3

// check library towards shield
#include <SPI.h>
#include <Ethernet2.h>
#include <PubSubClient.h>
#include <TimerOne.h>

// Timer

TimerOne timer;

// Ethernet
 
byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0xC1, 0xDA };  // Can we set it ourselves?
IPAddress ip(192,168,20,222);  // Ethernet shield IP
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
 
const char hueHubIP[] = "192.168.20.107";  // Hue hub IP
const char hueUsername[] = "Emt5PLjKK1eKet8S3LBy9YFm0tFyuKO1Qqw5oEMH";  // Hue username
const int hueHubPort = 80;

// Hue variables

unsigned int hueLight;  // tar light
String hueOn;  // on/off
int hueBri;  // brightness value
long hueHue;  // hue value
String hueCmd = "{\"on\": false}";  // Hue command
String Message;
bool toggle = true;
String readString;
char c;
int counter;

// Setup

void setup()
{  
  Serial.begin(9600);
  Serial.println("Connecting w. ethernet shield");
  
  Ethernet.begin(mac,ip);
  
  mqttClient.setServer(server, port);
  mqttClient.setCallback(callback);

  //timer.initialize(2000000);
  //timer.attachInterrupt(getInfoLamps);
  
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

// Hue -  light state (on,bri,hue)

boolean GET(String lamp)
{
  if (ethClient.connect(hueHubIP, hueHubPort))
  {
    //ethClient.print(" /api/q0fHaaAkdaG0KZipHwC6e4tbyeJCNH1jxAVNYEWu/lights/5/");
    ethClient.print(" /api/q0fHaaAkdaG0KZipHwC6e4tbyeJCNH1jxAVNYEWu/lights/"+lamp+"/");
    //client.print(hueUsername);
    //client.print("/lights/1");
    //client.print(1);  // hueLight zero based, add 1
    ethClient.println(" HTTP/1.1");
    ethClient.print("Host: ");
    ethClient.println(hueHubIP);
    ethClient.println("Content-type: application/json");
    ethClient.println("keep-alive");
    ethClient.println();
    delay(500);
    while (ethClient.connected())
    {
      while (ethClient.available())¨
      {
        counter++;
        c = ethClient.read();
        if (counter > 421){
          readString += c;
          // is it possible to use a findUntil 
          // instead of this solution
        }
      }
      break;
    }
    ethClient.stop();
    return true;  // captured on,bri,hue
  }
  else
    return false;  // error reading on,bri,hue
}

// Set hue -rename to somevalue or setcmd

boolean SetHue()
{
  if (ethClient.connect(hueHubIP, hueHubPort))
  {
    while (ethClient.connected())
    {
      ethClient.print("PUT /api/");
      ethClient.print(hueUsername);
      ethClient.print("/lights/");
      ethClient.print("1");  // hueLight zero based, lamp 5
      ethClient.println("/state HTTP/1.1");
      ethClient.println("keep-alive");
      ethClient.print("Host: ");
      ethClient.println(hueHubIP);
      ethClient.print("Content-Length: ");
      ethClient.println(hueCmd.length());
      ethClient.println("Content-Type: text/plain;charset=UTF-8");
      ethClient.println();  // blank line before body
      ethClient.println(hueCmd);  // Hue command
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
  mqttClient.disconnect();
  hueCmd = "{\"on\": false}";
  SetHue();
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
      Serial.println(" try again in 5 seconds");
      // Wait 0.5 seconds before retrying
      delay(5000);
    }
  }
}

void getInfoLamps() 
{
  if(mqttClient.connected()){
    mqttClient.publish(Lamp_1,"Hej");
    Serial.println("Publish OK");
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


