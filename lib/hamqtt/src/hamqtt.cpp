/* ================================================================================ */
/*                                                                                  */
/*   hamqtt.cpp                                                                     */
/*                                                                                  */
/*                                                                                  */
/*   Description:                                                                   */
/*               basic support for Home Assitant MQTT integration                   */
/* ================================================================================ */

#include <Arduino.h>
#include <ArduinoJson.h>
#include "hamqtt.hpp"
#include "DebugFnc.h"


MQTTClient Hamqtt::Client(512);

  char * Hamqtt::m_mqttUserName;
  char * Hamqtt::m_mqttPass;
  const char * Hamqtt::m_clientID; 
  WiFiClient * Hamqtt::m_wifiClientPtr;
  int Hamqtt::m_regObjNumb;
  Hamqtt * Hamqtt::m_regObjects[];
  unsigned long Hamqtt::m_datasend_normal_ltime=0;
  unsigned long Hamqtt::m_datasend_lowspeed_ltime=0;
  const char *  Hamqtt::DISCOVERY_PREFIX="homeassistant";

//static EntityConfData * Hamqtt::m_enitiyDB[MAX_REG_ENT];

Hamqtt::Hamqtt(char * const devName,char * const devIndex, char * const node_id, int expire_after):\
m_deviceName(devName),m_devIndex(devIndex),m_expire_after(expire_after),m_node_id(node_id){
  m_nrOFRegEnt=0;
  assert(m_regObjNumb<MAX_REG_OBJ);
  m_regObjects[m_regObjNumb]=this;
  m_regObjNumb++;
}

void Hamqtt::init(WiFiClient * wifiClient, IPAddress & brokerIP,char * mqttUserName,char * mqttPass,const char * clientID){
  Client.begin(brokerIP, *wifiClient);
  Client.onMessage(messageReceived);
  m_wifiClientPtr=wifiClient;
  m_mqttUserName=mqttUserName;
  m_mqttPass=mqttPass;
  m_clientID=clientID;
  
  connect();
}

void Hamqtt::connect() {
  
  DEBUG_LOG0(true,"Mqtt: nconnecting to broker...");    
  if(Client.connect(m_clientID,m_mqttUserName,m_mqttPass)){
    DEBUG_LOG0(true,"\n connected!");
  }
  else{
    DEBUG_LOG0(true,"\n not connected!");
  }
}

void Hamqtt::registerSensorEntity(char * ent_name,PeriodType perType, char * class_,char * unit_of_measurement,char * value_template=nullptr,char * icon=nullptr,char * const entity_category=nullptr){
TODO
}


void Hamqtt::registerEntity(char * const component, char * ent_name,Hamqtt::PeriodType perType, char * class_,char * unit_of_measurement,char * unique_id,char * value_template,char * icon,CmdCallbackType cmdCallback){
  assert(m_nrOFRegEnt<MAX_REG_ENT);
 
  m_enitiyDB[m_nrOFRegEnt]=new EntityConfData[1];
  assert(m_enitiyDB[m_nrOFRegEnt]!=nullptr);
  m_enitiyDB[m_nrOFRegEnt]->ent_name=ent_name;
  m_enitiyDB[m_nrOFRegEnt]->perType= perType;
  m_enitiyDB[m_nrOFRegEnt]->class_=class_;
  m_enitiyDB[m_nrOFRegEnt]->unit_of_measurement=unit_of_measurement;
  m_enitiyDB[m_nrOFRegEnt]->unique_id=unique_id;
  m_enitiyDB[m_nrOFRegEnt]->value_template=value_template;
  m_enitiyDB[m_nrOFRegEnt]->cmdCallback=cmdCallback;
  m_enitiyDB[m_nrOFRegEnt]->component = component;
  DEBUG_LOG0(true,"Mqtt: registerEntity step1");
  delay(100);
  m_enitiyDB[m_nrOFRegEnt]->stateTopicFull= (String)DISCOVERY_PREFIX+(String)"/"+ (String)m_enitiyDB[m_nrOFRegEnt]->component + (String)"/"+String(m_deviceName) +String("_") + String(m_devIndex)+ (String)"/state";
  DEBUG_LOG0(true,"Mqtt: registerEntity step2");
  delay(100);
  m_enitiyDB[m_nrOFRegEnt]->object_id=String(m_deviceName) +String("-") + String(m_devIndex) + String("-") +String(ent_name);
  if(strcmp(component,"number")==0){
    m_enitiyDB[m_nrOFRegEnt]->cmdTopicFull=String(DISCOVERY_PREFIX) + String("/") +  String(m_enitiyDB[m_nrOFRegEnt]->component) + String("/") +String(m_deviceName) +String("_") + String(m_devIndex)+ String("/set");
  }
  DEBUG_LOG0(true,"Mqtt: registerEntity step3");
  delay(100);
  m_enitiyDB[m_nrOFRegEnt]->icon=icon;
  m_enitiyDB[m_nrOFRegEnt]->vType=VTYPE_UNDEF;
  m_enitiyDB[m_nrOFRegEnt]->value.s=nullptr;

  String tmp=m_enitiyDB[m_nrOFRegEnt]->value_template;
  int pos=tmp.indexOf(".")+1;
  int end=tmp.indexOf("}");
  m_enitiyDB[m_nrOFRegEnt]->valueName=tmp.substring(pos,end);
  
  DEBUG_LOG0_NOF(true,m_enitiyDB[m_nrOFRegEnt]->valueName);
  delay(100);
  assert(m_enitiyDB[m_nrOFRegEnt]->valueName.length()<MAX_VALUE_NAME_LEN);
  if(m_enitiyDB[m_nrOFRegEnt]->cmdTopicFull != nullptr){
    Client.subscribe(m_enitiyDB[m_nrOFRegEnt]->cmdTopicFull);
  }
  publishConfOfEntity(m_nrOFRegEnt);
  m_nrOFRegEnt++;
}

void Hamqtt::publishConfOfEntity(int index_of_entity){
  StaticJsonBuffer<1500> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();  


  json["name"]=String(m_enitiyDB[index_of_entity]->ent_name);
  if(m_enitiyDB[index_of_entity]->class_ != nullptr)
    json["device_class"]=m_enitiyDB[index_of_entity]->class_;
  if(m_enitiyDB[index_of_entity]->stateTopicFull!=nullptr){
    json["state_topic"]=m_enitiyDB[index_of_entity]->stateTopicFull;
  }
  if(m_enitiyDB[index_of_entity]->cmdTopicFull!=nullptr)
    json["command_topic"]=m_enitiyDB[index_of_entity]->cmdTopicFull; 
  
  json["object_id"]=m_enitiyDB[index_of_entity]->object_id; 
  json["unit_of_measurement"]=m_enitiyDB[index_of_entity]->unit_of_measurement;
  json["value_template"]=m_enitiyDB[index_of_entity]->value_template;
  if(m_enitiyDB[index_of_entity]->unique_id != nullptr)
    json["unique_id"]= m_enitiyDB[index_of_entity]->unique_id; 
  else  
    json["unique_id"]= json["object_id"]; 
  /*
  JsonObject& device=json.createNestedObject("device"); //po vložení této sekce HA zařízení nevidí
  device["name"]=m_deviceName;
  device["sw_version"]="V1.1";
  */
  json["expire_after"]=m_expire_after;
  if(m_enitiyDB[index_of_entity]->icon != nullptr)
    json["icon"]=m_enitiyDB[index_of_entity]->icon;

  String confTopic = (String)DISCOVERY_PREFIX+(String)"/"+(String)m_enitiyDB[index_of_entity]->component+ String("/") +String(m_enitiyDB[index_of_entity]->object_id) +"/config";
  if(json.success()){
    String str_buf;
    json.prettyPrintTo(str_buf);
    DEBUG_LOG0_NOF(true,str_buf);
    Client.publish(confTopic,str_buf ,true,1);
    DEBUG_LOG0(true,"MQTT:published config");
  }
}

void Hamqtt::publishEntity(int index_of_entity){
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject(); 
  
  switch(m_enitiyDB[index_of_entity]->vType){
    case VTYPE_INT:
      json[m_enitiyDB[index_of_entity]->valueName]=(int)m_enitiyDB[index_of_entity]->value.i;
      break;
    case VTYPE_FLOAT:
      json[m_enitiyDB[index_of_entity]->valueName]=(float)m_enitiyDB[index_of_entity]->value.f;
      break;  

    case VTYPE_STRING:
      json[m_enitiyDB[index_of_entity]->valueName]=(char*)m_enitiyDB[index_of_entity]->value.s;
      break;

    case VTYPE_UNDEF:  
      assert(false);
    break;
  }
  //json[m_enitiyDB[index_of_entity]->ent_name]=m_enitiyDB[index_of_entity]->value;
  if(json.success()){
    DEBUG_LOG0(true,"MQTT:published data");
    String str_buf;
    json.prettyPrintTo(str_buf);
    Client.publish(m_enitiyDB[index_of_entity]->stateTopicFull,str_buf ,true,1);
    DEBUG_LOG0_NOF(true,str_buf);
  }else{  
    DEBUG_LOG0(!json.success(),"MQTT:publish error");
  }
}


void Hamqtt::publisValue(char * ent_name, char * value){
  for(int i=0;i<m_nrOFRegEnt;i++){
    if(strcmp(m_enitiyDB[i]->ent_name,ent_name)==0){
      m_enitiyDB[i]->value.s=value;
      m_enitiyDB[i]->vType=VTYPE_STRING;
      publishEntity(i);
      return;
    }
  }
  DEBUG_LOG0(true,"MQTT:publisValue:entity not found");
}

void Hamqtt::publisValue(char * ent_name, float value){
  for(int i=0;i<m_nrOFRegEnt;i++){
    if(strcmp(m_enitiyDB[i]->ent_name,ent_name)==0){
      m_enitiyDB[i]->value.f=value;
      m_enitiyDB[i]->vType=VTYPE_FLOAT;
      publishEntity(i);
      return;
    }
  }
  DEBUG_LOG0(true,"MQTT:publisValue:entity not found");
}

void Hamqtt::publisValuesPer(PeriodType period){
  for(int i=0;i<m_nrOFRegEnt;i++){
    if(m_enitiyDB[i]->perType==period && m_enitiyDB[i]->vType!=VTYPE_UNDEF){
      publishEntity(i);
    }
  }
}


void Hamqtt::main(){
  unsigned long delTime=0;
  delTime = millis() - m_datasend_normal_ltime;  
  Client.loop();
  if(delTime >= DATASEND_NORMAL_PER){
    m_datasend_normal_ltime = millis();
    for(int objInd=0;objInd<m_regObjNumb;objInd++){
      Hamqtt * objPtr=m_regObjects[objInd];
      if(WiFi.status() == WL_CONNECTED){
        if (!Client.connected())connect();
        objPtr->publisValuesPer(PERTYPE_NORMAL);
      }
    }  
  }

  delTime = millis() - m_datasend_lowspeed_ltime;
  if(delTime >= DATASEND_LOWSPEED_PER){
    m_datasend_lowspeed_ltime = millis();
    for(int objInd=0;objInd<m_regObjNumb;objInd++){
      Hamqtt * objPtr=m_regObjects[objInd];   
      if(WiFi.status() == WL_CONNECTED){
        if (!Client.connected())connect();
        objPtr->publisValuesPer(PERTYPE_LOWSPEED);
      }
    }  
  }

} 


void Hamqtt::messageReceived(String &topic, String &payload){
  DEBUG_LOG0_NOF(true,"MQTT:messageReceived [" + topic + "] " + payload);
  for(int objInd=0;objInd<m_regObjNumb;objInd++){
    assert(m_regObjects[objInd]!=nullptr);
    Hamqtt * objPtr=m_regObjects[objInd];
    for(int i=0;i<objPtr->m_nrOFRegEnt;i++){
      if(objPtr->m_enitiyDB[i]->cmdTopicFull==topic){
        if(objPtr->m_enitiyDB[i]->cmdCallback!=nullptr){
          objPtr->m_enitiyDB[i]->cmdCallback(i,payload);
        }
      }
    }
  }  
}