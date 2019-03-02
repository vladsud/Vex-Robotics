#include "shooter.h"
#include "logger.h"

// Distance based on front of robot
constexpr float Distances[]{24, 30, 48, 55, 78, 108};
//   0: flat - loading
// 156: highest angle we can do (roughly 60 degrees)
// At value 72 (roughly 30 degree angle), +1 value results in ~1/3" shift on target, assuming 4th position (54" from target)
constexpr unsigned int AnglesHigh[]{120, 110, 95, 90, 60, 80};
constexpr unsigned int AnglesMedium[]{90, 74, 55, 48, 40, 55};

constexpr unsigned int LastDistanceCount = CountOf(Distances) - 1;

constexpr unsigned int ConvertAngleToPotentiometer(unsigned int angle)
{
    return 1860 - angle * 5;
}

// Angle potentiometer:
constexpr unsigned int anglerLow = 22;
const unsigned int anglePotentiometerLow = ConvertAngleToPotentiometer(anglerLow);
const unsigned int anglePotentiometerHigh = 1060;

StaticAssert(CountOf(Distances) == CountOf(AnglesHigh));
StaticAssert(CountOf(Distances) == CountOf(AnglesMedium));

StaticAssert(ConvertAngleToPotentiometer(AnglesMedium[0]) < anglePotentiometerLow);
StaticAssert(ConvertAngleToPotentiometer(AnglesHigh[LastDistanceCount]) >= anglePotentiometerHigh);

StaticAssert(Distances[0] < Distances[1]);
StaticAssert(Distances[1] < Distances[2]);
StaticAssert(Distances[2] < Distances[3]);
StaticAssert(Distances[3] < Distances[4]);

StaticAssert(AnglesHigh[0] >= AnglesHigh[1]);
StaticAssert(AnglesHigh[1] >= AnglesHigh[2]);
//StaticAssert(AnglesHigh[2] >= AnglesHigh[3]);
// StaticAssert(AnglesHigh[3] >= AnglesHigh[4]);

StaticAssert(AnglesMedium[0] >= AnglesMedium[1]);
StaticAssert(AnglesMedium[1] >= AnglesMedium[2]);
StaticAssert(AnglesMedium[2] >= AnglesMedium[3]);
// StaticAssert(AnglesMedium[3] >= AnglesMedium[4]);

StaticAssert(AnglesMedium[0] < AnglesHigh[0]);
StaticAssert(AnglesMedium[1] < AnglesHigh[1]);
StaticAssert(AnglesMedium[2] < AnglesHigh[2]);
StaticAssert(AnglesMedium[3] < AnglesHigh[3]);
// StaticAssert(AnglesMedium[4] < AnglesHigh[4]);

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

StaticAssert(CalcAngle(Flag::Loading, Distances[2]) == anglerLow);

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
{
    m_distanceInches = Distances[2];
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

bool Shooter::IsShooting()
{
    if (m_flag == Flag::Loading)
        return false;

    // Allow "shoot" button to be pressed all the time and shoot once angle is settled
    if (m_fMoving)
    {
        Assert(!m_overrideShooting); // atonomous should always wait till angle is settled. 
        return false;
    }
    return m_overrideShooting || joystickGetDigital(7, JOY_LEFT);
}
 
bool Shooter::IsMovingAngle()
{
    if (m_fMoving)
        return true;
    unsigned int dist = abs(analogRead(anglePotPort) - (int)m_angleToMove);
    ReportStatus("Angle dist: %d\n", dist);
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
    unsigned int current = analogRead(anglePotPort);
    int distance = current - m_angleToMove;
    int diff = distance - m_lastAngleDistance;

    // sometimes we get really wrong readings.
    if (diff > m_diffAdjusted + 5)
        diff = m_diffAdjusted + 5;
    else if (diff < m_diffAdjusted - 5)
        diff = m_diffAdjusted - 5;

    distance = diff + m_lastAngleDistance;

    unsigned int distanceAbs = abs(distance);

    m_diffAdjusted = diff; // (diff + m_diffAdjusted) / 2;

    m_count++;

    // Safety net - we want to stop after some time and let other steps in autonomous to play out.
    if ((m_fMoving && m_count >= 200) || (distanceAbs <= 10 && abs(m_diffAdjusted) <= 3) || (distance > 0 && m_flag == Flag::Loading))
    {
        if (m_fMoving)
        {
            if (PrintDiagnostics(Diagnostics::Angle))
                ReportStatus("STOP: (%d) Dest: %d   Reading: %d, Distance: %d/%d, Diff: %d, DiffAdj: %d\n\n\n",
                    m_count, m_angleToMove, current, current - m_angleToMove, distance, diff, m_diffAdjusted);
            StopMoving();
        }
        // if (m_flag == Flag::Loading)
            motorSet(anglePort, 0);
        return;
    }

    if (m_fMoving && m_flag == Flag::Loading)
    {
        if (distance > 0)
            speed = 0;
        else
            speed = -26 + distance / 4 + m_diffAdjusted * 4;
    }
    else if (distance > 40) // going up
        speed = 20 + distance / 4 + m_diffAdjusted * 2;
    else if (distance > 10) // going up
        speed = 22 + distance / 5  + m_diffAdjusted * 3;
    else if (distance > 0) // already there
        speed = m_fMoving ? -10 : 0;
    else if (!m_fMoving && distance > -10)
        speed = 0;
    else  // going down, overshoot
        speed = -20 + distance / 3 + m_diffAdjusted * 2;


    if (speed * distance < 0 && distanceAbs > 20)
        speed = 0;

    const int angleMotorSpeed = distanceAbs > 20 ? 75 : 20;

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
    ReportStatus("Shooter distance (%d)s: %d\n", millis(), distance);
    m_distanceInches = distance;
    StartMoving();
}

void Shooter::SetFlag(Flag flag)
{
    if (m_flag == flag)
        return;
    ReportStatus("Shooter flag (%d): %d\n", millis(), (int)flag);
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
    StaticAssert(lightSensorBallIn < lightSensorBallOut);
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

    // User can disable auto-reload by moving angle up / down against system.
    // If it happens, we set m_Manual = false for this cycle (until readng starts matching user actions)
    bool lostball = false;
    if (!m_Manual)
    {
        if (ball == BallPresence::HasBall && m_flag == Flag::Loading)
            SetFlag(Flag::High);
        if (ball == BallPresence::NoBall && m_flag != Flag::Loading)
        {
            lostball = true;
            SetFlag(Flag::Loading);
            UpdateIntakeFromShooter(IntakeShoterEvent::LostBall, false /*forceDown*/);
        }
    }

    // We tell intake to go down when shooting to clear space for the ball that we are about to shoot.
    // But if user stopped shooting, then we want to keep second ball and not lose it, thus we need to let intake know
    // about this event and start moving up. Similar logic exists for when we lose ball (as result of shooting),
    // but this case is required for case when the ball was not shot
    // Note that we do not want to do this in autonomouns / when we lost a ball, as this happens after we moved to nezt step,
    // and thus we can overwrite autonomous command that instructed intake to go down.
    if (!userShooting && m_userShooting && !isAuto())
        UpdateIntakeFromShooter(IntakeShoterEvent::LostBall, false /*forceDown*/);

    m_userShooting = userShooting;
    m_preloading = needPreload;

    // Did shot just hapened?
    // We can't use potentiometer here, because we are very close to dead zone, so it can jump from 0 to 4000.
    // But having user pressing shoot button and losing the ball is a good indicator we are done.
    // This does not handle case when ball escapes, but shooter still did not fire - we will misfire and reload -
    // likely in time for next ball to land in shooter, so it's not a big issue.
    if (m_timeSinseShooting <= 20 && lostball)
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

    bool shooting = userShooting || needPreload || m_preloadAfterShotCounter > 0;
    if (userShooting && !isAuto() && m_flag == Flag::Middle && m_distanceInches >= Distances[2])
    {
        if (m_shooterWait == 0)
        {
            m_shooterWait = 10;
        }
        m_shooterWait++;
        if (m_shooterWait < 25)
            shooting = false;
    }
    else
    {
        m_shooterWait = 0;
    }
    
    // Shooter motor.
    if (shooting)
    {
        motorSet(shooterPort, shooterMotorSpeed);
		motorSet(shooter2Port, shooterMotorSpeed);
    }
    else
    {
        motorSet(shooterPort, 0);
		motorSet(shooter2Port, 0);
    }

    // Keep moving angle to rigth position
    KeepMoving();
}