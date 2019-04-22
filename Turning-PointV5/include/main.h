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
#define ReportStatus(format, ...) printf("%ld: " format, pros::c::millis(), ##__VA_ARGS__)

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
#define leftBackDrivePort 9
#define leftFrontDrivePort 2
#define rightFrontDrivePort 4
#define rightBackDrivePort 8

#define intakePort 5
#define angleMotorPort 12
#define shooterPort 7

#define descorerPort 99

#define VisionPort 20


/*******************************************************************************
*
* ANALOG SENSORS
*
*******************************************************************************/
#define gyroPort 5
#define anglePotPort 99
#define lineTrackerLeftPort 99
#define lineTrackerRightPort 99
#define ballPresenceSensorUp 3
#define ballPresenceSensorDown 4
#define ballPresenceSensorDown2 1
#define shooterPreloadPoterntiometer 2


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
#define lightSensorBallIn 2000
#define lightSensorBallOut 2300

#define ShooterPreloadEnd 300
#define ShooterPreloadStart 500

#define DistanveBetweenLineSensors 390 // in clicks, rouhly 14"
