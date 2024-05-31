#ifndef SETTINGS_H
#define SETTINGS_H

#include <WiFi.h> 
#include <Arduino.h> 
#include <EEPROM.h>
#include <Ticker.h>

#define QUOTE(...) #__VA_ARGS__

#define EEPROM_START_ADDRESS    0
#define EEPROM_SIZE             2048

#define FW_VERSION              "0.5"
#define CONF_WEBSERVER_PORT     80
#define CONF_WEBSOCKET_PORT     81
#define CONF_SERIAL_BAUD        115200

#define G_VERSION                   "GVER"   
#define G_TIME                      "GTIME"   

#define U_MQTT_TAG                  "UMQTT" 
#define U_LOGG_TAG                  "ULOGG" 

#define N_NTPINTERVAL_TAG           "NNTPI" 
#define N_NTPSERVER_TAG             "NNTPS"
#define N_HOSTNAME_TAG              "NHOST"

#define MN_POLLINTERVAL             "MNPIV"
#define MN_HOTWATER                 "MNHWA"  
#define MN_BAUD                     "MNBAU"  
#define MN_RX_PIN                   "MNRXP"  
#define MN_TX_PIN                   "MNTXP"  
#define MN_EN_PIN                   "MNENP"  
#define MN_EN_STATE                 "MNENS"  

#define M_HASS_TAG                  "MHASS"
#define M_PORT_TAG                  "MPORT"
#define M_HOST_TAG                  "MBROK"
#define M_CLIENT_ID_TAG             "MCLID"
#define M_USER_TAG                  "MUSER"
#define M_PASS_TAG                  "MPASS" 
#define M_TOPIC_TAG                 "MTOP"


struct Settings_t{
    //General
    char        version[5]              ;     
    uint8_t     u_MQTT                  ;    
    uint8_t     u_LOGGING               ;
    uint32_t    u_chipid                ;

    //network
    uint32_t    n_ntpinterval           ;
    char        n_ntpserver[32]         ;    
    char        n_hostname[32]          ;

    //Micronova
    uint32_t    mn_pollinterval         ;
    uint8_t     mn_hotwater             ;
    uint16_t    mn_ser_baud             ;
    uint8_t     mn_ser_rx_pin           ;
    uint8_t     mn_ser_tx_pin           ;
    uint8_t     mn_ser_en_pin           ;
    uint8_t     mn_ser_en_state         ;

    //MQTT
    uint16_t    m_port                  ;
    char        m_host[32]              ;    
    char        m_client_id[32]         ;
    char        m_user[32]              ;
    char        m_pass[32]              ;
    char        m_topic[32]             ;
    uint8_t     m_homeassistant         ;
    
}  __attribute__((packed));

extern Settings_t settings;

extern uint8_t    doRestart;

const char PROP_STR[]    PROGMEM    = QUOTE( "{key}":"{val}", );
const char PROP_INT[]    PROGMEM    = QUOTE( "{key}":{val}, );

extern void     SettingsInit();

extern void     SettingsSetDefaults();

extern uint32_t SettingsGetChecksum();
extern void     SettingsSetValue(String key, String value);
extern String   SettingsToJson();

extern void     SettingsWrite();
extern void     SettingsRead();
extern void     SettingsClear();

extern void     SettingsTick();
extern void     SettingsUpdate();

extern void     SettingsWifiReset();
extern void     SettingsSoftRestart();
extern void     SettingsTick();

extern uint32_t getChipID();

#endif