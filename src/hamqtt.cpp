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

CircularBuffer<Hamqtt::BuffElemnet *,5>  Hamqtt::m_buffer;
PubSubClient Hamqtt::MQTTClient;
  unsigned long Hamqtt::m_DatasendNormalPer;
  unsigned long Hamqtt::m_DatasendLowPer;
  unsigned long Hamqtt::m_DatasendHighPer;
  const char * Hamqtt::m_mqttUserName;
  const char * Hamqtt::m_mqttPass;
  const char * Hamqtt::m_clientID; 
  int Hamqtt::m_regObjNumb;
  Hamqtt * Hamqtt::m_regObjects[];
  unsigned long Hamqtt::m_datasend_normal_ltime=0;
  unsigned long Hamqtt::m_datasend_lowspeed_ltime=0;
  unsigned long Hamqtt::m_datasend_highspeed_ltime=0;
  const char *  Hamqtt::DISCOVERY_PREFIX="homeassistant";
  unsigned long Hamqtt::m_lastConnectAttemp;
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
  m_lastConnectAttemp=0;
}

void Hamqtt::init(WiFiClient * wifiClient, IPAddress & brokerIP,const char * mqttUserName,const char * mqttPass,const char * clientID,unsigned int normalPer,unsigned int lowPer,unsigned int highPer){
  
  MQTTClient.setBufferSize(768);
  MQTTClient.setClient(*wifiClient);
  MQTTClient.setServer(*brokerIP,1883);
  MQTTClient.setCallback(messageReceived);
  
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


void Hamqtt::connect(bool recon) {
  if((millis() - m_lastConnectAttemp) < 3000)
    return; 
  DEBUG_LOG0(true,"Mqtt: connecting to broker...");    
  if(MQTTClient.connect(m_clientID,m_mqttUserName,m_mqttPass)){
    DEBUG_LOG0(true,"connected! \n");
           
    if(recon){
      for(int objInd=0;objInd<m_regObjNumb;objInd++){
        Hamqtt * objPtr=m_regObjects[objInd];   
        for(int i=0;i<objPtr->m_nrOFRegEnt;i++){
          if(!objPtr->m_enitiyDB[i]->cmdTopicFull.isEmpty()){
            MQTTClient.subscribe(objPtr->m_enitiyDB[i]->cmdTopicFull.c_str());
          }
        }
      }      
    } 
  }
  else{
    m_lastConnectAttemp=millis();
    DEBUG_LOG0(true,"\n not connected!");
  }
}


void Hamqtt::registerSensorEntity(const char * ent_name,PeriodType perType, const char * class_,const char * unit_of_measurement,const char * icon,int entNumber,bool grStTopic){
  registerEntity("sensor",ent_name,perType,class_,unit_of_measurement,nullptr,icon,nullptr,"diagnostic",entNumber,grStTopic);
}
void Hamqtt::registerNumberEntity(const char * ent_name,PeriodType perType, const char * class_,const char * unit_of_measurement,const char * icon,CmdCallbackType cmdCallback,bool grStTopic,float max, float min){
  registerEntity("number",ent_name,perType,class_,unit_of_measurement,nullptr,icon,cmdCallback,"config",1,grStTopic);
}

void Hamqtt::registerSwitchEntity(const char * ent_name,PeriodType perType, const char * class_,const char * icon,CmdCallbackType cmdCallback,bool grStTopic){
  registerEntity("switch",ent_name,perType,class_,nullptr,nullptr,icon,cmdCallback,"config",1,grStTopic);
}



void Hamqtt::registerEntity(const char * component, const char * ent_name,Hamqtt::PeriodType perType, const char * class_,const char * unit_of_measurement,const char * unique_id,const char * icon,\
CmdCallbackType cmdCallback,const char * entity_category, int entNumber,bool grStTopic,float max, float min){
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
  m_enitiyDB[m_nrOFRegEnt]->max=max;
  m_enitiyDB[m_nrOFRegEnt]->min=min;
  m_enitiyDB[m_nrOFRegEnt]->cmdTopicFull.clear();

  assert(m_enitiyDB[m_nrOFRegEnt]->value != nullptr);
  if(strcmp(m_enitiyDB[m_nrOFRegEnt]->component,"switch")==0){
    for(int ind=0;ind<entNumber;ind++)
      m_enitiyDB[m_nrOFRegEnt]->value[ind].s="";
  }

  if(grStTopic){
    m_grStateTopic=true;
    if(m_grStateTopicFull.isEmpty())
      m_grStateTopicFull=(String)DISCOVERY_PREFIX+(String)"/"+ (String)m_enitiyDB[m_nrOFRegEnt]->component + (String)"/"+String(m_deviceName) + String(m_devIndex)+ (String)"/state"; //set topic common for grouped entities
    m_enitiyDB[m_nrOFRegEnt]->stateTopicFull = m_grStateTopicFull;	
  }
  else
    m_enitiyDB[m_nrOFRegEnt]->stateTopicFull= (String)DISCOVERY_PREFIX+(String)"/"+ (String)m_enitiyDB[m_nrOFRegEnt]->component + (String)"/"+String(m_deviceName) + String(m_devIndex)+(String)"/" + String(m_enitiyDB[m_nrOFRegEnt]->ent_name) + (String)"/state"; //set topic individual for entity

  m_enitiyDB[m_nrOFRegEnt]->object_id=String(m_deviceName) + String(m_devIndex) + String("-") +String(ent_name);
  if((strcmp(component,"number")==0)||\
    (strcmp(component,"switch")==0)){
    m_enitiyDB[m_nrOFRegEnt]->cmdTopicFull=String(DISCOVERY_PREFIX) + String("/") +  String(m_enitiyDB[m_nrOFRegEnt]->component) + String("/") +String(m_deviceName) + String(m_devIndex)+  String("/set");
  }
    
 
  m_enitiyDB[m_nrOFRegEnt]->icon=icon;
  m_enitiyDB[m_nrOFRegEnt]->vType=VTYPE_UNDEF;
  //m_enitiyDB[m_nrOFRegEnt]->value.s=nullptr;

  m_enitiyDB[m_nrOFRegEnt]->valueName=m_enitiyDB[m_nrOFRegEnt]->ent_name;
  
  DEBUG_LOG0_NOF(true,m_enitiyDB[m_nrOFRegEnt]->valueName);
  delay(100);
  assert(m_enitiyDB[m_nrOFRegEnt]->valueName.length()<MAX_VALUE_NAME_LEN);
  if(!m_enitiyDB[m_nrOFRegEnt]->cmdTopicFull.isEmpty()){
    MQTTClient.subscribe(m_enitiyDB[m_nrOFRegEnt]->cmdTopicFull.c_str());
  }
  for(int i=0;i<m_enitiyDB[m_nrOFRegEnt]->entNumber;i++){
    publishConfOfEntity(m_nrOFRegEnt,i);
  }
  m_nrOFRegEnt++;
}

void Hamqtt::publishConfOfEntity(int index_of_entity, int index_of_item){
  StaticJsonBuffer<1500> * jsonBuffer = new StaticJsonBuffer<1500>;
  JsonObject& json = jsonBuffer->createObject();  

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


  if(!m_enitiyDB[index_of_entity]->cmdTopicFull.isEmpty()){
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

  if(m_enitiyDB[index_of_entity]->max !=m_enitiyDB[index_of_entity]->min){
    json["max"]=m_enitiyDB[index_of_entity]->max;
    json["min"]=m_enitiyDB[index_of_entity]->min;
  }
  
  if(m_enitiyDB[index_of_entity]->entity_category != nullptr)
    json["entity_category"]=m_enitiyDB[index_of_entity]->entity_category;
  String confTopic = (String)DISCOVERY_PREFIX+(String)"/"+(String)m_enitiyDB[index_of_entity]->component+ String("/") +String(m_enitiyDB[index_of_entity]->object_id)+getIndexStr(index_of_entity,index_of_item) +"/config";
  if(json.success()){
    String str_buf;
    json.prettyPrintTo(str_buf);
    DEBUG_LOG0_NOF(true,str_buf);
    bool pubStatus = MQTTClient.publish(confTopic.c_str(),str_buf.c_str(),true);
    DEBUG_LOG0(pubStatus,"MQTT:published config");
    //DEBUG_LOG(!pubStatus,"MQTT:not published config !!! error=",MQTTClient.lastError()); 
  }

  delete jsonBuffer;
}



void Hamqtt::publishEntity(int index_of_entity, int index_of_item){
  StaticJsonBuffer<300> jsonBuffer;
  JsonObject& json = jsonBuffer.createObject(); 
  if(!m_pubEnabled)
    return;
  assert(m_enitiyDB[index_of_entity]->entNumber>index_of_item);
  switch(m_enitiyDB[index_of_entity]->vType){
    case VTYPE_UINT32:
      json[m_enitiyDB[index_of_entity]->valueName + getIndexStr(index_of_entity,index_of_item)]=(uint32_t)m_enitiyDB[index_of_entity]->value[index_of_item].u32;
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
    MQTTClient.publish(m_enitiyDB[index_of_entity]->stateTopicFull.c_str(),str_buf.c_str() ,false);
    DEBUG_LOG0_NOF(true,str_buf);
  }else{  
    DEBUG_LOG0(!json.success(),"MQTT:publish error");
  }
}


void Hamqtt::publishValue(const char  * ent_name, const char * value,bool onlyChange){
  ValueType value_;
  value_.s=value;
  publishValue_int(ent_name,VTYPE_STRING, value_,onlyChange);
}

void Hamqtt::publishValue(const char  * ent_name, float value,bool onlyChange){
  ValueType value_;
  value_.f=value;  
   publishValue_int(ent_name,VTYPE_FLOAT, value_,onlyChange);
}

void Hamqtt::publishValue(const char * ent_name, uint32_t value,bool onlyChange){
  ValueType value_;
  value_.u32=value;  
   publishValue_int(ent_name,VTYPE_UINT32, value_,onlyChange);
}
void Hamqtt::publishValue(const char * ent_name, bool value,bool onlyChange){
  ValueType value_;
  value_.u32=value;  
   publishValue_int(ent_name,VTYPE_UINT32, value_,onlyChange);
}

void Hamqtt::publishSwitch(const char * ent_name, bool value,bool onlyChange){
  ValueType value_;
  value_.s= value? "ON":"OFF";
  publishValue_int(ent_name,VTYPE_STRING, value_,onlyChange);
}



void Hamqtt::publishValue_int(const char * ent_name, VType value_type, ValueType value,bool onlyChange){
  assert(value_type != VTYPE_UNDEF);
  for(int i=0;i<m_nrOFRegEnt;i++){
    if(strcmp(m_enitiyDB[i]->ent_name,ent_name)==0){
      assert(m_enitiyDB[i]->entNumber==1);
      assert(m_enitiyDB[i]->grStateTopic==false); 
      bool changeValue=false;
      switch(value_type){
        case VTYPE_UINT32:
          changeValue=m_enitiyDB[i]->value[0].u32!=value.u32;
          break;
        case VTYPE_FLOAT:
          changeValue=m_enitiyDB[i]->value[0].f!=value.f;
          break;
        case VTYPE_STRING:
          changeValue=strcmp(m_enitiyDB[i]->value[0].s,value.s)!=0;
          break;
        case  VTYPE_UNDEF:

          break;
      }
      
      if(!onlyChange || m_enitiyDB[i]->vType==VTYPE_UNDEF || changeValue){
        switch(value_type){
          case VTYPE_UINT32:
              m_enitiyDB[i]->value[0]=value;
              m_enitiyDB[i]->vType=VTYPE_UINT32;
            break;
          case VTYPE_FLOAT:
              m_enitiyDB[i]->value[0]=value;
              m_enitiyDB[i]->vType=VTYPE_FLOAT;
            break;
          case VTYPE_STRING:
              m_enitiyDB[i]->value[0]=value;
              m_enitiyDB[i]->vType=VTYPE_STRING;
            break;

          case  VTYPE_UNDEF:

          break;  
        }
        publishEntity(i,0);
      }
      return;
    }
  }
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
          case VTYPE_UINT32:
            json[m_enitiyDB[i]->valueName + getIndexStr(i,j)]=(int)m_enitiyDB[i]->value[j].u32;
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
    bool pubStatus=MQTTClient.publish(m_grStateTopicFull.c_str(),str_buf.c_str() ,false);
    assert(pubStatus);
    DEBUG_LOG0_NOF(true,str_buf);
  }else{  
    DEBUG_LOG0(!json.success(),"MQTT:publish error");
  }
}


unsigned long Hamqtt::lastTimeOfAct(){
  return m_lastTimeOfAct;
}

void Hamqtt::main(){
  process_callback();
  unsigned long delTime=0;  
  delTime = millis() - m_datasend_normal_ltime;  
  MQTTClient.loop();
  //delay(10);
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
        if (!MQTTClient.connected())
          connect(true);
        if(MQTTClient.connected()){
          if(objPtr->m_grStateTopic && objPtr->m_grPerType==perType)
            objPtr->publishGroupedEntities();

          objPtr->publisValuesPer(perType);
        }  
      }
    }
  }
}

void Hamqtt::messageReceived(char *topic, byte*payload,unsigned int length){
  DEBUG_LOG0_NOF(true,"MQTT:messageReceived [" + (String)topic + (String)"]"); // + payload);
  BuffElemnet * elemPtr;

  if(m_buffer.isFull()){
    elemPtr=m_buffer.pop();
    delete elemPtr;
  }
  elemPtr=new BuffElemnet;
  assert(elemPtr!=nullptr);
  elemPtr->topic=topic;
  char * tmpBuf=new char[length+1];
  assert(tmpBuf!=nullptr);
  assert(sizeof(byte)==sizeof(char)); // "code assumes same size of both data types and not testable in constant expression.
  memcpy(tmpBuf,(char *)payload, length);
  tmpBuf[length]=0;
  elemPtr->payload=tmpBuf;  
  delete tmpBuf;
  m_buffer.push(elemPtr);
}

void Hamqtt::process_callback(){
  noInterrupts();
  if(m_buffer.isEmpty()){
    interrupts();
    return;
  }
  BuffElemnet * elemPtr;
  elemPtr = m_buffer.pop();
  interrupts();
  assert(elemPtr !=nullptr);
  for(int objInd=0;objInd<m_regObjNumb;objInd++){
    assert(m_regObjects[objInd]!=nullptr);
    Hamqtt * objPtr=m_regObjects[objInd];
    for(int i=0;i<objPtr->m_nrOFRegEnt;i++){
      if(objPtr->m_enitiyDB[i]->cmdTopicFull==elemPtr->topic){
        objPtr->m_lastTimeOfAct=millis();
        if(objPtr->m_enitiyDB[i]->cmdCallback!=nullptr){
          objPtr->m_enitiyDB[i]->cmdCallback(i,elemPtr->payload);
        }
      }
    }
  }  
  delete elemPtr;
}

unsigned long Hamqtt::getPeriod(int index_of_entity){
  
  switch(m_enitiyDB[index_of_entity]->perType){
    case PERTYPE_NORMAL:
      return m_DatasendNormalPer;
    case PERTYPE_LOWSPEED:
      return m_DatasendLowPer;
    case PERTYPE_HIGHSPEED:
      return m_DatasendHighPer;
    case PERTYPE_NO_PERIOD:
      return 0;
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


void Hamqtt::writeSwitch(const char * ent_name, bool value,int item){
  for(int i=0;i<m_nrOFRegEnt;i++){
    if(m_enitiyDB[i]->ent_name==ent_name){
      m_enitiyDB[i]->vType=VTYPE_STRING;
      assert(m_enitiyDB[i]->entNumber>item);
      m_enitiyDB[i]->value[item].s=value? "ON":"OFF";;
      return;
    }
  }
  assert(false);
}
void Hamqtt::writeValue(const char * ent_name, uint32_t value,int item){
  for(int i=0;i<m_nrOFRegEnt;i++){
    if(m_enitiyDB[i]->ent_name==ent_name){
      m_enitiyDB[i]->vType=VTYPE_UINT32;
      assert(m_enitiyDB[i]->entNumber>item);
      m_enitiyDB[i]->value[item].u32=value;
      return;
    }
  }
  assert(false);
}



const char *Hamqtt::getEntName(int indexOfEnt){
  assert((indexOfEnt < m_nrOFRegEnt) && (indexOfEnt >= 0));  
  return m_enitiyDB[indexOfEnt]->ent_name;
}