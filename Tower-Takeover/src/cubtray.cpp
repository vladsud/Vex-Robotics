#include "intake.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>

using namespace pros;
using namespace pros::c;

void Cubtray::Update()
{
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
    {
        m_direction = Direction::Up;
        motor_move(cubtrayPort, 95);
    }
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2))
    {
        m_direction = Direction::Down;
        motor_move(cubtrayPort, -95);
    }
    else
    {
        motor_move(cubtrayPort, 0);
        m_direction = Direction::None;
    }

}
