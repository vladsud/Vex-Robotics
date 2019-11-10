#include "cubetray.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>

using namespace pros;
using namespace pros::c;

void Cubetray::Update()
{
    int actual = motor_get_actual_velocity(cubetrayPort);

    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
    {
        if (actual < targetSpeed - 20)
        {
            motor_move(cubetrayPort, targetSpeed);
        }
        else
        {
            int diff = targetSpeed - motor_get_actual_velocity(intakeLeftPort);
            int setSpeed = currentSpeed + diff * K;
            motor_move(cubetrayPort, setSpeed);
        }
        m_direction = Direction::Up;
    }
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2))
    {
        motor_move(cubetrayPort, -127);
        m_direction = Direction::Down;
    }
    else
    {
        motor_move(cubetrayPort, 0);
        m_direction = Direction::None;
    }
}

