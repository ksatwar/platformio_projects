#include<Arduino.h>
#include <Adafruit_MCP23X17.h>
#define SDA_8266 12
#define SCL_8266 13
#define RELAY_1 0  
#define RELAY_2 1
#define RELAY_3 2
#define RELAY_4 3

Adafruit_MCP23X17 mcp;

void setup() {
  Serial.begin(115200);
  //while (!Serial);
  Serial.println("MCP2307 Test!");
  Wire.begin(SDA_8266,SCL_8266);
  // uncomment appropriate mcp.begin
  if (!mcp.begin_I2C()) {
  //if (!mcp.begin_SPI(CS_PIN)) {
    Serial.println("Error.");
    while (1);
  }
  
  // configure LED pin for output
  mcp.pinMode(RELAY_1, OUTPUT);
  mcp.pinMode(RELAY_2, OUTPUT);
  mcp.pinMode(RELAY_3, OUTPUT);
  mcp.pinMode(RELAY_4, OUTPUT);


  // configure button pin for input with pull up
  //mcp.pinMode(BUTTON_PIN, INPUT_PULLUP);

  Serial.println("Looping...");
}

void loop() {
  int i=1;
  while (i)
  {
  mcp.digitalWrite(RELAY_1, !mcp.digitalRead(RELAY_1));
  delay(1000);
  mcp.digitalWrite(RELAY_2, !mcp.digitalRead(RELAY_2));
  delay(1000);
  mcp.digitalWrite(RELAY_3, !mcp.digitalRead(RELAY_3));
  delay(1000);
  mcp.digitalWrite(RELAY_4, !mcp.digitalRead(RELAY_4));
  delay(1000);
  mcp.digitalWrite(RELAY_4, !mcp.digitalRead(RELAY_4));
  delay(1000);
  mcp.digitalWrite(RELAY_3, !mcp.digitalRead(RELAY_3));
  delay(1000);
  mcp.digitalWrite(RELAY_2, !mcp.digitalRead(RELAY_2));
  delay(1000);
  mcp.digitalWrite(RELAY_1, !mcp.digitalRead(RELAY_1));
  delay(1000);
  }
}