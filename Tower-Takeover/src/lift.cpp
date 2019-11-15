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
    
    // Update current Arm and Tray value
    int currentArm = sm.armValue;
    int currentTray = sm.trayValue;

    //int kPTray = 2;
    int kPArm = 5;
    int kI = 4000;

    // Target value
    int armValue = 1100;
    //int trayValue = 2250;

    // If button is pressed reset errors
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X) || joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
    {
        totalTrayError = 0;
        totalArmError = 0;
    }   

    if (sm.GetState() == State::ArmsUpMid) {
        
        int currArmError = currentArm - armValue;
        totalArmError += currArmError;

        int currArmSpeed = currArmError / kPArm + totalArmError / kI;
        SetLiftMotor(currArmSpeed);
        
        //printf("Error:%d\n", currArmError);

        /*
        if (currentArm < 2150)
        {    
            int currTrayError = currentTray - trayValue;
            totalTrayError += currTrayError;

            int currTraySpeed = currTrayError / kPTray + totalTrayError / kI;
            motor_move(cubetrayPort, -currTraySpeed);

            if (currTrayError < 5) 
            {
            goUP = false;
            }
        }
        */


        //printf("Tray Error: %d", currTrayError);

        //printf("Arm: %d\n", currentArm);
        //printf("TrayError: %d\n", currTrayError);
    }
    if (sm.GetState() == State::Rest) {
        if (currentTray < 2850) {
            if (currentArm > 1300)
            {
                motor_move(cubetrayPort, 80);
            }
        } else {
            motor_move(cubetrayPort, 0);
        }

        if (currentArm > 2200)
        {
            SetLiftMotor(0);
        }
        else
        {
            SetLiftMotor(-120);
        }

    }

    //printf("UP: %d DOWN: %d\n", goUP, goDOWN);
}

