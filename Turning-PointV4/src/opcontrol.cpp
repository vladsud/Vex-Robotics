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

void Main::Update()
{	
	// taskDelayUntil( )is better then delay() as it allows us to "catch up" if we spend too much time in one cycle,
	// i.e. have consistent frequency of updates.
	// That said, it assumes that we are missing the tick infrequently.
	// If it's not the case, we can hog CPU and not allow other tasks to have their share.  
	// delay(trackerPullTime);
	taskDelayUntil(&m_LastWakeUp, trackerPullTime); 

	m_Ticks += trackerPullTime;
	m_TicksToMainUpdate -= trackerPullTime;

	UpdateWithoutWaiting();
}

void Main::UpdateWithoutWaiting()
{
	if (PrintDiagnostics(Diagnostics::General) && (m_Ticks  % 500) == 0)
	{
		printf("(%lu) Encoders: %d : %d     Angle: %d,   Shooter angle 2nd: %d    Shooter preloader: %d   Gyro: %d  Light: %d\n",
		m_maxCPUTime,
		encoderGet(g_leftDriveEncoder),
		encoderGet(g_rightDriveEncoder),
		analogRead(anglePotPort),
		analogRead(ShooterSecondaryPotentiometer),
		analogRead(shooterPreloadPoterntiometer),
		gyroGet(g_gyro),
		analogRead(lightSensor));
	}

	unsigned long time = micros();

	// has to be the first one!
	tracker.Update();

	// If this assert fires, than numbers do not represent actual timing.
	// It's likley not a big deal, but something somwhere might not work because of it.
	static_assert((trackerPullTime % trackerPullTime) == 0);

	if (m_TicksToMainUpdate <= 0)
	{
		m_TicksToMainUpdate = allSystemsPullTime;

		drive.Update();
		lcd.Update();
		intake.Update();
		shooter.Update();
		descorer.Update();
	}

	time = micros() - time;
	m_maxCPUTime = max(m_maxCPUTime, time);
}

//Operator Control
void operatorControl()
{
	if (isAuto())
		autonomous();

	Main& main = GetMain();
	while (true)
	{
		main.Update();
	}
}

//Created by Jason Zhang, September 22, 2018
