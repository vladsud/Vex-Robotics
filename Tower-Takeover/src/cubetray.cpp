#include "cubetray.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>

using namespace pros;
using namespace pros::c;

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
    int armValue = sm.armValue;

    if (sm.stateChange) {
        totalError = 0;
    }

    State desiredState = sm.GetState();
    if (desiredState == State::TrayOut)
    {
        kP = 25;
        kI = 8000;

        int currError = currentRotation - upValue;
        totalError += currError;

        int currSpeed = currError / kP + totalError / kI;
        motor_move(cubetrayPort, -currSpeed);
        //printf("Pot: %d  Error: %d  Total Error: %d Speed: %d\n", currentRotation, currError, totalError, currSpeed);
    }
    else if (desiredState == State::Rest)
    {
        if (armValue < 1900)
        {
            motor_move(cubetrayPort, 0);
        }
        else
        {
            if (currentRotation < 2850) 
            {
                motor_move(cubetrayPort, 127);
            } 
            else 
            {
                motor_move(cubetrayPort, 0);
            }
        }
        //printf("Rotation: %d", currentRotation);
    }
    else if (desiredState == State::ArmsUpMid) 
    {
        int currentArm = sm.armValue;
        kP = 2;
        kI = 4000;

        int currTrayError = currentRotation - midValue;
        totalError += currTrayError;
        int currTraySpeed = currTrayError / kP + totalError / kI;
        motor_move(cubetrayPort, -currTraySpeed);

    }
    else if (desiredState == State::InitializationState) 
    {
        int currentArm = sm.armValue;
        if (currentArm < 2000)
        {
            //printf("Move Tray Now");
            kP = 2;
            kI = 4000;

            int currTrayError = currentRotation - 2500;
            totalError += currTrayError;
            int currTraySpeed = currTrayError / kP + totalError / kI;
            motor_move(cubetrayPort, -currTraySpeed);
        }

    }
}

