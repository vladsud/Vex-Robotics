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

    // Target value
    StateMachine& sm = GetMain().sm;

    int currentRotation = sm.trayValue;

    State desiredState = sm.GetState();
    if (desiredState == State::TrayOut)
    {
        motor_move(cubetrayPort, pid.GetPower(currentRotation, upValue, -25, -8000));
        //printf("Pot: %d  Error: %d  Total Error: %d Speed: %d\n", currentRotation, currError, totalError, currSpeed);
    }
    else if (desiredState == State::Rest)
    {
        if (sm.armValue < 1900 || currentRotation >= restValue)
        {
            motor_move(cubetrayPort, 0);
        }
        else
        {
            motor_move(cubetrayPort, 127);
        }
        //printf("Rotation: %d", currentRotation);
    }
    else if (desiredState == State::ArmsUpMid) 
    {
        motor_move(cubetrayPort, pid.GetPower(currentRotation, midValue, -2, -4000));
    }
    else if (desiredState == State::InitializationState) 
    {
        //printf("Move Tray Now");
        motor_move(cubetrayPort, pid.GetPower(currentRotation, initValue, -4, -4000));
    } else {
        motor_move(cubetrayPort, 0);
    }
}

