#include "actionsMove.h"
#include "aton.h"
#include "actions.h"
#include "position.h"
#include "drive.h"
// #include "lineTracker.h"

#include <limits.h>

extern const  bool g_leverageLineTrackers;

int Sign(int value)
{
   if (value < 0)
      return -1;
   if (value > 0)
      return 1;
   return 0;
}

// unsigned int points[] {30, 50, 1600, UINT_MAX};
// unsigned int speeds[] {0,  60, 4000, 4000};
unsigned int SpeedFromDistances(unsigned int distance, const unsigned int* points, const unsigned int* speeds)
{
    unsigned int lowPoint = 0;
    unsigned int lowSpeed = 0;
    while (true)
    {
        unsigned int highPoint = *points;
        unsigned int highSpeed = *speeds;
        Assert(lowPoint < highPoint);
        Assert(lowSpeed <= highSpeed);
        if (distance <= highPoint)
            return lowSpeed + (highSpeed - lowSpeed) * (distance - lowPoint) / (highPoint - lowPoint);
        points++;
        speeds++;
        lowPoint = highPoint;
        lowSpeed = highSpeed;
    }
}

int SpeedFromDistances(int distance, const unsigned int* points, const unsigned int* speeds)
{
    return SpeedFromDistances((unsigned int)abs(distance), points, speeds) * Sign(distance);
}


struct MoveAction : public Action
{
    Drive& m_drive = GetDrive();

    const char* Name() override { return "MoveAction"; }

    MoveAction(int distance, int power = 85)
        : m_forward(distance >= 0)
    {
        Assert(distance != 0);
        if (!m_forward)
        {
            distance = -distance;
            power = -power;
        }
        m_distanceToMove = distance;
        m_origDistanceToMove = distance;
        
        m_drive.ResetTrackingState();
        Assert(m_drive.m_distance == 0);
        m_drive.OverrideInputs(power, 0/*turn*/);
    }

    bool ShouldStopOnCollision()
    {
        if (m_stopOnCollision)
        {
            // in RPM
            unsigned int fwrd = abs(GetForwardVelocity());
            unsigned int back = abs(GetBackVelocity());
            
            // ReportStatus("Detect stop: %d %d\n", fwrd, back);

            if (fwrd <= 10 || back <= 10)
            {
                ReportStatus("   Collision detected! distance %d / %d, speeds: %d, %d\n",
                        m_drive.m_distance, m_origDistanceToMove, fwrd, back);
                m_stopOnCollision = false;
                return true;
            }
        }
        return false;
    }

    bool ShouldStop() override
    {
        unsigned int distance = m_drive.m_distance;

        if (ShouldStopOnCollision())
            return true;

        return (m_drive.m_distance >= m_distanceToMove);
    }

    void Stop() override
    {
        if (m_stopOnCollision)
            ReportStatus("Move did not hit wall: dist = %d\n", m_origDistanceToMove);
        m_drive.OverrideInputs(0, 0);
    }

  protected:
    unsigned int m_distanceToMove;
    const bool m_forward;
    bool m_stopOnCollision = false;
private:
    unsigned int m_origDistanceToMove; // used for logging - m_distanceToMove can change over lifetime of this class 
};

void MoveWithFixedPower(int distance, int power) {
    Do(MoveAction(distance, power));
}

void MoveStreight(int distance, int power, int angle) {
    TurnToAngleIfNeeded(angle);
    // KeepAngle angleObj(angle);
    MoveWithFixedPower(distance, power);
    WaitAfterMove();
}

struct StopAction : public MoveAction
{
    StopAction()
        : MoveAction(50000, -30)
    {
        m_stopOnCollision = true;
    }
    const char* Name() override { return "StopAction"; }
};

void MoveStop()
{
    Do(StopAction(), 500 /*timeout*/);
}

struct MoveExactAction : public MoveAction
{
    const char* Name() override { return "MoveExactAction"; }

    MoveExactAction(int distance, int angle, bool stopOnCollision = false)
        : MoveAction(distance, 0 /*power*/),
          m_angle(angle),
          m_engageStopOnCollision(stopOnCollision)
    {
    }

    virtual int SpeedFromDistance(int error)
    {
        static constexpr unsigned int points[] = {30, 31, 60, 1600, UINT_MAX};
        static constexpr unsigned int speeds[] = {0, 100, 300, 4000, 4000};
        return SpeedFromDistances(error, points, speeds);
    }

    bool ShouldStop() override
    {
        int velocity = GetRobotVelocity();
        if (m_engageStopOnCollision)
        {
            if (abs(velocity) >= 100 || GetElapsedTime() >= 600)
                m_stopOnCollision = true;
        }

        if (ShouldStopOnCollision())
            return true;

        unsigned int distance = m_drive.m_distance;
        int error = (int)m_distanceToMove - int(distance);

        // 1 tick/ms on each wheel (roughly 36"/sec - unreachable speed) == 72 in actualSpeed
        int actualSpeed = 20 * velocity;
        int idealSpeed = SpeedFromDistance(error);
        if (!m_forward)
            actualSpeed = -actualSpeed; // make it positive

        if ((idealSpeed == 0 && abs(actualSpeed) <= 100) || error < -30)
            return true;
 
        // Moving counter-clockwise: Positive means we need to speed up rotation
        // moveing clockwise: negative means we need to speed up rotation
        // Overall: positive means add to counter-clockwise direction
        // In gyro ticks per second. 256 is one degree per second
        int diff = idealSpeed - actualSpeed;

        // Power calculation. Some notes:
        // We want to have smaller impact of speed difference, to keep system stable.
        // For that reason, bigger kick is comming from
        // a) "stable" power to jeep motion going - that's fixed size (with right sign)
        // b) power proportional to ideal speed - the higher maintained speed, the more energy is needed to sustain it.
        // The rest is addressed by difference between nominal and desired speeds
        int power = 0;
        if (error < 0 || idealSpeed == 0)
            power = -10 + idealSpeed / 60 + diff / 50; // Stopping!
        else if (idealSpeed != 0)
        {
            // Moving forward
            if (diff > 0) // accelerating
                power = 22 + idealSpeed / 50 + diff / 100;
            else
                power = 15 + idealSpeed / 80 + diff / 100;
        }

        // If robot stopped, or about to stop (and far from target), then give it a boost
        // Friction is too high for formular above to start moving with right speed - it's structured
        // to be stable around desired speed.
        if (actualSpeed == 0 || (abs(actualSpeed) < 20 && error > 50))
        {
            power += maxSpeed / 2;
        }

        // Start slowly, for better accuracy.
        // Reach full power in 1 second, 50% powet in .3 seconds
        int powerLimit = 45 + (GetElapsedTime()) / 5;
        if (power > powerLimit)
            power = powerLimit;

        if (power > maxSpeed)
            power = maxSpeed;

        // ReportStatus("MoveExact: er=%d speed=%d ideal=%d diff=%d power=%d gyro=%d\n", error, actualSpeed, idealSpeed, diff, power, GetGyroReading());

        if (!m_forward)
            power = -power;
        m_power = (power + m_power) / 2;

        m_drive.OverrideInputs(m_power, 0);
        return false;
    }

  protected:
    KeepAngle m_angle;
    static const int maxSpeed = 127;
    int m_power = 45;
    bool m_engageStopOnCollision = false;
};

void MoveExact(int distance, int angle)
{
    Do(MoveExactAction(distance, angle));
    WaitAfterMoveReportDistance(distance);
}

struct MoveExactFastAction : public MoveExactAction
{
    // Keep going forever, untill we hit the wall...
    static const int distancetoStopMotors = 60;
    static const int distanceToAdd = 20;

    MoveExactFastAction(int distance, int angle, bool stopOnCollision = false)
        : MoveExactAction(distance + Sign(distance) * distanceToAdd, angle, stopOnCollision)
    {}

    bool ShouldStop() override
    {
        unsigned int distance = m_drive.m_distance;
        int error = (int)m_distanceToMove - int(distance);
        // ReportStatus("A: %d %d %d\n ", distance, m_distanceToMove, error);
        if (error < distancetoStopMotors)
            return true;

        return MoveExactAction::ShouldStop();
    }
private:
    bool m_stopOnHit;
};

using MoveFlipCapAction = MoveExactFastAction;

void MoveExactFastWithAngle(int distance, int angle, bool stopOnHit)
{
    TurnToAngleIfNeeded(angle);
    Do(MoveExactFastAction(distance, angle, stopOnHit));
    WaitAfterMoveReportDistance(distance, 500);
}


struct MoveHitWallAction : public MoveExactAction
{
    // Keep going forever, untill we hit the wall...
    static const int distanceToKeep = 400;

    MoveHitWallAction(int distance, int angle)
        : MoveExactAction(distance + Sign(distance) * distanceToKeep, angle, true /*stopOnCollision*/)
    {
    }

    int SpeedFromDistance(int error) override
    {
        if (abs(error) <= distanceToKeep)
        {
            m_distanceToMove = m_drive.m_distance + distanceToKeep;
        }
        return MoveExactAction::SpeedFromDistance(error);
    }
};

unsigned int HitTheWall(int distanceForward, int angle)
{
    TurnToAngleIfNeeded(angle);
    Do(MoveHitWallAction(distanceForward, angle), 1000 + abs(distanceForward) /*trimeout*/);
    WaitAfterMove();

    unsigned int distance = GetDrive().m_distance;
    return distance;
}

#if 0
template<typename TMoveAction>
struct MoveExactWithLineCorrectionAction : public TMoveAction
{
    LineTracker& m_trackerLeft = GetLineTrackerLeft();
    LineTracker& m_trackerRight = GetLineTrackerLeft();

    MoveExactWithLineCorrectionAction(int fullDistance, unsigned int distanceAfterLine, int angle)
        : TMoveAction(fullDistance, angle),
          m_distanceAfterLine(distanceAfterLine),
          m_angle(angle)
    {
        Assert(distanceAfterLine >= 0);
        m_trackerLeft.Reset();
        m_trackerRight.Reset();
    }

    bool ShouldStop() override
    {
        int shouldHaveTravelled = (int)TMoveAction::m_distanceToMove - (int)m_distanceAfterLine;        

        // Adjust distance based only on one line tracker going over line.
        // This might be useful in cases where second line tracker will never cross the line.
        // Like when driving to climb platform.
        // If we hit line with both trackers, we will re-calibrate distance based on that later on.
        if (!m_adjustedDistance)
        {
            unsigned int distance = 0;
            if (m_trackerLeft.HasWhiteLine(shouldHaveTravelled))
            {
                distance = m_trackerLeft.GetWhiteLineDistance(false/*pop*/);
                m_adjustedDistance = true;
            }
            if (m_trackerRight.HasWhiteLine(shouldHaveTravelled))
            {
                distance = m_trackerRight.GetWhiteLineDistance(false/*pop*/);
                m_adjustedDistance = true;
            }
            if (m_adjustedDistance)
            {
                ReportStatus("Single line correction: travelled: %d, Dist: %d -> %d\n",
                    distance, TMoveAction::m_distanceToMove - distance, m_distanceAfterLine + 50);
                if (g_leverageLineTrackers)
                    TMoveAction::m_distanceToMove = m_distanceAfterLine + 50 + distance;
            }
        }
        else if (m_fActive && m_trackerLeft.HasWhiteLine(shouldHaveTravelled) && m_trackerRight.HasWhiteLine(shouldHaveTravelled))
        {
            m_fActive = false; // ignore any other lines
            unsigned int left = m_trackerLeft.GetWhiteLineDistance(true/*pop*/);
            unsigned int right = m_trackerRight.GetWhiteLineDistance(true/*pop*/);
            unsigned int distance = (left + right) / 2;
            
            int diff = right - left;
            // if angles are flipped, then m_angle is flipped. SetAngle() will also flip angle to get to real one.
            if (TMoveAction::m_drive.IsXFlipped())
                diff = -diff;

            float angle = atan2(diff, DistanveBetweenLineSensors * 2); // left & right is double of distanve
            int angleI = angle * 180 / PositionTracker::Pi;

            ReportStatus("Double line correction: travelled: %d, Dist: %d -> %d, angle+: %d\n",
                distance, TMoveAction::m_distanceToMove - int(distance), m_distanceAfterLine, angleI);

            if (g_leverageLineTrackers)
            {
                TMoveAction::m_distanceToMove = m_distanceAfterLine + distance;
                GetTracker().SetAngle(m_angle - angleI);
            }
        }

        bool res = TMoveAction::ShouldStop();
        if (res && m_fActive)
        {
            ReportStatus("Line correction did not happen! Max brightness: %d,  %d\n",
                m_trackerLeft.MinValue(), m_trackerRight.MinValue());
        }
        return res;
    }

protected:
    unsigned int m_distanceAfterLine;
    int m_angle;
    bool m_fActive = true;
    bool m_adjustedDistance = false;
};

void MoveExactWithLineCorrection(int fullDistance, unsigned int distanceAfterLine, int angle)
{
    TurnToAngleIfNeeded(angle);
    Do(MoveExactWithLineCorrectionAction<MoveExactAction>(fullDistance, distanceAfterLine, angle));
    WaitAfterMove();
}
#endif

struct TurnPrecise : public Action
{
    Drive& m_drive = GetDrive();

    const char* Name() override { return "TurnPrecise"; }

    TurnPrecise(int turn)
        : m_turn(AdjustAngle(turn))
    {
        // ReportStatus("TurnPrecise: %d, curr=%d\n", m_turn / GyroWrapper::Multiplier, GetGyroReading() / GyroWrapper::Multiplier);

        m_drive.ResetTrackingState();
        m_initialAngle = GetGyroReading();
    }

    void Stop()
    {
        m_drive.OverrideInputs(0, 0);
    }

    // Relative to current position
    void ChangeTurn(int turn)
    {
        m_turn += turn - GetError();
    }

    int GetError()
    {
        return m_initialAngle + m_turn - GetGyroReading();
    }

    bool ShouldStop() override
    {
        // 10 points per degree of angle
        static constexpr unsigned int points[] = { 8,  9, 200, 500, UINT_MAX};
        static constexpr unsigned int speeds[] = { 0, 18,  80, 200, 221};

        // positive for positive (clock-wise) turns
        int error = GetError();
        int sign = Sign(error);

        // positive means counter-clockwise
        int actualSpeed = 1000 * GetTracker().LatestPosition(false /*clicks*/).gyroSpeed / GyroWrapper::Multiplier; // degrees per second
        int idealSpeed = SpeedFromDistances(error * 10 / GyroWrapper::Multiplier, points, speeds);

        if ((idealSpeed == 0 && abs(actualSpeed) <= 15) || (m_turn * sign < 0 && abs(error) >= GyroWrapper::Multiplier * 0.8))
        {
            if (abs(error) >= GyroWrapper::Multiplier)
                ReportStatus("   Turn stop! Error: error=%d turn=%d\n", error, m_turn);
            return true;
        }

        // Moving counter-clockwise: Positive means we need to speed up rotation
        // moveing clockwise: negative means we need to speed up rotation
        // Overall: positive means add to counter-clockwise direction
        // In gyro ticks per second. 256 is one degree per second
        int diff = idealSpeed - actualSpeed;

        // Power calculation. Some notes:
        // We want to have smaller impact of speed difference, to keep system stable.
        // For that reason, bigger kick is comming from
        // a) "stable" power to jeep motion going - that's fixed size (with right sign)
        // b) power proportional to ideal speed - the higher maintained speed, the more energy is needed to sustain it.
        // The rest is addressed by difference between nominal and desired speeds
        int power;
        unsigned int errorAbs = abs(error);
        if (idealSpeed == 0)
            power = -Sign(actualSpeed) * 14;
        else
            power = sign * 30 + idealSpeed * (4 + abs(idealSpeed) / 9) / 100 + diff * 0.36;

        int maxSpeed = 80;
        if (errorAbs <= 2 * GyroWrapper::Multiplier)
            maxSpeed = 30;

        if (power > maxSpeed)
            power = maxSpeed;
        else if (power < -maxSpeed)
            power = -maxSpeed;

        if (actualSpeed == 0 && abs(power) < 40)
            m_power = sign * 40;
        else
            m_power = (power + m_power) / 2;

        // ReportStatus("Turn error = %d, power = %d, ideal speed = %d, actual = %d\n", error, power, idealSpeed, actualSpeed);
        m_drive.OverrideInputs(0, m_power);
        return false;
    }

  protected:
    int m_turn;

  private:
    int m_initialAngle;
    int m_power = 0;
};

void TurnToAngle(int turn)
{
    const auto mult = GyroWrapper::Multiplier;
    auto angle = turn * mult - GetGyroReading();
    Do(TurnPrecise(angle));
    WaitAfterMove();
    if (abs(turn * mult - GetGyroReading()) >= mult/2)
        ReportStatus("!!! Turn Error: (x10) curr angle = %d, desired angle = %d\n", GetGyroReading() * 10 / mult, 10 * turn);
}

void TurnToAngleIfNeeded(int angle)
{
    int angleDiff = AdjustAngle(GetGyroReading() - angle * GyroWrapper::Multiplier);
    if (abs(angleDiff) > 8 * GyroWrapper::Multiplier)
        TurnToAngle(angle);
}
