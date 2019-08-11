#include "cubetray.h"
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
        // Remember to keep unloading
        m_unload = true;
        
    }
    else
    {
        // keep moving until fully unloaded
        motor_move_absolute(cubetrayPort, m_initializationDistance, 30);
        GetMain().intake.UpdateIntake (Direction::Down);

        // If the motor gets there --> stop unloading and initialize
        if (motor_get_position(cubetrayPort) >= (m_initializationDistance - 10))
        {
            m_unload = false;
            GetMain().intake.UpdateIntake (Direction::None);
        }

        // override inputs
        return;
    }
    
}

void Cubetray::Update()
{
    // If not initialized and lift is already initialized --> initialize to reset and find starting position
    if (!m_initialize && GetMain().lift.IsInitialized())
    {
        Initialize();
    }

    m_count = 0;
    // Unload 
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_UP) || m_unload)
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

