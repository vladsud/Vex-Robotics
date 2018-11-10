#include "main.h"

bool intake = false;
bool oldIntakeState = false;

bool GetIntake()
{
	return joystickGetDigital(1, 8, JOY_LEFT);
}
void SetIntakeMotor(float speed)
{
	motorSet(INTAKEPORT, speed);
}

void UpdateIntake()
{
	bool newIntakeState = GetIntake();

	if (oldIntakeState == false && newIntakeState == true)
	{
		intake = !intake;
		if (intake)
			SetIntakeMotor(INTAKEMOTORSPEED);
		else
			SetIntakeMotor(0);
	}
	oldIntakeState = newIntakeState;
}