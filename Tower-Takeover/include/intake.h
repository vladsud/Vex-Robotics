#pragma once

#include "pros/adi.hpp"

enum class IntakeMode {
    Intake,
    Hold,
    Outtake,
    Stop,
    IntakeTower,
};

class Intake {
private:
    pros::ADIAnalogIn intakeLineTracker;

    const int cubeIn = 2830;
    const int cubeOut = 2930;
    const int intake_normal_speed = 127; 
    const int intake_slow_speed = 70; 

    int count = 0;

public:
    Intake();
    void Update();
    bool IsCubeIn(pros::ADIAnalogIn& sensor);
    IntakeMode m_mode = IntakeMode::Stop;
    int m_tick = 100;
    const int tickDown = 15;
};

Intake& GetIntake();
void SetIntake(int speed);
