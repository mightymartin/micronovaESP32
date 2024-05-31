#include "micronova.h"

//### PRIVATE
uint8_t MicronovaStove::stoveIsOn(){
    if( stoveValues.stoveState == STATE_STARTING ||
        stoveValues.stoveState == STATE_LOADING ||
        stoveValues.stoveState == STATE_IGNITION ||
        stoveValues.stoveState == STATE_WORK ||
        stoveValues.stoveState == STATE_CLEANING ){
        return 1;
    }

    return 0;
}

void MicronovaStove::enableRX(bool value){
    pinMode(settings.mn_ser_en_pin, OUTPUT);
    if((settings.mn_ser_en_state && value) || (!settings.mn_ser_en_state && !value)){
        WebLogDebug("RXEN_HIGH");
        digitalWrite(settings.mn_ser_en_pin, HIGH);
    }else{
        WebLogDebug("RXEN_LOW");
        digitalWrite(settings.mn_ser_en_pin, LOW);
    }
}

uint8_t MicronovaStove::calcChecksum(uint8_t reg, uint8_t address, uint8_t value){
    uint8_t checksum = 0;
    checksum = reg+address+value;
    if (checksum>=256) {
        checksum=checksum-256;
    } 
    return checksum;
}

void MicronovaStove::clearAnswer(){
    rxData[0] = 0x00;
    rxData[1] = 0x00;
}

uint8_t MicronovaStove::readAnswer(uint8_t address){
    clearAnswer();    
    uint8_t rxCount = 0;
    while (StoveSerial.available()){ //It has to be exactly 2 bytes, otherwise it's an error    
        if(rxCount < 2){
            rxData[rxCount] = StoveSerial.read();
        }
        rxCount++;
    }    

    uint8_t val = rxData[1];
    uint8_t checksum = rxData[0];    
    uint8_t retAdress = checksum - val;
    if(rxCount == 2){            
        if(address == retAdress){
            errorCounter = 0; 
            stoveValues.stoveConnected = 1;            
            return 1;
        }        
    }

    // ERROR CASE
    WebLogError("address:" + String(address,HEX) + ", rx_ctn:" + String(rxCount,HEX) + ", val:" + String(val,HEX) + ", chksum:" + String(checksum,HEX) + ", ret_address:" + String(retAdress,HEX));    
    if(errorCounter < 10){
        errorCounter++;
    }

    if(errorCounter >= 5){
        stoveValues.stoveConnected = 0;       
    }

    return 0;
}

uint8_t MicronovaStove::sendCommand(uint8_t reg, uint8_t address, uint8_t value){
    enableRX(false);
    uint8_t checksum = calcChecksum(reg, address, value);
    StoveSerial.write(reg);
    delay(1);
    StoveSerial.write(address);
    delay(1);
    StoveSerial.write(value);
    delay(1);
    StoveSerial.write(checksum);
    enableRX(true);

    delay(80);
    return readAnswer(address);
}

uint8_t MicronovaStove::sendRequest(uint8_t reg, uint8_t address, uint8_t answAddress){
    enableRX(false);
    StoveSerial.write(reg);
    delay(1);
    StoveSerial.write(address);   
    enableRX(true);

    delay(80);
    return readAnswer(answAddress);
}

//### PUBLIC
void MicronovaStove::init(){
    enableRX(true);
    StoveSerial.begin(settings.mn_ser_baud, SWSERIAL_8N2, settings.mn_ser_rx_pin, settings.mn_ser_tx_pin, false, 256);

    //inital connect
    this->updateTick();

    //dann alle 30 sek
    MNTimer.attach_ms(30000,+[](MicronovaStove* stoveInstance) { stoveInstance->updateTick(); }, this);
}

void MicronovaStove::updateTick(){
    if(stoveValues.stoveConnected){
        getAll();
        return;        
    }
    //aktualisiert den stove state und checkt die verbindung
    getStoveState();
}

void MicronovaStove::getStoveState(){
    if(sendRequest(reqStoveState[0],reqStoveState[1],reqStoveState[2])){
        stoveValues.stoveState = rxData[1];
    }
}

void MicronovaStove::getTargetTemp(){
    if(sendRequest(reqTargetTemp[0],reqTargetTemp[1], reqTargetTemp[2])){
        stoveValues.targetTemp = (float)rxData[1];
    }
}

void MicronovaStove::getTargetPowerlevel(){
    if(sendRequest(reqTargetPowerLevel[0],reqTargetPowerLevel[1], reqTargetPowerLevel[2])){
        stoveValues.targetPowerlevel = rxData[1];
    }
}

void MicronovaStove::getAmbientTemp(){
    if(sendRequest(reqAmbientTemp[0],reqAmbientTemp[1], reqAmbientTemp[2])){
        stoveValues.ambientTemp = (float)rxData[1] / 2;
    }
}

void MicronovaStove::getFumeTemp(){
    if(sendRequest(reqFumeTemp[0],reqFumeTemp[1], reqFumeTemp[2])){
        stoveValues.fumeTemp = (float)rxData[1];
    }
}

void MicronovaStove::getFanSpeed(){
    if(sendRequest(reqFanSpeed[0],reqFanSpeed[1], reqFanSpeed[2])){
        stoveValues.fanSpeed = (uint16_t)rxData[1]*10;
    }
}

void MicronovaStove::getFlamepower(){
    if(sendRequest(reqFlamePower[0],reqFlamePower[1], reqFlamePower[2])){
        if(stoveIsOn()){
            stoveValues.flamePower = map(rxData[1],0,16,10,100);
        }else{
            stoveValues.flamePower = 0;
        }        
    }
}

void MicronovaStove::getWaterTemp(){
    if(sendRequest(reqWaterTemp[0],reqWaterTemp[1], reqWaterTemp[2])){
        stoveValues.waterTemp = (float)rxData[1];
    }
}

void MicronovaStove::getWaterPress(){
    if(sendRequest(reqWaterPress[0],reqWaterPress[1], reqWaterPress[2])){
        stoveValues.waterPress = (float)rxData[1] / 10;
    }
}

void MicronovaStove::getAll(){
    polling = true;
    getStoveState();
    delay(100);
    getTargetTemp();
    delay(100);
    getTargetPowerlevel();
    delay(100);
    getAmbientTemp();
    delay(100);
    getFumeTemp();
    delay(100);
    getFanSpeed();
    delay(100);
    getFlamepower();
    delay(100);
    getWaterTemp();
    delay(100);
    getWaterPress();
    polling = false;
}

uint8_t MicronovaStove::setStoveOn(){
    if(!stoveIsOn()){
        return sendCommand(stoveOn[0],stoveOn[1],stoveOn[2]);
    }
    return 0;
}

uint8_t MicronovaStove::setStoveOff(uint8_t force){
    if(stoveIsOn()){
        if(force){
            return sendCommand(forceOff[0],forceOff[1],forceOff[2]);
        }else{
            return sendCommand(stoveOff[0],stoveOff[1],stoveOff[2]);
        }
    }
    return 0;
}

uint8_t MicronovaStove::decreaseTargetTemp(){
    uint8_t newValue = (stoveValues.targetTemp > 0) ? stoveValues.targetTemp - 1 : 0;
    return setTargetTemp(newValue);
}

uint8_t MicronovaStove::increaseTargetTemp(){
    uint8_t newValue = (stoveValues.targetTemp < 255) ? stoveValues.targetTemp + 1 : 0;
    return setTargetTemp(newValue);
}

uint8_t MicronovaStove::decreasePowerlevel(){
    uint8_t newValue = (stoveValues.targetPowerlevel > 0) ? stoveValues.targetPowerlevel - 1 : 0;
    return setPowerlevel(newValue);
}

uint8_t MicronovaStove::increasePowerlevel(){
    uint8_t newValue = (stoveValues.targetPowerlevel < 255) ? stoveValues.targetPowerlevel + 1 : 0;
    return setPowerlevel(newValue);
}

uint8_t MicronovaStove::setTargetTemp(uint8_t value){
    if(stoveIsOn()){        
        if(value < MIN_TARGETTEMP) { value = MIN_TARGETTEMP;} 
        if(value > MAX_TARGETTEMP) { value = MAX_TARGETTEMP;} 
        WebLogInfo(String(value));

        return sendCommand(targetTemp[0],targetTemp[1],value);                
    }
    return 0;
}

uint8_t MicronovaStove::setPowerlevel(uint8_t value){
    if(stoveIsOn()){
        if(value < MIN_POWERLEVEL) { value = MIN_POWERLEVEL;} 
        if(value > MAX_POWERLEVEL) { value = MAX_POWERLEVEL;}         
        WebLogInfo(String(value));

        return sendCommand(powerLevel[0],powerLevel[1],value);        
    }    
    return 0;
}

String MicronovaStove::getStoveStateDispName(){ 
    String stateDispNames[11] = {
            STATE_DISP_NAME_OFF,         
            STATE_DISP_NAME_STARTING,
            STATE_DISP_NAME_LOADING,    
            STATE_DISP_NAME_IGNITION,    
            STATE_DISP_NAME_WORK,        
            STATE_DISP_NAME_CLEANING,    
            STATE_DISP_NAME_FIN_CLEANING,
            STATE_DISP_NAME_STANDBY,     
            STATE_DISP_NAME_FUEL_ALARM,  
            STATE_DISP_NAME_IGN_ALARM,   
            STATE_DISP_NAME_GEN_ALARM
    };   

    return String(stateDispNames[stoveValues.stoveState]);
}

String MicronovaStove::getPropInt(String key, int32_t val){
    String ret = FPSTR(PROP_INT);
    ret.replace("{key}", key);
    ret.replace("{val}", String(val) );
    return ret;
}  

String MicronovaStove::getPropStr(String key, String val){
    String ret = FPSTR(PROP_STR);
    ret.replace("{key}", key);
    ret.replace("{val}", val );
    return ret;
}

String MicronovaStove::getPropFloat(String key, float val){
    String ret = FPSTR(PROP_INT);
    ret.replace("{key}", key);
    ret.replace("{val}", String(val) );
    return ret;
}  

uint32_t MicronovaStove::getChecksum(){
    uint32_t checksum = 0;
    uint8_t bufferP[sizeof(stoveValues)];
    memcpy(bufferP, &stoveValues, sizeof(stoveValues));

    for (unsigned int i=0; i<sizeof(bufferP); i++){
        checksum += bufferP[i];
    }     
    return checksum;
}

bool MicronovaStove::isPolling(){
    return polling;
}

StoveValues_t MicronovaStove::getValues(){
    return stoveValues;
}

String MicronovaStove::getJsonValues(){
    String jsonDest = "{";       
        
    jsonDest += getPropInt(     MN_CONNECTED_TAG,    stoveValues.stoveConnected);
    jsonDest += getPropInt(     MN_STATE_TAG,        stoveValues.stoveState);
    jsonDest += getPropFloat(   MN_TARG_TEMP_TAG,    stoveValues.targetTemp);
    jsonDest += getPropInt(     MN_TARG_POWER_TAG,   stoveValues.targetPowerlevel);
    jsonDest += getPropFloat(   MN_AMBI_TEMP_TAG,    stoveValues.ambientTemp);
    jsonDest += getPropFloat(   MN_FUME_TEMP_TAG,    stoveValues.fumeTemp);
    jsonDest += getPropInt(     MN_FAN_SPEED_TAG,    stoveValues.fanSpeed);
    jsonDest += getPropInt(     MN_FLAME_POWER_TAG,  stoveValues.flamePower);    
    
    if(settings.mn_hotwater > 0){
        jsonDest += getPropFloat(   MN_WATER_TEMP_TAG,   stoveValues.waterTemp);
        jsonDest += getPropFloat(   MN_WATER_PRESS_TAG,  stoveValues.waterPress);
    }
    
    jsonDest.remove(jsonDest.length()-1);   

    jsonDest += "}";

    return jsonDest;
}
