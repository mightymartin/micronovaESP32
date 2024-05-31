#include "mqtt.h"

static Ticker MQTTTimer;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

uint32_t lastSettingChecksum = 0;
uint32_t lastStoveChecksum = 0;

String cmdTopic, stateTopic, mnStateTopic;
char *mqtt_user = NULL;
char *mqtt_pass = NULL;

MicronovaStove *mqttStove;


HAMqttDevice *HAM_ctrl;
HAMqttDevice *HAM_targetTemp;
HAMqttDevice *HAM_targetPower;
HAMqttDevice *HAM_connetionState;
HAMqttDevice *HAM_state;
HAMqttDevice *HAM_ambientTemp;
HAMqttDevice *HAM_fumeTemp;
HAMqttDevice *HAM_fanSpeed;
HAMqttDevice *HAM_flamePower;
HAMqttDevice *HAM_waterTemp;
HAMqttDevice *HAM_waterPressure;
HAMqttDevice *HAM_systime;

unsigned long lastReconnectAt = millis();

void MQTTUpdate(){    
    MQTTPublish(true); 
}

boolean MQTTReconnect(){
    if ((millis() - lastReconnectAt) > 1000) {
        if(!mqttClient.connected()) {     
            WebLogInfo("Attempting MQTT connection...");        

            String willTopic = settings.m_homeassistant > 0 ? HAM_ctrl->getLWTTopic() : "0";
            String willMsg = settings.m_homeassistant > 0 ? LWT_OFFLINE : "0";

            if (mqttClient.connect(settings.m_client_id, mqtt_user, mqtt_pass, willTopic.c_str() ,0, true, willMsg.c_str())) { 
                WebLogInfo("Connect succeeded");      
                mqttClient.subscribe(cmdTopic.c_str());
                WebLogDebug("subscribe to " + String(cmdTopic)); 

                //############ IF HA -> hier config publishen und subscriben
                if(settings.m_homeassistant > 0){
                    // Beim Connect Config pushen
                    mqttClient.publish(HAM_ctrl->getConfigTopic().c_str(),HAM_ctrl->getConfigPayload().c_str());
                    mqttClient.publish(HAM_targetTemp->getConfigTopic().c_str(),HAM_targetTemp->getConfigPayload().c_str());
                    mqttClient.publish(HAM_targetPower->getConfigTopic().c_str(),HAM_targetPower->getConfigPayload().c_str());
                    mqttClient.publish(HAM_connetionState->getConfigTopic().c_str(),HAM_connetionState->getConfigPayload().c_str());
                    mqttClient.publish(HAM_state->getConfigTopic().c_str(),HAM_state->getConfigPayload().c_str());
                    mqttClient.publish(HAM_ambientTemp->getConfigTopic().c_str(),HAM_ambientTemp->getConfigPayload().c_str());
                    mqttClient.publish(HAM_fumeTemp->getConfigTopic().c_str(),HAM_fumeTemp->getConfigPayload().c_str());
                    mqttClient.publish(HAM_fanSpeed->getConfigTopic().c_str(),HAM_fanSpeed->getConfigPayload().c_str());
                    mqttClient.publish(HAM_flamePower->getConfigTopic().c_str(),HAM_flamePower->getConfigPayload().c_str());
                    mqttClient.publish(HAM_systime->getConfigTopic().c_str(),HAM_systime->getConfigPayload().c_str());

                    // Beim Connect LWT auf online setzen
                    mqttClient.publish(HAM_ctrl->getLWTTopic().c_str(),LWT_ONLINE);
                    
                    if(settings.mn_hotwater > 0){
                        // Beim Connect Config pushen
                        mqttClient.publish(HAM_waterTemp->getConfigTopic().c_str(),HAM_waterTemp->getConfigPayload().c_str());
                        mqttClient.publish(HAM_waterPressure->getConfigTopic().c_str(),HAM_waterPressure->getConfigPayload().c_str());
                    }
                    
                    mqttClient.subscribe(HAM_ctrl->getCommandTopic().c_str());
                    mqttClient.subscribe(HAM_targetTemp->getCommandTopic().c_str());
                    mqttClient.subscribe(HAM_targetPower->getCommandTopic().c_str());
                }
            } else {
                WebLogError("Connect failed, rc=" + String(mqttClient.state()));                  
            }        
        }  
        lastReconnectAt = millis();
    }
    return mqttClient.connected();
}

String MQTTStatus(){    
    if(mqttClient.connected()){
        return F("Connected");
    }else{
        return "not Connected, rc=" + String(mqttClient.state());
    }    
}

void MQTTTick(){  
    mqttClient.loop();    
    MQTTPublish(false);    
}

void MQTTSubCallback(char* topic, byte* payload, unsigned int length) {
    payload[length] = '\0'; //terminate that nasty boy    
    String  value       = String((char *)payload);      
    uint8_t intValue    = (uint8_t)value.toInt();
    
    WebLogInfo(HAM_ctrl->getCommandTopic() + " / " + String(topic) + " : " +  value + " / " + intValue);

    if(settings.m_homeassistant > 0 && String(topic).equals(HAM_ctrl->getCommandTopic())){    
        if(value.equalsIgnoreCase("ON")){            
            mqttStove->setStoveOn();
        }
        if(value.equalsIgnoreCase("OFF")){
            mqttStove->setStoveOff(0);
        }
    }else if(settings.m_homeassistant > 0 && String(topic).equals(HAM_targetTemp->getCommandTopic())){
        WebLogInfo("1");
        if(intValue >= MIN_TARGETTEMP && intValue <= MAX_TARGETTEMP){    
            WebLogInfo("2");
            mqttStove->setTargetTemp(intValue);               
        }
    }else if(settings.m_homeassistant > 0 && String(topic).equals(HAM_targetPower->getCommandTopic())){
        if(intValue >= MIN_POWERLEVEL && intValue <= MAX_POWERLEVEL){    
            mqttStove->setPowerlevel(intValue);
        }
    }else{
        //#### Lagacy Shit
        uint8_t   validCmd = 0;
        uint8_t   validValue = 0;
        String  command = String(strrchr(topic, '/')).substring(1);    
        bool    isTrueValue   = length > 0 && (intValue == 1 || value.equalsIgnoreCase("ON") || value.equalsIgnoreCase("AN") || value.equalsIgnoreCase("JA") || value.equalsIgnoreCase("YES") || value.equalsIgnoreCase("true") || value.equalsIgnoreCase("open"));    
        bool    isFalseValue  = length > 0 && (intValue == 0 || value.equalsIgnoreCase("OFF") || value.equalsIgnoreCase("AUS") || value.equalsIgnoreCase("NEIN") || value.equalsIgnoreCase("NO") || value.equalsIgnoreCase("false") || value.equalsIgnoreCase("close"));     

        if(command.equalsIgnoreCase(MQTT_TAG_ONOFF)){
            validCmd = 1;
            if(isTrueValue){                                    
                mqttStove->setStoveOn();
                validValue = 1;
            }        
            if(isFalseValue){                        
                mqttStove->setStoveOff(0);
                validValue = 1;
            }   
        }

        if(command.equalsIgnoreCase(MQTT_TAG_TTEMP)){
            validCmd = 1;
            if(intValue >= MIN_TARGETTEMP && intValue <= MAX_TARGETTEMP){    
                mqttStove->setTargetTemp(intValue);
                validValue = 1;                    
            }
        }

        if(command.equalsIgnoreCase(MQTT_TAG_TPOWER)){
            validCmd = 1;
            if(intValue >= MIN_POWERLEVEL && intValue <= MAX_POWERLEVEL){    
                mqttStove->setPowerlevel(intValue);
                validValue = 1;                    
            }
        }

        if(validCmd == 0){
            WebLogInfo("MQTT invalid Command " + command + " : " +  value + ".");
        }
        if(validValue == 0){
            WebLogInfo("MQTT invalid Value " + command + " : " +  value + ".");
        }
    }

}

void MQTTPublish(bool force) {    
    if(MQTTReconnect()){     
        if(lastSettingChecksum != SettingsGetChecksum() || force){                   
            WebLogInfo(stateTopic + " = " + SettingsToJson());
            mqttClient.publish(stateTopic.c_str(),SettingsToJson().c_str());                
            lastSettingChecksum = SettingsGetChecksum();
        }    

        if((lastStoveChecksum !=  mqttStove->getChecksum() && !mqttStove->isPolling()) || force){                
            WebLogInfo(mnStateTopic + " = " + mqttStove->getJsonValues());
            mqttClient.publish(mnStateTopic.c_str(),mqttStove->getJsonValues().c_str());    
            lastStoveChecksum = mqttStove->getChecksum();

            if(settings.m_homeassistant > 0){
                mqttClient.publish(HAM_ctrl->getStateTopic().c_str(),            mqttStove->stoveIsOn() ? "ON" : "OFF");
                mqttClient.publish(HAM_targetTemp->getStateTopic().c_str(),      String(mqttStove->getValues().targetTemp).c_str());
                mqttClient.publish(HAM_targetPower->getStateTopic().c_str(),     String(mqttStove->getValues().targetPowerlevel).c_str());
                mqttClient.publish(HAM_connetionState->getStateTopic().c_str(),  mqttStove->getValues().stoveConnected == 1 ? "Connected" : "Disconnected");
                mqttClient.publish(HAM_state->getStateTopic().c_str(),           mqttStove->getStoveStateDispName().c_str());
                mqttClient.publish(HAM_ambientTemp->getStateTopic().c_str(),     String(mqttStove->getValues().ambientTemp).c_str());
                mqttClient.publish(HAM_fumeTemp->getStateTopic().c_str(),        String(mqttStove->getValues().fumeTemp).c_str());
                mqttClient.publish(HAM_fanSpeed->getStateTopic().c_str(),        String(mqttStove->getValues().fanSpeed).c_str());
                mqttClient.publish(HAM_flamePower->getStateTopic().c_str(),      String(mqttStove->getValues().flamePower).c_str());
                mqttClient.publish(HAM_systime->getStateTopic().c_str(),         String(TimeformatedTime()).c_str());
                
                if(settings.mn_hotwater > 0){
                    mqttClient.publish(HAM_waterTemp->getStateTopic().c_str(),       String(mqttStove->getValues().waterTemp).c_str());
                    mqttClient.publish(HAM_waterPressure->getStateTopic().c_str(),   String(mqttStove->getValues().waterPress).c_str());
                }
            }
        } 

          
    }
}

void MQTTHAInit(){
    HAMqttDevice::HAMqttThing stoveThing = {"Micronova Stove Ctrl","Martin Krämer","MK2", String(getChipID()), "0.1", settings.version, "", LWT_ONLINE, LWT_OFFLINE};
    HAM_ctrl = new HAMqttDevice("Control", HAMqttDevice::SWITCH, stoveThing);
    HAM_targetTemp = new HAMqttDevice("Target Temperature", HAMqttDevice::NUMBER, stoveThing);
    HAM_targetPower = new HAMqttDevice("Target Powerlevel", HAMqttDevice::NUMBER, stoveThing);
    HAM_connetionState = new HAMqttDevice("Connection State", HAMqttDevice::SENSOR, stoveThing);
    HAM_state = new HAMqttDevice("Stove State", HAMqttDevice::SENSOR, stoveThing);
    HAM_ambientTemp = new HAMqttDevice("Ambient Temperature", HAMqttDevice::SENSOR, stoveThing);
    HAM_fumeTemp = new HAMqttDevice("Fume Temperature", HAMqttDevice::SENSOR, stoveThing);
    HAM_fanSpeed = new HAMqttDevice("Fan Speed", HAMqttDevice::SENSOR, stoveThing);
    HAM_flamePower = new HAMqttDevice("Flame Power", HAMqttDevice::SENSOR, stoveThing);
    HAM_waterTemp = new HAMqttDevice("Water Temperature", HAMqttDevice::SENSOR, stoveThing);
    HAM_waterPressure  = new HAMqttDevice("Water Pressure", HAMqttDevice::SENSOR, stoveThing);
    HAM_systime        = new HAMqttDevice("Time", HAMqttDevice::SENSOR, stoveThing);


    HAM_ctrl
        ->addConfigVar("icon","mdi:power");

    HAM_targetTemp
        ->addConfigVar("device_class","temperature")
        .addConfigVar("state_class","measurement")
        .addConfigVar("unit_of_measurement","°C")
        .addConfigVar("icon","mdi:thermometer")
        .addConfigVar("min_temp", String(MIN_TARGETTEMP))
        .addConfigVar("max_temp", String(MAX_TARGETTEMP))
        .addConfigVar("min", String(MIN_TARGETTEMP))
        .addConfigVar("max", String(MAX_TARGETTEMP))
        .addConfigVar("step","0.5");

   
    HAM_targetPower
        ->addConfigVar("icon","mdi:heat-wave")
        .addConfigVar("min", String(MIN_POWERLEVEL))
        .addConfigVar("max", String(MAX_POWERLEVEL))
        .addConfigVar("step","1");
        
                
    HAM_connetionState
        ->addConfigVar("icon","mdi:connection");

    HAM_state
        ->addConfigVar("icon","mdi:cog");

    HAM_systime
        ->addConfigVar("icon","mdi:cog");

    HAM_ambientTemp
        ->addConfigVar("device_class","temperature")
        .addConfigVar("state_class","measurement")
        .addConfigVar("unit_of_measurement","°C")
        .addConfigVar("icon","mdi:thermometer");

    HAM_fumeTemp
        ->addConfigVar("device_class","temperature")
        .addConfigVar("state_class","measurement")
        .addConfigVar("unit_of_measurement","°C")
        .addConfigVar("icon","mdi:thermometer");

    HAM_fanSpeed
        ->addConfigVar("unit_of_measurement","rpm")
        .addConfigVar("icon","mdi:fan");

    HAM_flamePower        
        ->addConfigVar("unit_of_measurement","%")
        .addConfigVar("icon","mdi:fire");

    HAM_waterTemp
        ->addConfigVar("device_class","temperature")
        .addConfigVar("state_class","measurement")
        .addConfigVar("unit_of_measurement","°C")
        .addConfigVar("icon","mdi:thermometer");

    HAM_waterPressure
        ->addConfigVar("device_class","pressure")
        .addConfigVar("unit_of_measurement","bar")
        .addConfigVar("icon","mdi:gauge");
}

void MQTTInit(MicronovaStove *stove){
    if (strlen(settings.m_user) > 0) mqtt_user = settings.m_user;
    if (strlen(settings.m_pass) > 0) mqtt_pass = settings.m_pass;

    //micronova
    mqttStove = stove;

    //homeassistant
    if(settings.m_homeassistant > 0){
        MQTTHAInit();
    }

    if(strlen(settings.m_topic) > 0 && strlen(settings.m_client_id) > 0 && strlen(settings.m_host) > 0 && settings.m_port > 0){
        cmdTopic = String(MQTT_TOPIC_PREFIX + String(settings.m_topic) + MQTT_CMD_TOPIC);
        stateTopic = String(MQTT_TOPIC_PREFIX + String(settings.m_topic) + MQTT_STATE_TOPIC);
        mnStateTopic = String(MQTT_TOPIC_PREFIX + String(settings.m_topic) + MQTT_MN_STATE_TOPIC);
                
        mqttClient.setServer(settings.m_host, settings.m_port);
        mqttClient.setCallback(MQTTSubCallback);
        MQTTTimer.attach_ms(60000,MQTTUpdate);

        MQTTPublish(true);
    }else{
        WebLogError("MQTT Missing Parameter"); 
    }
}


