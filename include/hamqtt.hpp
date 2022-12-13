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
    Hamqtt(const char * devName,const char *  devIndex, const char * model, const char * manufacturer, const char * swVersion, const char * identifiers, const char * configuration_url, const char * hw_version, const char * via_device, int expire_after);
    void registerEntity(const char * component, const char * ent_name,PeriodType perType, const char * class_,const char * unit_of_measurement,const char * unique_id=nullptr,const char * value_template=nullptr,const char * icon=nullptr,CmdCallbackType cmdCallback=nullptr,const char * entity_category=nullptr);
    void registerSensorEntity(const char * ent_name,PeriodType perType, const char * class_,const char * unit_of_measurement,const char * value_template=nullptr,const char * icon=nullptr);
    void registerNumberEntity(const char * ent_name,PeriodType perType, const char * class_,const char * unit_of_measurement,const char * value_template=nullptr,const char * icon=nullptr,CmdCallbackType cmdCallback=nullptr);
    void publisValue(const char * ent_name, const char * value);  
    void publisValue(const char * ent_name, float value);    
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
        const char * value_template;
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
    };
    static void connect();    
    EntityConfData * m_enitiyDB[MAX_REG_ENT];
    void publisValuesPer(PeriodType period);  
    void publishEntity(int index_of_entity);    
    void publishConfOfEntity(int index_of_entity);
    static void messageReceived(String &topic, String &payload);

    
    static WiFiClient * m_wifiClientPtr;
    static MQTTClient Client;  
};
#endif //HAMQTT_HPP
