/* =====================================================================================================*/
/*                                                                                                      */
/*   hamqtt.hpp                                                                                         */
/*        Version info is in library.json file !                                                        */
/*   Description:                                                                                       */
/*              basic support for Home Assitant MQTT integration.                                       */
/*              One Instance simulate one device. Per one instance is possible                          */
/*              register more entities. One entity can be configured as "multiple"                      */
/*              to simulate array data type. API contains also many configuration                       */
/*                variables of MQTT componennt integration in HA, that API parameters are signed        */
/*                by (HACV). Description of that parameters can be found in HA                          */
/*                documentation e.g. "https://www.home-assistant.io/integrations/sensor.mqtt/"          */
/*                                                                                                      */
/* Attention !!! IF it is not noted explictly in some parameter, mainly registering/init functions      */
/* of API  generally assumes that input string parameters are globaly constant, to save global memory !!! */
/* ==================================================================================================== */

#ifndef HAMQTT_HPP
#define HAMQTT_HPP
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <CircularBuffer.h>

class Hamqtt{

   
    public:
    enum PeriodType{
        PERTYPE_NORMAL,
        PERTYPE_LOWSPEED,
        PERTYPE_HIGHSPEED,
        PERTYPE_NO_PERIOD   //for only event transmission or no transmittion to HA
    }; 
    typedef void (*CmdCallbackType) (int indOfEnt, String &payload);
    /**
     * @brief init - init module, must be called before any other function
     * @param wifiClient - pointer to WiFiClient object
     * @param brokerIP - IP address of MQTT broker
     * @param mqttUserName - MQTT user name
     * @param mqttPass - MQTT password
     * @param clientID - MQTT client ID
     * @param normalPer - period of publish data for normal speed entity(PERTYPE_NORMAL)
     * @param lowPer - period of publish data for low speed entity(PERTYPE_LOWSPEED)
     * @param highPer - period of publish data for high speed entity(PERTYPE_HIGHSPEED)
    */
    static void init(WiFiClient * wifiClient, IPAddress & brokerIP,const char * mqttUserName,const char * mqttPass,const char * clientID,unsigned int normalPer=5000,unsigned int lowPer=30000ul,unsigned int highPer=1000);
    /**
     * @brief Hamqtt - constructor
     * @param devName - device name
     * @param devIndex - device index, use to distinguish multiple devices of the same type in the same network
     * @param grPerType - period type for grouping entities into one topic
     * @param model - (HACV)device model name 
     * @param manufacturer - (HACV)device manufacturer name
     * @param swVersion - (HACV)software version
     * @param identifiers - (HACV)device identifiers, unique identifier of the device !!!
     * @param configuration_url - (HACV)device configuration URL
     * @param hw_version - (HACV)device hardware version
     * @param via_device - (HACV)device via device
     * @param expire_after - multiplier of parameter "perType" in registerxxxx functions to calculate (HACV)expire_after parameter
    */
    Hamqtt(const char * devName,const char *  devIndex=nullptr, PeriodType grPerType=PERTYPE_LOWSPEED, const char * model=nullptr,\
        const char * manufacturer=nullptr, const char * swVersion=nullptr, const char * identifiers=nullptr, const char * configuration_url=nullptr,\
        const char * hw_version=nullptr, const char * via_device=nullptr, int expire_after=3);

    /**
     *  @brief setDynamic - set object parameters, which has not valid value during allocation of object 
     */
    void setDynamic(const char * configuration_url){
        m_configuration_url=configuration_url;
    }

    /**
     * @brief registerEntity -0 universal register of entity
     * @param component - HA component name of, see https://www.home-assistant.io/integrations/#search/MQTT
     * @param ent_name - entity name, must be unique in device, if entity is multiple, index of item is added to name. 
     * Note ! ent_name parametr passed  can be localy allocated. Function this parameter copy in to local buffer. Note! 
     * @param perType - period type of published state data
     * @param class_ - (HACV)device class
     * @param unit_of_measurement - (HACV)unit of measurement
     * @param unique_id - (HACV)unique ID
     * @param icon - (HACV)icon
     * @param cmdCallback - callback function for command topic, if defined
     * @param entity_category - (HACV)entity category
     * @param entNumber - number of entity items, if entity is multiple
     * @param grStTopic - all entities with set grStTopic=true are grouped into one state topic
     * @param max - up limit of range
    *  @param min - low limit of range
    */
    void registerEntity(const char * component, const char * ent_name,PeriodType perType, const char * class_,\
        const char * unit_of_measurement=nullptr,const char * unique_id=nullptr,const char * icon=nullptr,\
        CmdCallbackType cmdCallback=nullptr,const char * entity_category=nullptr, int entNumber=1,bool grStTopic=false,\
        float max=0, float min=0);
    /**
     * @brief registerSensorEntity - optimised registering function for sensor component type, see https://www.home-assistant.io/integrations/sensor.mqtt/
    */
    void registerSensorEntity(const char * ent_name,PeriodType perType, const char * class_,const char * unit_of_measurement=nullptr,\
        const char * icon=nullptr, int entNumber=1,bool grStTopic=false);	
    /**
     * @brief registerNumberEntity - optimised registering function for number component type, see https://www.home-assistant.io/integrations/number.mqtt/
    */
    void registerNumberEntity(const char * ent_name,PeriodType perType, const char * class_,const char * unit_of_measurement=nullptr,\
        const char * icon=nullptr,CmdCallbackType cmdCallback=nullptr,bool grStTopic=false,float max=0, float min=0);

    /**
     * @brief registerSwitchEntity - optimised registering function for Switch component type, see https://www.home-assistant.io/integrations/switch.mqtt/
    */
    void registerSwitchEntity(const char * ent_name,PeriodType perType, const char * class_,const char * icon=nullptr,CmdCallbackType cmdCallback=nullptr,bool grStTopic=false);

    /**
     * @brief registerButtonEntity - optimised registering function for Burron component type, see https://www.home-assistant.io/integrations/button.mqtt/
    */
    void registerButtonEntity(const char * ent_name, const char * class_,CmdCallbackType cmdCallback,const char * icon=nullptr);

    /**
     * @brief registerBinSensorEntity - optimised registering function for binary sensor component type, see https://www.home-assistant.io/integrations/binary_sensor.mqtt/
    */
    void registerBinSensorEntity(const char * ent_name,PeriodType perType, const char * class_,const char * icon=nullptr, int entNumber=1,bool grStTopic=false);

    /**
     * @brief write and publish value - only for simple and ungrouped entity
     * @param onlyCahnge - if true, value is published only if it is different from previous value 
     */
    void publishValue(const char * ent_name, const char * value,bool onlyChange=true);  
    void publishValue(const char * ent_name, float value,bool onlyChange=true);
    void publishValue(const char * ent_name, uint32_t value,bool onlyChange=true);
    void publishValue(const char * ent_name, bool value,bool onlyChange=true);
    void publishSwitch(const char * ent_name, bool value,bool onlyChange=true);
    inline void publishBinSen(const char * ent_name, bool value,bool onlyChange=true){
        publishSwitch(ent_name,value,onlyChange=true);
    }


    /**
     * @brief write value - primary for grouped entity in one state topic, useable also for other entities
    */
    void writeValue(const char * ent_name, const char * value,int item=0);  
    void writeValue(const char * ent_name, float value,int item=0);
    void writeSwitch(const char * ent_name, bool value,int item=0);
    void writeValue(const char * ent_name, uint32_t value,int item=0);

   /**
    * @brief startPublishing - start publishing data to MQTT broker, i would be called after all entities are set to valid values.
   */
    void startPublishing();
    /**
    * @brief lastTimeOfHAact - last time of HA activity, useable only for enities using command topic.
    * @return  [ms], time of last command from HA, Use time 32 bits counter, so after reach the max value counter overflows !
    */
    unsigned long lastTimeOfAct();

    /**
     * @brief Returns last time of connection into MQTT broker
     */
    unsigned long connected(){

        return Hamqtt::m_connected_time;
    }    

    const char *getEntName(int indexOfEnt);
    static void main();

    private:
    
     //constant parameters
    static const int MAX_REG_ENT=20;
    static const int CONFIG_PER=60;
    static unsigned long m_datasend_normal_ltime;
    static unsigned long m_datasend_lowspeed_ltime;
    static unsigned long m_datasend_highspeed_ltime;
    static const int MAX_REG_OBJ = 5;
    static const int MAX_VALUE_NAME_LEN=30; 
    static const char * DISCOVERY_PREFIX;    
        
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
    unsigned long   m_lastTimeOfAct;
    static Hamqtt * m_regObjects[MAX_REG_OBJ];
    static int m_regObjNumb;
    struct BuffElemnet{
        String topic;
        String payload;
    };
    static CircularBuffer<BuffElemnet *,5> m_buffer;
    union ValueType{
      uint32_t u32;
      float f;
      const char * s;
    };
   
    enum VType{
        VTYPE_UNDEF,
        VTYPE_UINT32,
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
        ValueType   *value;
        VType      vType;
        const char * component;
        String object_id;
        const char * entity_category;
        int entNumber;
        bool grStateTopic;
        float max;
        float min;
    };
    static void process_callback();
    static void main_int(PeriodType perType);
    static void connect(bool recon=false);    
    EntityConfData * m_enitiyDB[MAX_REG_ENT];
    void publisValuesPer(PeriodType period);  
    void publishEntity(int index_of_entity, int index_of_item);  
    void publishConfOfEntity(int index_of_entity, int index_of_item);
    void publishGroupedEntities();
    static void messageReceived(char *topic, byte*payload,unsigned int length);
    unsigned long getPeriod(int index_of_entity);
    void publishValue_int(const char * ent_name, VType value_type, ValueType value,bool onlyChange=true);
    static PubSubClient MQTTClient;  
    bool m_pubEnabled;
    static unsigned long m_lastConnectAttemp;
    static unsigned long m_connected_time;
};
#endif //HAMQTT_HPP
