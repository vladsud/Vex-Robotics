#include "intake.h"
#include "logger.h"
#include "pros/motors.h"

using namespace pros;
using namespace pros::c;

void Intake::ResetState()
{
    m_power = 0;
    m_direction = Direction::None;
    m_doublePressed = false;
}

void Intake::SetIntakeDirection(Direction direction)
{
    m_direction = direction;

    if (direction == Direction::None)
    {
        m_power = 0;
        SetIntakeMotor(0);
        return;
    }

    int power = (m_direction == Direction::Up) ? -intakeMotorSpeedUp : intakeMotorSpeedDown;

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
        m_ballGoDownState = false;
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
        m_ballGoDownState = false;
        direction = Direction::Down;
    }
    else
    {
        m_doublePressed = false;
        if (m_ballGoDownState)
        {
            int pos = motor_get_position(intakePort);
            if (pos < 0)
                motor_tare_position(intakePort);
            if (pos > 150)
            {
                m_ballGoDownState = false;
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
        //if (m_direction == Direction::None && !m_doublePressed)
        SetIntakeDirection(Direction::Up);
        break;
    case IntakeShoterEvent::Shooting:
        SetIntakeDirection(Direction::None);
        break;
    case IntakeShoterEvent::TooManyBalls:
        motor_tare_position(intakePort);
        m_ballGoDownState = true;
        SetIntakeDirection(Direction::Down);
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
            motor_move(descorerPort, 85);
        }
    }
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_R2))
    {
        m_direction = Direction::Down;
        m_count = 0;
        motor_move(descorerPort, -85);
    }
    else
    {
        m_count++;
        if (m_count > 20)
        {
            motor_move(descorerPort, 0);
            m_direction = Direction::None;
        }
        else if (m_direction == Direction::Up)
        {
            motor_move(descorerPort, -15);
        }
        else
        {
            motor_move(descorerPort, 15);
        }
    }

}
