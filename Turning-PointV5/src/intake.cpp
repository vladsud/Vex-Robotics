#include "intake.h"
#include "logger.h"
#include "pros/motors.h"
#include "cycle.h"
#include <stdio.h>

using namespace pros;
using namespace pros::c;

void Intake::ResetState()
{
    m_power = 0;
    m_direction = Direction::None;
    m_doublePressed = false;
    motor_set_reversed(intakePort, true);
}

void Intake::SetIntakeDirection(Direction direction)
{
    m_direction = direction;

    int power;
    if (direction == Direction::None)
        power = 0;
    else if (m_direction == Direction::Up)
        power = intakeMotorSpeedUp * 0.85;
    else if (m_direction == Direction::DownWithTooManyBalls)
        power = -intakeMotorSpeedDown / 2;
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
    motor_move(intakePort, speed);
}

void Intake::Update()
{
    Direction direction = m_direction;

    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1))
    {
        if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2))
        {
            m_doublePressed = true;
            m_direction = Direction::None;
        }
        else
        {
            direction = Direction::Up;
        }
    }
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2))
    {
        direction = Direction::Down;
    }
    else
    {
        m_doublePressed = false;
        if (direction == Direction::DownWithTooManyBalls)
        {
            int pos = motor_get_position(intakePort);
            // There is a lot of intertia, but ball is not moving up any more
            // DEtect when intake actually reverses, and start counting from there...
            if (pos > 0 || GetMain().shooter.Ball2Status() == BallPresence::HasBall)
                motor_tare_position(intakePort);
            if (pos < -100)
            {
                direction = Direction::None;
                m_power = 0; // remove latency!
            }
        }
    }

    if (m_doublePressed)
        direction = m_direction;
    
    SetIntakeDirection(direction);
}

void Intake::UpdateIntakeFromShooter(IntakeShoterEvent event)
{
    switch (event)
    {
    case IntakeShoterEvent::LostBall:
        SetIntakeDirection(Direction::Up);
        break;
    case IntakeShoterEvent::Shooting:
        // Do nothing here, a these events wrap around LostBall and prevent us from receiving new ball
        // SetIntakeDirection(Direction::None);
        break;
    case IntakeShoterEvent::TooManyBalls:
        motor_tare_position(intakePort);
        SetIntakeDirection(Direction::DownWithTooManyBalls);
        break;
    }
}

void Descorer::Update()
{
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R1))
    {
        // if (m_direction == Direction::Up || joystickGetDigital(JoystickDescorerGroup, JOY_UP))
        {
            m_count = 0;
            m_direction = Direction::Up;
            motor_move(descorerPort, 95);
        }
    }
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2))
    {
        m_direction = Direction::Down;
        m_count = 0;
        motor_move(descorerPort, -95);
    }
    else
    {
        motor_move(descorerPort, 0);
        m_direction = Direction::None;
    }

}
