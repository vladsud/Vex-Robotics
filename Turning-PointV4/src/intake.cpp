#include "intake.h"
#include "logger.h"

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

    int power = (m_direction == Direction::Up) ? intakeMotorSpeedUp : -intakeMotorSpeedDown;

    if (power * m_power >= 0)
        m_power = power;
    else
        // slow down transition when change in direciton happens
        m_power += Sign(power);

    SetIntakeMotor(m_power);
    // motorSet(intakePort2, -m_power);
}

void Intake::SetIntakeMotor(int speed)
{
    motorSet(intakePort, speed);
    // motorSet(intakePort2, -speed);
}

void Intake::Update()
{
    Direction direction = m_direction;

    if (joystickGetDigital(JoystickIntakeGroup, JOY_DOWN))
        m_downABit = 50;
    
    if (joystickGetDigital(JoystickDescorerGroup, JOY_UP))
    {
        m_downABit = 0;
        if (joystickGetDigital(JoystickDescorerGroup, JOY_DOWN) || joystickGetDigital(JoystickIntakeGroup, JOY_UP))
        {
            m_doublePressed = true;
            m_direction = Direction::None;
        }
        else
        {
            direction = Direction::Up;
        }
    }
    else if (joystickGetDigital(JoystickDescorerGroup, JOY_DOWN))
    {
        m_downABit = 0;
        direction = Direction::Down;
    }
    else
    {
        m_doublePressed = false;
    }

    if (m_doublePressed)
        direction = m_direction;

    if (m_downABit > 0)
    {
        m_downABit--;
        direction = (m_downABit == 0) ? Direction::None : Direction::Down;
    }

    SetIntakeDirection(direction);
}

void Intake::UpdateIntakeFromShooter(IntakeShoterEvent event, bool forceDown)
{
    switch (event)
    {
    case IntakeShoterEvent::LostBall:
        //if (m_direction == Direction::None && !m_doublePressed)
        SetIntakeDirection(Direction::Up);
        break;
    case IntakeShoterEvent::Shooting:
        if (forceDown)
            SetIntakeDirection(Direction::Down);
        else
            SetIntakeDirection(Direction::None);
        break;
    }
}

void Descorer::Update()
{
    if (joystickGetDigital(JoystickIntakeGroup, JOY_UP))
    {
        // if (m_direction == Direction::Up || joystickGetDigital(JoystickDescorerGroup, JOY_UP))
        {
            m_count = 0;
            m_direction = Direction::Up;
            motorSet(descorerPort, 85);
        }
    }
    else if (joystickGetDigital(JoystickIntakeGroup, JOY_DOWN))
    {
        m_direction = Direction::Down;
        m_count = 0;
        //if (m_count > 75)
        //motorSet(descorerPort, -35);
        //else
        motorSet(descorerPort, -85);
    }
    else
    {
        m_count++;
        if (m_count > 20)
        {
            motorSet(descorerPort, 0);
            m_direction = Direction::None;
        }
        else if (m_direction == Direction::Up)
        {
            motorSet(descorerPort, -15);
        }
        else
        {
            motorSet(descorerPort, 15);
        }
    }
}