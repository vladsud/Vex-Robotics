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
    int currentRotation = sm.armValue;

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
        motor_move(liftMotorPort, -60);
        //printf("Pot: %d  Error: %d  Total Error: %d Speed: %d\n", currentRotation, currError, totalError, currSpeed);
        m_direction = Direction::Up;
    }
    else if (desiredState == State::Rest)
    {
        motor_move(liftMotorPort, 0);
        if (currentRotation < 2850) {
            motor_move(cubetrayPort, 127);
        } else {
            motor_move(cubetrayPort, 0);
        }
        printf("Rotation: %d", currentRotation);
        m_direction = Direction::Down;
        totalError = 0;
    }
    else if (desiredState == State::ArmsUpMid) 
    {
        int currentArm = m_anglePot.get_value();
        kP = 2;
        kI = 4000;

        if (currentArm < 2150)
        {    
            int currTrayError = midValue - currentRotation;
            totalError += currTrayError;

            int currTraySpeed = currTrayError / kP + totalError / kI;
            motor_move(cubetrayPort, -currTraySpeed);
        }
    }
}

