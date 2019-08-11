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

bool Lift::IsInitialized()
{
    return m_initialize;
}

void Lift::Initialize()
{   
    // Move m_initializationDistance
    motor_move_relative(liftMotorPort, m_initializationDistance, 100);
    // If there stop initializing
    if (motor_get_position(cubetrayPort) >= (m_initializationDistance - 10))
        m_initialize = true;
    return;
}

void Lift::Update()
{
    // If not initialized --> initialize
    if (!m_initialize)
        Initialize();
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X))
    {
        SetLiftMotor(90);
    }
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
    {
        SetLiftMotor(-90);
    }
    else
    {
        SetLiftMotor(0);
    }
}

