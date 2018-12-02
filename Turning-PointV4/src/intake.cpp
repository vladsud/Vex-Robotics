#include "intake.h"

void Intake::SetIntakeMotor(int speed)
{

    motorSet(intakePort, speed);
}

void Intake::Update()
{
    Direction direction;

    if (joystickGetDigital(6, JOY_UP))
    {
         if (joystickGetDigital(6, JOY_DOWN) || joystickGetDigital(5, JOY_UP))
         {
            m_doublePressed = true;
            SetIntakeMotor(0);
            return;
         }
         direction = Direction::Up;
    }
    else if (joystickGetDigital(6, JOY_DOWN))
    {
        direction = Direction::Down;
    }
    else
    {
        m_doublePressed = false;
        return;
    }

    if (m_doublePressed)
        return;

    m_direction = direction;
    SetIntakeMotor(m_direction == Direction::Up ? -intakeMotorSpeed : intakeMotorSpeed);
}


void Descorer::Update()
{
    if (joystickGetDigital(5, JOY_UP))
    {
        if (m_direction == Direction::Up || joystickGetDigital(6, JOY_UP))
        {
            m_count++;
            m_direction = Direction::Up;
            motorSet(descorerPort, 85);
        }
    }
    else if (joystickGetDigital(5, JOY_DOWN))
    {
        m_direction = Direction::Down;
        m_count++;
        if (m_count > 75)
            motorSet(descorerPort, -35);
        else
            motorSet(descorerPort, -70);
    }
    else
    {
        m_direction = Direction::None;
        m_count = 0;
        motorSet(descorerPort, 0);
    }
}