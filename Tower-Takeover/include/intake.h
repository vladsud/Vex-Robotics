#pragma once
#include "main.h"
#include "pros/adi.hpp"
#include <string>

enum class IntakeMode {
    Intake,
    Hold,
    Stop,
    IntakeTower,
};

class Intake {
private:
    pros::ADIAnalogIn leftIntakeLineTracker;
    pros::ADIAnalogIn rightIntakeLineTracker;

    const int cubeIn = 1650;
    const int cubeOut = 1850;
    const int intake_normal_speed = 127; 
    const int intake_slow_speed = 60; 

public:
    Intake();
    void Update();
    bool IsCubeIn(pros::ADIAnalogIn& sensor);
    bool is_intake = false;
    bool tower = false;
    IntakeMode m_mode = IntakeMode::Intake;
};
