#pragma once
#include <cstdint>
#include "forwards.h"

/*****************************
 *
 * Forward declarations
 * 
 ****************************/

extern "C" {
   void autonomous();
   void initialize();
   void disabled(void);
   void competition_initialize(void);
   void opcontrol();
}

// C++ definitions
#ifndef UINT_MAX
#define UINT_MAX 0xffffffff
#endif

extern "C" {
   int printf(const char * format, ... );
}

/*****************************
 *
 * Autonomous & cycle update
 * 
 ****************************/
bool isAuto();
bool SmartsOn();
class Main &SetupMain();
unsigned int GetTime();
void MainRunUpdateCycle();


/*****************************
 *
 * Logging
 * 
 ****************************/
void LcdPrintMessage(const char *message);

enum class Log
{
   Motion = 0,
   Drive,
   Gyro,
   States,
   Position,
   Automation,
   
   Verbose,
   // Starting with Info, all categories below are enabled by default
   Info,
   Warning,
   Error,

   Max,
};

void EnableConsoleLogs(Log logCategory);
const char* LogCategoryName(Log logCategory);

void ReportStatusCore(Log logCategory, const char* format, ...);
#define ReportStatusCoreInline(logCategory, format, ...) printf(format, ##__VA_ARGS__)
#define ReportStatus(logCategory, format, ...) ReportStatusCore(logCategory, "%4ld: %s" format, pros::c::millis(), LogCategoryName(logCategory), ##__VA_ARGS__)

#define Assert(f) AssertSz(f, #f)
#define AssertSz(f, format, ...) do { \
   if (!(f)) \
      ReportStatus(Log::Error, "*** ASSERT: %s:%d: " format " ***\n\n", __FILE__, __LINE__, ##__VA_ARGS__); \
   } while(false)

#define StaticAssert(a) static_assert(a, #a)


/*****************************
 *
 * Macros
 * 
 ****************************/
#define __noop(...)
#define CountOf(a) (sizeof(a) / sizeof(a[0]))
#define RgC(a) a, CountOf(a)
#define UNUSED_VARIABLE(a) (void)a;


/*****************************
 *
 * min / max / abs
 * 
 ****************************/
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

template <typename T>
inline T abs(T value) {
   return value > 0 ? value : -value;
}

/*****************************
 *
 * PidPrecision & PidImpl
 * 
 ****************************/
enum PidPrecision
{
   Precise, // have to stop within precision bounds
   HigherOk, // any number above target is Ok
   LowerOk, // any number below target is Ok
};

class PidImpl {
public:
   PidImpl(int precision)
      : m_precision(precision)
   {
   }

   int GetPower(int reading, int target, float kp, int ki, PidPrecision precision = PidPrecision::Precise);
   void Reset() {
      m_target = 0;
   }

private:
   int m_target = 0;
   int m_precision;
   int m_errorAccumulated = 0;
};

/*****************************
 *
 * Defines
 * 
 ****************************/

#define joystickMax 127

/*******************************************************************************
*
* MOTOR SPEEDS
*
*******************************************************************************/
#define shooterMotorSpeed 127
#define intakeMotorSpeedUp 127
#define intakeMotorSpeedDown 127
#define MotorMaxSpeed 127


/*******************************************************************************
*
* MOTOR PORTS
*
* BAD PORTS: 17
*
* 
*******************************************************************************/
#define leftFrontDrivePort 5
#define leftMiddleDrivePort 17
#define leftBackDrivePort 18
#define rightFrontDrivePort 13
#define rightMiddleDrivePort 15
#define rightBackDrivePort 11

#define intakeLeftPort 99
#define intakeRightPort 99
#define cubetrayPort 99
#define liftMotorPort 99

#define VisionPort 99

#define inertialPort 99

/*******************************************************************************
*
* ANALOG SENSORS
*
*******************************************************************************/

#define leftEncoderPortTop 'A'
#define leftEncoderPortBottom 'B' 
#define rightEncoderPortTop 'G'
#define rightEncoderPortBottom 'H'


#define liftPotPort 99
#define cubetrayPotPort 5

#define gyroPortImu 99
#define gyroPort 99
#define gyroPort2 99

#define intakeLineTrackerPort 6

/*******************************************************************************
*
* JOYSTICK GROUPING
*
*******************************************************************************/
#define JoystickCubetrayGroup 5 // Right bottom
#define JoystickIntakeGroup 6   // Left bottom


/*******************************************************************************
*
* OTHER CONSTANTS
*
*******************************************************************************/
#define DistanveBetweenLineSensors 390 // in clicks, rouhly 14"

/*******************************************************************************
*
* OLD VALUES
*
*******************************************************************************/
#define lineTrackerLeftPort 99
#define lineTrackerRightPort 99

