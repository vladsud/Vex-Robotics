#include "lift.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>


using namespace pros;
using namespace pros::c;

Lift::Lift() : m_anglePot(liftPotPort)
{
    motor_set_brake_mode(liftMotorPort, E_MOTOR_BRAKE_BRAKE);
}

void Lift::SetLiftMotor(int speed)
{
    motor_move(liftMotorPort, speed);
}

void Lift::Update()
{

    // Get a reference of the state machine
    StateMachine& sm = GetMain().sm;
    
    // Update current Arm and Tray value
    int currentArm = sm.armValue;
    int currentTray = sm.trayValue;

    //int kPTray = 2;
    int kPArm = 5;
    int kI = 4000;

    // Target value
    int armValue = 1100;

    // If button is pressed reset errors
    if (sm.stateChange)
    {
        totalTrayError = 0;
        totalArmError = 0;
    }   

    if (sm.GetState() == State::ArmsUpMid) 
    {
        if (currentTray < 2850)
        {
            int currArmError = currentArm - armValue;
            totalArmError += currArmError;

            int currArmSpeed = currArmError / kPArm + totalArmError / kI;
            SetLiftMotor(currArmSpeed);
        }
    }
    if (sm.GetState() == State::Rest) 
    {
        if (currentArm < 2200)
        {
            SetLiftMotor(-120);         
        }
        else
        {
            SetLiftMotor(0);
        }

    }

    //printf("UP: %d DOWN: %d\n", goUP, goDOWN);
}

