#include "cubetray.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>

using namespace pros;
using namespace pros::c;

Cubetray::Cubetray() 
    : m_anglePot(cubetrayPotPort)
{
    
}


void Cubetray::Update()
{
    int kP = 10;
    int kI = 200;

    // Target value
    int upValue = 1329;
    int currentRotation = m_anglePot.get_value();

    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
    {

        int currError = currentRotation - upValue;
        totalError += currError;

        int currSpeed = currError / kP + totalError / kI;
        motor_move(cubetrayPort, -currSpeed);
        //printf("Pot: %d  Error: %d  Speed: %d\n", currentRotation, currError, currSpeed);
        m_direction = Direction::Up;
    }
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2))
    {
        motor_move(cubetrayPort, 127);
        m_direction = Direction::Down;
    }
    else
    {
        motor_move(cubetrayPort, 0);
        m_direction = Direction::None;
    }
}

