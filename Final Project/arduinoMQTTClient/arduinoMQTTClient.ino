/*
 * Code that connects an Arduino + ethernet shield to an MQTT broker.
 * With an established connection it then subscribes to a topic.
 * Via the callback method messages are recieved and princed to the console
 * 
 * Authors: Aron P, Filip N, Jesper A
 */

// check library towards shield
#include <SPI.h>
#include <Ethernet2.h>
#include <PubSubClient.h>

//  Ethernet
 
byte mac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };  // Can we set it ourselves?
IPAddress ip(192,168,20,200);  // Ethernet shield IP
EthernetClient ethClient;

//  Hue constants
 
const char hueHubIP[] = "192.168.20.151";  // Hue hub IP
const char hueUsername[] = "19eSZFKKJaNnYeRtIRYXV0MobofTkSBrPXrNKUn4";  // Hue username
const int hueHubPort = 80;
const char lamp1 = 1;
const char lamp2 = 2;
const char lamp3 = 3;

// Hue variables

unsigned int hueLight;  // target light
String hueOn;  // on/off
int hueBri;  // brightness value
long hueHue;  // hue value
String hueCmd;  // Hue command
String hueBriTest = "";

// PubSubClient

PubSubClient client(ethClient);
// all valiables for MQTT are user specific
// change if you use another broker
IPAddress server(54,75,8,165);
int port = 13789;
char* myClientID = "manuels";
char* myUsername = "cadobfeg";
char* myPassword = "GadV6ZHExG7T";
char* topic_lamp_1 = lamp_1;
char* topic_lamp_2 = lamp_2;
char* topic_lamp_3 = lamp_3;

// Other variables

bool messageRecieved = false;


// Setup

void setup() {
  pinMode(LED_PIN, OUTPUT);
  
  Serial.begin(9600);
  Serial.println("Connecting w. ethernet shield");
  
  client.setServer(server, port);
  client.setCallback(callback);
  
  Ethernet.begin(mac,ip);
  
  delay(2000);
  Serial.println("Ready.");
}

// Main Loop

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  
  // logic for getting and setting Hue lamps
  // insert here
  
  client.loop();
}

// Callback - called when message is recieved

void callback(char* topic, byte* payload, unsigned int length) {
  messageRecieved = !messageRecieved;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  delay(10) 
}

// Reconnect

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(myClientID, myUsername, myPassword)) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe(topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// GetHue - Get light state (on,bri,hue)

boolean GetHue(char lamp)
{
  if (client.connect(hueHubIP, hueHubPort))
  {
    client.print("GET /api/"+hueUsername[]+"/lights/"+lamp+"/");
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(hueHubIP);
    client.println("Content-type: application/json");
    client.println("keep-alive");
    client.println();
    delay(500);
    while (client.connected())
    {
      while (client.available())
      {
        client.findUntil("\"bri\":", "\"bri\":");
        hueBriTest = client.readStringUntil(',');
        break;  // not capturing other light attributes yet
      }
      break;
    }
    client.stop();
    return true;  // captured on,bri,hue
  }
  else
    return false;  // error reading on,bri,hue
}

// SetHue - Set light state using hueCmd command

boolean SetHue()
{
  if (client.connect(hueHubIP, hueHubPort))
  {
    while (client.connected())
    {
      client.print("PUT /api/");
      client.print(hueUsername);
      client.print("/lights/");
      client.print(1);  // hueLight zero based, add 1
      client.println("/state HTTP/1.1");
      client.println("keep-alive");
      client.print("Host: ");
      client.println(hueHubIP);
      client.print("Content-Length: ");
      client.println(hueCmd.length());
      client.println("Content-Type: text/plain;charset=UTF-8");
      client.println();  // blank line before body
      client.println(hueCmd);  // Hue command
    }
    client.stop();
    return true;  // command executed
  }
  else
    return false;  // command failed
}
