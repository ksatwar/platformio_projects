#ifndef PIN_ACTUATE_H
#define PIN_ACTUATE_H
#include<Arduino.h>
#include<Adafruit_MCP23X17.h>
#include"LittleFS.h"
bool pinActuate(uint8_t pin,uint8_t cmd);
void I2CBeginCustom();
void LittleFSBeginCustom();
extern Adafruit_MCP23X17 mcp;
#endif