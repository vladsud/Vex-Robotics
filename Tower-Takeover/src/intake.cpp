#include "intake.h"
using namespace pros;
using namespace pros::c;
#include "pros/motors.h"
#include <cstdlib>
  

Intake::Intake()
    : leftIntakeLineTracker(leftIntakeLineTrackerPort), rightIntakeLineTracker(rightIntakeLineTrackerPort)
{
    motor_set_brake_mode(intakeLeftPort, E_MOTOR_BRAKE_HOLD);
    motor_set_brake_mode(intakeRightPort, E_MOTOR_BRAKE_HOLD);
}

bool Intake::IsCubeIn(pros::ADIAnalogIn& sensor)
{
    // Check if we can detect ball present.
    // Use two stops to make sure we do not move angle up and down if we are somewhere in gray area (on the boundary)
    unsigned int sensorValue = sensor.get_value();

    int avg = (cubeIn + cubeOut)/2;

    return sensorValue > avg ? false : true;
}


void Intake::Update()
{
    // Start tower stacking
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_UP)) //slow outake 
    {
        tower = true;
        
    }

    // Get new controller press
    if (controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1) ||
            controller_get_digital_new_press(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2)){
        tower = false;
        if (!is_intake)
            is_intake = 1;
        else
        {
            is_intake = 0;
        }
    }

    // If tower stacking action
    if (tower)
    {
        is_intake = 1;
        // If cube is not in slowing intake
        if (!IsCubeIn(leftIntakeLineTracker) || !IsCubeIn(rightIntakeLineTracker))
        {
            motor_move(intakeLeftPort, 40);
            motor_move(intakeRightPort, -40);
        }
        // When in, stop intaking and cancel action
        else
        {
            tower = false;
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
            motor_move(intakeLeftPort, -25);
            motor_move(intakeRightPort, 25);
        }
        else
        {
            motor_move(intakeLeftPort, 0);
            motor_move(intakeRightPort, 0);
        }
        return;
    }

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