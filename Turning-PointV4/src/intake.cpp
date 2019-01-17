#include "intake.h"
#include "logger.h"

void Intake::SetIntakeDirection(Direction direction)
{
    m_direction = direction;

    if (direction == Direction::None)
    {
        m_power = 0;
	SetIntakeMotor(0);
	return;
    }

    int power = (m_direction == Direction::Up) ? -intakeMotorSpeed : intakeMotorSpeed;
    
    if (power * m_power >= 0)
        m_power = power;
    else
        // slow down transition when change in direciton happens
        m_power += Sign(power);

    motorSet(intakePort, m_power);
}

void Intake::SetIntakeMotor(int speed)
{
    motorSet(intakePort, speed);
}

void Intake::Update()
{
    Direction direction = m_direction;

    if (joystickGetDigital(JoystickDescorerGroup, JOY_UP))
    {
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
        direction = Direction::Down;
    }
    else
    {
        m_doublePressed = false;
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
	    if (m_direction == Direction::None && !m_doublePressed)
        	SetIntakeDirection(Direction::Up);
	    break;
	case IntakeShoterEvent::Shooting:
        	SetIntakeDirection(Direction::None);
		break;
    }	
}

void Descorer::Update()
{
    if (joystickGetDigital(JoystickIntakeGroup, JOY_UP))
    {
        if (m_direction == Direction::Up || joystickGetDigital(JoystickDescorerGroup, JOY_UP))
        {
            m_count++;
            m_direction = Direction::Up;
            motorSet(descorerPort, 85);
        }
    }
    else if (joystickGetDigital(JoystickIntakeGroup, JOY_DOWN))
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
        motorSet(descorerPort, -10);
    }
}