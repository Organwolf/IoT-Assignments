#include <SPI.h>
#include <Ethernet.h>

//  Hue constants
 
const char hueHubIP[] = "192.168.20.173";  // Hue hub IP
const char hueUsername[] = "jA4whBOy7c6SvvjeHkLnXO40SwuMEFlPRFA9YRSt";  // Hue username
const int hueHubPort = 80;

// Hue variables

unsigned int hueLight;  // target light
boolean hueOn;  // on/off
int hueBri;  // brightness value
long hueHue;  // hue value
String hueCmd;  // Hue command

//  Ethernet
 
byte mac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };  // Can we set it ourselves?
IPAddress ip(192,168,20,200);  // Arduino IP
EthernetClient client;

// Ultra sound

const unsigned int MAX_DIST = 23200;
const int TRIG_PIN = 9;
const int ECHO_PIN = 8;

// Setup

void setup() {

  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  
  Serial.begin(9600);
  Serial.println("Connecting w. ethernet shield");
  
  Ethernet.begin(mac,ip);
  
  delay(2000);
  Serial.println("Ready.");

}

void loop() {
  
  // Ultra sound variables
  unsigned long t1;
  unsigned long t2;
  unsigned long pulse_width;
  float cm;

  // Ultra sound logic
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  while ( digitalRead(ECHO_PIN) == 0 );
  t1 = micros();
  while ( digitalRead(ECHO_PIN) == 1);
  t2 = micros();
  pulse_width = t2 - t1;
  cm = pulse_width / 58.0;
    if ( pulse_width > MAX_DIST ) {
    Serial.println("Out of range");
  } else {
    Serial.print(cm);
    Serial.print(" cm \t");
  }
  
  // Hue communication
  if (GetHue()) {
     Serial.println("Hue on/off: " + hueOn);
     Serial.println("Hue bri: " + hueBri);
     Serial.println("Hue hue: " + hueHue);
  }
  else {
    Serial.println("Get request failed");
  }
  delay(1000);
  
}

/*
 
    GetHue
    Get light state (on,bri,hue)
 
*/

boolean GetHue()
{
  if (client.connect(hueHubIP, hueHubPort))
  {
    client.print("GET /api/");
    client.print(hueUsername);
    // Is this needed when we only have one light?
    client.print("/lights/");
    client.print(hueLight + 1);  // hueLight zero based, add 1
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(hueHubIP);
    client.println("Content-type: application/json");
    client.println("keep-alive");
    client.println();
    while (client.connected())
    {
      if (client.available())
      {
        client.findUntil("\"on\":", "\0");
        hueOn = (client.readStringUntil(',') == "true");  // if light is on, set variable to true
 
        client.findUntil("\"bri\":", "\0");
        hueBri = client.readStringUntil(',').toInt();  // set variable to brightness value
 
        client.findUntil("\"hue\":", "\0");
        hueHue = client.readStringUntil(',').toInt();  // set variable to hue value
        
        break;  // not capturing other light attributes yet
      }
    }
    client.stop();
    return true;  // captured on,bri,hue
  }
  else
    return false;  // error reading on,bri,hue
}

/*
 
    SetHue
    Set light state using hueCmd command
 
*/

boolean SetHue()
{
  if (client.connect(hueHubIP, hueHubPort))
  {
    while (client.connected())
    {
      client.print("PUT /api/");
      client.print(hueUsername);
      client.print("/lights/");
      client.print(hueLight + 1);  // hueLight zero based, add 1
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
