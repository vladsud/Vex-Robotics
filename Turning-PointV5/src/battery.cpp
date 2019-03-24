#include "main.h"
#include "battery.h"

using namespace pros::c;

float GetMainPower(){
    return battery_get_capacity();
}
