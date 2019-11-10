#pragma once
#include "main.h"
#include <string>

class Intake{
public:
    void Update();
    int is_intake = 0; 
    int intake_normal_speed = 127; 
    int intake_slow_speed = 40; 
};