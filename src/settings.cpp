#include "settings.h"
#include "timeNTP.h"
#include "logging.h"

static Ticker SettingsTimer;

Settings_t settings;

uint8_t doRestart = 0;

void SettingsUpdate(){
    //Need Restart
    if(doRestart){
        doRestart = 0;
        ESP.restart();
    }
}

void SettingsInit(){
    SettingsRead();
    if(!String(settings.version).equals(FW_VERSION)){
        WebLogInfo("Settings Version mismatch!");   
        settings = {};     
        SettingsClear();        
        SettingsSetDefaults();
        SettingsWrite();
    }else{
        WebLogInfo("Done");        
    }
    SettingsTimer.attach_ms(1000,SettingsUpdate);
}

void SettingsSetDefaults(){
    
    settings.u_chipid = getChipID();
    strcpy(settings.version,FW_VERSION); 
    
    //micronova
    settings.mn_hotwater             = 0;
    settings.mn_ser_baud             = 1200;
    settings.mn_ser_rx_pin           = 35;
    settings.mn_ser_tx_pin           = 33;
    settings.mn_ser_en_pin           = 32;
    settings.mn_ser_en_state         = LOW;
    settings.mn_pollinterval         = 20000;

    settings.u_MQTT                  = 0;
    settings.u_LOGGING               = 2;
    
    settings.n_ntpinterval           = 60000;    
    strcpy(settings.n_ntpserver,    "de.pool.ntp.org");    
    strcpy(settings.n_hostname ,    String("mightyESP32"+String(settings.u_chipid)).c_str() );
    settings.m_port                  = 1883;    
    strcpy(settings.m_host,         "192.168.33.253");    
    strcpy(settings.m_client_id,    String("mightyESP32"+String(settings.u_chipid)).c_str() );    
    strcpy(settings.m_topic,        String("mightyESP32"+String(settings.u_chipid)).c_str() );  
    settings.m_homeassistant         = 0;    
}

void    SettingsWrite(){    
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.put(EEPROM_START_ADDRESS,settings);
    delay(200);
    EEPROM.commit();
    EEPROM.end();
    WebLogInfo("Wrote Settings Version: " + String(settings.version));
}

void    SettingsRead(){
    EEPROM.begin(EEPROM_SIZE);
    EEPROM.get(EEPROM_START_ADDRESS,settings);
    delay(200);
    EEPROM.end();
    WebLogInfo("Read Settings Version: " + String(settings.version));
}

void    SettingsClear(){    
    EEPROM.begin(EEPROM_SIZE);
    for (int i = EEPROM_START_ADDRESS ; i < EEPROM_SIZE ; i++) {
        EEPROM.write(i, 0);
    }
    delay(200);
    EEPROM.commit();
    EEPROM.end();
    WebLogInfo("EEPROM Cleared");
}

void    SettingsWifiReset(){    
    //Erase Wifi
    WiFi.persistent(false);     
    WiFi.disconnect(true);
    WiFi.persistent(true);

}

uint32_t SettingsGetChecksum(){
    uint32_t checksum = 0;
    uint8_t bufferP[sizeof(settings)];
    memcpy(bufferP, &settings, sizeof(settings));

    for (unsigned int i=0; i<sizeof(bufferP); i++){
        checksum += bufferP[i];
    }     
    return checksum;
}

void    SettingsSetValue(String key, String value){
    if(key.equals(U_MQTT_TAG)){
        settings.u_MQTT = (uint8_t)value.toInt();    
    }else if(key.equals(U_LOGG_TAG)){
        settings.u_LOGGING = (uint8_t)value.toInt();
    }else if(key.equals(N_NTPINTERVAL_TAG)){    
        settings.n_ntpinterval = (uint32_t)value.toInt();
    }else if(key.equals(N_NTPSERVER_TAG)){    
        strcpy(settings.n_ntpserver, value.c_str());
    }else if(key.equals(N_HOSTNAME_TAG)){    
        strcpy(settings.n_hostname, value.c_str());
    }else if(key.equals(M_HASS_TAG)){
        settings.m_homeassistant = (uint8_t)value.toInt();
    }else if(key.equals(M_PORT_TAG)){
        settings.m_port = (uint16_t)value.toInt();
    }else if(key.equals(M_HOST_TAG)){    
        strcpy(settings.m_host, value.c_str());
    }else if(key.equals(M_CLIENT_ID_TAG)){    
        strcpy(settings.m_client_id, value.c_str());
    }else if(key.equals(M_USER_TAG)){    
        strcpy(settings.m_user, value.c_str());
    }else if(key.equals(M_PASS_TAG)){    
        strcpy(settings.m_pass, value.c_str());
    }else if(key.equals(M_TOPIC_TAG)){
        strcpy(settings.m_topic, value.c_str());

    //micronova
    }else if(key.equals(MN_POLLINTERVAL)){    
        settings.mn_pollinterval = (uint32_t)value.toInt();
    }else if(key.equals(MN_HOTWATER)){
        settings.mn_hotwater = (uint8_t)value.toInt();
    }else if(key.equals(MN_BAUD)){
        settings.mn_ser_baud = (uint16_t)value.toInt();
    }else if(key.equals(MN_RX_PIN)){
        settings.mn_ser_rx_pin = (uint8_t)value.toInt();
    }else if(key.equals(MN_TX_PIN)){
        settings.mn_ser_tx_pin = (uint8_t)value.toInt();
    }else if(key.equals(MN_EN_PIN)){
        settings.mn_ser_en_pin= (uint8_t)value.toInt();
    }else if(key.equals(MN_EN_STATE)){
        settings.mn_ser_en_state = (uint8_t)value.toInt();
    }
}

String  getPropInt(String key, int32_t val){
    String ret = FPSTR(PROP_INT);
    ret.replace("{key}", key);
    ret.replace("{val}", String(val) );
    return ret;
}  

String  getPropStr(String key, String val){
    String ret = FPSTR(PROP_STR);
    ret.replace("{key}", key);
    ret.replace("{val}", val );
    return ret;
}

String    SettingsToJson(){ 
    String jsonDest = "{";       
    jsonDest += getPropStr(N_HOSTNAME_TAG,              String(settings.n_hostname));
    jsonDest += getPropStr(G_VERSION,                   String(settings.version));
    jsonDest += getPropStr(G_TIME,                      String(TimeformatedTime()));

    jsonDest.remove(jsonDest.length()-1);   

    jsonDest += "}";

    return jsonDest;
}

//##############
//### misc. 
//##############

uint32_t getChipID(){
    uint32_t uid = 0; 
    for(int i=0; i<17; i=i+8) {
	  uid |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
	}
    return uid;
}

void SettingsSoftRestart(){
    doRestart = 1;
}

