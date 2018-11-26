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

#include "drive.h"
#include "shooter.h"
#include "intake.h"
#include "angle.h"

//Operator Control
void operatorControl()
{
	Drive drive;
	Shooter shooter;
	Intake intake;
	Descorer descorer;
	Angle angle;

	int count = 0;
	while (true)
	{
		count++;
		// printf("%d ", gyroGet(g_gyro));

		if (count % 50 == 0)
		{
			printf("Encoders: %d : %d     Angle: %d     Gyro: %p, %d\n",
			encoderGet(g_leftDriveEncoder),
			encoderGet(g_rightDriveEncoder),
			analogRead(anglePotPort), 
			g_gyro, gyroGet(g_gyro));
		}

		// save power
		// gyroShutdown(g_gyro);

		drive.Update();
		shooter.Update();
		intake.Update();
		angle.Update();
		descorer.Update();

		delay(10);
	}
}

//Created by Jason Zhang, September 22, 2018
