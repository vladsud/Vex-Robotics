#pragma once

#include "main.h"

class Battery
{
    public:
        float GetMainPower();
        float GetExpanderPower();   
        float GetBackupPower();
};