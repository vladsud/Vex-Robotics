#include "intake.h"
using namespace pros;
using namespace pros::c;
#include "pros/motors.h"
#include "cycle.h"
#include <cstdlib>
 

Intake::Intake()
    : leftIntakeLineTracker(leftIntakeLineTrackerPort), rightIntakeLineTracker(rightIntakeLineTrackerPort)
{
    motor_set_brake_mode(intakeLeftPort, E_MOTOR_BRAKE_HOLD);
    motor_set_brake_mode(intakeRightPort, E_MOTOR_BRAKE_HOLD);
}

bool Intake::IsCubeIn(pros::ADIAnalogIn& sensor)
{
    unsigned int sensorValue = sensor.get_value();
    int avg = (cubeIn + cubeOut)/2;
    return sensorValue > avg ? false : true;
}

void Intake::Update()
{
    StateMachine& sm = GetMain().sm;

    //printf("tray: %d    arm: %d\n", sm.trayValue, sm.armValue);

    if (sm.trayValue < 2850 && sm.armValue > 2300)
    {
        printf("Stop Intake because of Tray and Arm \n");
        motor_move(intakeLeftPort, 0);
        motor_move(intakeRightPort, 0);
        return;
    }

    if (isAuto())
    {
        return;
    }

    // Get new controller press
    if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1) ||
            controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2)) {
        is_intake = !is_intake;
    } else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_UP)) //slow outake 
    {
        // Start tower stacking
        is_intake = true;
        // If cube is not in slowing intake
        if (!IsCubeIn(leftIntakeLineTracker) || !IsCubeIn(rightIntakeLineTracker))
        {
            motor_move(intakeLeftPort, 60);
            motor_move(intakeRightPort, -60);
        }
        // When in, stop intaking and cancel action
        else
        {
            motor_move(intakeLeftPort, 0);
            motor_move(intakeRightPort, 0);
        }
        return;
    }


    // If not intaking
    if (!is_intake){
        //printf("Left: %d Right: %d LeftBool: %d RightBool %d \n", leftIntakeLineTracker.get_value(), rightIntakeLineTracker.get_value(), IsCubeIn(leftIntakeLineTracker), IsCubeIn(rightIntakeLineTracker));
        if (!IsCubeIn(leftIntakeLineTracker) || !IsCubeIn(rightIntakeLineTracker))
        {
            if (sm.GetState() == State::Rest && sm.armValue)
            {
                motor_move(intakeLeftPort, -40);
                motor_move(intakeRightPort, 40);
            }
        }
        else
        {
            motor_move(intakeLeftPort, 0);
            motor_move(intakeRightPort, 0);
        }
    } else {
        // Intaking up
        if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1)) //fast intake
        {
            motor_move(intakeLeftPort, intake_normal_speed);
            motor_move(intakeRightPort, -intake_normal_speed);
        }


        //Intaking down    
        if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2)) //slow outake 
        {
            motor_move(intakeLeftPort, -intake_slow_speed);
            motor_move(intakeRightPort, intake_slow_speed);
        }
    }
}