//wifi shield pins 3, 4, 7, 10, 11, 12, 13 are all in use
#include <Adafruit_NeoPixel.h>
#include <SPI.h>
#include <WiFi.h>

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)

Adafruit_NeoPixel strips[] = {
  Adafruit_NeoPixel(5, 14, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(5, 8, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(5, 6, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(5, 5, NEO_GRB + NEO_KHZ800),
  Adafruit_NeoPixel(5, 2, NEO_GRB + NEO_KHZ800) };
  
byte punctuation[3][6] = { 
  {2, 0, 0, 0, 0, 0}, //SPACE (32)
  {2, 0, 29, 0, 0, 0}, //! (33)
  {3, 0, 24, 24, 0, 0} //" (34)
};

//our bitmask representation of A, first number is width of array
byte capitals[26][6] = {
  {4, 0, 15, 20, 15, 0}, //A (65)
  {4, 0, 10, 21, 31, 0}, //B
  {4, 0, 17, 17, 31, 0}, //C
  {4, 0, 14, 17, 31, 0}, //D
  {4, 0, 21, 21, 31, 0}, //E
  {4, 0, 20, 20, 31, 0}, //F
  {4, 0, 23, 21, 15, 0}, //G
  {4, 0, 31, 4, 31, 0},  //H
  {4, 0, 17, 31, 17, 0}, //I 
  {4, 0, 31, 17, 18, 0}, //J
  {4, 0, 17, 10, 31, 0}, //K
  {4, 0, 1, 1, 31, 0},   //L
  {5, 15, 16, 14, 16, 15}, //M
  {4, 0, 15, 16, 31, 0},  //N
  {4, 0, 15, 17, 15, 0}, //O
  {4, 0, 8, 20, 31, 0},  //P
  {5, 1, 14, 17, 17, 14},//Q
  {4, 0, 11, 20, 31, 0}, //R
  {4, 0, 22, 21, 9, 0},  //S
  {4, 0, 16, 31, 16, 0}, //T
  {4, 0, 31, 1, 31, 0},  //U
  {4, 0, 30, 1, 30, 0},  //V
  {5, 30, 1, 15, 1, 30}, //W
  {4, 0, 27, 4, 27, 0},  //X
  {4, 0, 24, 7, 24, 0},  //Y
  {4, 0, 25, 21, 19, 0}  //Z
};

byte onchar[] = {4, 0, 4, 14, 4, 0};

int draw_count = 0;
int character_ints[3] = {0};
int scroll_shifts[3] = {-4};

IPAddress ip(192, 168, 0, 17);
IPAddress dns(192, 168, 0, 1);
IPAddress gateway(192, 168, 0, 1);

char ssid[] = "SKY0C026";     //  your network SSID (name) 
char pass[] = "FDBUTCRX";    // your network password

//char ssid[] = "HTC Portable Hotspot";     //  your network SSID (name) 
//char pass[] = "helsingen";    // your network password

int status = WL_IDLE_STATUS;     // the Wifi radio's status
char server[] = "rorygianni.me.uk"; 
WiFiClient client;
boolean has_connected = false;

char inString[32]; // string for incoming serial data
int stringPos = 0; // string index counter
boolean startRead = false; // is reading?
String message = "\"CANT CONNECT\"";
boolean messageSet = false; //do we have a message?
byte messageSetRetries = 6; //retry getting message
byte next_char = 0;

void setup() {  
  Serial.begin(9600); //Initialize serial and wait for port to open:
  printMacAddress();
  // attempt to connect to Wifi network:
  //WiFi.config(ip, dns, gateway);
  
  //Init neo-pixels
  for(uint16_t i=0; i < 5; i++) {
    strips[i].begin();
    strips[i].show();// Initialize all pixels to 'off'
  }
  
  //Draw a '+' to show we are on
  drawCharacter(onchar, 0);
  showStrips();
  
  while(status != WL_CONNECTED){ 
    listNetworks();
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:    
    status = WiFi.begin(ssid, pass);  
    Serial.print("Status: ");
    Serial.println(status);
    // wait 10 seconds before re-connect:
    delay(10000);
  }
  
  stripsOff();
  resetScroller();
}

void loop(){
  if (!messageSet && messageSetRetries > 0){
    if(client.connected()){ readPage(); }
    else { serverConnect(); }
  } else {
    stripsOff();
    scroll();
    showStrips();
  }
  delay(600);
}