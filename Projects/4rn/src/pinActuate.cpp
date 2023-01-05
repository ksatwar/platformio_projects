#include "pinActuate.h"
Adafruit_MCP23X17 mcp;
#define SDA_8266 12
#define SCL_8266 13
#define RELAY_1 0  
#define RELAY_2 1
#define RELAY_3 2
#define RELAY_4 3
enum command:uint8_t
{off,on,toggle};
uint8_t toggleFlag=0;
int pinStatus=0;
int readPinStatus=0;
char c=0;
int hex2int(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}
void I2CBeginCustom(){
Wire.begin(SDA_8266,SCL_8266);
  // uncomment appropriate mcp.begin
  if (!mcp.begin_I2C()) {
  //if (!mcp.begin_SPI(CS_PIN)) {
    Serial.println("I2C Begin Custom Error.");
    while (1);
  }
mcp.pinMode(RELAY_1, OUTPUT);
mcp.pinMode(RELAY_2, OUTPUT);
mcp.pinMode(RELAY_3, OUTPUT);
mcp.pinMode(RELAY_4, OUTPUT);
}
void readData(){
  File file = LittleFS.open("/pinStatus.txt", "r");
     if(!file){
    Serial.println("No Saved Data!");
    return;
   }
    while(file.available()){
    Serial.println("File Contents:");
    //Serial.println(file.read()); 
    //c=file.read();
    readPinStatus=hex2int(file.read());
    //Serial.println("The value of c is:");
    }
    file.close();
    pinStatus=readPinStatus;
    for(int i=0;i<4;i++)
    {
      mcp.digitalWrite(i,((((readPinStatus) >> (i)) & 1)));//pinWise write getBit operation
      Serial.println("PIN STATUS:");
      Serial.println((((readPinStatus) >> (i)) & 1));
    }
}
void LittleFSBeginCustom()
{
    if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;
  }
  else
  Serial.println("FS LOAD SUCCESS");
  delay(1);
  readData();
  pinStatus=readPinStatus;
}
void writeData(int pinStatus)
{
  //Open the file 
  File file = LittleFS.open("/pinStatus.txt", "w");
  //Write to the file
  file.print(pinStatus,16);
  //Close the file
  file.close();
  //delay(1); 
  Serial.println("Write successful");
  Serial.println(pinStatus);
}
bool pinActuate(uint8_t pin,uint8_t cmd)
{   
  pin-=1;
  if pin    
      if (cmd==off&&(mcp.digitalRead(pin)==HIGH))
      {
          mcp.digitalWrite(pin,LOW);
          //toggleFlag=0;
          pinStatus=pinStatus&~(1<<pin);//reset bit
          writeData(pinStatus);
          // Serial.println("pinStatus:");
          // Serial.println(pinStatus);
          // readData();
          return true;
      }
      else if (cmd==on&&(mcp.digitalRead(pin)==LOW))
      {
          mcp.digitalWrite(pin,HIGH);
          //toggleFlag=1;
          pinStatus=pinStatus|(1<<pin);//set bit
          writeData(pinStatus);
          // Serial.println("pinStatus:");
          // Serial.println(pinStatus);        
          // readData();
          return true;
      }
      else if (cmd==toggle)
      {
          mcp.digitalWrite(pin,!mcp.digitalRead(pin));
          //mcp.digitalWrite(pin,!toggleFlag);
          pinStatus=pinStatus^(1<<pin);//toggle bit
          // if(toggleFlag){
          //   pinStatus=pinStatus|(1<<pin);
          // }
          // else
          //   pinStatus=pinStatus&~(1<<pin);
          //toggleFlag=!toggleFlag;
          writeData(pinStatus);
          // Serial.println("pinStatus:");
          // Serial.println(pinStatus);        
          // readData();
          return true;
      }
      else
      return false;
}