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


#define getIndexStr(entIndex,itemIndex) ((m_enitiyDB[entIndex]->entNumber==1)? "":String(itemIndex))


MQTTClient Hamqtt::Client(768);
  unsigned long Hamqtt::m_DatasendNormalPer;
  unsigned long Hamqtt::m_DatasendLowPer;
  unsigned long Hamqtt::m_DatasendHighPer;
  const char * Hamqtt::m_mqttUserName;
  const char * Hamqtt::m_mqttPass;
  const char * Hamqtt::m_clientID; 
  WiFiClient * Hamqtt::m_wifiClientPtr;
  int Hamqtt::m_regObjNumb;
  Hamqtt * Hamqtt::m_regObjects[];
  unsigned long Hamqtt::m_datasend_normal_ltime=0;
  unsigned long Hamqtt::m_datasend_lowspeed_ltime=0;
  unsigned long Hamqtt::m_datasend_highspeed_ltime=0;
  const char *  Hamqtt::DISCOVERY_PREFIX="homeassistant";
 
//static EntityConfData * Hamqtt::m_enitiyDB[MAX_REG_ENT]; 
Hamqtt::Hamqtt(const char * devName,const char *  devIndex, PeriodType grPerType, const char * model, const char * manufacturer, const char * swVersion, const char * identifiers, const char * configuration_url, const char * hw_version, const char * via_device, int expire_after):\
m_devIndex(devIndex),m_expire_after(expire_after),m_deviceName(devName),m_model(model),m_manufacturer(manufacturer),m_swVersion(swVersion),m_identifiers(identifiers),m_configuration_url(configuration_url),m_hw_version(hw_version),m_via_device(via_device),m_grPerType(grPerType){
  if(m_devIndex==nullptr)
    m_devIndex="";
  m_nrOFRegEnt=0;
  assert(m_regObjNumb<MAX_REG_OBJ);
  assert(m_deviceName!=nullptr);
  m_regObjects[m_regObjNumb]=this;
  m_regObjNumb++;
  m_pubEnabled=false;
}

void Hamqtt::init(WiFiClient * wifiClient, IPAddress & brokerIP,const char * mqttUserName,const char * mqttPass,const char * clientID,unsigned int normalPer,unsigned int lowPer,unsigned int highPer){
  Client.begin(brokerIP, *wifiClient);
  Client.onMessage(messageReceived);
  m_wifiClientPtr=wifiClient;
  m_mqttUserName=mqttUserName;
  m_mqttPass=mqttPass;
  m_clientID=clientID;
  m_DatasendNormalPer=normalPer;
  m_DatasendLowPer=lowPer;
  m_DatasendHighPer=highPer;
  
  connect();
}

void Hamqtt::startPublishing(){
  m_pubEnabled=true;
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


void Hamqtt::registerSensorEntity(const char * ent_name,PeriodType perType, const char * class_,const char * unit_of_measurement,const char * icon,int entNumber,bool grStTopic){
  registerEntity("sensor",ent_name,perType,class_,unit_of_measurement,nullptr,icon,nullptr,"diagnostic",entNumber,grStTopic);
}
void Hamqtt::registerNumberEntity(const char * ent_name,PeriodType perType, const char * class_,const char * unit_of_measurement,const char * icon,CmdCallbackType cmdCallback,bool grStTopic){
  registerEntity("number",ent_name,perType,class_,unit_of_measurement,nullptr,icon,cmdCallback,"config",1,grStTopic);
}

void Hamqtt::registerEntity(const char * component, const char * ent_name,Hamqtt::PeriodType perType, const char * class_,const char * unit_of_measurement,const char * unique_id,const char * icon,CmdCallbackType cmdCallback,const char * entity_category, int entNumber,bool grStTopic){
  assert(m_nrOFRegEnt<MAX_REG_ENT);
 
  m_enitiyDB[m_nrOFRegEnt]=new EntityConfData[1];
  assert(m_enitiyDB[m_nrOFRegEnt]!=nullptr);
  m_enitiyDB[m_nrOFRegEnt]->grStateTopic =grStTopic;
  m_enitiyDB[m_nrOFRegEnt]->entNumber=entNumber;
  m_enitiyDB[m_nrOFRegEnt]->ent_name=ent_name;
  m_enitiyDB[m_nrOFRegEnt]->perType= perType;
  m_enitiyDB[m_nrOFRegEnt]->class_=class_;
  m_enitiyDB[m_nrOFRegEnt]->unit_of_measurement=unit_of_measurement;
  m_enitiyDB[m_nrOFRegEnt]->unique_id=unique_id;
  m_enitiyDB[m_nrOFRegEnt]->cmdCallback=cmdCallback;
  m_enitiyDB[m_nrOFRegEnt]->component = component;
  m_enitiyDB[m_nrOFRegEnt]->entity_category=entity_category;
  m_enitiyDB[m_nrOFRegEnt]->value=new ValueType[entNumber];
  assert(m_enitiyDB[m_nrOFRegEnt]->value != nullptr);
  if(grStTopic){
    m_grStateTopic=true;
    if(m_grStateTopicFull.isEmpty())
      m_grStateTopicFull=(String)DISCOVERY_PREFIX+(String)"/"+ (String)m_enitiyDB[m_nrOFRegEnt]->component + (String)"/"+String(m_deviceName) + String(m_devIndex)+ (String)"/state"; //set topic common for grouped entities
    m_enitiyDB[m_nrOFRegEnt]->stateTopicFull = m_grStateTopicFull;	
  }
  else
    m_enitiyDB[m_nrOFRegEnt]->stateTopicFull= (String)DISCOVERY_PREFIX+(String)"/"+ (String)m_enitiyDB[m_nrOFRegEnt]->component + (String)"/"+String(m_deviceName) + String(m_devIndex)+(String)"/" + String(m_enitiyDB[m_nrOFRegEnt]->ent_name) + (String)"/state"; //set topic individual for entity

  m_enitiyDB[m_nrOFRegEnt]->object_id=String(m_deviceName) + String(m_devIndex) + String("-") +String(ent_name);
  if(strcmp(component,"number")==0){
    m_enitiyDB[m_nrOFRegEnt]->cmdTopicFull=String(DISCOVERY_PREFIX) + String("/") +  String(m_enitiyDB[m_nrOFRegEnt]->component) + String("/") +String(m_deviceName) + String(m_devIndex)+  String("/set");
  }
    
 
  m_enitiyDB[m_nrOFRegEnt]->icon=icon;
  m_enitiyDB[m_nrOFRegEnt]->vType=VTYPE_UNDEF;
  //m_enitiyDB[m_nrOFRegEnt]->value.s=nullptr;

  m_enitiyDB[m_nrOFRegEnt]->valueName=m_enitiyDB[m_nrOFRegEnt]->ent_name;
  
  DEBUG_LOG0_NOF(true,m_enitiyDB[m_nrOFRegEnt]->valueName);
  delay(100);
  assert(m_enitiyDB[m_nrOFRegEnt]->valueName.length()<MAX_VALUE_NAME_LEN);
  if(m_enitiyDB[m_nrOFRegEnt]->cmdTopicFull != nullptr){
    Client.subscribe(m_enitiyDB[m_nrOFRegEnt]->cmdTopicFull);
  }
  for(int i=0;i<m_enitiyDB[m_nrOFRegEnt]->entNumber;i++){
    publishConfOfEntity(m_nrOFRegEnt,i);
  }
  m_nrOFRegEnt++;
}

void Hamqtt::publishConfOfEntity(int index_of_entity, int index_of_item){
  StaticJsonBuffer<1500> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();  

  DEBUG_LOG(true,"publishConfOfEntity: index_of_item=",index_of_item);  
  json["name"]=String(m_enitiyDB[index_of_entity]->ent_name)+getIndexStr(index_of_entity,index_of_item);
  if(m_enitiyDB[index_of_entity]->class_ != nullptr)
    json["device_class"]=m_enitiyDB[index_of_entity]->class_;
  if(m_enitiyDB[index_of_entity]->stateTopicFull!=nullptr){
    json["state_topic"]=m_enitiyDB[index_of_entity]->stateTopicFull;
  }

  JsonObject& device=json.createNestedObject("device"); //po vložení této sekce HA zařízení nevidí
 
  //JsonArray& ident_list= device.createNestedArray("identifiers");
  //ident_list.add(String(m_deviceName) +String("-") + String(m_devIndex));
  if(m_model!=nullptr)
    device["model"]=m_model;
  if(m_identifiers!=nullptr)
    device["identifiers"]=m_identifiers;
  device["name"]=m_deviceName;
  if(m_swVersion!=nullptr)
    device["sw_version"]=m_swVersion;
  if(m_manufacturer!=nullptr)
    device["manufacturer"]=m_manufacturer;
  if(m_hw_version!=nullptr)
    device["hw_version"]=m_hw_version;
  if(m_configuration_url!=nullptr)
    device["configuration_url"]=m_configuration_url;
  if(m_via_device!=nullptr)
    device["via_device"]=m_via_device;


  if(m_enitiyDB[index_of_entity]->cmdTopicFull!=nullptr){
    assert(m_enitiyDB[index_of_entity]->entNumber==1);
    json["command_topic"]=m_enitiyDB[index_of_entity]->cmdTopicFull; 
  }
  json["object_id"]=m_enitiyDB[index_of_entity]->object_id+getIndexStr(index_of_entity,index_of_item); 
  if(m_enitiyDB[index_of_entity]->unit_of_measurement==nullptr)
    json["unit_of_measurement"]="";
  else
    json["unit_of_measurement"]=m_enitiyDB[index_of_entity]->unit_of_measurement;
  json["value_template"]=String("{{value_json.") + String(m_enitiyDB[index_of_entity]->ent_name)+getIndexStr(index_of_entity,index_of_item) + String("}}");
  
  if(m_enitiyDB[index_of_entity]->unique_id != nullptr)
    json["unique_id"]= m_enitiyDB[index_of_entity]->unique_id+getIndexStr(index_of_entity,index_of_item);
  else  
    json["unique_id"]= m_enitiyDB[index_of_entity]->object_id+getIndexStr(index_of_entity,index_of_item); 
  
  
  json["expire_after"]=getPeriod(index_of_entity) * m_expire_after/1000;
  if(m_enitiyDB[index_of_entity]->icon != nullptr)
    json["icon"]=m_enitiyDB[index_of_entity]->icon;
  if(m_enitiyDB[index_of_entity]->entity_category != nullptr)
    json["entity_category"]=m_enitiyDB[index_of_entity]->entity_category;
  String confTopic = (String)DISCOVERY_PREFIX+(String)"/"+(String)m_enitiyDB[index_of_entity]->component+ String("/") +String(m_enitiyDB[index_of_entity]->object_id)+getIndexStr(index_of_entity,index_of_item) +"/config";
  if(json.success()){
    String str_buf;
    json.prettyPrintTo(str_buf);
    DEBUG_LOG0_NOF(true,str_buf);
    bool pubStatus = Client.publish(confTopic,str_buf ,true,1);
    DEBUG_LOG0(pubStatus,"MQTT:published config");
    DEBUG_LOG(!pubStatus,"MQTT:not published config !!! error=",Client.lastError()); 
  }
}



void Hamqtt::publishEntity(int index_of_entity, int index_of_item){
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject(); 
  if(!m_pubEnabled)
    return;
  assert(m_enitiyDB[index_of_entity]->entNumber>index_of_item);
  switch(m_enitiyDB[index_of_entity]->vType){
    case VTYPE_INT:
      json[m_enitiyDB[index_of_entity]->valueName + getIndexStr(index_of_entity,index_of_item)]=(int)m_enitiyDB[index_of_entity]->value[index_of_item].i;
      break;
    case VTYPE_FLOAT:
      json[m_enitiyDB[index_of_entity]->valueName+ getIndexStr(index_of_entity,index_of_item)]=(float)m_enitiyDB[index_of_entity]->value[index_of_item].f;
      break;  

    case VTYPE_STRING:
      json[m_enitiyDB[index_of_entity]->valueName+ getIndexStr(index_of_entity,index_of_item)]=(char*)m_enitiyDB[index_of_entity]->value[index_of_item].s;
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


void Hamqtt::publishValue(const char  * ent_name, const char * value,bool onlyChange){

  for(int i=0;i<m_nrOFRegEnt;i++){
    if(strcmp(m_enitiyDB[i]->ent_name,ent_name)==0){
      assert(m_enitiyDB[i]->entNumber==1);
      assert(m_enitiyDB[i]->grStateTopic==false);      
      if(!onlyChange || strcmp(m_enitiyDB[i]->value[0].s,value)!=0){
        m_enitiyDB[i]->value[0].s=value;
        m_enitiyDB[i]->vType=VTYPE_STRING;
        publishEntity(i,0);
      }  
      return;
    }
  }
  DEBUG_LOG0(true,"MQTT:publisValue:entity not found");
}

void Hamqtt::publishValue(const char  * ent_name, float value,bool onlyChange){
  for(int i=0;i<m_nrOFRegEnt;i++){
    if(strcmp(m_enitiyDB[i]->ent_name,ent_name)==0){
      assert(m_enitiyDB[i]->entNumber==1);
      assert(m_enitiyDB[i]->grStateTopic==false);  
      if(!onlyChange || m_enitiyDB[i]->value[0].f!=value){
        m_enitiyDB[i]->value[0].f=value;
        m_enitiyDB[i]->vType=VTYPE_FLOAT;
        publishEntity(i,0);
      }
      return;
    }
  }
  DEBUG_LOG0(true,"MQTT:publisValue:entity not found");
}
/**
 * @brief publish value of entity, only entites which are not grouped in one topic
*/
void Hamqtt::publisValuesPer(PeriodType period){
  for(int i=0;i<m_nrOFRegEnt;i++){
    if(m_enitiyDB[i]->perType==period && m_enitiyDB[i]->vType!=VTYPE_UNDEF && !m_enitiyDB[i]->grStateTopic){
      assert(m_enitiyDB[i]->entNumber=1); //multiple entities must be grouped in one topic
      publishEntity(i,0);
    }
  }
}

void Hamqtt::publishGroupedEntities(){
  StaticJsonBuffer<1000> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject(); 
  for(int i=0;i<m_nrOFRegEnt;i++){
    if(m_enitiyDB[i]->grStateTopic){
      for(int j=0;j<m_enitiyDB[i]->entNumber;j++){
        switch(m_enitiyDB[i]->vType){
          case VTYPE_INT:
            json[m_enitiyDB[i]->valueName + getIndexStr(i,j)]=(int)m_enitiyDB[i]->value[j].i;
            break;
          case VTYPE_FLOAT:
            json[m_enitiyDB[i]->valueName+ getIndexStr(i,j)]=(float)m_enitiyDB[i]->value[j].f;
            break;  

          case VTYPE_STRING:
            json[m_enitiyDB[i]->valueName+ getIndexStr(i,j)]=(char*)m_enitiyDB[i]->value[j].s;
            break;

          case VTYPE_UNDEF:  
            assert(false);
          break;
        }
      }
    }
  }
  if(json.success()){
    DEBUG_LOG0(true,"MQTT:published data");
    String str_buf;
    json.prettyPrintTo(str_buf);
    bool pubStatus=Client.publish(m_grStateTopicFull,str_buf ,true,1);
    assert(pubStatus);
    DEBUG_LOG0_NOF(true,str_buf);
  }else{  
    DEBUG_LOG0(!json.success(),"MQTT:publish error");
  }
}


void Hamqtt::main(){
  unsigned long delTime=0;  
  delTime = millis() - m_datasend_normal_ltime;  
  Client.loop();
  if(delTime >= m_DatasendNormalPer){
    m_datasend_normal_ltime = millis();
    main_int(PERTYPE_NORMAL);
  }

  delTime = millis() - m_datasend_lowspeed_ltime;
  if(delTime >= m_DatasendLowPer){
    m_datasend_lowspeed_ltime = millis();
    main_int(PERTYPE_LOWSPEED);
  }

  delTime = millis() - m_datasend_highspeed_ltime;
  if(delTime >= m_DatasendHighPer){
    m_datasend_highspeed_ltime = millis();
    main_int(PERTYPE_HIGHSPEED);
  }
} 

void Hamqtt::main_int(PeriodType perType){
  for(int objInd=0;objInd<m_regObjNumb;objInd++){
    Hamqtt * objPtr=m_regObjects[objInd];   
    if(objPtr->m_pubEnabled){
      if(WiFi.status() == WL_CONNECTED){
        if (!Client.connected())
          connect();
        if(objPtr->m_grStateTopic && objPtr->m_grPerType==perType)
          objPtr->publishGroupedEntities();

        objPtr->publisValuesPer(perType);
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

unsigned long Hamqtt::getPeriod(int index_of_entity){
  
  switch(m_enitiyDB[index_of_entity]->perType){
    case PERTYPE_NORMAL:
      return m_DatasendNormalPer;
    case PERTYPE_LOWSPEED:
      return m_DatasendLowPer;
    case PERTYPE_HIGHSPEED:
      return m_DatasendHighPer;
  }

  assert(false);
  return 0;
}

void Hamqtt::writeValue(const char * ent_name, const char * value,int item){
  for(int i=0;i<m_nrOFRegEnt;i++){
    if(m_enitiyDB[i]->ent_name==ent_name){
      m_enitiyDB[i]->vType=VTYPE_STRING;
      assert(m_enitiyDB[i]->entNumber>item);
      m_enitiyDB[i]->value[item].s=value;
      return;
    }
  }
  assert(false);
} 
void Hamqtt::writeValue(const char * ent_name, float value,int item){
  for(int i=0;i<m_nrOFRegEnt;i++){
    if(m_enitiyDB[i]->ent_name==ent_name){
      m_enitiyDB[i]->vType=VTYPE_FLOAT;
      assert(m_enitiyDB[i]->entNumber>item);
      m_enitiyDB[i]->value[item].f=value;
      return;
    }
  }
  assert(false);
}