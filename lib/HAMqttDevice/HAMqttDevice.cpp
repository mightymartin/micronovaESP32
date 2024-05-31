#include "HAMqttDevice.h"

HAMqttDevice::HAMqttDevice(
    const String &deviceName,
    const DeviceType deviceType,
    const HAMqttThing  &deviceThing,
    const String &haMQTTPrefix) : _deviceName(deviceName),
                                  _deviceType(deviceType),
                                  _deviceThing(deviceThing)
{
    // Id = name to lower case replacing spaces by underscore (ex: name="Kitchen Light" -> id="kitchen_light")
    _deviceIdentifier = deviceName;
    _deviceIdentifier.replace(' ', '_');
    _deviceIdentifier.toLowerCase();
    

    // Define the MQTT topic of the device
    _deviceTopic = haMQTTPrefix + '/' + deviceTypeToStr(_deviceType) + '/' + getThingUID() + '/' + _deviceIdentifier;
    _thingTopic = haMQTTPrefix + "/things/" + getThingUID() ;

    
    // Preconfigure mandatory config vars that we already know
    addConfigVar("~", _deviceTopic);
    addConfigVar("name", _deviceName);
    addConfigVar("uniq_id", getThingUID() + "_" + _deviceIdentifier);

    // Global default behaviour
    addConfigVar("ret", "false");
    addConfigVar("opt", "false");
    addConfigVar("qos", "0");   

    // When the command topic is mandatory, enable it.
    if (deviceType == DeviceType::ALARM_CONTROL_PANEL||
        deviceType == DeviceType::FAN ||
        deviceType == DeviceType::LIGHT ||
        deviceType == DeviceType::LOCK ||
        deviceType == DeviceType::NUMBER ||
        deviceType == DeviceType::SWITCH ||
        deviceType == DeviceType::BUTTON ||
        deviceType == DeviceType::TEXT )
    {
        enableCommandTopic();
    }

    // When the state topic is mandatory, enable it.
    if (deviceType == DeviceType::ALARM_CONTROL_PANEL||
        deviceType == DeviceType::BINARY_SENSOR ||
        deviceType == DeviceType::FAN ||
        deviceType == DeviceType::LIGHT ||
        deviceType == DeviceType::LOCK ||
        deviceType == DeviceType::NUMBER ||    
        deviceType == DeviceType::SENSOR || 
        deviceType == DeviceType::SWITCH ||
        deviceType == DeviceType::TEXT )
    {
        enableStateTopic();
    }

    if(deviceType == DeviceType::SWITCH)
    {
        addConfigVar("pl_on", "ON");
        addConfigVar("pl_off", "OFF");
        addConfigVar("stat_on", "ON");
        addConfigVar("stat_off", "OFF");
        enableStateTopic();
    }

}

HAMqttDevice::~HAMqttDevice() {}

HAMqttDevice &HAMqttDevice::enableCommandTopic()
{
    addConfigVar("cmd_t", "~/cmd");
    return *this;
}

HAMqttDevice &HAMqttDevice::enableStateTopic()
{
    addConfigVar("stat_t", "~/state");
    return *this;
}


HAMqttDevice &HAMqttDevice::addConfigVar(const String &name, const String &value)
{
    _configVars.push_back({name, value});
    return *this;
}

HAMqttDevice &HAMqttDevice::addAttribute(const String &name, const String &value)
{
    _attributes.push_back({name, value});
    return *this;
}

HAMqttDevice &HAMqttDevice::clearAttributes()
{
    _attributes.clear();
    return *this;
}

const String HAMqttDevice::getDevicePayload()
{
    String devicePayload = "{";

    devicePayload.concat("\"ids\": [\"" + getThingUID() + "\"],");
    
    if(!_deviceThing.name.isEmpty()){
        devicePayload.concat("\"name\": \"" + _deviceThing.name + "\",");
    }

    if(!_deviceThing.manufacturer.isEmpty()){
        devicePayload.concat("\"mf\": \"" + _deviceThing.manufacturer + "\",");
    }

    if(!_deviceThing.model.isEmpty()){
        devicePayload.concat("\"mdl\": \"" + _deviceThing.model + "\",");
    }

    if(!_deviceThing.serial_number.isEmpty()){
        devicePayload.concat("\"sn\": \"" + _deviceThing.serial_number + "\",");
    }

    if(!_deviceThing.hw_version.isEmpty()){
        devicePayload.concat("\"hw\": \"" + _deviceThing.hw_version + "\",");
    }

    if(!_deviceThing.sw_version.isEmpty()){
        devicePayload.concat("\"sw\": \"" + _deviceThing.sw_version + "\",");
    }

    if(!_deviceThing.configuration_url.isEmpty()){
        devicePayload.concat("\"url\": \"" + _deviceThing.configuration_url + "\",");
    }  
      
    devicePayload.setCharAt(devicePayload.length() - 1, '}');

    return devicePayload;
}

const String HAMqttDevice::getConfigPayload() 
{
    String configPayload = "{";

    for (uint8_t i = 0; i < _configVars.size(); i++)
    {
        configPayload.concat('"');
        configPayload.concat(_configVars[i].key);
        configPayload.concat("\":");

        bool valueIsDictionnary = _configVars[i].value[0] == '{';

        if (!valueIsDictionnary)
            configPayload.concat('"');

        configPayload.concat(_configVars[i].value);

        if (!valueIsDictionnary)
            configPayload.concat('"');

        configPayload.concat(',');
    }

    configPayload.concat("\"dev\":");
    configPayload.concat(getDevicePayload()+",");
    
    configPayload.concat("\"avty\":");
    configPayload.concat(getAvailabilityPayload()+",");

    configPayload.setCharAt(configPayload.length() - 1, '}');

    return configPayload;
}

const String HAMqttDevice::getAvailabilityPayload() 
{
    String availabilityPayload = "{";

    if(!_deviceThing.lwtOnline.isEmpty()){
        availabilityPayload.concat("\"pl_avail\": \"" + _deviceThing.lwtOnline + "\",");
    }
    if(!_deviceThing.lwtOffline.isEmpty()){
        availabilityPayload.concat("\"pl_not_avail\": \"" + _deviceThing.lwtOffline + "\",");
    }
    availabilityPayload.concat("\"t\": \"" + getLWTTopic() + "\",");

    availabilityPayload.setCharAt(availabilityPayload.length() - 1, '}');
    
    return availabilityPayload;
}

const String HAMqttDevice::getThingUID(){ 
    String thingID = _deviceThing.serial_number + "_" + _deviceThing.name;
    thingID.replace(' ', '_');
    thingID.toLowerCase();

    return thingID;
}

String HAMqttDevice::deviceTypeToStr(DeviceType type)
{
    switch (type)
    {
    case DeviceType::ALARM_CONTROL_PANEL:
        return "alarm_control_panel";
    case DeviceType::BINARY_SENSOR:
        return "binary_sensor";
    case DeviceType::CAMERA:
        return "camera";
    case DeviceType::COVER:
        return "cover";
    case DeviceType::FAN:
        return "fan";
    case DeviceType::LIGHT:
        return "light";
    case DeviceType::LOCK:
        return "lock";
    case DeviceType::SENSOR:
        return "sensor";
    case DeviceType::SWITCH:
        return "switch";
    case DeviceType::CLIMATE:
        return "climate";
    case DeviceType::VACUUM:
        return "vacuum";
    case DeviceType::NUMBER:
        return "number";
    case DeviceType::BUTTON:
        return "button";
    case DeviceType::TEXT:
        return "text";        
    default:
        return "[Unknown DeviceType]";
    }
}
