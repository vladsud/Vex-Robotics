#pragma once

#include "pros/adi.hpp"

enum class IntakeMode {
    Intake,
    Hold,
    Outtake,
    Stop,
    IntakeTower,
    Transition,
};

class Intake {
private:
    pros::ADIAnalogIn intakeLineTracker;

    const int cubeIn = 2340;
    const int cubeOut = 2925;
    const int intake_normal_speed = 127; 
    const int intake_slow_speed = 85; 

    int count = 0;

public:
    Intake();
    void Update();
    bool IsCubeIn(pros::ADIAnalogIn& sensor);
    IntakeMode m_mode = IntakeMode::Stop;
};

Intake& GetIntake();
void SetIntake(int speed);
