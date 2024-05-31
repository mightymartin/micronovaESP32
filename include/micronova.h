#ifndef MICRONOVA_H
#define MICRONOVA_H

#include "settings.h"
#include "logging.h"
#include <SoftwareSerial.h>

#define STATE_OFF           0  //OFF
#define STATE_STARTING      1  //Starting
#define STATE_LOADING       2  //Pellet loading
#define STATE_IGNITION      3  //Ignition
#define STATE_WORK          4  //Work
#define STATE_CLEANING      5  //Brazier cleaning
#define STATE_FIN_CLEANING  6  //Final cleaning
#define STATE_STANDBY       7  //Standby
#define STATE_FUEL_ALARM    8  //Pellet missing alarm
#define STATE_IGN_ALARM     9  //Ignition failure alarm
#define STATE_GEN_ALARM     10 //Alarms (to be investigated)

#define STATE_DISP_NAME_OFF           "OFF"
#define STATE_DISP_NAME_STARTING      "Starting"
#define STATE_DISP_NAME_LOADING       "Pellet loading"
#define STATE_DISP_NAME_IGNITION      "Ignition"
#define STATE_DISP_NAME_WORK          "Work"
#define STATE_DISP_NAME_CLEANING      "Brazier cleaning"
#define STATE_DISP_NAME_FIN_CLEANING  "Final cleaning"
#define STATE_DISP_NAME_STANDBY       "Standby"
#define STATE_DISP_NAME_FUEL_ALARM    "Pellet missing alarm"
#define STATE_DISP_NAME_IGN_ALARM     "Ignition failure alarm"
#define STATE_DISP_NAME_GEN_ALARM     "Generic alarm"

#define MN_CONNECTED_TAG    "MN_CONNECTED"
#define MN_STATE_TAG        "MN_STATE"
#define MN_TARG_TEMP_TAG    "MN_TARGET_TEMP"
#define MN_TARG_POWER_TAG   "MN_TARGET_POWER"
#define MN_AMBI_TEMP_TAG    "MN_AMBI_TEMP"
#define MN_FUME_TEMP_TAG    "MN_FUME_TEMP"
#define MN_FLAME_POWER_TAG  "MN_FLAME_POWER"
#define MN_FAN_SPEED_TAG    "MN_FAN_SPEED"
#define MN_WATER_TEMP_TAG   "MN_WATER_TEMP"
#define MN_WATER_PRESS_TAG  "MN_WATER_PRESS"

#define MQTT_MN_STATE_TOPIC "/MNSTATE"
#define MQTT_TAG_ONOFF      "ONOFF"
#define MQTT_TAG_TTEMP      "TTEMP"
#define MQTT_TAG_TPOWER     "TPOWER"


//###### VALUE BOUNDS
#define MIN_TARGETTEMP  15
#define MAX_TARGETTEMP  30
#define MIN_POWERLEVEL  1
#define MAX_POWERLEVEL  4

//###### REGISTER
#define REG_RAM_READ     0x00
#define REG_EEPROM_READ  0x20
#define REG_RAM_WRITE    0x80
#define REG_EEPROM_WRITE 0xA0

//###### REQUEST ADRESS
#define REQ_STOVE_STATE         0x21                //RAM
#define REQ_TARGET_TEMP         0x7D
#define REQ_TARGET_POWERLEVEL   0x7F
#define REQ_AMBIENT_TEMP        0x01                //RAM
#define REQ_FUME_TEMP           0x3E
#define REQ_FLAME_POWER         0x34
#define REQ_FAN_SPEED           0x37
#define REQ_WATER_TEMP          0x03
#define REQ_WATER_PRESS         0x3C

//###### DIFF ANSWER ADDRESS
#define ANSW_TARGET_TEMP        0x9D
#define ANSW_TARGET_POWERLEVEL  0x9F

struct StoveValues_t{
            uint8_t     stoveConnected         = 0 ;
            uint8_t     stoveState             = 0 ;
            float       targetTemp             = 0.0;
            uint8_t     targetPowerlevel       = 0 ;
            float       ambientTemp            = 0.0;
            float       fumeTemp               = 0.0;
            uint16_t    fanSpeed               = 0 ;
            uint8_t     flamePower             = 0 ;
            float       waterTemp              = 0.0;
            float       waterPress             = 0.0;
        }  __attribute__((packed));



class MicronovaStove {
    private:
        char            rxData[2] = {0x00,0x00};
        uint8_t         errorCounter = 0;
        bool            polling = false;
        SoftwareSerial  StoveSerial;
        StoveValues_t   stoveValues;
        Ticker          MNTimer;

        //###### Command (Register, Address, Value)
        const uint8_t stoveOn[3] =  {REG_RAM_WRITE, REQ_STOVE_STATE, 0x01};
        const uint8_t stoveOff[3] = {REG_RAM_WRITE, REQ_STOVE_STATE, 0x06};
        const uint8_t forceOff[3] = {REG_RAM_WRITE, REQ_STOVE_STATE, 0x00};

        //###### SetCommand (Register, Address, --- input ---)
        const uint8_t powerLevel[2] = {REG_EEPROM_WRITE, REQ_TARGET_POWERLEVEL}; 
        const uint8_t targetTemp[2] = {REG_EEPROM_WRITE, REQ_TARGET_TEMP};

        //###### requests (Register,Address,AnswAdress)
        const uint8_t reqStoveState[3] =        {REG_RAM_READ, REQ_STOVE_STATE, REQ_STOVE_STATE}; 
        const uint8_t reqAmbientTemp[3] =       {REG_RAM_READ, REQ_AMBIENT_TEMP, REQ_AMBIENT_TEMP}; 
        
        const uint8_t reqTargetPowerLevel[3] =  {REG_EEPROM_READ, REQ_TARGET_POWERLEVEL, ANSW_TARGET_POWERLEVEL}; 
        const uint8_t reqTargetTemp[3] =        {REG_EEPROM_READ, REQ_TARGET_TEMP, ANSW_TARGET_TEMP};

        const uint8_t reqFumeTemp[3] =          {REG_RAM_READ, REQ_FUME_TEMP, REQ_FUME_TEMP}; 
        const uint8_t reqFanSpeed[3] =          {REG_RAM_READ, REQ_FAN_SPEED, REQ_FAN_SPEED}; 
        const uint8_t reqFlamePower[3] =        {REG_RAM_READ, REQ_FLAME_POWER, REQ_FLAME_POWER}; 

        const uint8_t reqWaterTemp[3] =         {REG_RAM_READ, REQ_WATER_TEMP, REQ_WATER_TEMP}; 
        const uint8_t reqWaterPress[3] =        {REG_RAM_READ, REQ_WATER_PRESS, REQ_WATER_PRESS};         
        
        void    enableRX(bool value);
               
        uint8_t calcChecksum(uint8_t reg, uint8_t address, uint8_t value);
        void    clearAnswer();
        uint8_t readAnswer(uint8_t address);    
        uint8_t sendCommand(uint8_t reg, uint8_t address, uint8_t value);
        uint8_t sendRequest(uint8_t reg, uint8_t address, uint8_t answAddress);

        String  getPropInt(String key, int32_t val);
        String  getPropStr(String key, String val);
        String  getPropFloat(String key, float val);
                
    public:
        void     init();
        void     getStoveState();
        void     getTargetTemp();
        void     getTargetPowerlevel();
        void     getAmbientTemp();
        void     getFumeTemp();
        void     getFanSpeed();
        void     getFlamepower();
        void     getWaterTemp();
        void     getWaterPress();
        void     getAll();      

        uint8_t  stoveIsOn(); 
        uint8_t  setStoveOn();
        uint8_t  setStoveOff(uint8_t force);

        uint8_t  setTargetTemp(uint8_t value);
        uint8_t  setPowerlevel(uint8_t value);

        uint8_t  decreaseTargetTemp();
        uint8_t  increaseTargetTemp();
        uint8_t  decreasePowerlevel();
        uint8_t  increasePowerlevel();
        StoveValues_t getValues();

        String   getJsonValues();
        String   getStoveStateDispName();
        
        uint32_t getChecksum();
        bool     isPolling();

        void     updateTick();
};


#endif