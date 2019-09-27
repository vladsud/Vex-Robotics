#include "intake.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>

using namespace pros;
using namespace pros::c;

void Intake::UpdateIntake(Direction direction)
{
    int power;
    if (direction == Direction::None)
        power = 0;
    else if (direction == Direction::Up)
        power = intakeMotorSpeedUp;
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
    motor_move(intakeLeftPort, -speed);
    motor_move(intakeRightPort, speed);
}

void Intake::Update()
{
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1))
    {
        if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2))
        {
            m_direction = Direction::None;
        }
        else
        {
            m_direction = Direction::Up;
        }
    }
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2))
    {
        m_direction = Direction::Down;
    }
    UpdateIntake(m_direction);
}

