#pragma once

#include "main.h"

#include "drive.h"
#include "intake.h"
#include "angle.h"
#include "lcd.h"
#include "gyro.h"
#include "position.h"

/*******************************************************************************
* 
* MAIN CYCLE
*
*******************************************************************************/
class Main
{
	// Gyro update every 2ms, so it's useful to have same frequency of updates to get as accurate position info as possible.
	// Also more often we check sensors, the more precise we capture moment when encoder clicks, allowing us more precise estimation of speed.
	// Main user of faster update - position trackign code.
	// But, it can't ft into 1ms, so we use 2ms update cycle
	static const int trackerPullTime = PositionTrackingRefreshRate; // 2 
	// For everything else 10ms is sufficient.
	static const int allSystemsPullTime = 10;

public:
	Drive drive;
	Intake intake;
	Descorer descorer;
	Shooter shooter;
	GyroWrapper gyro;
	PositionTracker tracker;
	LCD lcd;

	// Time is im milliseconds!
	// But time resolution might be coarser, in the range of 1-10 ms
	int GetTime() { return m_Ticks; }

   	void UpdateWithoutWaiting();
   	void Update();

private:
	unsigned long m_Ticks = 0; // in ms
	unsigned long m_LastWakeUp = millis();
	int m_TicksToMainUpdate = 0;
	unsigned long m_maxCPUTime = 0;
};

/*******************************************************************************
* 
* Functions
*
*******************************************************************************/
extern Main& GetMain();
