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
    
    // Target value
    int armTarget = 1100;

    // Update current Arm and Tray value
    int currentArm = sm.armValue;
    int currentTray = sm.trayValue;
    int currArmError = currentArm - armTarget;

    //int kPTray = 2;
    int kPArm = 3;
    int kI = 1000;

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
            totalArmError += currArmError;

            int currArmSpeed = currArmError / kPArm + totalArmError / kI;
            SetLiftMotor(currArmSpeed);
        }
    }
    else if (sm.GetState() == State::Rest) 
    {
        count++;
        if (currentArm < 2200)
        {
            SetLiftMotor(-127);         
        }
        else
        {
            if (count % 100 < 50)
            {
                SetLiftMotor(-10);
                //printf("On\n");
            }
            else
            {
                SetLiftMotor(0);
                //printf("Off\n");
            }
        }
    }
    else if (sm.GetState() == State::InitializationState) 
    {
        int initK = 2;
        int initI = 1000;
        totalArmError += currArmError;

        int currArmSpeed = currArmError / initK + totalArmError / initI;
        printf("Arm Speed: %d   Current Arm: %d\n", currArmSpeed, currentArm);
        SetLiftMotor(currArmSpeed);
    }

    //printf("UP: %d DOWN: %d\n", goUP, goDOWN);
}

