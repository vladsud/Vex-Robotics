#include "lift.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>

using namespace pros;
using namespace pros::c;

void Lift::SetLiftMotor(int speed)
{
    motor_move(liftMotorPort, speed);
}

void Lift::Update()
{
    int avg = (motor_get_actual_velocity(intakeLeftPort) + motor_get_actual_velocity(intakeRightPort))/2;
    // Up
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X))
    {
        if (avg < targetSpeed - 20)
        {
            SetLiftMotor(targetSpeed - 20);
        }
        else
        {
            int diff = targetSpeed - motor_get_actual_velocity(intakeLeftPort);
            int setSpeed = currentSpeed + diff * K;
            SetLiftMotor(setSpeed);
        }
    }
    // Down
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
    {
        if (avg > -targetSpeed + 20)
        {
            SetLiftMotor(-targetSpeed + 20);
        }
        else
        {
            int diff = -targetSpeed - motor_get_actual_velocity(intakeLeftPort);
            int setSpeed = currentSpeed + diff * K;
            SetLiftMotor(-setSpeed);
        }
    }
    else
    {
        SetLiftMotor(0);
    }
}

