#pragma once
#include "API.h"

// *** WARNING ***
// Always define it for competation!!!!
// #define OFFICIAL_RUN

// Helper function to use both oysticsk
bool joystickGetDigital(unsigned char buttonGroup, unsigned char button);
void memmove(char *dest, char *src, size_t size);
bool isAuto();
bool SmartsOn();
class Main &SetupMain();

enum class IntakeShoterEvent
{
   LostBall,
   Shooting,
};
void UpdateIntakeFromShooter(IntakeShoterEvent event, bool forceDown);
void SetSkillSelection(bool skills);

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

constexpr float abs(float fl)
{
   return (fl < 0) ? -fl : fl;
}

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
#define shooterMotorSpeed 100 // it botherwise burns controller / port
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
//DRIVE MOTOR PORTS
#define leftDrivePortY 4 // middle
#define leftDrivePort2 3 // back
#define rightDrivePortY 7
#define rightDrivePort2 9
//OTHER MOTOR PORTS
//#define intakePort1 6
#define intakePort 5

#define descorerPort 6
#define shooterPort 2 // "C" on extender
#define anglePort 8

/*******************************************************************************
* 
* DIGITAL SENSORS
*
*******************************************************************************/
// left back : 1, 2
#define rightDriveEncoderBotPort 3
#define rightDriveEncoderTopPort 4

#define leftDriveEncoderBotPort 1
#define leftDriveEncoderTopPort 2

#define sideEncoderBotPort 7 // not implemented yet
#define sideEncoderTopPort 8 // not implemented yet

extern Encoder g_leftDriveEncoder;
extern Encoder g_rightDriveEncoder;
extern Encoder g_sideEncoder;

/*******************************************************************************
* 
* ANALOG SENSORS
*
*******************************************************************************/

#define gyroPort 4
#define lightSensor 5
#define lightSensorBallIn 2600
#define lightSensorBallOut 2900
#define shooterPreloadPoterntiometer 6
#define ShooterPreloadEnd 950
#define ShooterPreloadStart 1150
#define ShooterSecondaryPotentiometer 7
#define anglePotPort 8

/*******************************************************************************
* 
* JOYSTICK GROUPING
*
*******************************************************************************/
#define JoystickIntakeGroup 6   // Left bottom
#define JoystickDescorerGroup 5 // Right bottom
