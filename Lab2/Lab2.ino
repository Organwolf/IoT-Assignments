// Ethernet 2 is needed bec of Jespers shield
// we might want to try an older shield with Ethernet.h
// and see if that makes any difference

#include <SPI.h>
#include <Ethernet.h>

//  Hue constants
 
const char hueHubIP[] = "192.168.20.151";  // Hue hub IP
const char hueUsername[] = "19eSZFKKJaNnYeRtIRYXV0MobofTkSBrPXrNKUn4";  // Hue username
const int hueHubPort = 80;

// Hue variables

unsigned int hueLight;  // target light
String hueOn;  // on/off
int hueBri;  // brightness value
long hueHue;  // hue value
String hueCmd;  // Hue command
String hueBriTest = "";

//  Ethernet
 
byte mac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };  // Can we set it ourselves?
IPAddress ip(192,168,20,200);  // Ethernet shield IP
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

  // Code to turn on the hue lamp during setup
  
  hueCmd = "{\"on\": true}";
  SetHue();

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
    Serial.println(" cm \t");
  }

  // SetHue logic which varies the brightness
  // depending on the readings from the sensor
  
  if (cm < 8) {
    hueCmd = "{\"bri\": 50}";
  }
  else {
    hueCmd = "{\"bri\": 150}";
  } 
  GetHue();
  Serial.println("Hue brightness " + hueBriTest);
  SetHue();   
  delay(500);

  // Fix the GET method
  // Can't read info from hue light
  // at the moment. Needed to complete 
  // the assignment.
  
}

/*
 
    GetHue
    Get light state (on,bri,hue)
 
*/

boolean GetHue()
{
  if (client.connect(hueHubIP, hueHubPort))
  {
    client.print("GET /api/19eSZFKKJaNnYeRtIRYXV0MobofTkSBrPXrNKUn4/lights/1/");
    //client.print(hueUsername);
    //client.print("/lights/1");
    //client.print(1);  // hueLight zero based, add 1
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(hueHubIP);
    client.println("Content-type: application/json");
    client.println("keep-alive");
    client.println();
    delay(500);
    while (client.connected())
    {
      //delay(500);
//      char c = client.read();
//      Serial.print(c);
      while (client.available())
      {
//        char c = client.read();
//        Serial.print(c);
//        c = client.read();
//        Serial.print(c);
//        c = client.read();
//        Serial.print(c);
//        c = client.read();
//        Serial.print(c);
//        c = client.read();
//        Serial.print(c);
//        client.findUntil("\"on\":", "\0");
//        if((client.readStringUntil(',') == "true")){
//          hueOn = "true";
//        }
        //hueOn = (client.readStringUntil(',') == "true");  // if light is on, set variable to true
// 
        client.findUntil("\"bri\":", "\"bri\":");
        //hueBri = client.readStringUntil(',').toInt();  // set variable to brightness value
        hueBriTest = client.readStringUntil(',');
//        client.findUntil("\"hue\":", "\0");
//        hueHue = client.readStringUntil(',').toInt();  // set variable to hue value
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
