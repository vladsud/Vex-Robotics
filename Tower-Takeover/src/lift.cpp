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

    int kPTray = 2;
    int kPArm = 15;
    int kI = 4000;

    // Target value
    int armValue = 1300;
    int trayValue = 2250;

    int currentArm = m_anglePot.get_value();
    int currentTray = GetMain().cubetray.m_anglePot.get_value();

    // Up
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X))
    {
        int currTrayError = currentTray - trayValue;
        int currArmError = currentArm - armValue;
        totalTrayError += currTrayError;
        totalArmError += currArmError;

        int currTraySpeed = currTrayError / kPTray + totalTrayError / kI;
        motor_move(cubetrayPort, -currTraySpeed);
        
        int currArmSpeed = currArmError / kPArm + totalArmError / kI;
        SetLiftMotor(currArmSpeed);

        //printf("Arm: %d\n", currentArm);
        //printf("TrayError: %d\n", currTrayError);

    }
    // Down
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
    {
        SetLiftMotor(-90);

        if (currentTray < 2850) {
            motor_move(cubetrayPort, 75);
        } else {
            motor_move(cubetrayPort, 0);
        }
    }
    else
    {
        SetLiftMotor(0);
    }
}

