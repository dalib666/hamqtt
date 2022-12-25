#include <Arduino.h>
#include "DebugFnc.h"
#include "Mqtt.hpp"
#include "hamqtt.hpp"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266HTTPClient.h>



void setup() {
  Serial.begin(115200);
  DEBUG_PART(Serial.println("Starting..."));

  pinMode(LED_BUILTIN, OUTPUT);

  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;

  wifiManager.setConnectTimeout(30);

//fetches ssid and pass from eeprom and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
#ifdef DEBUG_MODE
 if(false){//suppress info from buttom, set standard connect to WIFI 
#else
 if(digitalRead(WIFICONF_BUT_PIN)){
#endif
  DEBUG_PART(Serial.println("startConfigPortal")); 
  //create config portal
  wifiManager.setConfigPortalTimeout(300); 
  wifiManager.startConfigPortal("HamQtt");
 } 
 else{ 
   wifiManager.setConfigPortalTimeout(1); // suppress config portal  - set minimal time of config portal
  // try to connect with last credentials, if not possible do not create config portal
  if(wifiManager.autoConnect()){
    DEBUG_PART(Serial.println(""));
    DEBUG_PART(Serial.print("Connected..."));
    DEBUG_PART(Serial.print("IP address: "));
    DEBUG_PART(Serial.println(WiFi.localIP()));

    Mqtt_init();

    DEBUG_PART(Serial.println("HTTP server started"));

    }
    else{
      DEBUG_PART(Serial.println("Not Connected into WIFI."));
    } 
    DEBUG_PART(Serial.println(""));
  }
  



  delay(100);

}

void loop() {
static bool BuiltInLed;
static int devider=0;
static unsigned long lastloop=0;
  devider++;
  if(devider>1000){
    devider=0;
    BuiltInLed=!BuiltInLed;
    digitalWrite(LED_BUILTIN, BuiltInLed);
  }

  if((millis()-lastloop)>5000ul){
    lastloop=millis();
    Mqtt_loop5s();
  }
  Hamqtt::main();
}