#include "main.h"

bool spinner = false;
bool oldSpinnerState = false;

bool GetSpinner()
{
	return joystickGetDigital(1, 6, JOY_DOWN);
}

void SetSpinnerMotor(float speed)
{
	motorSet(SPINNERPORT, speed);
}

void UpdateSpinner()
{
	bool newSpinnerState = GetSpinner();
	if (oldSpinnerState == false && newSpinnerState == true)
	{
		spinner = !spinner;
	}
	if (spinner)
	{
		if (analogRead(SPINNERPOTPORT) < 2000)
		{
			while (analogRead(SPINNERPOTPORT) < 3600)
			{
				SetSpinnerMotor(-1 * abs(analogRead(SPINNERPOTPORT) - 3600) *
								SPINNERMOTORCONSTANT);
			}
			SetSpinnerMotor(0);
		}
		else
		{
			while (analogRead(SPINNERPOTPORT) > 500)
			{
				SetSpinnerMotor(abs(analogRead(SPINNERPOTPORT) - 450) *
								SPINNERMOTORCONSTANT);
			}
			SetSpinnerMotor(0);
		}
		spinner = !spinner;
	}
}