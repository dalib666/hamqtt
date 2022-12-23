/* ================================================================================ */
/*                                                                                  */
/*   hamqtt.hpp                                                                     */
/*                                                                                  */
/*                                                                                  */
/*   Description:                                                                   */
/*               basic support for Home Assitant MQTT integration                   */
/* ================================================================================ */

#ifndef HAMQTT_HPP
#define HAMQTT_HPP
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <MQTTClient.h>

class Hamqtt{
    //default setting of params
    static const unsigned long DATASEND_NORMAL_PER=1000ul;      
    static const unsigned long DATASEND_LOWSPEED_PER=30000ul;
    static const unsigned long DATASEND_HIGHSPEED_PER=200ul;
    //constant parameters
    static const int MAX_REG_ENT=20;
    static const int CONFIG_PER=60;
    static unsigned long m_datasend_normal_ltime;
    static unsigned long m_datasend_lowspeed_ltime;
    static unsigned long m_datasend_highspeed_ltime;
    static const int MAX_REG_OBJ = 5;
    static const int MAX_VALUE_NAME_LEN=30; 
    static const char * DISCOVERY_PREFIX;    
    public:
    enum PeriodType{
        PERTYPE_NORMAL,
        PERTYPE_LOWSPEED,
        PERTYPE_HIGHSPEED,
    }; 
    typedef void (*CmdCallbackType) (int indOfEnt, String &payload);

    static void init(WiFiClient * wifiClient, IPAddress & brokerIP,const char * mqttUserName,const char * mqttPass,const char * clientID,unsigned int normalPer=0,unsigned int lowPer=0,unsigned int highPer=0);
    Hamqtt(const char * devName,const char *  devIndex=nullptr, PeriodType grPerType=PERTYPE_LOWSPEED, const char * model=nullptr, const char * manufacturer=nullptr, const char * swVersion=nullptr, const char * identifiers=nullptr, const char * configuration_url=nullptr, const char * hw_version=nullptr, const char * via_device=nullptr, int expire_after=3);
    void registerEntity(const char * component, const char * ent_name,PeriodType perType, const char * class_,const char * unit_of_measurement=nullptr,const char * unique_id=nullptr,const char * icon=nullptr,CmdCallbackType cmdCallback=nullptr,const char * entity_category=nullptr, int entNumber=1,bool grStTopic=false);
    void registerSensorEntity(const char * ent_name,PeriodType perType, const char * class_,const char * unit_of_measurement=nullptr,const char * icon=nullptr, int entNumber=1,bool grStTopic=false);	
    void registerNumberEntity(const char * ent_name,PeriodType perType, const char * class_,const char * unit_of_measurement=nullptr,const char * icon=nullptr,CmdCallbackType cmdCallback=nullptr,bool grStTopic=false);
    void publishValue(const char * ent_name, const char * value,int item=-1);  
    void publishValue(const char * ent_name, float value,int item=-1);
    static void main();
    
    private:
    static unsigned long m_DatasendNormalPer;
    static unsigned long m_DatasendLowPer;
    static unsigned long m_DatasendHighPer;
    static const char * m_mqttUserName;
    static const char * m_mqttPass;
    int m_nrOFRegEnt;
    const char * m_devIndex;
    int m_expire_after;
    static const char * m_clientID;  
    const char * m_deviceName;
    const char * m_model;
    const char * m_manufacturer;
    const char * m_swVersion;
    const char * m_identifiers;
    const char * m_configuration_url; 
    const char * m_hw_version;
    const char * m_via_device;
    String m_grStateTopicFull;
    bool m_grStateTopic;
    PeriodType    m_grPerType;

    static Hamqtt * m_regObjects[MAX_REG_OBJ];
    static int m_regObjNumb;
    union ValueType{
      int i;
      float f;
      const char * s;
    };
   
    enum VType{
        VTYPE_UNDEF,
        VTYPE_INT,
        VTYPE_FLOAT,
        VTYPE_STRING
    };
    
    struct EntityConfData{
        const char * ent_name;
        const char * class_;
        const char * unit_of_measurement;
        const char * unique_id;
        String valueName;
        String stateTopicFull;
        String cmdTopicFull;
        CmdCallbackType cmdCallback;
        const char * icon;
        PeriodType    perType;
        ValueType   value;
        VType      vType;
        const char * component;
        String object_id;
        const char * entity_category;
        int entNumber;
        bool grStateTopic;
    };
    static void connect();    
    EntityConfData * m_enitiyDB[MAX_REG_ENT];
    void publisValuesPer(PeriodType period);  
    void publishEntity(int index_of_entity, int index_of_item);  
    void publishConfOfEntity(int index_of_entity, int index_of_item);
    void publishGroupedEntities();
    static void messageReceived(String &topic, String &payload);
    unsigned long getPeriod(int index_of_entity);
    static WiFiClient * m_wifiClientPtr;
    static MQTTClient Client;  
};
#endif //HAMQTT_HPP
