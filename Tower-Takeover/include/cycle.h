#pragma once

#include "main.h"

#include "drive.h"
#include "gyro.h"
#include "lcd.h"
#include "Logger.h"
#include "position.h"
#include "lineTracker.h"
#include "vision.h"
#include "intake.h"
#include "cubetray.h"


/*******************************************************************************
* 
* MAIN CYCLE
*
*******************************************************************************/
class Main
{
	// Gyro updates every 2ms, so it's useful to have same frequency of updates to get as accurate position info as possible.
	// Also more often we check sensors, the more precise we capture moment when encoder clicks, allowing us more precise estimation of speed.
	// Main user of faster update - position trackign code.
	// But, it can't ft into 1ms, so we use 2ms update cycle
	static const int trackerPullTime = PositionTrackingRefreshRate; // 2
	// For everything else 10ms is sufficient.
	static const int allSystemsPullTime = 10;

  public:
	Drive drive;
	GyroWrapper gyro;
	PositionTracker tracker;
	LCD lcd;
	Logger logger;
	Intake intake;
	Cubtray cubtray;
	//Vision vision;
	LineTracker lineTrackerLeft {lineTrackerLeftPort};
	LineTracker lineTrackerRight {lineTrackerRightPort};

	// Time is im milliseconds!
	// But time resolution might be coarser, in the range of 1-10 ms
	unsigned int GetTime() { return m_Ticks; }
	void Update();
	void UpdateAllSystems();
	void ResetState();

	int initialTime;

  protected:
	// returns true when it's good time for external system (like autonomous) consume some CPU cycles
	bool UpdateWithoutWaiting();
	void UpdateFastSystems();

  private:
	unsigned long m_Ticks = 0; // in ms
	unsigned long m_LastWakeUp = 0;
	int m_TicksToMainUpdate = allSystemsPullTime;

};

/*******************************************************************************
* 
* Functions
*
*******************************************************************************/
extern Main &GetMain();
