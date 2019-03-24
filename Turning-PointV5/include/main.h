#pragma once
#include <stdint.h>

// Reduce number of includes to speed up compilation!
// Do not include API.h - it includes a ton of headers we do not need!
#include "pros/misc.h"

// *** WARNING ***
// Always define it for competation!!!!
// #define OFFICIAL_RUN

// Helper function to use both oysticsk
bool joystickGetDigital(pros::controller_id_e_t id, pros::controller_digital_e_t button);
bool isAuto();
bool SmartsOn();
class Main &SetupMain();
int AdjustAngle(int angle);
void StartSkillsinManual();

void opcontrol();
void autonomous();
void initialize();

enum class IntakeShoterEvent
{
   LostBall,
   Shooting,
};
void UpdateIntakeFromShooter(IntakeShoterEvent event, bool forceDown);

#define __noop(...)

#ifdef OFFICIAL_RUN
#define Assert(f) __noop()
#define AssertSz(f, sz) __noop()
#else
#define Assert(f) AssertCore(f, #f, __FILE__, __LINE__)
#define AssertSz(f, sz) AssertCore(f, sz, __FILE__, __LINE__)
void AssertCore(bool condition, const char *message, const char *file, int line);
#endif

#define CountOf(a) (sizeof(a) / sizeof(a[0]))
#define UNUSED_VARIABLE(a) (void)a;

#ifdef OFFICIAL_RUN
#define ReportStatus __noop
#else
#define ReportStatus printf
#endif // OFFICIAL_RUN

#define joystickMax 127

#define StaticAssert(a) static_assert(a, #a)

inline int Sign(int value)
{
   if (value < 0)
      return -1;
   if (value > 0)
      return 1;
   return 0;
}

template <typename T>
constexpr T max(T a, T b)
{
   return a > b ? a : b;
}

template <typename T>
constexpr T min(T a, T b)
{
   return a > b ? b : a;
}


/*******************************************************************************
*
* MOTOR SPEEDS
*
*******************************************************************************/
#define shooterMotorSpeed 127
#define intakeMotorSpeedUp 127
#define intakeMotorSpeedDown 127
#define driveMotorMaxSpeed 127


/*******************************************************************************
*
* MOTOR PORTS
*    Externder: 2-5

Extender::
A: Descorrer
B: Back left drive
C: Shooter
D: left y drive
*
*******************************************************************************/
#define shooterPort 17
#define leftBackDrivePort 18
#define leftFrontDrivePort 19
#define intakePort 3
#define rightFrontDrivePort 20
#define rightBackDrivePort 16
#define angleMotorPort 10

#define descorerPort 99


/*******************************************************************************
*
* ANALOG SENSORS
*
*******************************************************************************/
#define gyroPort 1
#define anglePotPort 8

#define lineTrackerLeftPort 99
#define lineTrackerRightPort 99
#define ballPresenceSensor 99
#define shooterPreloadPoterntiometer 99
#define ShooterSecondaryPotentiometer 99


/*******************************************************************************
*
* JOYSTICK GROUPING
*
*******************************************************************************/
#define JoystickDescorerGroup 5 // Right bottom
#define JoystickIntakeGroup 6   // Left bottom


/*******************************************************************************
*
* OTHER CONSTANTS
*
*******************************************************************************/
#define lightSensorBallIn 2600
#define lightSensorBallOut 2900

#define ShooterPreloadEnd 950
#define ShooterPreloadStart 1150

#define DistanveBetweenLineSensors 390 // in clicks, rouhly 14"
