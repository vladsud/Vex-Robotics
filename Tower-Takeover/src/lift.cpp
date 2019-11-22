#include "lift.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>


using namespace pros;
using namespace pros::c;

static PidImpl pid(1 /*precision*/);

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
        SetLiftMotor(pid.GetPower(currentArm, 1300, 3, 1000, PidPrecision::LowerOk));
    }
    else if (sm.GetState() == State::InitializationState)
    {
        int currArmSpeed = pid.GetPower(currentArm, 1700, 2, 1000, PidPrecision::LowerOk);
        SetLiftMotor(currArmSpeed);
    }
    else if (sm.GetState() == State::Rest) 
    {
        count++;

        int currArmSpeed = pid.GetPower(currentArm, 2530, 1, 0, PidPrecision::HigerOk);
        if (currArmSpeed == 0 && (count % 100) < 50)
        {
            currArmSpeed = -20;
        }
        SetLiftMotor(currArmSpeed);
    } else {
        SetLiftMotor(0);
    }

    //printf("UP: %d DOWN: %d\n", goUP, goDOWN);
}

