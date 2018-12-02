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
#include "cycle.h"

// Scans both joysticks, allowing secondary operator to help with controlling non-driving functions.
bool joystickGetDigital(unsigned char buttonGroup, unsigned char button)
{
	return ::joystickGetDigital(1, buttonGroup, button) || ::joystickGetDigital(2, buttonGroup, button);
}


void Main::Update()
{
	m_count++;
	// printf("%d ", gyroGet(g_gyro));

	if (m_count % 50 == 0)
	{
		printf("Encoders: %d : %d     Angle: %d     Gyro: %p, %d  %d\n",
		encoderGet(g_leftDriveEncoder),
		encoderGet(g_rightDriveEncoder),
		analogRead(anglePotPort), 
		g_gyro, gyroGet(g_gyro),
		analogRead(lightSensor));
	}

	// save power
	// gyroShutdown(g_gyro);

	g_lcd.Update();
	drive.Update();
	intake.Update();
	shooter.Update();
	descorer.Update();
}

//Operator Control
void operatorControl()
{
	if (isAuto())
	{
		delay(2000);
		autonomous();
	}

	Main main;

	main.shooter.SetDistance(48);
	main.shooter.SetFlag(Flag::Middle);
	
	while (true)
	{
		main.Update();
		delay(10);
	}
}

//Created by Jason Zhang, September 22, 2018
