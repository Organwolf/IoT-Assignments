/*
 * Code that connects an Arduino + ethernet shield to an MQTT broker.
 * With an established connection it then subscribes to a topic.
 * Via the callback method messages are recieved and princed to the console
 * 
 * Authors: Aron P, Filip N, Jesper A
 */

#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

//  Ethernet
 
byte mac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };  // Can we set it ourselves?
IPAddress ip(192,168,20,200);  // Ethernet shield IP
EthernetClient ethClient;

// PubSubClient

PubSubClient client(ethClient);
IPAddress server(54,75,8,165);
int port = 13789;
char* myClientID = "manuels";
char* myUsername = "cadobfeg";
char* myPassword = "GadV6ZHExG7T";
char* topic = "foo/bar";

// Other variables

bool messageRecieved = false;
const int LED_PIN = 9;

// Callback

void callback(char* topic, byte* payload, unsigned int length) {
  messageRecieved = !messageRecieved;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  delay(10);
  if (messageRecieved) {
      digitalWrite(LED_PIN, HIGH);
  }
  else { 
      digitalWrite(LED_PIN, LOW);
  } 
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
  client.loop();
}
