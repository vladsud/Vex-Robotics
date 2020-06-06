#pragma once

#include "main.h"

#include "drive.h"
#include "lcd.h"
#include "position.h"
#include "lineTracker.h"
// #include "vision.h"
#include "intake.h"

/*******************************************************************************
* 
* MAIN CYCLE
*
*******************************************************************************/
class Main
{
	// More often we check sensors, the more precise we capture moment when encoder clicks, allowing us more precise estimation of speed.
	// Main user of faster update - position trackign code.
	static const int trackerPullTime = 1;
	// For everything else 10ms is sufficient.
	static const int allSystemsPullTime = 10;

  public:
	Drive drive;
	PositionTracker tracker;
	LCD lcd;
	Intake intake;
	//Vision vision;
#if LineTracker
	LineTracker lineTrackerLeft {lineTrackerLeftPort};
	LineTracker lineTrackerRight {lineTrackerRightPort};
#endif
	// Time is im milliseconds!
	// But time resolution might be coarser, in the range of 1-10 ms
	unsigned int GetTime();
	void Update();
	void ResetState();

	int initialTime;

  protected:
	void UpdateFastSystems();
	void UpdateSlowSystems();

  private:
	unsigned long m_Ticks = 0; // in ms
	unsigned long m_LastWakeUp = 0;
};

