#pragma once

/*****************************
 *
 * Forward declarations
 * 
 ****************************/

typedef unsigned int  size_t;

extern "C" {
   void autonomous();
   void initialize();
   void disabled(void);
   void competition_initialize(void);
   void opcontrol();
}

// C++ definitions
#define UINT_MAX 0x7fffffff
extern "C" {
   int printf(const char * format, ... );
   double sin(double);
   double cos(double);
   int snprintf(char * str, size_t n, const char * format, ... );
}

float cos (float x);
float sin (float x);
float atan2 (float y, float x);
double atan2(double, double);

unsigned int _millis();

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
enum class Log
{
   Motion = 0,
   Drive,
   Gyro,
   States,
   
   Verbose,
   // Starting with Info, all categories below are enabled by default
   Info,
   Warning,
   Error,

   Max,
};

void EnableLogs(Log logCategory);
const char* LogCategoryName(Log logCategory);

void ReportStatusCore(Log logCategory, const char* format, ...);
#define ReportStatusCoreInline(logCategory, format, ...) printf(format, ##__VA_ARGS__)
#define ReportStatus(logCategory, format, ...) ReportStatusCore(logCategory, "%4ld: %s" format, _millis(), LogCategoryName(logCategory), ##__VA_ARGS__)

#define Assert(f) AssertSz(f, #f)
#define AssertSz(f, format, ...) do { \
   if (!(f)) \
      ReportStatus(Log::Error, "\n*** ASSERT: %s:%d: " format " ***\n\n", __FILE__, __LINE__, ##__VA_ARGS__); \
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
#define leftBackDrivePort 15
#define leftFrontDrivePort 17
#define rightFrontDrivePort 11
#define rightBackDrivePort 16

#define intakeLeftPort 1
#define intakeRightPort 21
#define cubetrayPort 2
#define liftMotorPort 20

#define VisionPort 99

#define inertialPort 99

/*******************************************************************************
*
* ANALOG SENSORS
*
*******************************************************************************/
#define liftPotPort 99
#define cubetrayPotPort 4

#define gyroPortImu 18
#define gyroPort 2
#define gyroPort2 3

#define intakeLineTrackerPort 8

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