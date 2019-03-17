#include "main.h"
#include "battery.h"


float GetMainPower(){
    return pros::c::battery_get_capacity() / 1000.0;
}
