
#include "main.h"
#include "cycle.h"
#include "logger.h"
#include <cstdio>

#include "pros/adi.h"
#include "pros/motors.h"
#include "pros/rtos.h"

using namespace pros;
using namespace pros::c;

static Main *g_main = nullptr;

int startTime;
bool haveRumbled;

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

Logger &GetLogger() { return GetMain().logger; }
GyroWrapper &GetGyro() { return GetMain().gyro; }
PositionTracker &GetTracker() { return GetMain().tracker; }
int GetGyroReading() { return GetTracker().GetGyro(); }

void UpdateIntakeFromShooter(IntakeShoterEvent event)
{
	GetMain().intake.UpdateIntakeFromShooter(event);
}

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
	// We go through line very quickly, so we do not have enough precision if we check it
	// every 10 ms.
	drive.UpdateDistanes();
	lineTrackerLeft.Update();
	lineTrackerRight.Update();
}

void Main::UpdateAllSystems()
{
	UpdateFastSystems();

	lcd.Update();
	vision.Update();
	descorer.Update();
	intake.Update();
	shooter.Update();
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

	if (PrintDiagnostics(Diagnostics::General) && (m_Ticks % 500) == 8)
	{
		ReportStatus("Encoders: %d : %d     Angle: %d,   Shooter preloader: %d   Gyro: %d  Light: %d\n",
					 motor_get_position(leftBackDrivePort),
					 motor_get_position(rightBackDrivePort),
					 adi_analog_read(anglePotPort),
					 adi_analog_read(shooterPreloadPoterntiometer),
					 GetGyroReading() * 10 / GyroWrapper::Multiplier,
					 adi_analog_read(ballPresenceSensorUp));
	}

	return res;
}

void Main::ResetState()
{
	// reset wake-up logic
	m_LastWakeUp = millis() - 1;

	drive.ResetState();
	intake.ResetState();
	gyro.ResetState();
	shooter.ResetState();
}

//Operator Control
void opcontrol()
{
	startTime = millis();
	haveRumbled = false;

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

		if (!haveRumbled)
		{
			if ((millis() - startTime) > 90000)
			{
				haveRumbled = true;
				controller_rumble(E_CONTROLLER_MASTER, "-");
				controller_set_text(E_CONTROLLER_MASTER, 1, 1, "Rumble");
			}
		}
	}
}
