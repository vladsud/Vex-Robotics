#include "main.h"
#include "battery.h"


float Battery::GetMainPower(){
    return powerLevelMain();
}


float Battery::GetExpanderPower(){
    float rawPower = (float)analogRead(ExpanderBatteryStatus);
    return rawPower/270.0;
}

float Battery::GetBackupPower(){
    return powerLevelBackup();
}