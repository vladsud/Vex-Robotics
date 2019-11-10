#pragma once
#include "main.h"
#include <string>

class Intake{
    private:
        int kP = 25;
        int kI = 100;

        int totalError = 0;
        bool intakeFirstTime = false;
    public:
        void Update();
        int is_intake = 0; 

        int intake_normal_speed = 127; 
        int intake_slow_speed = 40; 
};