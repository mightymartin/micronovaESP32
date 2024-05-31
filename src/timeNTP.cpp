#include "timeNTP.h"
#include "settings.h"

WiFiUDP ntpUDP;

NTPClient ntpClient(ntpUDP); 

TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone timezone(CEST, CET);

time_t utcEpoche,localeEpoche = 0;

void TimeInit(){
    ntpClient.begin();
    ntpClient.setUpdateInterval(settings.n_ntpinterval);
    ntpClient.setPoolServerName(settings.n_ntpserver);
    TimeTick();
}

void TimeTick(){
    ntpClient.update();        
    utcEpoche = ntpClient.getEpochTime();
    localeEpoche = timezone.toLocal(utcEpoche);        
}

uint8_t TimeSeconds(){
    return second(localeEpoche);
}

uint8_t TimeMinutes(){
    return minute(localeEpoche);
}

uint8_t TimeHours(){
    return hour(localeEpoche);
}

uint8_t TimeDay(){
    return day(localeEpoche);
}

uint8_t TimeMonth(){
    return month(localeEpoche);
}

uint16_t TimeYear(){
    return year(localeEpoche);
}

String TimeformatedDateTime(){
    String day   = TimeDay() < 10 ? "0" + String(TimeDay()) : String(TimeDay());
    String month = TimeMonth() < 10 ? "0" + String(TimeMonth()) : String(TimeMonth());
    String year  = TimeYear() < 10 ? "0" + String(TimeYear()) : String(TimeYear());

    return day + "." + month + "." + year + " " + TimeformatedTime();
}

String TimeformatedTime(){    
    String hoursStr = TimeHours() < 10 ? "0" + String(TimeHours()) : String(TimeHours());
    String minuteStr = TimeMinutes() < 10 ? "0" + String(TimeMinutes()) : String(TimeMinutes());
    String secondStr = TimeSeconds() < 10 ? "0" + String(TimeSeconds()) : String(TimeSeconds());

    return hoursStr + ":" + minuteStr + ":" + secondStr;
}