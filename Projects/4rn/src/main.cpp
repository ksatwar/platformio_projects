#include<AsyncMqttClient.h>
#include<WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include<Ticker.h>
#include<errno.h>
#include<limits.h>
#include"pinActuate.h"
#include"cmdActuate.h"
#include"pseudoJSON.h"
//cmdActuate

#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883
#define PLIMIT uint8_t(30) //payload string length

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;
void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}
bool mqttPayloadHandler(char* topic,char *payload,size_t len){

  bool retval=true;//return value
  size_t i;
  char pcpy[PLIMIT]={'\0'};//payload copy
  if (len>PLIMIT){
    return false;
  }
  for (i=0;i<len;i++)
  {
    pcpy[i]=payload[i];
  }// note, that we do not check till len-1 because pcpy[len] should be NULL
  pcpy[i]='\0';
  Serial.println(pcpy);

  // 4rn/dev 4rn/pin
  if (strcmp(topic,"4rn/pin")==0){
  errno = 0;
  char *endPtr;
  long result = strtol(pcpy, &endPtr, 10);
  if (endPtr==pcpy||*endPtr !='\0')
    {
        retval=false;//case of bad input(FFl ah or 18r etc.) as well as input string null
    }
    if (errno!=0||(result==LONG_MAX||result==LONG_MIN))
    {
         //EINVAL and ERANGE or *val is set to LONG_MIN or LONG_MAX in case of overflow
        retval=false;
    }
    /*
    10 20 30 40
    11 21 31 41
    12 22 32 42
    */
    if(retval){
    pinActuate(result/10),result%10);
    }
  }

  else if (strcmp(topic,"4rn/dev")==0){
  if (len<2){
    cmdActuate(pcpy[0]);//'r' and all
  }
  }
  return retval;
}

void connectToWifi() {
  //WiFiManager, Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wm;

    // reset settings - wipe stored credentials for testing
    // these are stored by the esp library
    //wm.resetSettings();

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ( "AutoConnectAP"),
    // if empty will auto generate SSID, if password is blank it will be anonymous AP (wm.autoConnect())
    // then goes into a blocking loop awaiting configuration and will return success result

    bool res;
    // res = wm.autoConnect(); // auto generated AP name from chipid
    // res = wm.autoConnect("AutoConnectAP"); // anonymous ap
    res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

    if(!res) {
        Serial.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        Serial.println("connected...yeey :)");
        connectToMqtt();
    }
}
void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("4rn/pin", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
  uint16_t packetIdSub1 = mqttClient.subscribe("4rn/dev", 2);
  // mqttClient.publish("test/lol", 0, true, "test 1");
  // Serial.println("Publishing at QoS 0");
  // uint16_t packetIdPub1 = mqttClient.publish("test/lol", 1, true, "test 2");
  // Serial.print("Publishing at QoS 1, packetId: ");
  // Serial.println(packetIdPub1);
  // uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
  // Serial.print("Publishing at QoS 2, packetId: ");
  // Serial.println(packetIdPub2);
}
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  //{"pin":"1","command":0}
  mqttPayloadHandler(topic,payload,len);
  // Serial.println("Publish received.");
  // Serial.print("  topic: ");
  // Serial.println(topic);
  // Serial.print("  qos: ");
  // Serial.println(properties.qos);
  // Serial.print("  dup: ");
  // Serial.println(properties.dup);
  // Serial.print("  retain: ");
  // Serial.println(properties.retain);
  // Serial.print("  len: ");
  // Serial.println(len);
  // Serial.print("  index: ");
  // Serial.println(index);
  // Serial.print("  total: ");
  // Serial.println(total);
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}
void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  //wifiReconnectTimer.once(2, connectToWifi);
}
void setup() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    // put your setup code here, to run once:
  Serial.begin(115200);
  I2CBeginCustom();
  LittleFSBeginCustom();

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWifi();
}
void loop() {
    // put your main code here, to run repeatedly:

}