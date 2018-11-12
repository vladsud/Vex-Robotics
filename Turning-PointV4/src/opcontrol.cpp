/** @file opcontrol.c
 * @brief File for operator control code
 *
 * This file should contain the user operatorControl() function and any functions related to it.
 *
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 *
 * PROS contains FreeRTOS (http://www.freertos.org) whose source code may be
 * obtained from http://sourceforge.net/projects/freertos/files/ or on request.
 */
#include "main.h"

#include "spinner.h"
#include "drive.h"
#include "lift.h"
#include "shooter.h"
#include "intake.h"
#include "angle.h"

//Operator Control
void operatorControl()
{
	Drive drive;
	Spinner spinner;
	Lift lift;
	Shooter shooter;
	Intake intake;
	Angle angle;

	int count = 0;
	while (true)
	{
		count++;
		if (count % 50 == 0)
		{
			printf("LD: %d     LL: %d     A: %d     S:%d    RL: %d     RD: %d\n", encoderGet(leftDriveEncoder), analogRead(leftLiftPotPort), analogRead(anglePotPort), analogRead(spinnerPotPort), analogRead(rightLiftPotPort), encoderGet(rightDriveEncoder));
		}

		drive.Update();
		lift.Update();
		spinner.Update();
		shooter.Update();
		intake.Update();
		angle.Update();

		delay(10);
	}
}

//Created by Jason Zhang, September 22, 2018
