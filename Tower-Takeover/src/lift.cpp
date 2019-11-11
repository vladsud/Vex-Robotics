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
    // Up
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X))
    {
        SetLiftMotor(90);
    }
    // Down
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
    {
        SetLiftMotor(-90);
    }
    else
    {
        SetLiftMotor(0);
    }
}

