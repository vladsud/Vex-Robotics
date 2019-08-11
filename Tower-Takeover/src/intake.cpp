#include "intake.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>

using namespace pros;
using namespace pros::c;

void Intake::UpdateIntake(Direction direction)
{
    m_direction = direction;

    int power;
    if (direction == Direction::None)
        power = 0;
    else if (m_direction == Direction::Up)
        power = intakeMotorSpeedUp * 0.85;
    else
        power = -intakeMotorSpeedDown / 2;

    if (power * m_power >= 0)
        m_power = power;
    else
        // slow down transition when change in direciton happens
        m_power += Sign(power) * 5;

    SetIntakeMotor(m_power);
}

void Intake::SetIntakeMotor(int speed)
{
    motor_move(intakeLeftPort, speed);
    motor_move(intakeRightPort, speed);
}

void Intake::Update()
{
    Direction direction = m_direction;

    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1))
    {
        direction = Direction::Up;
    }
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2))
    {
        direction = Direction::Down;
    }
    else if (m_doublePressed)
        direction = m_direction;
    
    UpdateIntake(direction);
}

