#include "website.h"

WebServer *_server;
MicronovaStove *webStove;

void WebsiteInit(WebServer *server, MicronovaStove *stove){
    _server = server;
    _server->on(REQ_START, WebsiteStartPage);
    _server->on(REQ_DYNSTART, WebsiteDynamicStartPage);
    _server->on(REQ_CONFIG, WebsiteConfigPage);    
    _server->on(REQ_CONF_NETWORK, WebsiteNetworkConfigPage);    
    _server->on(REQ_CONF_MQTT, WebsiteMQTTConfigPage);        
    _server->on(REQ_CONF_MISC, WebsiteMiscConfigPage);          
    _server->on(REQ_INFO, WebsiteInfoPage);
    _server->on(REQ_FACTORY_RESET, WebsiteFactoryResetPage);  
    _server->on(REQ_OTA_SELECT, WebsiteFirmwareUpdate);  
    _server->on(REQ_CONSOLE, WebsiteConsolePage);      

    //Micronova
    _server->on(REQ_CONF_MICRONOVA, WebsiteMicronovaConfigPage);      
    webStove = stove;

}

void WebsideApplyArgs(){
    for (int i = 0; i < _server->args(); i++) {
        SettingsSetValue(_server->argName(i),_server->arg(i));
    }
}

void WebsiteAction(){
    if (_server->hasArg("ACTION") == true){
        String message = "";
        uint8_t reqReboot = 0;
        if(_server->arg("ACTION").equalsIgnoreCase("SAVE")){
            WebsideApplyArgs();
            SettingsWrite();  
        } else if(_server->arg("ACTION").equalsIgnoreCase("SAVER")){
            WebsideApplyArgs();
            SettingsWrite();            
            reqReboot = 1;
            message = "Settings saved -> Reboot";
        
        //micronova
        } else if(_server->arg("ACTION").equalsIgnoreCase("POWON")){            
            if(webStove->setStoveOn()){
                message = "power on stove";
            }            
        } else if(_server->arg("ACTION").equalsIgnoreCase("POWOFF")){
            if(webStove->setStoveOff(0)){
                message = "power off stove";
            }            
        } else if(_server->arg("ACTION").equalsIgnoreCase("POWOFFF")){            
            if(webStove->setStoveOff(1)){
                message = "force power off stove";
            }
        } else if(_server->arg("ACTION").equalsIgnoreCase("MN_TARGET_TEMP_UP")){
            if(webStove->increaseTargetTemp()){
                message = "increase stove temp";
            }        
        } else if(_server->arg("ACTION").equalsIgnoreCase("MN_TARGET_TEMP_DOWN")){
            if(webStove->decreaseTargetTemp()){
                message = "decrease stove temp";
            }        
        } else if(_server->arg("ACTION").equalsIgnoreCase("MN_TARGET_POWER_UP")){
            if(webStove->increasePowerlevel()){
                message = "increase stove power";
            }
        } else if(_server->arg("ACTION").equalsIgnoreCase("MN_TARGET_POWER_DOWN")){
            if(webStove->decreasePowerlevel()){
                message = "decrease stove power";
            }


        } else if(_server->arg("ACTION").equalsIgnoreCase("APPLY")){
            WebsideApplyArgs();            
        } else if(_server->arg("ACTION").equalsIgnoreCase("REBOOT")){            
            reqReboot = 1;
            message = "Manual Reboot";
        } else if(_server->arg("ACTION").equalsIgnoreCase("RESETS")){
            SettingsClear();                  
            reqReboot = 1;
            message = "Settings Reset";
        } else if(_server->arg("ACTION").equalsIgnoreCase("RESETW")){
            SettingsWifiReset();                  
            reqReboot = 1;
            message = "Wifi Reset; Wifimanager enabled";
        }

        if(message.length() > 0){
            WebLogInfo(message);
        }

        if(reqReboot > 0 ){
            //Seite muss neu geladen werden. Nach X sekunden wird redirected
            String page = FPSTR(SITE_HEAD);    
            page += FPSTR(SITE_BGN_FULL); 

            page.replace("{phead}", message);    
            page.replace("{pcat}" , "Restarting...");
    
            page += FPSTR(SITE_RELOAD_WAIT); 
            page += FPSTR(SITE_END_FULL); 

            WebsiteSend(page);  
            
            SettingsSoftRestart();           
        }
    }
}


void WebsiteDynamicStartPage(){
    String page = FPSTR(SITE_HEAD);  
    page += FPSTR(SITE_BGN_EMBEDDED);  

    if(webStove->getValues().stoveConnected){
        page += FPSTR(SITE_SEPARATOR); 

        page += FPSTR(SITE_DL_LINE);  
        page.replace("{tit}", F("ambient temperature"));
        page.replace("{val}", String(webStove->getValues().ambientTemp) + " °C");    

        page += FPSTR(SITE_DL_LINE);  
        page.replace("{tit}", F("fume temperature"));
        page.replace("{val}", String(webStove->getValues().fumeTemp) + " °C");    

        if(settings.mn_hotwater > 0){
            page += FPSTR(SITE_DL_LINE);  
            page.replace("{tit}", F("water temperature"));
            page.replace("{val}", String(webStove->getValues().waterTemp) + " °C");    

            page += FPSTR(SITE_DL_LINE);  
            page.replace("{tit}", F("water pressure"));
            page.replace("{val}", String(webStove->getValues().waterPress) + " bar");    
        }

        page += FPSTR(SITE_DL_LINE);  
        page.replace("{tit}", F("fan speed"));
        page.replace("{val}", String(webStove->getValues().fanSpeed) + " rpm");  

        page += FPSTR(SITE_DL_LINE);  
        page.replace("{tit}", F("flame power"));
        page.replace("{val}", String(webStove->getValues().flamePower) + "%");  

        page += FPSTR(SITE_DL_LINE);  
        page.replace("{tit}", F("status"));
        page.replace("{val}", webStove->getStoveStateDispName());    

        page += FPSTR(SITE_SEPARATOR); 

        if(webStove->stoveIsOn()){
            page += FPSTR(SITE_HREF_EXT);  
            page.replace("{tit}", F("Poweroff Stove"));
            page.replace("{id}",  F("ACTION"));
            page.replace("{val}", F("POWOFF"));
            page.replace("{col}", F(""));
            page.replace("{dest}", REQ_START);            
        }else{
            page += FPSTR(SITE_HREF_EXT);  
            page.replace("{tit}", F("Start Stove"));
            page.replace("{id}",  F("ACTION"));
            page.replace("{val}", F("POWON"));
            page.replace("{col}", F(""));
            page.replace("{dest}", REQ_START);
        }

        if(webStove->stoveIsOn()){
            page += FPSTR(SITE_SEPARATOR); 

            page += FPSTR(SITE_INP_UPDOWN);  
            page.replace("{tit}", F("target temperature"));
            page.replace("{id}",  F(MN_TARG_TEMP_TAG)); 
            page.replace("{val}", String(webStove->getValues().targetTemp)  + " °C");        

            page += FPSTR(SITE_INP_UPDOWN);  
            page.replace("{tit}", F("target powerlevel"));
            page.replace("{id}",  F(MN_TARG_POWER_TAG)); 
            page.replace("{val}", String(webStove->getValues().targetPowerlevel));        
        }
    }else{
        page += FPSTR(SITE_SEPARATOR); 

        page += FPSTR(SITE_DL_LINE);  
        page.replace("{tit}", F("status"));
        page.replace("{val}", F("stove disconnected"));    

        page += FPSTR(SITE_SEPARATOR); 
    }
    
    
    
    WebsiteSend(page);  
}

void WebsiteStartPage(){
    WebsiteAction();

    String page = FPSTR(SITE_HEAD);    
    page += FPSTR(SITE_BGN);  
    
    //dynamic Startpage    
    page += FPSTR(SITE_IFRAME); 
    page.replace("{src}", REQ_DYNSTART);            
    
    
    //static Startpage
    page += FPSTR(SITE_NL); 
    page += FPSTR(SITE_NL); 

    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Infos"));
    page.replace("{dest}", REQ_INFO);
    
    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Console"));
    page.replace("{dest}", REQ_CONSOLE);

    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Config"));
    page.replace("{dest}", REQ_CONFIG);

    page += FPSTR(SITE_END); 
    
    WebsiteSend(page);    
}

void WebsiteConsolePage(){
    WebsiteAction();

    String page = FPSTR(SITE_HEAD);    
    page += FPSTR(SITE_BGN); 
    page.replace("{phead}", "Console");    

    page += FPSTR(SITE_CONSOLE);
    
    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Back"));
    page.replace("{dest}", REQ_START);

    page += FPSTR(SITE_END); 

    WebsiteSend(page);  
}

void WebsiteFirmwareUpdate(){
    WebsiteAction();

    String page = FPSTR(SITE_HEAD);    
    page += FPSTR(SITE_BGN_FULL);  
    page.replace("{pcat}" , F("Firmwareupdate"));

    page += FPSTR(SITE_UPDATE_FORM);
    page.replace("{dest}" , REQ_OTA);

    page += FPSTR(SITE_FIELDSET_END); 
    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Back"));
    page.replace("{dest}", REQ_CONFIG);

    page += FPSTR(SITE_END); 
    
    WebsiteSend(page); 
    
}

void WebsiteFactoryResetPage(){
    WebsiteAction();

    String page = FPSTR(SITE_HEAD);    
    page += FPSTR(SITE_BGN_FULL);  
    page.replace("{pcat}" , F("Factory Reset"));

    page += FPSTR(SITE_FORM_BGN);
    page.replace("{dest}", REQ_CONFIG);

    page += FPSTR(SITE_BUTTON);  
    page.replace("{tit}", F("Abort"));
    page.replace("{id}",  F("ACTION"));
    page.replace("{val}", F(""));
    page.replace("{col}", F("bgrn"));

    page += FPSTR(SITE_BUTTON);  
    page.replace("{tit}", F("Reset Settings"));
    page.replace("{id}",  F("ACTION"));
    page.replace("{val}", F("RESETS"));
    page.replace("{col}", F("bred"));

    page += FPSTR(SITE_BUTTON);  
    page.replace("{tit}", F("Reset WiFi"));
    page.replace("{id}",  F("ACTION"));
    page.replace("{val}", F("RESETW"));
    page.replace("{col}", F("bred"));

    page += FPSTR(SITE_FORM_END);

    page += FPSTR(SITE_END_FULL); 
    
    WebsiteSend(page);  
}

void WebsiteInfoPage(){
    WebsiteAction();

    String page = FPSTR(SITE_HEAD);    
    page += FPSTR(SITE_BGN_FULL);  
    page.replace("{pcat}" , F("Infos"));

    page += FPSTR(SITE_DL_BGN);  

    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("Firmware Version"));
    page.replace("{val}", String(settings.version) );

    page += F("<br/>"); 
    page += F("<br/>"); 

    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("Chip ID"));
    page.replace("{val}", String(settings.u_chipid) );
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("Flash Chip ID"));
    page.replace("{val}", String("TODO CHIPID") );
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("IDE Flash Size"));
    page.replace("{val}", String(ESP.getFlashChipSize() / 1024)+F("kB"));    
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("Program Size"));
    page.replace("{val}", String(ESP.getSketchSize() / 1024)+F("kB") );
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("Free Program Space"));
    page.replace("{val}", String(ESP.getFreeSketchSpace() / 1024)+F("kB") );
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("Free Memory"));
    page.replace("{val}", String(ESP.getFreeHeap() / 1024)+F("kB") );

    if(settings.u_MQTT){
        page += F("<br/>");  
    
        page += FPSTR(SITE_DL_LINE);  
        page.replace("{tit}", F("MQTT Status"));
        page.replace("{val}", MQTTStatus()  );

        page += FPSTR(SITE_DL_LINE);  
        page.replace("{tit}", F("MQTT state topic"));
        page.replace("{val}", stateTopic );

        page += FPSTR(SITE_DL_LINE);  
        page.replace("{tit}", F("MQTT cmd topic"));
        page.replace("{val}", cmdTopic );    
    }    

    page += F("<br/>");  

    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("Cores"));
    page.replace("{val}", String(ESP.getChipCores()));
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("CPU Freq"));
    page.replace("{val}", String(ESP.getCpuFreqMHz())+F("Mhz"));
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("Model"));
    page.replace("{val}", ESP.getChipModel());    
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("Revision"));
    page.replace("{val}", String(ESP.getChipRevision()));    
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("SDK"));
    page.replace("{val}", ESP.getSdkVersion());
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("Time"));
    page.replace("{val}", String(TimeformatedDateTime()) );
    
    page += F("<br/>");  

    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("IP"));
    page.replace("{val}", WiFi.localIP().toString() );
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("MASK"));
    page.replace("{val}", WiFi.subnetMask().toString() );
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("GATEWAY"));
    page.replace("{val}", WiFi.gatewayIP().toString() );
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("DNS"));
    page.replace("{val}", WiFi.dnsIP().toString() );
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("MAC"));
    page.replace("{val}", WiFi.macAddress());
    page += FPSTR(SITE_DL_LINE);  
    page.replace("{tit}", F("SSID"));
    page.replace("{val}", WiFi.SSID());
    
    page += FPSTR(SITE_DL_END);  
    
    page += FPSTR(SITE_FIELDSET_END); 
    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Back"));
    page.replace("{dest}", REQ_START);

    page += FPSTR(SITE_END); 
    
    WebsiteSend(page);    
}


void WebsiteConfigPage(){
    WebsiteAction();

    String page = FPSTR(SITE_HEAD);    
    page += FPSTR(SITE_BGN_FULL);   
    page.replace("{pcat}" , F("Config"));

    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Network Config"));
    page.replace("{dest}", REQ_CONF_NETWORK);

    if(settings.u_MQTT > 0){
        page += FPSTR(SITE_HREF);  
        page.replace("{tit}", F("MQTT Config"));
        page.replace("{dest}", REQ_CONF_MQTT);
    }

    //micronova
    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Micronova Config"));
    page.replace("{dest}", REQ_CONF_MICRONOVA);

    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Misc. Config"));
    page.replace("{dest}", REQ_CONF_MISC);

    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Firmware Update"));
    page.replace("{dest}", REQ_OTA_SELECT);

    page += FPSTR(SITE_HREF_EXT);  
    page.replace("{tit}", F("Restart"));
    page.replace("{id}",  F("ACTION"));
    page.replace("{val}", F("REBOOT"));
    page.replace("{col}", F("bred"));
    page.replace("{dest}", REQ_START);

    page += FPSTR(SITE_HREF_EXT);  
    page.replace("{tit}", F("Factory Reset"));
    page.replace("{id}",  F("ACTION"));
    page.replace("{val}", F(""));
    page.replace("{col}", F("bred"));
    page.replace("{dest}", REQ_FACTORY_RESET);

    page += FPSTR(SITE_FIELDSET_END); 
    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Back"));
    page.replace("{dest}", REQ_START);
   
    page += FPSTR(SITE_END); 

    WebsiteSend(page); 
}

void WebsiteNetworkConfigPage(){
    WebsiteAction();

    String page = FPSTR(SITE_HEAD);    
    page += FPSTR(SITE_BGN_FULL);  
    page.replace("{pcat}" , F("Network Config"));
    
    page += FPSTR(SITE_FORM_BGN);  
    page.replace("{dest}", REQ_CONF_NETWORK);
    
    page += FPSTR(SITE_INP_T);  
    page.replace("{tit}", F("Hostname"));
    page.replace("{id}",  F(N_HOSTNAME_TAG)); 
    page.replace("{val}", String(settings.n_hostname));
    page.replace("{len}", F("32"));

    page += FPSTR(SITE_INP_N);  
    page.replace("{tit}", F("NTP Interval"));
    page.replace("{id}",  F(N_NTPINTERVAL_TAG)); 
    page.replace("{val}", String(settings.n_ntpinterval));
    page.replace("{min}", F("10000"));
    page.replace("{max}", F("60000"));
    page.replace("{step}", F("1000"));
    
    page += FPSTR(SITE_INP_T);  
    page.replace("{tit}", F("NTP Server"));
    page.replace("{id}",  F(N_NTPSERVER_TAG)); 
    page.replace("{val}", String(settings.n_ntpserver));
    page.replace("{len}", F("32"));
    

    page += FPSTR(SITE_BUTTON);  
    page.replace("{tit}", F("Save & Restart"));
    page.replace("{id}",  F("ACTION"));
    page.replace("{val}", F("SAVER"));
    page.replace("{col}", F("bred"));

    page += FPSTR(SITE_FORM_END);  
    
    page += FPSTR(SITE_FIELDSET_END); 
    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Back"));
    page.replace("{dest}", REQ_CONFIG);

    page += FPSTR(SITE_END); 
     
    WebsiteSend(page); 
}

//micronova
void WebsiteMicronovaConfigPage(){
    WebsiteAction();

    String page = FPSTR(SITE_HEAD);    
    page += FPSTR(SITE_BGN_FULL);  
    page.replace("{pcat}" , F("Micronova Config"));

    page += FPSTR(SITE_FORM_BGN);  
    page.replace("{dest}", REQ_CONF_MICRONOVA);

    page += FPSTR(SITE_INP_N);  
    page.replace("{tit}", F("Poll Interval"));
    page.replace("{id}",  F(MN_POLLINTERVAL)); 
    page.replace("{val}", String(settings.mn_pollinterval));
    page.replace("{min}", F("10000"));
    page.replace("{max}", F("60000"));
    page.replace("{step}", F("1000"));

    page += FPSTR(SITE_INP_N);  
    page.replace("{tit}", F("Baudrate"));
    page.replace("{id}",  F(MN_BAUD)); 
    page.replace("{val}", String(settings.mn_ser_baud));
    page.replace("{min}", F("900"));
    page.replace("{max}", F("115200"));
    page.replace("{step}", F("1"));

    page += FPSTR(SITE_INP_N);  
    page.replace("{tit}", F("RX pin"));
    page.replace("{id}",  F(MN_RX_PIN)); 
    page.replace("{val}", String(settings.mn_ser_rx_pin));
    page.replace("{min}", F("0"));
    page.replace("{max}", F("39"));
    page.replace("{step}", F("1"));

    page += FPSTR(SITE_INP_N);  
    page.replace("{tit}", F("TX pin"));
    page.replace("{id}",  F(MN_TX_PIN)); 
    page.replace("{val}", String(settings.mn_ser_tx_pin));
    page.replace("{min}", F("0"));
    page.replace("{max}", F("39"));
    page.replace("{step}", F("1"));

    page += FPSTR(SITE_INP_N);  
    page.replace("{tit}", F("EN pin"));
    page.replace("{id}",  F(MN_EN_PIN)); 
    page.replace("{val}", String(settings.mn_ser_en_pin));
    page.replace("{min}", F("0"));
    page.replace("{max}", F("39"));
    page.replace("{step}", F("1"));

    page += FPSTR(SITE_INP_CBX_BGN);  
    page.replace("{tit}", F("EN pin state"));
    page.replace("{val}", String(settings.mn_ser_en_state));
    page.replace("{id}",  F(MN_EN_STATE)); 
    page += FPSTR(SITE_INP_CBX_OPT);  
    page.replace("{otit}", F("Active HIGH"));
    page.replace("{oval}", F("1"));
    page.replace("{oopt}", (settings.mn_ser_en_state == 1) ? F("selected") : F(""));
    page += FPSTR(SITE_INP_CBX_OPT);  
    page.replace("{otit}", F("Active LOW"));
    page.replace("{oval}", F("0"));
    page.replace("{oopt}", (settings.mn_ser_en_state == 0) ? F("selected") : F(""));
    page += FPSTR(SITE_INP_CBX_END);  

    page += FPSTR(SITE_INP_CBX_BGN);  
    page.replace("{tit}", F("stove with hot water circle"));
    page.replace("{val}", String(settings.mn_hotwater));
    page.replace("{id}",  F(MN_HOTWATER)); 
    page += FPSTR(SITE_INP_CBX_OPT);  
    page.replace("{otit}", F("yes"));
    page.replace("{oval}", F("1"));
    page.replace("{oopt}", (settings.mn_hotwater == 1) ? F("selected") : F(""));
    page += FPSTR(SITE_INP_CBX_OPT);  
    page.replace("{otit}", F("no"));
    page.replace("{oval}", F("0"));
    page.replace("{oopt}", (settings.mn_hotwater == 0) ? F("selected") : F(""));
    page += FPSTR(SITE_INP_CBX_END); 

    page += FPSTR(SITE_BUTTON);  
    page.replace("{tit}", F("Save & Restart"));
    page.replace("{id}",  F("ACTION"));
    page.replace("{val}", F("SAVER"));
    page.replace("{col}", F("bred"));

    page += FPSTR(SITE_FORM_END); 

    page += FPSTR(SITE_FIELDSET_END); 

    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Back"));
    page.replace("{dest}", REQ_CONFIG);

    page += FPSTR(SITE_END); 
     
    WebsiteSend(page); 
}


void WebsiteMQTTConfigPage(){
    WebsiteAction();

    String page = FPSTR(SITE_HEAD);    
    page += FPSTR(SITE_BGN_FULL);  
    page.replace("{pcat}" , F("MQTT Config"));
    
    page += FPSTR(SITE_FORM_BGN);  
    page.replace("{dest}", REQ_CONF_MQTT);
    
    page += FPSTR(SITE_INP_N);  
    page.replace("{tit}", F("Port"));
    page.replace("{id}",  F(M_PORT_TAG)); 
    page.replace("{val}", String(settings.m_port));
    page.replace("{min}", F("1"));
    page.replace("{max}", F("60000"));
    page.replace("{step}", F("1"));

    page += FPSTR(SITE_INP_T);  
    page.replace("{tit}", F("Broker"));
    page.replace("{id}",  F(M_HOST_TAG)); 
    page.replace("{val}", String(settings.m_host));
    page.replace("{len}", F("32"));

    page += FPSTR(SITE_INP_T);  
    page.replace("{tit}", F("Client ID"));
    page.replace("{id}",  F(M_CLIENT_ID_TAG)); 
    page.replace("{val}", String(settings.m_client_id ));
    page.replace("{len}", F("32"));

    page += FPSTR(SITE_INP_T);  
    page.replace("{tit}", F("User"));
    page.replace("{id}",  F(M_USER_TAG)); 
    page.replace("{val}", String(settings.m_user));
    page.replace("{len}", F("32"));

    page += FPSTR(SITE_INP_T);  
    page.replace("{tit}", F("Password"));
    page.replace("{id}",  F(M_PASS_TAG)); 
    page.replace("{val}", String(settings.m_pass));
    page.replace("{len}", F("32"));
    
    page += FPSTR(SITE_INP_T);  
    page.replace("{tit}", F("Topic unique part"));
    page.replace("{id}",  F(M_TOPIC_TAG)); 
    page.replace("{val}", String(settings.m_topic));
    page.replace("{len}", F("32"));

    page += FPSTR(SITE_INP_CBX_BGN);  
    page.replace("{tit}", F("Homeassistant mode?"));
    page.replace("{val}", String(settings.m_homeassistant));
    page.replace("{id}",  F(M_HASS_TAG)); 
    page += FPSTR(SITE_INP_CBX_OPT);  
    page.replace("{otit}", F("yes"));
    page.replace("{oval}", F("1"));
    page.replace("{oopt}", (settings.m_homeassistant == 1) ? F("selected") : F(""));
    page += FPSTR(SITE_INP_CBX_OPT);  
    page.replace("{otit}", F("no"));
    page.replace("{oval}", F("0"));
    page.replace("{oopt}", (settings.m_homeassistant == 0) ? F("selected") : F(""));
    page += FPSTR(SITE_INP_CBX_END); 

    page += FPSTR(SITE_BUTTON);  
    page.replace("{tit}", F("Save & Restart"));
    page.replace("{id}",  F("ACTION"));
    page.replace("{val}", F("SAVER"));
    page.replace("{col}", F("bred"));

    page += FPSTR(SITE_FORM_END); 

    page += FPSTR(SITE_FIELDSET_END); 
    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Back"));
    page.replace("{dest}", REQ_CONFIG);

    page += FPSTR(SITE_END); 
     
    WebsiteSend(page); 
}

void WebsiteMiscConfigPage(){
    WebsiteAction();

    String page = FPSTR(SITE_HEAD);    
    page += FPSTR(SITE_BGN_FULL);  
    page.replace("{pcat}" , F("Misc. Config"));
    
    page += FPSTR(SITE_FORM_BGN);  
    page.replace("{dest}", REQ_CONF_MISC);

    page += FPSTR(SITE_INP_CBX_BGN);  
    page.replace("{tit}", F("use MQTT"));
    page.replace("{val}", String(settings.u_MQTT));
    page.replace("{id}",  F(U_MQTT_TAG)); 
    page += FPSTR(SITE_INP_CBX_OPT);  
    page.replace("{otit}", F("yes"));
    page.replace("{oval}", F("1"));
    page.replace("{oopt}", (settings.u_MQTT == 1) ? F("selected") : F(""));
    page += FPSTR(SITE_INP_CBX_OPT);  
    page.replace("{otit}", F("no"));
    page.replace("{oval}", F("0"));
    page.replace("{oopt}", (settings.u_MQTT == 0) ? F("selected") : F(""));
    page += FPSTR(SITE_INP_CBX_END);  
    
    page += FPSTR(SITE_INP_CBX_BGN);  
    page.replace("{tit}", F("Log Level"));
    page.replace("{id}",  F(U_LOGG_TAG)); 
    page.replace("{val}",  String(settings.u_LOGGING)); 
    page += FPSTR(SITE_INP_CBX_OPT);  
    page.replace("{otit}", F("off"));
    page.replace("{oval}", String(LOGLEVEL_OFF));
    page.replace("{oopt}", (settings.u_LOGGING == LOGLEVEL_OFF) ? F("selected") : F(""));
    page += FPSTR(SITE_INP_CBX_OPT);  
    page.replace("{otit}", F("error"));
    page.replace("{oval}", String(LOGLEVEL_ERR));
    page.replace("{oopt}", (settings.u_LOGGING == LOGLEVEL_ERR) ? F("selected") : F(""));
    page += FPSTR(SITE_INP_CBX_OPT);  
    page.replace("{otit}", F("info"));
    page.replace("{oval}", String(LOGLEVEL_INF));
    page.replace("{oopt}", (settings.u_LOGGING == LOGLEVEL_INF) ? F("selected") : F(""));
    page += FPSTR(SITE_INP_CBX_OPT);  
    page.replace("{otit}", F("debug"));
    page.replace("{oval}", String(LOGLEVEL_DBG));
    page.replace("{oopt}", (settings.u_LOGGING == LOGLEVEL_DBG) ? F("selected") : F(""));
    page += FPSTR(SITE_INP_CBX_END); 

    page += FPSTR(SITE_BUTTON);  
    page.replace("{tit}", F("Save & Restart"));
    page.replace("{id}",  F("ACTION"));
    page.replace("{val}", F("SAVER"));
    page.replace("{col}", F("bred"));

    page += FPSTR(SITE_FORM_END); 

    page += FPSTR(SITE_FIELDSET_END); 
    page += FPSTR(SITE_HREF);  
    page.replace("{tit}", F("Back"));
    page.replace("{dest}", REQ_CONFIG);

    page += FPSTR(SITE_END); 
     
    WebsiteSend(page); 
}

void WebsiteSend(String page){
    page.replace("{phead}", settings.n_hostname);    
    page.replace("{ptit}" , settings.n_hostname);
    
    _server->sendHeader("Content-Length", String(page.length()));
    _server->send(200, "text/html", page); 

}
