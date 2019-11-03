#include "cubetray.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>

using namespace pros;
using namespace pros::c;

void Cubetray::Update()
{
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
    {
        m_direction = Direction::Up;
        motor_move(cubetrayPort, 50);
    }
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2))
    {
        m_direction = Direction::Down;
        motor_move(cubetrayPort, -95);
    }
    else
    {
        motor_move(cubetrayPort, 0);
        m_direction = Direction::None;
    }
}

