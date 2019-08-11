#include "intake.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>

using namespace pros;
using namespace pros::c;

void Cubetray::Initialize()
{
    m_count++;
    motor_move(cubetrayPort, -50);
    int curr = motor_get_position(cubetrayPort);

    // Before 40 count do not move cubetray
    if (m_count < 40)
        return;

    // After if the position is still skip to last 10 count
    if (m_lastPos == curr && m_count < 180)
        m_count = 180;

    m_lastPos = curr;

    // if count is 200 --> cubetray is there --> stop initializing
    if (m_count == 200) 
    {
        motor_tare_position(cubetrayPort);
        m_initialize = true;
    }
}

void Cubetray::Unload()
{
    // If not alreadying unloading --> unload
    if (!m_unload)
    {
        m_unload = true;
        motor_move_absolute(cubetrayPort, 100, 30);
        GetMain().intake.UpdateIntake (Direction::Down);

        // If the motor gets there --> stop unloading and initialize
        if (motor_get_position(cubetrayPort) >= 100)
        {
            m_unload = false;
            GetMain().intake.UpdateIntake (Direction::None);
        }
    }
}

void Cubetray::Update()
{
    // If not initalized --> initialize to reset and find starting position
    if (!m_initialize)
    {
        Initialize();
    }

    m_count = 0;
    // Unload
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_UP))
    {
        Unload();
    }

    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
    {
        m_direction = Direction::Up;
        motor_move(cubetrayPort, 95);
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

