
#include "main.h"
#include "cycle.h"
#include "forwards.h"

#include "pros/misc.h"
#include "pros/imu.h"

using namespace pros;
using namespace pros::c;

static Main *g_main = nullptr;

void initialize()
{
  SetupMain();
  ReportStatus(Log::Info, "Battery %.2f \n", battery_get_capacity());
  ReportStatus(Log::Info, "Initialized\n");
}

Main &SetupMain()
{
	if (g_main == nullptr)
		g_main = new Main();
	return *g_main;
}

Main &GetMain()
{
	Assert(g_main != nullptr);
	return *g_main;
}

Drive& GetDrive() { return GetMain().drive; }
PositionTracker &GetTracker() { return GetMain().tracker; }
LCD& GetLcd() { return GetMain().lcd; }
Intake& GetIntake() { return GetMain().intake; }

#if LineTracker
LineTracker& GetLineTrackerLeft() { return GetMain().lineTrackerLeft; }
LineTracker& GetLineTrackerRight() { return GetMain().lineTrackerRight; }
#endif // LineTracker

void LcdPrintMessage(const char *message)
{
	if (g_main != nullptr)
		g_main->lcd.PrintMessage(message);
}

void MainRunUpdateCycle() { GetMain().Update(); }

unsigned int GetTime() { return GetMain().GetTime(); }


unsigned int Main::GetTime()
{
	// Can use m_LastWakeUp + 1 instead
	return millis();
}

void Main::Update()
{
	// Run through all key motor subsystems - this applies commands that were just issues (in autonomous mode)
	UpdateSlowSystems();

	do
	{
		// Check that full cycle fits into 1ms
		// Assert(m_LastWakeUp + 1 == millis());

		// task_delay_until() is better then delay() as it allows us to "catch up" if we spend too much time in one cycle,
		// i.e. have consistent frequency of updates.
		// That said, it assumes that we are missing the tick infrequently.
		// If it's not the case, we can hog CPU and not allow other tasks to have their share.
		task_delay_until(&m_LastWakeUp, trackerPullTime);
		// Assert(m_LastWakeUp + 1 == millis());

		m_Ticks += trackerPullTime;

		// Note: Code below should match code in ResetState() in terms of what gets updated

		UpdateFastSystems();

		StaticAssert((allSystemsPullTime % trackerPullTime) == 0);
	} while ((m_Ticks % allSystemsPullTime) != 0);

	// Good place for external code to consume some cycles:
	// We have just updated all the odometry (gyro, drive, position)
	// In autonomous mode it's perfect time to run logic and issue commands to various subsystems.
	// Once we return to Update(), these commands will take immidiate effect through UpdateSlowSystems() call above
}

void Main::UpdateFastSystems()
{
	tracker.Update();

#if LineTracker
	// We go through line very quickly, so we do not have enough precision if we check it
	// every 10 ms.
	lineTrackerLeft.Update();
	lineTrackerRight.Update();
#endif // LineTracker
}

void Main::UpdateSlowSystems()
{
	lcd.Update();

	intake.Update();
	//vision.Update(); // before drive, for it to update where to drive
	drive.Update();
}

void Main::ResetState()
{
	// reset wake-up logic
	m_LastWakeUp = millis();

	// Code relies we have some granulariy
	m_Ticks = (millis() / trackerPullTime) * trackerPullTime;

	tracker.ResetState();
	drive.ResetState();

	// Last part of Update() cycle.
	// UpdateSlowSystems() will be called right away in both automonomous & op control modes
    UpdateFastSystems();
}

//Operator Control
void opcontrol()
{

	// This is required for testing purposes, but also for auto-Skills run in manual modde
	if (isAuto())
	{
		Assert(!competition_is_autonomous());
		autonomous();
	}

	Main &main = SetupMain();
	main.ResetState();

	while (true)
	{
		main.Update();
	}
}
