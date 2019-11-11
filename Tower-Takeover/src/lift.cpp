#include "lift.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>


using namespace pros;
using namespace pros::c;

Lift::Lift() : m_anglePot(liftPotPort)
{
    
}

void Lift::SetLiftMotor(int speed)
{
    motor_move(liftMotorPort, speed);
}

void Lift::Update()
{

    int kP = 25;
    int kI = 8000;

    // Target value
    int armValue = 2800;
    int trayValue = 2100;

    int currentArm = m_anglePot.get_value();
    int currentTray = GetMain().cubetray.m_anglePot.get_value();

    // Up
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X))
    {
        
        SetLiftMotor(90);

        int currTrayError = currentTray - trayValue;
        int currArmError = currentArm - armValue;
        totalTrayError += currTrayError;
        totalArmError += currArmError;

        int currTraySpeed = currTrayError / kP + totalTrayError / kI;
        motor_move(cubetrayPort, -currTraySpeed);
        
        int currArmSpeed = currArmError / kP + totalArmError / kI;
        SetLiftMotor(currArmSpeed);
    }
    // Down
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
    {
        SetLiftMotor(-90);

        if (currentRotation < 2850) {
            motor_move(cubetrayPort, 127);
        } else {
            motor_move(cubetrayPort, 0);
        }
    }
    else
    {
        SetLiftMotor(0);
    }
}

