#pragma once
#include <stdint.h>
#include "pros/rtos.h"

// Reduce number of includes to speed up compilation!
// Do not include API.h - it includes a ton of headers we do not need!
#include "pros/misc.h"

// Helper function to use both oysticsk
bool joystickGetDigital(pros::controller_id_e_t id, pros::controller_digital_e_t button);
bool isAuto();
bool SmartsOn();
class Main &SetupMain();
int AdjustAngle(int angle);

unsigned int SpeedFromDistances(unsigned int distance, const unsigned int* points, const unsigned int* speeds);
int SpeedFromDistances(int distance, const unsigned int* points, const unsigned int* speeds);

void opcontrol();
void autonomous();
void initialize();

enum class IntakeShoterEvent
{
   LostBall,
   Shooting,
   TooManyBalls,
};
void UpdateIntakeFromShooter(IntakeShoterEvent event);

#define __noop(...)

#define Assert(f) AssertCore(f, #f, __FILE__, __LINE__)
#define AssertSz(f, sz) AssertCore(f, sz, __FILE__, __LINE__)
void AssertCore(bool condition, const char *message, const char *file, int line);
#define ReportStatus printf

#define CountOf(a) (sizeof(a) / sizeof(a[0]))
#define UNUSED_VARIABLE(a) (void)a;

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
*
* BAD PORTS: 17
*
* 
*******************************************************************************/
#define shooterPort 4

#define leftBackDrivePort 2
#define leftFrontDrivePort 18
#define intakePort 6
#define rightFrontDrivePort 9
#define rightBackDrivePort 5
#define angleMotorPort 16

#define descorerPort 99

#define VisionPort 7


/*******************************************************************************
*
* ANALOG SENSORS
*
*******************************************************************************/
#define gyroPort 3
#define anglePotPort 1

#define lineTrackerLeftPort 99
#define lineTrackerRightPort 99
#define ballPresenceSensorUp 2
#define ballPresenceSensorDown 4
#define shooterPreloadPoterntiometer 5


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
#define lightSensorBallOut 2700

#define ShooterPreloadEnd 100
#define ShooterPreloadStart 300

#define DistanveBetweenLineSensors 390 // in clicks, rouhly 14"
