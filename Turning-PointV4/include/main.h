#pragma once
#include "API.h"

// *** WARNING ***
// Always define it for competation!!!! 
//#define OFFICIAL_RUN



// Helper function to use both oysticsk
bool joystickGetDigital(unsigned char buttonGroup, unsigned char button);
void memmove(char* dest, char* src, size_t size);
bool isAuto();
bool SmartsOn();
class Main& SetupMain();

enum class IntakeShoterEvent
{
    LostBall,
    Shooting,
};
void UpdateIntakeFromShooter(IntakeShoterEvent event);
void SetSkillSelection(bool skills);

#define Assert(f) AssertCore(f, #f)
#define AssertSz(f, sz) AssertCore(f, sz)
void AssertCore(bool condition, const char* message);

#define CountOf(a) (sizeof(a)/sizeof(a[0]))

#ifdef OFFICIAL_RUN
#  define ReportStatus __noop
#  define __noop(...)
#else
#  define ReportStatus printf
#endif // OFFICIAL_RUN

#define joystickMax 127

inline float abs(float fl)
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

template <typename T> T min(T a, T b)
{
   return a > b ? b : a;
}


/*******************************************************************************
* 
* MOTOR SPEEDS
*
*******************************************************************************/
#define shooterMotorSpeed 100 // it botherwise burns controller / port
#define intakeMotorSpeed 90 // same, being protective
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
#define leftDrivePortY 2 // middle
#define leftDrivePort2 4 // back
#define rightDrivePortY 8
#define rightDrivePort2 7
//OTHER MOTOR PORTS
#define intakePort 9
#define descorerPort 5
#define shooterPort 3   // "C" on extender
#define anglePort 6


/*******************************************************************************
* 
* DIGITAL SENSORS
*
*******************************************************************************/
#define leftDriveEncoderBotPort 1
#define leftDriveEncoderTopPort 2
#define rightDriveEncoderBotPort 3
#define rightDriveEncoderTopPort 4
#define sideEncoderBotPort 5
#define sideEncoderTopPort 6

extern Encoder g_leftDriveEncoder;
extern Encoder g_rightDriveEncoder;
extern Encoder g_sideEncoder;


/*******************************************************************************
* 
* ANALOG SENSORS
*
*******************************************************************************/
#define ShooterSecondaryPotentiometer 4

#define lightSensor 5
#define lightSensorBallIn 2600
#define lightSensorBallOut 2900

#define shooterPreloadPoterntiometer 6
#define ShooterPreloadEnd 850
#define ShooterPreloadStart 1050

#define gyroPort 7

#define anglePotPort 8

/*******************************************************************************
* 
* JOYSTICK GROUPING
*
*******************************************************************************/
#define JoystickIntakeGroup 6 // Left bottom
#define JoystickDescorerGroup 5 // Right bottom
