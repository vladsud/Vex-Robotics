#pragma once
#include "main.h"
#include <string>

class Intake{
public:
    void Update();
    bool is_intake = 0; 
    std::string intake_state = ""; 
    int intake_normal_speed = 127; 
    int intake_slow_speed = 40; 
};