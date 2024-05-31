#ifndef HA_MQTT_DEVICE_H
#define HA_MQTT_DEVICE_H

#include "Arduino.h"
#include <vector>

#define LWT_ONLINE "online"
#define LWT_OFFLINE "offline"

class HAMqttDevice
{
public:
    struct HAMqttThing{        
        String name;
        String manufacturer;
        String model;
        String serial_number;
        String hw_version;
        String sw_version;
        String configuration_url;      
        String lwtOnline;
        String lwtOffline;        
    };  

    enum DeviceType
    {
        ALARM_CONTROL_PANEL,
        BINARY_SENSOR,
        CAMERA,
        COVER,
        FAN,
        LIGHT,
        LOCK,
        SENSOR,
        SWITCH,
        CLIMATE,
        VACUUM,
        NUMBER,
        BUTTON,
        TEXT
    };

private:
    // Device proprieties
    const String _deviceName;
    const DeviceType _deviceType;
    const HAMqttThing _deviceThing;

    String _deviceIdentifier;
    String _deviceTopic;
    String _thingTopic;

    // Config variables handling
    struct ConfigVar
    {
        String key;
        String value;
    };
    std::vector<ConfigVar> _configVars;

    // Device attributes handling
    struct Attribute
    {
        String key;
        String value;
    };
    std::vector<Attribute> _attributes;

public:
    HAMqttDevice(
        const String &deviceName,
        const DeviceType deviceType,
        const HAMqttThing  &deviceThing,
        const String &haMQTTPrefix = "homeassistant");

    ~HAMqttDevice();

    HAMqttDevice &enableCommandTopic();
    HAMqttDevice &enableStateTopic();
    HAMqttDevice &enableAttributesTopic();

    HAMqttDevice &addConfigVar(const String &key, const String &value);
    HAMqttDevice &addAttribute(const String &key, const String &value);
    HAMqttDevice &clearAttributes();

    const String getAvailabilityPayload();
    const String getDevicePayload();
    const String getConfigPayload();

    inline const String getLWTTopic() { return _thingTopic + "/LWT"; }

    inline const String getTopic() { return _deviceTopic; }
    inline const String getStateTopic() { return _deviceTopic + "/state"; }
    inline const String getConfigTopic() { return _deviceTopic + "/config"; }
    inline const String getAvailabilityTopic() { return _deviceTopic + "/availability"; }
    inline const String getCommandTopic() { return _deviceTopic + "/cmd"; }

private:
    const String getThingUID();
    static String deviceTypeToStr(DeviceType type);
};

#endif