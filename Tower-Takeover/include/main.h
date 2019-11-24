#pragma once

// C++ definitions
extern "C" int printf(const char * format, ... );
extern "C" int sprintf(char * str, const char * format, ... );
extern "C" double sin(double);
float cos (float x);
extern "C" double cos(double);
float sin (float x);
float atan2 (float y, float x);
double atan2(double, double);

bool isAuto();
bool SmartsOn();
class Main &SetupMain();
int AdjustAngle(int angle);
unsigned int GetTime();
void MainRunUpdateCycle();

void opcontrol();
void autonomous();
void initialize();

#define __noop(...)

#define Assert(f) AssertCore(f, #f, __FILE__, __LINE__)
#define AssertSz(f, sz) AssertCore(f, sz, __FILE__, __LINE__)
void AssertCore(bool condition, const char *message, const char *file, int line);

unsigned int _millis();
#define ReportStatus(format, ...) printf("%ld: " format, _millis(), ##__VA_ARGS__)

#define CountOf(a) (sizeof(a) / sizeof(a[0]))
#define UNUSED_VARIABLE(a) (void)a;

#define joystickMax 127

#define StaticAssert(a) static_assert(a, #a)

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

enum PidPrecision
{
   Precise, // have to stop within precision bounds
   HigerOk, // any number above target is Ok
   LowerOk, // any number below target is Ok
};

class PidImpl {
public:
   PidImpl(int precision)
      : m_precision(precision)
   {
   }

   int GetPower(int reading, int target, float kp, int ki, PidPrecision precision = PidPrecision::Precise);

private:
   int m_target = 0;
   int m_precision;
   int m_errorAccumulated = 0;
};

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
#define leftBackDrivePort 8
#define leftFrontDrivePort 5
#define rightFrontDrivePort 7
#define rightBackDrivePort 3

#define intakeLeftPort 20
#define intakeRightPort 16
#define cubetrayPort 11
#define liftMotorPort 9

#define VisionPort 99


/*******************************************************************************
*
* ANALOG SENSORS
*
*******************************************************************************/
#define liftPotPort 3
#define cubetrayPotPort 2

#define leftIntakeLineTrackerPort 6
#define rightIntakeLineTrackerPort 7

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
#define gyroPort 4
#define gyroPort2 5

#define lineTrackerLeftPort 99
#define lineTrackerRightPort 99