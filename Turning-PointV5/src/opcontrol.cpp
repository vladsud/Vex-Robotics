
#include "main.h"
#include "cycle.h"
#include "logger.h"

using namespace pros;
using namespace pros::c;

static Main *g_main = nullptr;

Main &SetupMain()
{
	if (g_main == nullptr)
		g_main = new Main();
	return *g_main;
}

Main &GetMain()
{
	return *g_main;
}

Logger &GetLogger() { return GetMain().logger; }
GyroWrapper &GetGyro() { return GetMain().gyro; }
PositionTracker &GetTracker() { return GetMain().tracker; }
int GetGyroReading() { return GetTracker().GetGyro(); }

void UpdateIntakeFromShooter(IntakeShoterEvent event, bool forceDown)
{
	GetMain().intake.UpdateIntakeFromShooter(event, forceDown);
}

void AssertCore(bool condition, const char *message, const char *file, int line)
{
	if (!condition)
	{
		ReportStatus("\n*** ASSERT: %s:%d: %s ***\n\n", file, line, message);
		GetMain().lcd.PrintMessage(message);
	}
}

void Main::Update()
{
	if (m_LastWakeUp == 0)
		m_LastWakeUp = millis();

	do
	{
		// taskDelayUntil( )is better then delay() as it allows us to "catch up" if we spend too much time in one cycle,
		// i.e. have consistent frequency of updates.
		// That said, it assumes that we are missing the tick infrequently.
		// If it's not the case, we can hog CPU and not allow other tasks to have their share.
		taskDelayUntil(&m_LastWakeUp, trackerPullTime);

		m_Ticks += trackerPullTime;
		m_TicksToMainUpdate -= trackerPullTime;
	} while (!UpdateWithoutWaiting());
}

void Main::UpdateAllSystems()
{
	lcd.Update();
	descorer.Update();
	intake.Update();
	shooter.Update();
	drive.Update();

	gyro.Integrate();
	tracker.Update();
}

bool Main::UpdateWithoutWaiting()
{
	bool res = false;

	// has to be the first one!
	gyro.Integrate();
	tracker.Update();

	// We go through line very quickly, so we do not have enough precision if we check it
	// every 10 ms.
	drive.UpdateDistanes();
	lineTrackerLeft.Update();
	lineTrackerRight.Update();

	// Trying to check intake more often, as some key pressed are not registered sometimes
	intake.Update();

	// If this assert fires, than numbers do not represent actual timing.
	// It's likley not a big deal, but something somwhere might not work because of it.
	StaticAssert((trackerPullTime % trackerPullTime) == 0);

	switch (m_TicksToMainUpdate / trackerPullTime)
	{
	case 0:
		StaticAssert(allSystemsPullTime / trackerPullTime >= 5);
		m_TicksToMainUpdate = allSystemsPullTime;
		// cheap sytems are grouped together
		lcd.Update();
		// descorer.Update();
		break;
	case 1:
		shooter.Update();
		break;
	case 2:
		drive.Update();
		break;
	case 3:
		// Good place for external code to consume some cycles
		res = true;
		break;
	}

	if (PrintDiagnostics(Diagnostics::General) && (m_Ticks % 500) == 8)
	{
		ReportStatus("Encoders: %d : %d     Angle: %d,   Shooter angle 2nd: %d    Shooter preloader: %d   Gyro: %d  Light: %d\n",
			   motor_get_position(leftDrivePort2),
			   motor_get_position(rightDrivePort2),
			   adi_analog_read(anglePotPort),
			   adi_analog_read(ShooterSecondaryPotentiometer),
			   adi_analog_read(shooterPreloadPoterntiometer),
			   GetGyroReading() * 10 / GyroWrapper::Multiplier,
			   adi_analog_read(lightSensor));
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
}

//Operator Control
void operatorControl()
{
	if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L1) && joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_L2))
		StartSkillsinManual();

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
