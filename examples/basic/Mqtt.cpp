#include <ESP8266WiFi.h>

#include "hamqtt.hpp"

#include "DebugFnc.h"  

IPAddress BrokerIP(192,168,1,5);     
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
Hamqtt DevObj("BMS", "","BatCC01","DK",SW_VERSION,"001","www.google.com",HW_VERSION,nullptr,EXPIRATION_TIME);

void entCallBack(int indOfEnt, String &payload){
  /*DEBUG_LOG(true,"entCallBack:indOfEnt= ",indOfEnt);
  DEBUG_LOG_NOF(true,"payload= ",payload.c_str());
  DevObj.publishValue("Req_Power", payload.toFloat()); //test value*/
}


void Mqtt_init(){
    Hamqtt::init(&Wclient, BrokerIP,MqttUserName,MqttPass,ClientID);
    DEBUG_LOG0(true,"Mqtt init");
    //DevObj.registerEntity("sensor","Water_Temp",Hamqtt::PERTYPE_NORMAL,"temperature","°C",nullptr,"{{value_json.wTemp}}","mdi:thermometer");
    DevObj.registerSensorEntity("soc",Hamqtt::PERTYPE_LOWSPEED,"battery","%");
    DevObj.registerSensorEntity("alert",Hamqtt::PERTYPE_NORMAL,nullptr,nullptr);
    DevObj.registerSensorEntity("warning",Hamqtt::PERTYPE_NORMAL,nullptr,nullptr);  
    DevObj.registerSensorEntity("ubat",Hamqtt::PERTYPE_LOWSPEED,"voltage","V");  
    DevObj.registerSensorEntity("ibat",Hamqtt::PERTYPE_HIGHSPEED,"current","A");  
    DevObj.registerSensorEntity("tbat",Hamqtt::PERTYPE_LOWSPEED,"temperature","°C",nullptr,6);      
    DEBUG_LOG0(true,"registerEntity");
    DevObj.publishValue("soc", 88.2f); //test value
    DevObj.publishValue("alert", "alert_text"); //test value
    DevObj.publishValue("warning", "warning_text"); //test value
    DevObj.publishValue("ubat", 259.1f); //test value
    DevObj.publishValue("ibat", 9.9f); //test value
    DevObj.publishValue("tbat", 10.0f,0); //test value
    DevObj.publishValue("tbat", 10.1f,1); //test value
    DevObj.publishValue("tbat", 10.2f,2); //test value    
    DevObj.publishValue("tbat", 10.3f,3); //test value
    DevObj.publishValue("tbat", 10.4f,4); //test value
    DevObj.publishValue("tbat", 10.5f,5); //test value  
    DEBUG_LOG0(true,"publisValue");
}

void Mqtt_loop5s(){
    static float testVal = 0.0f;
    testVal+=1.1;
    if (testVal > 100.0f) testVal = 0.0f;

    //DevObj.publishValue("Water_Temp", testVal); //test value
    
}
 //publishConfig(DEVICE_INDEX_NAME,"Req Power",nullptr,PUB_CONFTOPIC2,"W",nullptr,"{{value_json.reqPower}}",PUB_TOPIC2, SUB_TOPIC2);
