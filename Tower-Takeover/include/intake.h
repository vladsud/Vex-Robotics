#pragma once
#include "main.h"
#include "pros/adi.hpp"
#include <string>

class Intake{
private:
    pros::ADIAnalogIn leftIntakeLineTracker;
    pros::ADIAnalogIn rightIntakeLineTracker;

    int cubeIn = 2550;
    int cubeOut = 2850;
public:
    Intake();
    void Update();
    bool IsCubeIn(pros::ADIAnalogIn& sensor);
    int is_intake = 0; 
    int intake_normal_speed = 127; 
    int intake_slow_speed = 40; 
};