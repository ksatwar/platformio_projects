#include <Arduino.h>
#include <ESP8266WiFi.h>
int serialInput;
// put function declarations here:
// int myFunction(int, int);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(230400);
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available()){
    serialInput = Serial.read();
    Serial.println("The input is:");
    Serial.println(serialInput);
  }
}

// put function definitions here:
/*
int myFunction(int x, int y) {
  return x + y;
}
*/
