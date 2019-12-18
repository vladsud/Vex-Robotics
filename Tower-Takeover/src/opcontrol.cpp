
#include "main.h"
#include "cycle.h"
#include "forwards.h"

#include "pros/misc.h"

using namespace pros;
using namespace pros::c;

unsigned int _millis() {
    return pros::c::millis();
}

static Main *g_main = nullptr;

void initialize()
{
  SetupMain();
  printf("Battery %.2f \n", battery_get_capacity());
  ReportStatus("Initialized\n");
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

GyroWrapper &GetGyro() { return GetMain().gyro; }
Drive& GetDrive() { return GetMain().drive; }
PositionTracker &GetTracker() { return GetMain().tracker; }
int GetGyroReading() { return GetTracker().GetGyrorReading(); }
StateMachine& GetStateMachine() { return GetMain().sm; }
CubeTray& GetCubeTray() { return GetMain().cubetray; }
Lift& GetLift() { return GetMain().lift; }
LCD& GetLcd() { return GetMain().lcd; }
Intake& GetIntake() { return GetMain().intake; }

#if LineTracker
LineTracker& GetLineTrackerLeft() { return GetMain().lineTrackerLeft; }
LineTracker& GetLineTrackerRight() { return GetMain().lineTrackerRight; }
#endif // LineTracker

void MainRunUpdateCycle() { GetMain().Update(); }

unsigned int GetTime() { return GetMain().GetTime(); }

void AssertCore(bool condition, const char *message, const char *file, int line)
{
	if (!condition)
	{
		ReportStatus("\n*** ASSERT: %s:%d: %s ***\n\n", file, line, message);
		if (g_main != nullptr)
			g_main->lcd.PrintMessage(message);
	}
}

void Main::Update()
{
	if (m_Ticks % 500 == 0)
	{
		sm.PrintController();
		//printf("Controller Printing... \n");
	}


	if (m_LastWakeUp == 0)
		m_LastWakeUp = millis();

	do
	{
		// task_delay_until( )is better then delay() as it allows us to "catch up" if we spend too much time in one cycle,
		// i.e. have consistent frequency of updates.
		// That said, it assumes that we are missing the tick infrequently.
		// If it's not the case, we can hog CPU and not allow other tasks to have their share.
		task_delay_until(&m_LastWakeUp, trackerPullTime);

		m_Ticks += trackerPullTime;
		m_TicksToMainUpdate -= trackerPullTime;
	} while (!UpdateWithoutWaiting());
}

void Main::UpdateFastSystems()
{
	gyro.Integrate();
	tracker.Update();

#if LineTracker
	// Line trackers depend on it
	drive.UpdateDistanes();

	// We go through line very quickly, so we do not have enough precision if we check it
	// every 10 ms.
	lineTrackerLeft.Update();
	lineTrackerRight.Update();
#endif // LineTracker
}

void Main::UpdateAllSystems()
{
	UpdateFastSystems();

	lcd.Update();

	sm.Update();

	intake.Update();
	cubetray.Update();
	lift.Update();
	//vision.Update(); // before drive, for it to update where to drive
	drive.Update();
}

bool Main::UpdateWithoutWaiting()
{
	bool res = false;

	// If this assert fires, than numbers do not represent actual timing.
	// It's likley not a big deal, but something somwhere might not work because of it.
	StaticAssert((trackerPullTime % trackerPullTime) == 0);

	if (m_TicksToMainUpdate / trackerPullTime == 0)
	{
		m_TicksToMainUpdate = allSystemsPullTime;

		UpdateAllSystems();

		// Good place for external code to consume some cycles
		res = true;
	}
	else
	{
		UpdateFastSystems();
	}

	return res;
}

void Main::ResetState()
{
	// reset wake-up logic
	m_LastWakeUp = millis() - 1;

	drive.ResetState();
	gyro.ResetState();
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
	main.UpdateAllSystems();

	while (true)
	{
		main.Update();
	}
}
