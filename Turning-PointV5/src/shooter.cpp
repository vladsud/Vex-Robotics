#include "shooter.h"
#include "logger.h"
#include <math.h>
#include <cstdio>
#include "pros/adi.h"
#include "pros/motors.h"
#include "pros/rtos.h"

using namespace pros;
using namespace pros::c;

// Distance based on front of the robot
constexpr float Distances[]{24, 30, 48, 55, 78, 108};

// 135 is the max!!!
constexpr unsigned int AnglesHigh[]   { 110, 120, 175, 190, 195, 200};
constexpr unsigned int AnglesMedium[] { 10,   20,  75, 90, 100, 110};

constexpr unsigned int LastDistanceCount = CountOf(Distances) - 1;

constexpr unsigned int ConvertAngleToPotentiometer(unsigned int angle)
{
    return 4000 - angle * 20;
}


StaticAssert(CountOf(Distances) == CountOf(AnglesHigh));
StaticAssert(CountOf(Distances) == CountOf(AnglesMedium));

StaticAssert(Distances[0] < Distances[1]);
StaticAssert(Distances[1] < Distances[2]);
StaticAssert(Distances[2] < Distances[3]);
StaticAssert(Distances[3] < Distances[4]);

StaticAssert(AnglesHigh[0] <= AnglesHigh[1]);
StaticAssert(AnglesHigh[1] <= AnglesHigh[2]);
StaticAssert(AnglesHigh[2] <= AnglesHigh[3]);
StaticAssert(AnglesHigh[3] <= AnglesHigh[4]);

StaticAssert(AnglesMedium[0] <= AnglesMedium[1]);
StaticAssert(AnglesMedium[1] <= AnglesMedium[2]);
StaticAssert(AnglesMedium[2] <= AnglesMedium[3]);
StaticAssert(AnglesMedium[3] <= AnglesMedium[4]);

StaticAssert(AnglesMedium[0] < AnglesHigh[0]);
StaticAssert(AnglesMedium[1] < AnglesHigh[1]);
StaticAssert(AnglesMedium[2] < AnglesHigh[2]);
StaticAssert(AnglesMedium[3] < AnglesHigh[3]);
StaticAssert(AnglesMedium[4] < AnglesHigh[4]);

constexpr unsigned int CalcAngle(Flag flag, float distanceInches)
{
    const unsigned int *angles = (flag == Flag::High) ? AnglesHigh : AnglesMedium;
    unsigned int count = CountOf(Distances);

    if (Distances[0] >= distanceInches)
        return angles[0];

    for (unsigned int i = 0; i < count; i++)
    {
        if (Distances[i] >= distanceInches)
        {
            return angles[i - 1] - (distanceInches - Distances[i - 1]) * int(angles[i - 1] - angles[i]) / (Distances[i] - Distances[i - 1]);
        }
    }

    return angles[count - 1];
}

constexpr bool AlmostSameAngle(unsigned int angle1, unsigned int angle2)
{
    return angle1 == angle2;
}


StaticAssert(CalcAngle(Flag::High, Distances[0] - 5) == AnglesHigh[0]);
StaticAssert(CalcAngle(Flag::High, Distances[0]) == AnglesHigh[0]);
StaticAssert(CalcAngle(Flag::High, Distances[2]) == AnglesHigh[2]);
StaticAssert(CalcAngle(Flag::High, Distances[LastDistanceCount]) == AnglesHigh[LastDistanceCount]);
StaticAssert(CalcAngle(Flag::High, Distances[LastDistanceCount] + 100) == AnglesHigh[LastDistanceCount]);
StaticAssert(AlmostSameAngle(CalcAngle(Flag::High, (Distances[2] + Distances[3]) / 2), (AnglesHigh[2] + AnglesHigh[3]) / 2));
StaticAssert(AlmostSameAngle(CalcAngle(Flag::High, (Distances[2] * 3 + Distances[3]) / 4), (AnglesHigh[2] * 3 + AnglesHigh[3]) / 4));

StaticAssert(CalcAngle(Flag::Middle, Distances[0] - 5) == AnglesMedium[0]);
StaticAssert(CalcAngle(Flag::Middle, Distances[0]) == AnglesMedium[0]);
StaticAssert(CalcAngle(Flag::Middle, Distances[2]) == AnglesMedium[2]);
StaticAssert(CalcAngle(Flag::Middle, Distances[LastDistanceCount]) == AnglesMedium[LastDistanceCount]);
StaticAssert(CalcAngle(Flag::Middle, Distances[LastDistanceCount] + 100) == AnglesMedium[LastDistanceCount]);
StaticAssert(AlmostSameAngle(CalcAngle(Flag::Middle, (Distances[2] + Distances[3]) / 2), (AnglesMedium[2] + AnglesMedium[3]) / 2));
StaticAssert(AlmostSameAngle(CalcAngle(Flag::Middle, (Distances[2] * 3 + Distances[3]) / 4), (AnglesMedium[2] * 3 + AnglesMedium[3]) / 4));

// Need to figure out initial position
Shooter::Shooter()
    : m_preloadSensor(shooterPreloadPoterntiometer),
      m_angleSensor(anglePotPort),
      m_ballPresenceSensorUp(ballPresenceSensorUp),
      m_ballPresenceSensorDown(ballPresenceSensorDown)
{
    motor_tare_position(angleMotorPort);
    m_distanceInches = Distances[2];
    ReportStatus("Shooter angle: %d\n", m_angleSensor.get_value());
    StartMoving();
}

unsigned int Shooter::CalcAngle()
{
    return ::CalcAngle(m_flag, m_distanceInches);
}

bool MoveToTopFlagPosition()
{
    return joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_UP);
}

bool MoveToMiddleFlagPosition()
{
    return joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_RIGHT);
}

bool Shooter::IsShooting()
{
    // Allow "shoot" button to be pressed all the time and shoot once angle is settled
    if (m_fMoving)
    {
        Assert(!m_overrideShooting); // atonomous should always wait till angle is settled.
        return false;
    }

    if (!m_overrideShooting && !joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_LEFT))
        return false;
    
    // if we do not hava a ball - cancel shooting
    // check for lack of ball - that speeds up the process of shooting when ball is getting in.
    if (BallStatus() == BallPresence::HasBall)
        return true;

    // no ball, cancel shooting.
    m_overrideShooting = false;
    return false;
}

bool Shooter::IsMovingAngle()
{
    if (m_fMoving)
        return true;
    unsigned int dist = abs(m_angleSensor.get_value() - (int)m_angleToMove);
    return dist > 10;
}

// Motors:
//   up: 100
//   down: -100
void Shooter::KeepMoving()
{
    // distance = error
    // diff = derivative
    int speed = 0;
    unsigned int current = m_angleSensor.get_value();
    int distance = current - m_angleToMove;
    int diff = distance - m_lastAngleDistance;

    unsigned int distanceAbs = abs(distance);

    m_diffAdjusted = diff; // (diff + m_diffAdjusted) / 2;

    m_count++;

    // Safety net - we want to stop after some time and let other steps in autonomous to play out.
    if ((m_fMoving && m_count >= 100) || (distanceAbs <= 20 && abs(m_diffAdjusted) <= 4))
    {
        if (m_fMoving)
        {
            if (PrintDiagnostics(Diagnostics::Angle))
                ReportStatus("STOP: (%d) Dest: %d   Reading: %d, Distance: %d/%d, Diff: %d, DiffAdj: %d\n\n\n",
                    m_count, m_angleToMove, current, current - m_angleToMove, distance, diff, m_diffAdjusted);
            StopMoving();
        }
        motor_move(angleMotorPort, 0);
        return;
    }

    // down: speed > 0
    speed = 18 * Sign(distance) + distance / 4 + m_diffAdjusted * 1.5;

    // do not allow to go backwards if too far frmom final zone
    if (speed * distance < 0 && distanceAbs > 100)
        speed = 0;

    const int angleMotorSpeed = distanceAbs > 100 ? 75 : 25;

    if (speed > angleMotorSpeed)
        speed = angleMotorSpeed;
    else if (speed < -angleMotorSpeed)
        speed = -angleMotorSpeed;

    if (PrintDiagnostics(Diagnostics::Angle))
    {
        if (m_fMoving)
            ReportStatus("ANG: (%d) P=%d Dest=%d R=%d, Dist: %d, Diff: %d\n", m_count, speed, m_angleToMove, current, current - m_angleToMove, diff);
        else if (speed != 0)
            ReportStatus("ANG ADJ: (%d) P=%d Dest=%d R=%d, Dist: %d, Diff: %d\n", m_count, speed, m_angleToMove, current, current - m_angleToMove, diff);
    }

    motor_move(angleMotorPort, -speed);

    m_lastAngleDistance = distance;
}

void Shooter::StartMoving()
{
    m_fMoving = true;
    m_angleToMove = ConvertAngleToPotentiometer(CalcAngle());
    m_diffAdjusted = 0;
    m_lastAngleDistance = m_angleSensor.get_value() - m_angleToMove;
    m_count = 0;

    ReportStatus("Angle start moving: %d -> %d\n", m_angleSensor.get_value(), m_angleToMove);
}

void Shooter::StopMoving()
{
    motor_move(angleMotorPort, 0);
    m_fMoving = false;
    m_count = 0;
}

void Shooter::SetDistance(unsigned int distance)
{
    if (m_distanceInches == distance)
        return;
    ReportStatus("Shooter::SetDistance(%d)\n", distance);
    m_distanceInches = distance;
    StartMoving();
}

void Shooter::SetFlag(Flag flag)
{
    if (m_flag == flag)
        return;
    ReportStatus("Shooter::SetFlag(%d)\n", (int)flag);
    m_flag = flag;
    StartMoving();
}

void Shooter::UpdateDistanceControls()
{
    if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_Y))
        SetDistance(Distances[0]);
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_X))
        SetDistance(Distances[1]);
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_A))
        SetDistance(Distances[2]);
    else if (joystickGetDigital(E_CONTROLLER_MASTER, E_CONTROLLER_DIGITAL_B))
        SetDistance(Distances[3]);
}

void Shooter::OverrideSetShooterMode(bool on)
{
    Assert(isAuto());
    m_overrideShooting = on;
}

BallPresence Shooter::BallStatus()
{
    return ::BallStatus(m_ballPresenceSensorUp);
}

BallPresence Shooter::Ball2Status()
{
    return ::BallStatus(m_ballPresenceSensorDown);
}

BallPresence BallStatus(pros::ADIAnalogIn& sensor)
{
    // Check if we can detect ball present.
    // Use two stops to make sure we do not move angle up and down if we are somewhere in gray area (on the boundary)
    unsigned int darkness = sensor.get_value();
    StaticAssert(lightSensorBallIn < lightSensorBallOut);
    bool ballIn = (darkness < lightSensorBallIn);
    bool ballOut = (darkness > lightSensorBallOut);

    if (ballOut){
        return BallPresence::NoBall;
    }
    if (ballIn){
        return BallPresence::HasBall;
    }
    return BallPresence::Unknown;
}

// Preloadign/shooting phases:
// 1) Preloaded, stopped:
//      m_preloadAfterShotCounter = 0
//      m_preloading = false
//      Potentiometer: between ShooterPreloadStart & ShooterPreloadEnd
// 2) Shooting:
//      Same, potentiometer can jump from 0 to 4000
// 3) Shot happened - preloading:
//      Same, but lost ball.
//      m_preloadAfterShotCounter = 150, counting down.
//      m_overrideShooting = false (reset)
// 4) Preloading:
//      Potentiometer goes from 0 to 4000
//      m_preloadAfterShotCounter = 0
//      m_preloading = true
// 5) Preloading - continuation:
//      m_preloadAfterShotCounter = 0
//      m_preloading = true
// 6) Preloading - continuation:
//      Potentiometer is between ShooterPreloadStart & ShooterPreloadEnd
// 7) Preloading - stopping:
//      Potentiometer reaches ShooterPreloadEnd
//      Motor off, potentiometer reading can jump back, but needs to stay between ShooterPreloadStart & ShooterPreloadEnd (otherwise we go to #6)
//      m_preloading = false;
void Shooter::Update()
{
    UpdateDistanceControls();

    const bool userShooting = IsShooting();
    int shooterPreloadPos = m_preloadSensor.get_value();

    m_timeSinseShooting++;
    if (userShooting)
    {
        m_timeSinseShooting = 0;
        UpdateIntakeFromShooter(IntakeShoterEvent::Shooting);
    }

    // Did we go from zero to 4000 on potentiometer?
    // That means shot was done, and we are starting "normal" preload.
    if (ShooterPreloadStart < shooterPreloadPos && shooterPreloadPos < 3500)
        m_preloadAfterShotCounter = 0;

    if (m_preloadAfterShotCounter > 0)
        m_preloadAfterShotCounter--;

    // remove jiggering by having two stops
    // Start moving if we are below first stop (ShooterPreloadStart)
    // and keep motors moving to further stop (ShooterPreloadEnd)
    // Physically system will move back until it catches next teath on stopper,
    // so we need to have some gap in between those stops not to have ssytem continuosly moving up and down.
    bool needPreload = !m_disablePreload && (m_preloadAfterShotCounter > 0 ||
                                             (m_preloading && shooterPreloadPos > ShooterPreloadEnd) ||
                                             shooterPreloadPos > ShooterPreloadStart);

    if (PrintDiagnostics(Diagnostics::Angle) && needPreload != m_preloading)
        ReportStatus("Preload state: %d\n", needPreload);

    // safety net - if something goes wrong, user needs to have a way to disable preload.
    // This is done by clicking and releasing shooter button quickly while not in the shooting zone.
    if (needPreload && userShooting && !m_userShooting && m_preloadAfterShotCounter == 0)
    {
        if (PrintDiagnostics(Diagnostics::Angle))
            ReportStatus("Disabling preload\n");
        m_disablePreload = true;
        needPreload = false;
    }

    // Check if we can detect ball present.
    BallPresence ball = BallStatus();
    BallPresence ball2 = Ball2Status();

    if (ball == BallPresence::HasBall && ball2 == BallPresence::HasBall && (!m_haveBall || !m_haveBall2))
    {
        UpdateIntakeFromShooter(IntakeShoterEvent::TooManyBalls);
    }

    if (ball != BallPresence::HasBall && m_haveBall)
    {
        UpdateIntakeFromShooter(IntakeShoterEvent::LostBall);
        if (m_flag == Flag::High)
            SetFlag(Flag::Middle);
        else if (m_flag == Flag::Middle)
            SetFlag(Flag::High);

        // Did shot just hapened?
        // We can't use potentiometer here, because we are very close to dead zone, so it can jump from 0 to 4000.
        // But having user pressing shoot button and losing the ball is a good indicator we are done.
        // This does not handle case when ball escapes, but shooter still did not fire - we will misfire and reload -
        // likely in time for next ball to land in shooter, so it's not a big issue.
        if (m_timeSinseShooting <= 20)
        {
            m_disablePreload = false;
            m_overrideShooting = false; // this is signal to autonomous!
            m_preloadAfterShotCounter = 50;
        }
    }

    m_haveBall = (ball == BallPresence::HasBall);
    m_haveBall2 = (ball2 == BallPresence::HasBall);
    m_userShooting = userShooting;
    m_preloading = needPreload;

    // Check angle direction based on user actions.
    bool topFlag = MoveToTopFlagPosition();
    bool middleFlag = MoveToMiddleFlagPosition();

    if (topFlag)
        SetFlag(Flag::High);
    if (middleFlag)
        SetFlag(Flag::Middle);

    bool shooting = userShooting || needPreload || m_preloadAfterShotCounter > 0;

    // Shooter motor.
    if (shooting)
    {
        motor_move(shooterPort, -shooterMotorSpeed);
    }
    else
    {
        motor_move(shooterPort, 0);
    }

    // Keep moving angle to rigth position
    KeepMoving();
}
