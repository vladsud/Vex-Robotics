#include "shooter.h"
#include "logger.h"

// Distance based on front of robot
constexpr float Distances[]{24, 30, 48, 75, 108};
//   0: flat - loading
// 156: highest angle we can do (roughly 60 degrees)
// At value 72 (roughly 30 degree angle), +1 value results in ~1/3" shift on target, assuming 4th position (54" from target)
constexpr unsigned int AnglesHigh[]{130, 120, 95, 72, 80};
constexpr unsigned int AnglesMedium[]{90, 80, 50, 40, 50};

constexpr unsigned int LastDistanceCount = CountOf(Distances) - 1;

constexpr unsigned int ConvertAngleToPotentiometer(unsigned int angle)
{
    return 1860 - angle * 5;
}

// Angle potentiometer:
constexpr unsigned int anglerLow = 3;
const unsigned int anglePotentiometerLow = ConvertAngleToPotentiometer(anglerLow);
const unsigned int anglePotentiometerHigh = 1060;

static_assert(CountOf(Distances) == CountOf(AnglesHigh));
static_assert(CountOf(Distances) == CountOf(AnglesMedium));

static_assert(ConvertAngleToPotentiometer(AnglesMedium[0]) < anglePotentiometerLow);
static_assert(ConvertAngleToPotentiometer(AnglesHigh[LastDistanceCount]) >= anglePotentiometerHigh);

static_assert(Distances[0] < Distances[1]);
static_assert(Distances[1] < Distances[2]);
static_assert(Distances[2] < Distances[3]);
static_assert(Distances[3] < Distances[4]);

static_assert(AnglesHigh[0] >= AnglesHigh[1]);
static_assert(AnglesHigh[1] >= AnglesHigh[2]);
static_assert(AnglesHigh[2] >= AnglesHigh[3]);
// static_assert(AnglesHigh[3] >= AnglesHigh[4]);

static_assert(AnglesMedium[0] >= AnglesMedium[1]);
static_assert(AnglesMedium[1] >= AnglesMedium[2]);
static_assert(AnglesMedium[2] >= AnglesMedium[3]);
// static_assert(AnglesMedium[3] >= AnglesMedium[4]);

static_assert(AnglesMedium[0] < AnglesHigh[0]);
static_assert(AnglesMedium[1] < AnglesHigh[1]);
static_assert(AnglesMedium[2] < AnglesHigh[2]);
static_assert(AnglesMedium[3] < AnglesHigh[3]);
// static_assert(AnglesMedium[4] < AnglesHigh[4]);

constexpr unsigned int CalcAngle(Flag flag, float distanceInches)
{
    if (flag == Flag::Loading)
        return anglerLow;

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

static_assert(CalcAngle(Flag::Loading, Distances[2]) == anglerLow);

static_assert(CalcAngle(Flag::High, Distances[0] - 5) == AnglesHigh[0]);
static_assert(CalcAngle(Flag::High, Distances[0]) == AnglesHigh[0]);
static_assert(CalcAngle(Flag::High, Distances[2]) == AnglesHigh[2]);
static_assert(CalcAngle(Flag::High, Distances[LastDistanceCount]) == AnglesHigh[LastDistanceCount]);
static_assert(CalcAngle(Flag::High, Distances[LastDistanceCount] + 100) == AnglesHigh[LastDistanceCount]);
static_assert(AlmostSameAngle(CalcAngle(Flag::High, (Distances[2] + Distances[3]) / 2), (AnglesHigh[2] + AnglesHigh[3]) / 2));
static_assert(AlmostSameAngle(CalcAngle(Flag::High, (Distances[2] * 3 + Distances[3]) / 4), (AnglesHigh[2] * 3 + AnglesHigh[3]) / 4));

static_assert(CalcAngle(Flag::Middle, Distances[0] - 5) == AnglesMedium[0]);
static_assert(CalcAngle(Flag::Middle, Distances[0]) == AnglesMedium[0]);
static_assert(CalcAngle(Flag::Middle, Distances[2]) == AnglesMedium[2]);
static_assert(CalcAngle(Flag::Middle, Distances[LastDistanceCount]) == AnglesMedium[LastDistanceCount]);
static_assert(CalcAngle(Flag::Middle, Distances[LastDistanceCount] + 100) == AnglesMedium[LastDistanceCount]);
static_assert(AlmostSameAngle(CalcAngle(Flag::Middle, (Distances[2] + Distances[3]) / 2), (AnglesMedium[2] + AnglesMedium[3]) / 2));
static_assert(AlmostSameAngle(CalcAngle(Flag::Middle, (Distances[2] * 3 + Distances[3]) / 4), (AnglesMedium[2] * 3 + AnglesMedium[3]) / 4));

// Need to figure out initial position
Shooter::Shooter()
{
    StartMoving();
}

unsigned int Shooter::CalcAngle()
{
    return ::CalcAngle(m_flag, m_distanceInches);
}

bool MoveToLoadingPosition()
{
    return joystickGetDigital(7, JOY_DOWN);
}

bool MoveToTopFlagPosition()
{
    return joystickGetDigital(7, JOY_UP);
}

bool MoveToMiddleFlagPosition()
{
    return joystickGetDigital(7, JOY_RIGHT);
}

// Motors:
//   up: 100
//   down: -100
void Shooter::KeepMoving()
{
    int speed = 0;
    unsigned int current = analogRead(anglePotPort);
    int distance = current - m_angleToMove;
    int diff = distance - m_lastAngleDistance;

    // sometimes we get really wrong readings.
    if (diff > m_diffAdjusted + 15)
        diff = m_diffAdjusted + 15;
    else if (diff < m_diffAdjusted - 15)
        diff = m_diffAdjusted - 15;

    distance = diff + m_lastAngleDistance;
    unsigned int distanceAbs = abs(distance);

    m_diffAdjusted = (m_diffAdjusted * 3 + diff) / 4;

    m_count++;

    // Safety net - we want to stop after some time and let other steps in autonomous to play out.
    if ((m_fMoving && m_count >= 200) || (distanceAbs <= 10 && abs(m_diffAdjusted) <= 1))
    {
        if (m_fMoving)
        {
            if (PrintDiagnostics(Diagnostics::Angle))
                printf("STOP: (%d) Dest: %d   Reading: %d, Distance: %d, Diff: %d, DiffAdj: %d\n\n\n", m_count, m_angleToMove, current, current - m_angleToMove, diff, m_diffAdjusted);
            StopMoving();
        }
        motorSet(anglePort, 0);
        return;
    }

    if (m_fMoving || m_flag != Flag::Loading)
    {
        if (distanceAbs <= 10)
            speed = m_diffAdjusted * 10;
        else if (distance > 0) // going up
            speed = 23 + distance * 2 / 3 + m_diffAdjusted * 5;
        else if (m_flag != Flag::Loading)
            speed = -14 + distance / 2 + m_diffAdjusted * 5;
        else
            speed = -25 + distance + m_diffAdjusted * 3; // / 5;
        if (PrintDiagnostics(Diagnostics::Angle))
        {
            if (m_fMoving)
                printf("ANG MOVE: (%d) Power: %d   Dest: %d   Reading: %d, Distance: %d, Diff: %d, DiffAdj: %d\n", m_count, speed, m_angleToMove, current, current - m_angleToMove, diff, m_diffAdjusted);
            else
                printf("ANG ADJ: (%d) Power: %d   Dest: %d   Reading: %d, Distance: %d, Diff: %d, DiffAdj: %d\n", m_count, speed, m_angleToMove, current, current - m_angleToMove, diff, m_diffAdjusted);
        }
    }

    const int angleMotorSpeed = 100;

    if (speed > angleMotorSpeed)
        speed = angleMotorSpeed;
    else if (speed < -angleMotorSpeed)
        speed = -angleMotorSpeed;

    motorSet(anglePort, speed);

    m_lastAngleDistance = distance;
}

void Shooter::StartMoving()
{
    m_fMoving = true;
    m_angleToMove = ConvertAngleToPotentiometer(CalcAngle());
    m_diffAdjusted = 0;
    m_lastAngleDistance = analogRead(anglePotPort) - m_angleToMove;
    m_count = 0;
}

void Shooter::StopMoving()
{
    motorSet(anglePort, 0);
    m_fMoving = false;
    m_count = 0;
}

void Shooter::Debug()
{
}

void Shooter::SetDistance(unsigned int distance)
{
    if (m_distanceInches == distance)
        return;
    m_distanceInches = distance;
    StartMoving();
}

void Shooter::SetFlag(Flag flag)
{
    if (m_flag == flag)
        return;
    m_flag = flag;
    StartMoving();
}

void Shooter::UpdateDistanceControls()
{
    if (joystickGetDigital(8, JOY_LEFT))
        SetDistance(Distances[0]);
    else if (joystickGetDigital(8, JOY_UP))
        SetDistance(Distances[1]);
    else if (joystickGetDigital(8, JOY_RIGHT))
        SetDistance(Distances[2]);
    else if (joystickGetDigital(8, JOY_DOWN))
        SetDistance(Distances[3]);
}

void Shooter::OverrideSetShooterMode(bool on)
{
    Assert(isAuto());
    Assert(!on || m_flag != Flag::Loading);

    m_overrideShooting = on;
}

BallPresence Shooter::BallStatus()
{
    // Check if we can detect ball present.
    // Use two stops to make sure we do not move angle up and down if we are somewhere in gray area (on the boundary)
    unsigned int darkness = analogRead(lightSensor);
    static_assert(lightSensorBallIn < lightSensorBallOut);
    bool ballIn = (darkness < lightSensorBallIn);
    bool ballOut = (darkness > lightSensorBallOut);

    if (ballOut)
        return BallPresence::NoBall;
    if (ballIn)
        return BallPresence::HasBall;
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

    bool userShooting = IsShooting();
    int shooterPreloadPos = analogRead(shooterPreloadPoterntiometer);

    m_timeSinseShooting++;
    if (userShooting)
    {
        m_timeSinseShooting = 0;
        UpdateIntakeFromShooter(IntakeShoterEvent::Shooting, m_flag == Flag::Middle && m_distanceInches > Distances[1] /*forceDown*/);
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
        printf("Preload state: %d\n", needPreload);

    // safety net - if something goes wrong, user needs to have a way to disable preload.
    // This is done by clicking and releasing shooter button quickly while not in the shooting zone.
    if (needPreload && userShooting && !m_userShooting && m_preloadAfterShotCounter == 0)
    {
        if (PrintDiagnostics(Diagnostics::Angle))
            print("Disabling preload\n");
        m_disablePreload = true;
        needPreload = false;
    }

    // Extra-caution: do not lose the ball if user was shooting, but this did not result in lost ball.
    // If ball is lost, then we stop intake from continuing going down.
    // But if the ball is not lost after shooting, intake will keep going down and we lose the ball.
    // This prevents that.
    if (!userShooting && m_userShooting)
        UpdateIntakeFromShooter(IntakeShoterEvent::LostBall, false /*forceDown*/);

    m_userShooting = userShooting;
    m_preloading = needPreload;

    // Check if we can detect ball present.
    BallPresence ball = BallStatus();

    // User can disable auto-reload by moving angle up / down against system.
    // If it happens, we set m_Manual = false for this cycle (until readng starts matching user actions)
    bool lostball = false;
    if (!m_Manual)
    {
        if (ball == BallPresence::HasBall && m_flag == Flag::Loading)
            SetFlag(Flag::Middle);
        if (ball == BallPresence::NoBall && m_flag != Flag::Loading)
        {
            lostball = true;
            SetFlag(Flag::Loading);
            UpdateIntakeFromShooter(IntakeShoterEvent::LostBall, false /*forceDown*/);
        }
    }

    // Did shot just hapened?
    // We can't use potentiometer here, because we are very close to dead zone, so it can jump from 0 to 4000.
    // But having user pressing shoot button and losing the ball is a good indicator we are done.
    // This does not handle case when ball escapes, but shooter still did not fire - we will misfire and reload -
    // likely in time for next ball to land in shooter, so it's not a big issue.
    if ((m_timeSinseShooting <= 20 || shooterPreloadPos < ShooterPreloadEnd) && lostball)
    {
        m_disablePreload = false;
        m_overrideShooting = false; // this is signal to autonomous!
        m_preloadAfterShotCounter = 100;
    }

    // Check angle direction based on user actions.
    bool topFlag = MoveToTopFlagPosition();
    bool middleFlag = MoveToMiddleFlagPosition();

    if (MoveToLoadingPosition())
    {
        if (m_flag != Flag::Loading)
        {
            if (ball == BallPresence::HasBall)
                m_Manual = true;
            if (ball == BallPresence::NoBall)
                m_Manual = false;
        }
        SetFlag(Flag::Loading);
    }
    else if (topFlag || middleFlag)
    {
        if (m_flag == Flag::Loading)
        {
            if (ball == BallPresence::NoBall)
                m_Manual = true;
            else if (ball == BallPresence::HasBall)
                m_Manual = false;
        }
        SetFlag(topFlag ? Flag::High : Flag::Middle);
    }

    // Shooter motor.
    if (userShooting || needPreload || m_preloadAfterShotCounter > 0)
    {
        motorSet(shooterPort, shooterMotorSpeed);
    }
    else
    {
        motorSet(shooterPort, 0);
    }

    // Keep moving angle to rigth position
    KeepMoving();
}