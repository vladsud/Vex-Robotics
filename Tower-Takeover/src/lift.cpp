#include "lift.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>


using namespace pros;
using namespace pros::c;

static PidImpl pid(100 /*precision*/);

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

    if (sm.GetState() == State::ArmsUpMid) 
    {
        if (currentTray < 2850)
        {
            SetLiftMotor(pid.GetPower(currentArm, 2850, 3, 1000));
        } else {
            SetLiftMotor(0);
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
        int currArmSpeed = pid.GetPower(currentArm, 1100, 2, 1000);
        printf("Arm Speed: %d   Current Arm: %d\n", currArmSpeed, currentArm);
        SetLiftMotor(currArmSpeed);
    }

    //printf("UP: %d DOWN: %d\n", goUP, goDOWN);
}

