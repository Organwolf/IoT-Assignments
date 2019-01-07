/*
 * Send a message to lamp_1
 * if the message is of lenght 2 you can toggle the 5th lamp
 * if the meggafge is of length 3 a get request is printed to the console
 * 
 * Authors: Aron P, Filip N, Jesper A
 */

// check library towards shield
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

//  Ethernet
 
byte mac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };  // Can we set it ourselves?
IPAddress ip(192,168,20,222);  // Ethernet shield IP
EthernetClient ethClient;

// PubSubClient

PubSubClient mqttClient(ethClient);
IPAddress server(54,75,8,165);
int port = 13789;
char* myClientID = "manuels";
char* myUsername = "cadobfeg";
char* myPassword = "GadV6ZHExG7T";
char* topic_lamp_1 = "lamp_1";
char* topic_lamp_2 = "lamp_2";
char* topic_lamp_3 = "lamp_3";

//  Hue constants
 
const char hueHubIP[] = "192.168.20.163";  // Hue hub IP
const char hueUsername[] = "q0fHaaAkdaG0KZipHwC6e4tbyeJCNH1jxAVNYEWu";  // Hue username
const int hueHubPort = 80;

// Hue variables

unsigned int hueLight;  // target light
String hueOn;  // on/off
int hueBri;  // brightness value
long hueHue;  // hue value
String hueCmd = "{\"on\": false}";  // Hue command
String getMessage;
bool toggle = true;
String readString;
char c;
int counter;

// GetHue - Get light state (on,bri,hue)

boolean GetHue()
{
  if (ethClient.connect(hueHubIP, hueHubPort))
  {
    ethClient.print("GET /api/q0fHaaAkdaG0KZipHwC6e4tbyeJCNH1jxAVNYEWu/lights/5/");
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
      while (ethClient.available())
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


// Setup

void setup() {
  
  Serial.begin(9600);
  Serial.println("Connecting w. ethernet shield");
  
  mqttClient.setServer(server, port);
  mqttClient.setCallback(callback);
  
  Ethernet.begin(mac,ip);
  
  delay(2000);
  Serial.println("Ready.");

}

// Main Loop

void loop() {

  if (!mqttClient.connected()) {
    reconnect(topic_lamp_1);
  }
  
  mqttClient.loop();

}

// Callback - called when message is recieved

void callback(char* topic, byte* payload, unsigned int length) {

  toggle = !toggle;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  delay(10);
  mqttClient.disconnect();
  if(length == 3){
    if(toggle){
      hueCmd = "{\"on\": true}";
      SetHue();
    }
    else {
      hueCmd = "{\"on\": false}";
      SetHue();
    }
  }
  else if(length == 2){
    mqttClient.disconnect();
    GetHue();
    Serial.println(readString);
  }
  else{
    Serial.println("other length");
  }


  
}

// Reconnect

void reconnect(char* topic) {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqttClient.connect(myClientID, myUsername, myPassword)) {
      Serial.println("connected");
      // ... and resubscribe
      mqttClient.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 0.5 seconds before retrying
      delay(5000);
    }
  }
}

void changeStuff() {
    delay(100);
  if (ethClient.connect(hueHubIP, hueHubPort))
  {
    Serial.println("connected to the ethernet again");
  }
    delay(2000);
    Serial.println("please work!");
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
      ethClient.print(5);  // hueLight zero based, lamp 5
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
