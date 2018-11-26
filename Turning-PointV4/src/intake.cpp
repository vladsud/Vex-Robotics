#include "intake.h"

void Intake::SetIntakeMotor(int speed)
{
    motorSet(intakePort, speed);
}

void Intake::Update()
{
    Direction direction;

    if (joystickGetDigital(1, 6, JOY_UP))
    {
         if (joystickGetDigital(1, 6, JOY_DOWN))
         {
            m_doublePressed = true;
            SetIntakeMotor(0);
            return;
         }
        direction = Direction::Up;
    }
    else if (joystickGetDigital(1, 6, JOY_DOWN))
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
    if (joystickGetDigital(1, 5, JOY_UP))
        motorSet(descorerPort, 85);
    else if (joystickGetDigital(1, 5, JOY_DOWN))
    {
        m_count++;
        if (m_count > 75)
            motorSet(descorerPort, -35);
        else
            motorSet(descorerPort, -70);
    }
    else
    {
        motorSet(descorerPort, 0);
        m_count = 0;
    }
}