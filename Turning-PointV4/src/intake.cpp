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
    if (oldIntakeState == false && newIntakeState == true)
    {
        intake = !intake;
        if (intake)
            SetIntakeMotor(intakeMotorSpeed);
        else
            SetIntakeMotor(0);
    }
    oldIntakeState = newIntakeState;
}