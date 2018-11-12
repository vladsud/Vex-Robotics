#include "intake.h"

bool Intake::GetIntake()
{
    return joystickGetDigital(1, 8, JOY_LEFT);
}
void Intake::SetIntakeMotor(float speed)
{
    motorSet(intakePort, speed);
}
void Intake::Update()
{
    bool newIntakeState = GetIntake();
    //intake
    if (m_oldIntakeState == false && newIntakeState == true)
    {
        m_intake = !m_intake;
        if (m_intake)
            SetIntakeMotor(intakeMotorSpeed);
        else
            SetIntakeMotor(0);
    }
    m_oldIntakeState = newIntakeState;
}