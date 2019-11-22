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

    int motor = 0;

    if (sm.GetState() == State::ArmsUpMid) 
    {
        motor = pid.GetPower(currentArm, 1300, 3, 1000, PidPrecision::LowerOk);
    }
    else if (sm.GetState() == State::InitializationState)
    {
        motor = pid.GetPower(currentArm, 1700, 2, 1000, PidPrecision::LowerOk);
    }
    else if (sm.GetState() == State::Rest) 
    {
        count++;

        motor = pid.GetPower(currentArm, 2530, 1, 0, PidPrecision::HigerOk);
        if (motor == 0 && (count % 100) < 50)
        {
            motor = -20;
        }
    }

    m_moving = (motor != 0);
    SetLiftMotor(motor);
    //printf("UP: %d DOWN: %d\n", goUP, goDOWN);
}

