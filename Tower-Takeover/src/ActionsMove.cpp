#include "actionsMove.h"
#include "motion.h"
#include "aton.h"
#include "actions.h"
#include "position.h"
#include "drive.h"
#include "stateMachine.h"
#include "intake.h"

#ifdef LineTracker
#include "lineTracker.h"
extern const bool g_leverageLineTrackers;
#endif // LineTracker

/*******************************************************************************
 * 
 * Stand-alone functions
 * 
 ******************************************************************************/
float AdjustAngle(float angle)
{
    while (angle > 180)
        angle -= 360;
    while (angle < -180)
        angle += 360;
    return angle;
}


/*******************************************************************************
 * 
 * MoveActionBase
 * 
 ******************************************************************************/
struct MoveActionBase : public Action
{
    Drive& m_drive = GetDrive();

    const char* Name() override { return "MoveAction"; }

    MoveActionBase(int distance)
        : m_origDistanceToMove(abs(distance))
    {
        m_drive.ResetTrackingState();
        Assert(m_drive.GetDistance() == 0);
    }

    bool ShouldStopOnCollision()
    {
        if (m_stopOnCollision)
        {
            if (GetTracker().GetRobotVelocity() <= 5)
            {
                ReportStatus(Log::Info, "   Collision detected! distance %d / %d\n",
                        m_drive.GetDistance(), m_origDistanceToMove);
                m_stopOnCollision = false;
                return true;
            }
        }
        return false;
    }

    void Stop() override
    {
        if (m_stopOnCollision)
            ReportStatus(Log::Warning, "Move did not hit wall: dist = %d\n", m_origDistanceToMove);
        m_drive.OverrideInputs(0, 0);
    }

  protected:
    bool m_stopOnCollision = false;
    const unsigned int m_origDistanceToMove; // used for logging - m_distanceToMove can change over lifetime of this class 
};


/*******************************************************************************
 * 
 * MoveAction & MoveStraight
 * 
 ******************************************************************************/
struct MoveAction : public MoveActionBase
{
    MoveAction(int distance, int power = 85)
        : MoveActionBase(distance)
    {
        Assert(distance != 0);
        if (distance < 0)
            power = -power;        
        m_drive.OverrideInputs(power, 0/*turn*/);
    }

    bool ShouldStop() override
    {
        if (ShouldStopOnCollision())
            return true;
        return (abs(m_drive.GetDistance()) >= m_origDistanceToMove);
    }
};


void MoveStraight(int distance, int power, int angle) {
    TurnToAngleIfNeeded(angle);
    KeepAngle angleObj(angle);
    Do(MoveAction(distance, power));
    WaitAfterMove();
}

/*******************************************************************************
 * 
 * StopAction & MoveStop
 * 
 ******************************************************************************/
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


/*******************************************************************************
 * 
 * MoveExactAction
 * 
 ******************************************************************************/
struct MoveExactAction : public MoveActionBase, public Motion
{
    const char* Name() override { return "MoveExactAction"; }

    MoveExactAction(int distance, int angle, unsigned int speedLimit = UINT_MAX, bool stopOnCollision = false)
        : MoveActionBase(distance),
          Motion(moveModel, speedLimit),
          m_angle(angle),
          m_distanceToMove(distance),
          m_engageStopOnCollision(stopOnCollision),
          m_sign(Sign(distance))
    {
    }

    int GetError() override
    {
        return m_distanceToMove - m_drive.GetDistance();
    }

    bool ShouldStop() override
    {
        if (m_engageStopOnCollision && !m_stopOnCollision)
        {
            if (abs(GetTracker().GetRobotVelocity()) >= 20 || GetElapsedTime() >= 500)
                m_stopOnCollision = true;
        }

        if (ShouldStopOnCollision())
            return true;

        return Motion::ShouldStop();
    }

    void SetMotorPower(int power) override
    {
        m_drive.OverrideInputs(power, 0);
    }

  protected:
    KeepAngle m_angle;
    int m_distanceToMove;
    const int m_sign;
    bool m_engageStopOnCollision = false;
};


/*******************************************************************************
 * 
 * MoveExactWithAngle
 * 
 ******************************************************************************/
void MoveExactWithAngle(
        int distance,
        int angle,
        unsigned int speedLimit,
        unsigned int timeout /*= 100000U*/,
        bool allowTurning /*= true*/)
{
    if (allowTurning)
        TurnToAngleIfNeeded(angle);
    Do(MoveExactAction(distance, angle, speedLimit), timeout);
    WaitAfterMoveReportDistance(distance);
}

/*******************************************************************************
 * 
 * MoveHitWallAction & HitTheWall
 * 
 ******************************************************************************/
struct MoveHitWallAction : public MoveExactAction
{
    // Keep going forever, untill we hit the wall...
    static const int distanceToKeep = 400;

    MoveHitWallAction(int distance, int angle)
        : MoveExactAction(distance + Sign(distance) * distanceToKeep, angle, UINT_MAX, true /*stopOnCollision*/)
    {
    }

    int GetError() override
    {
        int error = MoveExactAction::GetError();
        if (abs(error) <= distanceToKeep)
        {
            m_distanceToMove =  m_drive.GetDistance() + m_sign * distanceToKeep;
            error = m_sign * distanceToKeep;
        }
        return error;
    }
};

int HitTheWall(int distanceForward, int angle)
{
    TurnToAngleIfNeeded(angle);
    Do(MoveHitWallAction(distanceForward, angle), 1000 + abs(distanceForward) /*trimeout*/);
    WaitAfterMove();

    int distance = GetDrive().GetDistance();
    return distance;
}


/*******************************************************************************
 * 
 * MoveExactWithLineCorrectionAction & MoveExactWithLineCorrection
 * 
 ******************************************************************************/
#if LineTracker
template<typename TMoveAction>
struct MoveExactWithLineCorrectionAction : public TMoveAction
{
    LineTracker& m_trackerLeft = GetLineTrackerLeft();
    LineTracker& m_trackerRight = GetLineTrackerLeft();

    // Note: this code has been tested only for moving forward.
    // I.e. fullDistance being positive.
    // Algorithm likely does not work and needs adjustments for making it work moving backwards
    MoveExactWithLineCorrectionAction(unsigned int fullDistance, unsigned int distanceAfterLine, int angle)
        : TMoveAction(fullDistance, angle),
          m_distanceAfterLine(distanceAfterLine),
          m_angle(angle)
    {
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
            int distance = 0;
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
                ReportStatus(Log::info, "Single line correction: travelled: %d, Dist: %d -> %d\n",
                    distance, TMoveAction::m_distanceToMove - distance, m_distanceAfterLine + 50);
                if (g_leverageLineTrackers)
                    TMoveAction::m_distanceToMove = m_distanceAfterLine + 50 + distance;
            }
        }
        else if (m_fActive && m_trackerLeft.HasWhiteLine(shouldHaveTravelled) && m_trackerRight.HasWhiteLine(shouldHaveTravelled))
        {
            m_fActive = false; // ignore any other lines
            int left = m_trackerLeft.GetWhiteLineDistance(true/*pop*/);
            int right = m_trackerRight.GetWhiteLineDistance(true/*pop*/);
            int distance = (left + right) / 2;
            
            int diff = right - left;
            // if angles are flipped, then m_angle is flipped. SetAngle() will also flip angle to get to real one.
            if (TMoveAction::m_drive.IsXFlipped())
                diff = -diff;

            float angle = atan2(diff, DistanveBetweenLineSensors * 2); // left & right is double of distanve
            int angleI = angle * 180 / PositionTracker::PI;

            ReportStatus(Log::info, "Double line correction: travelled: %d, Dist: %d -> %d, angle+: %d\n",
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
            ReportStatus(Log::Warning, "Line correction did not happen! Max brightness: %d,  %d\n",
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


/*******************************************************************************
 * 
 * TurnPrecise
 * 
 ******************************************************************************/
struct TurnPrecise : public Action, public Motion
{
    Drive& m_drive = GetDrive();

    const char* Name() override { return "TurnPrecise"; }

    TurnPrecise(float turn)
        : Motion(turnModel)
        , m_turn(AdjustAngle(turn))
    {
        ReportStatus(Log::Verbose, "TurnPrecise: %f, curr=%f\n", m_turn, GetGyroReading());

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

    int GetError() override
    {
        return 100 * (m_turn - (GetGyroReading() - m_initialAngle));
    }

    bool ShouldStop() override
    {
        return Motion::ShouldStop();
    }

    void SetMotorPower(int power) override
    {
        m_drive.OverrideInputs(0, -power);
    }

  protected:
    float m_turn;

  private:
    float m_initialAngle;
    int m_power = 0;
};


/*******************************************************************************
 * 
 * TurnToAngle & TurnToAngleIfNeeded
 * 
 ******************************************************************************/
void TurnToAngle(int turn)
{
    auto angle = turn - GetGyroReading();
    Do(TurnPrecise(angle));
    WaitAfterMove();
    float err = AdjustAngle(turn - GetGyroReading());
    if (abs(err) >= 0.5)
        ReportStatus(Log::Warning, "!!! Turn Error: error = %f, desired angle = %f\n", err, (float)turn);
}

void TurnToAngleIfNeeded(int angle)
{
    float angleDiff = AdjustAngle(GetGyroReading() - angle);
    if (abs(angleDiff) > 8)
        TurnToAngle(angle);
}

/*******************************************************************************
 *
 * WaitAfterMove()
* 
 ******************************************************************************/
struct WaitTillStopsAction : public Action
{
    bool ShouldStop() override
    {
        return abs(GetTracker().GetRobotVelocity()) <= 2;
    }
    const char* Name() override { return "WaitTillStopsAction"; } 
};

// give some time for robot to completely stop
void WaitAfterMove(unsigned int timeout /*= 0*/)
{
    /*
    // Not enough time in "main" atonomous
    auto& lcd = GetLcd();
    if (timeout == 0)
        timeout = lcd.AtonSkills || !lcd.AtonProtected || !lcd.AtonClimbPlatform ? 500 : 200;
    */
    Do(WaitTillStopsAction(), timeout == 0 ? 200 : timeout);
}



/*******************************************************************************
 * 
 * MoveExactWithTray
 * 
 ******************************************************************************/

struct MoveExactActionWithTray : public MoveExactAction
{
    MoveExactActionWithTray(int distance, int angle, int ticks, unsigned int speedLimit = UINT_MAX, bool stopOnCollision = false)
        : ticksUntil(ticks), MoveExactAction(distance, angle, speedLimit, stopOnCollision)
    {

    }

    bool ShouldStop() override
    {
        if (GetError() < ticksUntil + 500) {
            SetIntake(-80);
        }
        if (GetError() < ticksUntil) {
            SetIntake(0);
            GetStateMachine().SetState(State::TrayOut);
        }
        return MoveExactAction::ShouldStop();
    }

    protected:
        int ticksUntil;
};

void MoveExactWithAngleAndTray(
        int distance,
        int angle,
        int ticksUntil,
        unsigned int speedLimit,
        unsigned int timeout /*= 100000U*/,
        bool allowTurning /*= true*/)
{
    if (allowTurning)
        TurnToAngleIfNeeded(angle);
    Do(MoveExactActionWithTray(distance, angle, ticksUntil, speedLimit), timeout);

    DoTrayAction(State::TrayOut);

    WaitAfterMoveReportDistance(distance);
}
