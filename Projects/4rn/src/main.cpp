#include<AsyncMqttClient.h>
#include<WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include<Ticker.h>
#include<errno.h>
#include<limits.h>
#include"pinActuate.h"
#include"cmdActuate.h"
#include"pseudoJSON.h"

#define MQTT_HOST "broker.hivemq.com"
#define MQTT_PORT 1883
#define PLIMIT uint8_t(30) //payload string length
#define SPORT Serial1
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;
void connectToMqtt() {
  SPORT.println("Connecting to MQTT...");
  mqttClient.connect();
}
bool mqttPayloadHandler(char* topic,char *payload,size_t len){
  char pseudoJSON[20];
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
  SPORT.println(pcpy);

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
      if(pinActuate((result/10),result%10)){
        pseudoJson('p');
      }   
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
        SPORT.println("Failed to connect");
        // ESP.restart();
    } 
    else {
        //if you get here you have connected to the WiFi    
        SPORT.println("connected...yeey :)");
        connectToMqtt();
    }
}
void onMqttConnect(bool sessionPresent) {
  SPORT.println("Connected to MQTT.");
  SPORT.print("Session present: ");
  SPORT.println(sessionPresent);
  uint16_t packetIdSub = mqttClient.subscribe("4rn/pin", 2);
  SPORT.print("Subscribing at QoS 2, packetId: ");
  SPORT.println(packetIdSub);
  uint16_t packetIdSub1 = mqttClient.subscribe("4rn/dev", 2);
  // mqttClient.publish("test/lol", 0, true, "test 1");
  // SPORT.println("Publishing at QoS 0");
  // uint16_t packetIdPub1 = mqttClient.publish("test/lol", 1, true, "test 2");
  // SPORT.print("Publishing at QoS 1, packetId: ");
  // SPORT.println(packetIdPub1);
  // uint16_t packetIdPub2 = mqttClient.publish("test/lol", 2, true, "test 3");
  // SPORT.print("Publishing at QoS 2, packetId: ");
  // SPORT.println(packetIdPub2);
}
void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  SPORT.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  SPORT.println("Subscribe acknowledged.");
  SPORT.print("  packetId: ");
  SPORT.println(packetId);
  SPORT.print("  qos: ");
  SPORT.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  SPORT.println("Unsubscribe acknowledged.");
  SPORT.print("  packetId: ");
  SPORT.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  //{"pin":"1","command":0}
  mqttPayloadHandler(topic,payload,len);
  // SPORT.println("Publish received.");
  // SPORT.print("  topic: ");
  // SPORT.println(topic);
  // SPORT.print("  qos: ");
  // SPORT.println(properties.qos);
  // SPORT.print("  dup: ");
  // SPORT.println(properties.dup);
  // SPORT.print("  retain: ");
  // SPORT.println(properties.retain);
  // SPORT.print("  len: ");
  // SPORT.println(len);
  // SPORT.print("  index: ");
  // SPORT.println(index);
  // SPORT.print("  total: ");
  // SPORT.println(total);
}

void onMqttPublish(uint16_t packetId) {
  SPORT.println("Publish acknowledged.");
  SPORT.print("  packetId: ");
  SPORT.println(packetId);
}
void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  SPORT.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  SPORT.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  //wifiReconnectTimer.once(2, connectToWifi);
}
void setup() {
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    // it is a good practice to make sure your code sets wifi mode how you want it.

    // put your setup code here, to run once:
  SPORT.begin(115200);
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