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
    motor_set_brake_mode(cubetrayPort, E_MOTOR_BRAKE_HOLD);
}


void Cubetray::Update()
{
    int kP = 25;
    int kI = 8000;

    // Target value
    int upValue = 1350;
    int currentRotation = m_anglePot.get_value();

    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
    {

        int currError = currentRotation - upValue;
        totalError += currError;

        int currSpeed = currError / kP + totalError / kI;
        motor_move(cubetrayPort, -currSpeed);
        motor_move(liftMotorPort, -60);
        //printf("Pot: %d  Error: %d  Total Error: %d Speed: %d\n", currentRotation, currError, totalError, currSpeed);
        m_direction = Direction::Up;
    }
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2))
    {
        motor_move(liftMotorPort, 0);
        if (currentRotation < 2850) {
            motor_move(cubetrayPort, 127);
        } else {
            motor_move(cubetrayPort, 0);
        }
        printf("Rotation: %d", currentRotation);
        m_direction = Direction::Down;
        totalError = 0;
    }
    else
    {
        motor_move(liftMotorPort, 0);
        motor_move(cubetrayPort, 0);
        m_direction = Direction::None;
        totalError = 0;
    }
}

