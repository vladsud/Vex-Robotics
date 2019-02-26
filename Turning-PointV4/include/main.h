#pragma once
#include "API.h"

// *** WARNING ***
// Always define it for competation!!!!
// #define OFFICIAL_RUN

// Helper function to use both oysticsk
bool joystickGetDigital(unsigned char buttonGroup, unsigned char button);
bool isAuto();
bool SmartsOn();
class Main &SetupMain();
int AdjustAngle(int angle);

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


extern Encoder g_leftDriveEncoder;
extern Encoder g_rightDriveEncoder;
extern Encoder g_sideEncoder;


/*******************************************************************************
* 
* MOTOR SPEEDS
*
*******************************************************************************/
#define shooterMotorSpeed 100 // it otherwise burns controller / port
#define intakeMotorSpeedUp 100 // same, being protective
#define intakeMotorSpeedDown 100
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
#define shooterPort 2 // "C" on extender
#define leftDrivePort2 3 // back
#define leftDrivePortY 4 // middle
#define intakePort 5
#define descorerPort 6
#define rightDrivePortY 7
#define anglePort 8
#define rightDrivePort2 9


/*******************************************************************************
* 
* DIGITAL SENSORS
*
*******************************************************************************/
#define leftDriveEncoderBotPort 1
#define leftDriveEncoderTopPort 2
#define rightDriveEncoderBotPort 3
#define rightDriveEncoderTopPort 4

#define sideEncoderBotPort 7 // not implemented yet
#define sideEncoderTopPort 8 // not implemented yet


/*******************************************************************************
* 
* ANALOG SENSORS
*
*******************************************************************************/
#define lineTrackerLeftPort 1
#define lineTrackerRightPort 2
#define ExpanderBatteryStatus 3
#define gyroPort 4
#define lightSensor 5
#define shooterPreloadPoterntiometer 6
#define ShooterSecondaryPotentiometer 7
#define anglePotPort 8


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