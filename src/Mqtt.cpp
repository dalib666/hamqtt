#include <ESP8266WiFi.h>

#include "hamqtt.hpp"
#define DEBUG_MODE 
#include "DebugFnc.h"  

IPAddress BrokerIP(192,168,1,6);     
char MqttUserName[] = "homeassistant"; 
char MqttPass[] = "ol3uuNeek6ke7eich8aiva7ZoxoiVei1aiteith0aighae0ieP7pahFaNgeiP8de";

const char ClientID[] ="WaterHeater";
#define DEVICE_NAME "W. Heater"
#define DEVICE_INDEX_NAME "1"   // to resolve more instancies of device, if it is sence to have more instancies in one installation, 
                                // then it must be as NV parameter
#define EXPIRATION_TIME 60    //[sec] expire time in HA after lost data
//topics - published

//#define PUB_CONFTOPIC1 "homeassistant/sensor/boyler/config"
//#define PUB_TOPIC1  "homeassistant/sensor/boyler/wTemp"         //actual water temperature
//#define PUB_CONFTOPIC2 "homeassistant/number/boyler/config"
#define SUB_TOPIC2 "homeassistant/number/boyler/reqPower/set"       //request power
#define PUB_TOPIC2 "homeassistant/number/boyler/reqPower/state"       //request power


void entCallBack(int indOfEnt, String &payload){
  DEBUG_LOG(true,"entCallBack:indOfEnt= ",indOfEnt);
  DEBUG_LOG_NOF(true,"payload= ",payload.c_str());
}

// Initialize the client library
WiFiClient Wclient;
Hamqtt DevObj(DEVICE_NAME,DEVICE_INDEX_NAME,"homeassistant/sensor/boyler/config", EXPIRATION_TIME,"homeassistant/number/boyler");
void Mqtt_init(){
    Hamqtt::init(&Wclient, BrokerIP,MqttUserName,MqttPass,ClientID);
    DEBUG_LOG0(true,"Mqtt init");
    DevObj.registerEntity("Water_Temp",Hamqtt::PERTYPE_NORMAL,"temperature","°C",nullptr,"{{value_json.wTemp}}","wTemp",nullptr,"mdi:thermometer");
    DevObj.registerEntity("Req_Power",Hamqtt::PERTYPE_NORMAL,nullptr,"W",nullptr,"{{value_json.actPower}}","reqPower/state","reqPower/set","material-symbols:mode-heat-outline",entCallBack);
    DEBUG_LOG0(true,"registerEntity");
    DevObj.publisValue("Water_Temp", 21.9f); //test value
    DevObj.publisValue("Req_Power", 890); //test value
    DEBUG_LOG0(true,"publisValue");
}


 //publishConfig(DEVICE_INDEX_NAME,"Req Power",nullptr,PUB_CONFTOPIC2,"W",nullptr,"{{value_json.reqPower}}",PUB_TOPIC2, SUB_TOPIC2);
