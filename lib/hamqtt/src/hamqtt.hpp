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
    static const int MAX_REG_ENT=20;
    static const int CONFIG_PER=60;
    static const unsigned long DATASEND_NORMAL_PER=1000ul;
    static const unsigned long DATASEND_LOWSPEED_PER=30000ul;
    static unsigned long m_datasend_normal_ltime;
    static unsigned long m_datasend_lowspeed_ltime;
    static const int MAX_REG_OBJ = 5;
    static const int MAX_VALUE_NAME_LEN=30; 
    static const char * DISCOVERY_PREFIX;    
    public:
    enum PeriodType{
        PERTYPE_NORMAL,
        PERTYPE_LOWSPEED
    }; 
    typedef void (*CmdCallbackType) (int indOfEnt, String &payload);

    static void init(WiFiClient * wifiClient, IPAddress & brokerIP,char * mqttUserName,char * mqttPass,const char * clientID);
    Hamqtt(char * const devName,char * const devIndex, char * const node_id, int expire_after);
    void registerEntity(char * const component, char * ent_name,PeriodType perType, char * class_,char * unit_of_measurement,char * unique_id=nullptr,char * value_template=nullptr,char * icon=nullptr,CmdCallbackType cmdCallback=nullptr);


    void publisValue(char * ent_name, char * value);  
    void publisValue(char * ent_name, float value);    
    static void main();
    
    private:
    
    static char * m_mqttUserName;
    static char * m_mqttPass;
    int m_nrOFRegEnt;
    char * const m_devIndex;
    int m_expire_after;
    static const char * m_clientID;  
    char * m_node_id;
    char * m_deviceName;
    static Hamqtt * m_regObjects[MAX_REG_OBJ];
    static int m_regObjNumb;
    union ValueType{
      int i;
      float f;
      char * s;
    };
   
    enum VType{
        VTYPE_UNDEF,
        VTYPE_INT,
        VTYPE_FLOAT,
        VTYPE_STRING
    };
    
    struct EntityConfData{
        char * ent_name;
        char * class_;
        char * unit_of_measurement;
        char * unique_id;
        char * value_template;
        String valueName;
        String stateTopicFull;
        String cmdTopicFull;
        CmdCallbackType cmdCallback;
        char * icon;
        PeriodType    perType;
        ValueType   value;
        VType      vType;
        char * component;
        String object_id;
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
