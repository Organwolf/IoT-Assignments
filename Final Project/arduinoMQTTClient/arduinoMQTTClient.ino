/*
 * Send a message to lamp_1
 * if the message is of lenght 2 you can toggle the 5th lamp
 * if the meggafge is of length 3 a  request is printed to the console
 *
 * Authors: Aron P, Filip N, Jesper A
 */

// create MQTT labels 1, 2, 3 and use the "light" int to publish

#include <SPI.h>
#include <Ethernet2.h>
#include <PubSubClient.h>
#include <TimerOne.h>

// Timer
TimerOne timer;
// Ethernet
byte mac[] = { 0x90, 0xA2, 0xDA, 0x10, 0xC1, 0xDA };
IPAddress ip(192, 168, 20, 222);
EthernetClient ethClient;
// PubSubClient
PubSubClient mqttClient(ethClient);
IPAddress server(54, 75, 8, 165);
int port =  13789;
char* myClientID = "test-instance";
char* myUsername = "cadobfeg";
char* myPassword = "GadV6ZHExG7T";
char* Lamp_status = "Lamp_status";
char* Lamp_set = "Lamp_set";
//  Hue constants
const char hueHubIP[] = "192.168.20.110";
const char hueUsername[] = "8RwuA4-RXfa1YiwMfOM0l6mSs4w5j6JMfswfD836";
const int hueHubPort = 80;
// Hue variables
unsigned int hueLight;
String hueOn = "";
String hueSat = "";
String hueBri = "";
String hueHue = "";
String lampInformation[4];

void setup()
{
  Serial.begin(9600);
  //Serial.println("Connecting w. ethernet shield");

  Ethernet.begin(mac, ip);
  Serial.println("Ready. Ethernet connected");
  delay(50);
  mqttClient.setServer(server, port);
  mqttClient.setCallback(callback);

  //timer.initialize(20000000);                              // doesn't work for 20 secs - try working with periods instead
  //timer.attachInterrupt(updateInfo);

}

void loop()
{
  if (!mqttClient.connected())
  {
    reconnect();
  }
  //Serial.println("Loop called");
  mqttClient.loop();
}

void GetHue(int light)
{
  if (ethClient.connect(hueHubIP, hueHubPort)) {
    ethClient.print("GET /api/");
    ethClient.print(hueUsername);
    ethClient.print("/lights/");
    ethClient.print(String(light));
    ethClient.println(" HTTP/1.1");
    ethClient.print("Host: ");
    ethClient.println(hueHubIP);
    ethClient.println("Content-type: application/json");
    ethClient.println("keep-alive");
    ethClient.println();
    while (ethClient.connected())
    {
      if (ethClient.available())
      {
        //Serial.println(ethClient.readStringUntil('\0'));
        ethClient.findUntil("\"on\":", "\"effect\":");
        hueOn = ethClient.readStringUntil(','); // if light is on, set variable to true
        ethClient.findUntil("\"bri\":", "\"effect\":");
        hueBri = ethClient.readStringUntil(','); // set variable to brightness value
        ethClient.findUntil("\"hue\":", "\"effect\":");     
        hueHue = ethClient.readStringUntil(','); // set variable to hue value
        ethClient.findUntil("\"sat\":", "\"effect\":");
        hueSat = ethClient.readStringUntil(','); // if light is on, set variable to true
        //lampInformation[light] = "{\"on\":" + hueOn + "\"bri\":" + hueBri + "\"hue\":" + hueHue + "\"sat\":" + hueSat + "}";
        lampInformation[light] = hueOn + "," + hueBri + "," + hueHue + "," + hueSat;
        break;
      }
    }
  }
  ethClient.stop();
}

void SetHue(int light, String cmd)
{
  Serial.println(light);
  if (ethClient.connect(hueHubIP, hueHubPort))
  {      
    while (ethClient.connected())
    {
      ethClient.print("PUT /api/");
      ethClient.print(hueUsername);
      ethClient.print("/lights/");
      ethClient.print(String(light));
      ethClient.println("/state HTTP/1.1");
      ethClient.println("keep-alive");
      ethClient.print("Host: ");
      ethClient.println(hueHubIP);
      ethClient.print("Content-Length: ");
      ethClient.println(cmd.length());
      ethClient.println("Content-Type: text/plain;charset=UTF-8");
      ethClient.println();
      ethClient.println(cmd);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.println("CB entered");
  Serial.println(String(length));
    int index = 1;
    String inputStr = "";
    //timer.stop();
    for (int i=0;i<length;i++) {
      char c = (char)payload[i];
      if(!(c == '*')){
        inputStr += c;
      }
      else {
        lampInformation[index] = inputStr;
        //Serial.println(inputStr);
        inputStr = "";
        index++; 
      }   
    }
    mqttClient.disconnect();
    // SetHue sätter inte värden efter topic längre isf   
    SetHue(1, lampInformation[1]);
    //delay(20);
    ethClient.stop();
    SetHue(2, lampInformation[2]);
    //delay(20);
    ethClient.stop();
    SetHue(3, lampInformation[3]);
    ethClient.stop();
    reconnect();
    //timer.resume();
}

void reconnect()
{
  // Loop until we're reconnected
  Serial.print("Connecting...");
  while (!mqttClient.connected()) {
    if (mqttClient.connect(myClientID, myUsername, myPassword)) {
      Serial.println("Connected");
      mqttClient.subscribe(Lamp_set);
      //delay(20);    
    } else {
      delay(1000);
    }
  }
}

void updateInfo()
{
  String lampStatus = "";
  mqttClient.disconnect();
  GetHue(1);
  GetHue(2);
  //Serial.println(lampInformation[2]);
  GetHue(3);
  // Concatenate info
  lampStatus = lampInformation[1]+"*"+lampInformation[2]+"*"+lampInformation[3]+"*";
  PublishToBroker(lampStatus);
}

void PublishToBroker(String lampStatus)
{
  Serial.println("Publishing ...");
  reconnect();
  if (mqttClient.connected()) {
    Serial.println("Published");
    mqttClient.publish(Lamp_status, lampStatus.c_str());
  }
}

