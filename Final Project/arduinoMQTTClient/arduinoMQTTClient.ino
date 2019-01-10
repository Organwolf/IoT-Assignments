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
const char hueHubIP[] = "192.168.20.107";
const char hueUsername[] = "BCBtcWiWSkyW7iiHn980aGJrtbA4PBrdVe9xSyBP";
const int hueHubPort = 80;
// Hue variables
unsigned int hueLight;
String hueOn = "";
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
  Serial.println("Loop called");
  mqttClient.loop();
}

void GET(int light)
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
        ethClient.findUntil("\"on\":", "\"effect\":");
        hueOn = ethClient.readStringUntil(','); // if light is on, set variable to true
        ethClient.findUntil("\"bri\":", "\"effect\":");
        hueBri = ethClient.readStringUntil(','); // set variable to brightness value
        ethClient.findUntil("\"hue\":", "\"effect\":");
        
        // logg saturation as well!!
        
        hueHue = ethClient.readStringUntil(','); // set variable to hue value
        lampInformation[light] = "Light: " + String(light) + " hueOn: " + hueOn + " hueBri: " + hueBri + " hueHue: " + hueHue;
        break;
      }
    }
  }
  ethClient.stop();
}

void SetHue(int light, String cmd)
{
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

// Callback - called when message is recieved

void callback(char* topic, byte* payload, unsigned int length)
// expected input: {"on":true, "sat":254, "bri":254, "hue":10000}
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
    //SetHue(2, lampInformation[2]);
    //SetHue(3, lampInformation[3]);
    ethClient.stop();
    reconnect();
    //timer.resume();
}

// Reconnect

void reconnect()
{
  // Loop until we're reconnected
  Serial.print("Connecting...");
  while (!mqttClient.connected()) {
    if (mqttClient.connect(myClientID, myUsername, myPassword)) {
      Serial.println("Connected");
      mqttClient.subscribe(Lamp_set);      
    } else {
      delay(1000);
    }
  }
}

void updateInfo()
{
  String lampStatus = "";
  mqttClient.disconnect();
  GET(1);
  //Serial.println(lampInformation[1]);
  GET(2);
  //Serial.println(lampInformation[2]);
  GET(3);
  
  // Concatenate all info from lamps seperated by '*'
  lampStatus = lampInformation[1]+"*"+lampInformation[2]+"*"+lampInformation[3]+"*";
  PublishToBroker(lampStatus);
}

void PublishToBroker(String lampStatus)
{
  reconnect();
  if (mqttClient.connected()) {
    mqttClient.publish(Lamp_status, lampStatus.c_str());
  }
}

//int lampNbr(char* topic){
//  if(strcmp(Lamp_1, topic)==0){
//    return 1;
//  } else if (strcmp(Lamp_2, topic)==0){
//    return 2;
//  } else { return 3; }
//}

