#include <ESP8266WiFi.h>

#include "hamqtt.hpp"

#include "DebugFnc.h"  

IPAddress BrokerIP(192,168,1,6);     
char MqttUserName[] = "homeassistant"; 
char MqttPass[] = "ol3uuNeek6ke7eich8aiva7ZoxoiVei1aiteith0aighae0ieP7pahFaNgeiP8de";
#define SW_VERSION "1.0.1"
#define HW_VERSION "1.0"
const char ClientID[] ="WaterHeater";
#define DEVICE_INDEX_NAME "1"   // to resolve more instancies of device, if it is sence to have more instancies in one installation, 
                                // then it must be as NV parameter
#define EXPIRATION_TIME 60    //[sec] expire time in HA after lost data
//topics - published



// Initialize the client library
WiFiClient Wclient;
Hamqtt DevObj("W_Heater", DEVICE_INDEX_NAME,"WHControl01","DK",SW_VERSION,"001","www.dummy.com",HW_VERSION,nullptr,EXPIRATION_TIME);

void entCallBack(int indOfEnt, String &payload){
  DEBUG_LOG(true,"entCallBack:indOfEnt= ",indOfEnt);
  DEBUG_LOG_NOF(true,"payload= ",payload.c_str());
  DevObj.publishValue("Req_Power", payload.toFloat()); //test value
}


void Mqtt_init(){
    Hamqtt::init(&Wclient, BrokerIP,MqttUserName,MqttPass,ClientID);
    DEBUG_LOG0(true,"Mqtt init");
    //DevObj.registerEntity("sensor","Water_Temp",Hamqtt::PERTYPE_NORMAL,"temperature","°C",nullptr,"{{value_json.wTemp}}","mdi:thermometer");
    DevObj.registerSensorEntity("Water_Temp",Hamqtt::PERTYPE_NORMAL,"temperature","°C","{{value_json.wTemp}}","mdi:thermometer");
    DevObj.registerEntity("number","Req_Power",Hamqtt::PERTYPE_NORMAL,nullptr,"W",nullptr,"{{value_json.actPower}}","material-symbols:mode-heat-outline",entCallBack);
    DEBUG_LOG0(true,"registerEntity");
    DevObj.publishValue("Water_Temp", 21.9f); //test value
    DevObj.publishValue("Req_Power", 0.0f); //test value
    DEBUG_LOG0(true,"publisValue");
}

void Mqtt_loop5s(){
    static float testVal = 0.0f;
    testVal+=1.1;
    if (testVal > 100.0f) testVal = 0.0f;

    DevObj.publishValue("Water_Temp", testVal); //test value
    DevObj.publishValue("Req_Power", testVal+1); //test value
}
 //publishConfig(DEVICE_INDEX_NAME,"Req Power",nullptr,PUB_CONFTOPIC2,"W",nullptr,"{{value_json.reqPower}}",PUB_TOPIC2, SUB_TOPIC2);
