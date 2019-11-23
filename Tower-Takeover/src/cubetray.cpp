#include "cubetray.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>

using namespace pros;
using namespace pros::c;

static PidImpl pid(100);

Cubetray::Cubetray() 
    : m_anglePot(cubetrayPotPort)
{
    motor_set_brake_mode(cubetrayPort, E_MOTOR_BRAKE_HOLD);
}

void Cubetray::Update()
{
    StateMachine& sm = GetMain().sm;
    int currentRotation = sm.trayValue;
    State desiredState = sm.GetState();
    int motor = 0;

    if (desiredState == State::TrayOut)
    {
        motor = pid.GetPower(currentRotation, upValue, -25, -8000);
        // printf("Moving out: %d %d\n", currentRotation, motor);
    }
    else if (desiredState == State::Rest)
    {
        if (sm.armValue >= 1900 && currentRotation < restValue)
        {
            motor = 127;
        }
        //printf("Rotation: %d", currentRotation);
    }
    else if (desiredState == State::ArmsUpMid) 
    {
        motor = pid.GetPower(currentRotation, midValue, -2, -4000);
    }
    else if (desiredState == State::InitializationState) 
    {
        //printf("Move Tray Now");
        motor = pid.GetPower(currentRotation, initValue, -4, -4000);
    }

    m_moving = (motor != 0);
    motor_move(cubetrayPort, motor);
}

