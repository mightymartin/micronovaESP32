#include "logging.h"

WebSocketsServer webSocket(CONF_WEBSOCKET_PORT);

static uint8_t isInit = 0;

void WebLogInit(){
    webSocket.begin();
    isInit = 1;
}

void WebLogDebug(String msg){
    if(settings.u_LOGGING >= LOGLEVEL_DBG){
        Serial.println(msg); 
        msg = TimeformatedDateTime() + String(" DBG ") + msg;
        if(isInit == 1){
            webSocket.broadcastTXT(msg.c_str(),msg.length());
        }        
    }
}

void WebLogInfo(String msg){
    if(settings.u_LOGGING >= LOGLEVEL_INF){
        Serial.println(msg); 
        msg = TimeformatedDateTime() + String(" INF ") + msg;
        if(isInit == 1){
            webSocket.broadcastTXT(msg.c_str(),msg.length());
        }
    }
}

void WebLogError(String msg){
    if(settings.u_LOGGING >= LOGLEVEL_ERR){
        Serial.println(msg); 
        msg = TimeformatedDateTime() + String(" ERR ") + msg;
        if(isInit == 1){
            webSocket.broadcastTXT(msg.c_str(),msg.length());
        }
    }
}

void WebLogTick(){
    if(isInit == 1){
        webSocket.loop();
    }
}