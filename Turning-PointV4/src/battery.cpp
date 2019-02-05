#include "main.h"
#include "battery.h"


float Battery::GetMainPower(){
    return powerLevelMain();
}


float Battery::GetExpanderPower(){
    // Need to plug in status to a port
    float rawPower = (float)analogRead(ExpanderBatteryStatus);

    // This is 80 or 270 depending on the value
    return rawPower/270.0;
}

float Battery::GetBackupPower(){
    return powerLevelBackup();
}