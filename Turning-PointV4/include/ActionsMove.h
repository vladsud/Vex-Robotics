#pragma once
#include <math.h>
#include "actions.h"
#include "position.h"

extern const  bool g_leverageLineTrackers;

struct MoveAction : public Action
{
    MoveAction(int distance, int forward = 85, bool stopOnCollision = false)
        : m_forward(forward),
          m_stopOnCollision(stopOnCollision)
    {
        if (distance > 0)
        {
            m_distanceToMove = distance;
        }
        else
        {
            m_distanceToMove = -distance;
            m_forward = -m_forward;
        }
        
        m_main.drive.ResetTrackingState();
        m_main.drive.OverrideInputs(m_forward, 0/*turn*/);

        // important for m_lastDistance!
        Assert(m_main.drive.m_distance == 0);
    }

    bool ShouldStop() override
    {
        unsigned int distance = m_main.drive.m_distance;

        if (m_stopOnCollision)
        {
            m_speed = (m_speed + (int)distance - (int)m_lastDistance) / 2;
            if (m_maxSpeed < m_speed)
                m_maxSpeed = m_speed;

            // We used the distance of the wheel that travelled the least in determining stop
            // This is the only indicator we have when ball gets under one of the sides and lifts
            // that side, thus allowing wheel on that side to spin freely wihtout robot moving
            // We use the other wheel to figure out that it actully is stuck.
            unsigned int minWheel = m_main.drive.MinWheelDistanceTravelled();

            // Collision?
            if (m_stopOnCollision &&
                ((m_maxSpeed >= speedToGainBeforeStop && m_speed < m_maxSpeed / 2) ||
                (distance >= 100 && minWheel <= m_lastMinWheel+1)))
            {
                // ReportStatus("   Collision detected: speed=%d maxspeed=%d, distance=%d, exp distance=%d\n", m_speed, m_maxSpeed, distance, m_distanceToMove);
                return true;
            }
            m_lastMinWheel = minWheel;
        }

        m_lastDistance = distance;
        if (m_main.drive.m_distance < m_distanceToMove)
            return false;
        /*
        if (m_stopOnCollision)
            ReportStatus("Move did not hit wall: dist = %d, speed=%d maxspeed=%d\n",
                m_distanceToMove, m_speed, m_maxSpeed);
        */
        return true;
    }

    void Stop() override
    {
        m_main.drive.OverrideInputs(0, 0);
    }

  protected:
    const int speedToGainBeforeStop = 10;
    unsigned int m_distanceToMove;
    int m_forward;
    unsigned int m_lastDistance = 0;
    int m_maxSpeed = 0;
    bool m_stopOnCollision;
private:
    int m_speed = 0;
    unsigned int m_lastMinWheel = 0;
};

struct MoveToPlatformAction : public MoveAction
{
    int m_diff = 0;
    int m_diffMax = 0;
    int m_diffMin = 10000; // some big number
    int m_slowCount = 0;
    bool m_fIsLow = false;
    int m_distanceFirstHit = 0;

    MoveToPlatformAction(int distance) : MoveAction(distance, 65) {}

    bool ShouldStop() override
    {
        int distance = m_main.drive.m_distance * 10;
        m_diff = (m_diff + (distance - (int)m_lastDistance)) / 2;

        if (m_diffMax < m_diff)
            m_diffMax = m_diff;
        if (m_diffMin > m_diff)
            m_diffMin = m_diff;

        // ReportStatus("MoveToPlatform: diff = %d/%d, max  = %d, min = %d\n", m_diff, distance - (int)m_lastDistance, m_diffMax, m_diffMin);

        if (!m_fIsLow && m_diff <= m_diffMax - 90)
        {
            // ReportStatus("MoveToPlatform: Slow down: diff = %d/%d, max  = %d, min = %d\n", m_diff, distance - (int)m_lastDistance, m_diffMax, m_diffMin);
            m_diffMin = m_diff;
            m_fIsLow = true;
            if (m_slowCount == 0)
            {
                m_main.drive.OverrideInputs(100, 0);
                m_distanceFirstHit = distance;
            }
        }

        if (m_fIsLow && distance >= m_distanceFirstHit + 1200 * 10)
        {
            // ReportStatus("MoveToPlatform: Stop \n");
            return true;
        }

        m_lastDistance = distance;

        if (m_main.drive.m_distance >= m_distanceToMove)
        {
            ReportStatus("MoveToPlatform: Stop based on disance!\n");
            return true;
        }
        return false;
    }
};

struct MoveTimeBasedAction : public MoveAction
{
    unsigned int m_time;

    MoveTimeBasedAction(int speed, int time, bool stopOnCollision)
        : MoveAction(50000, speed, stopOnCollision),
          m_time(time)
    {
    }

    bool ShouldStop()
    {
        unsigned int timeElapsed = m_main.GetTime() - m_timeStart;

        // We should stop right away if we are not moving!
        // This is different from behaviour of MoveAction, that 
        // expects that it will be able to gain some speed before hitting the wall
        if (timeElapsed > 20)
            m_maxSpeed = speedToGainBeforeStop;

        if (timeElapsed >= m_time)
        {
            ReportStatus("   MoveTimeBased: timed-out: %d\n", m_time);
            return true;
        }

        bool res = MoveAction::ShouldStop();
        if (!res)
            return false;

        ReportStatus("   MoveTimeBased: Time to stop: %d out of %d\n", timeElapsed, m_time);
        return true;
    }
};

struct MoveExactAction : public Action
{
    MoveExactAction(int distance)
        : m_distanceToMove(distance)
    {
        if (m_distanceToMove < 0)
        {
            m_forward = false;
            m_distanceToMove = -m_distanceToMove;
        }
        m_main.drive.ResetTrackingState();
        Assert(m_main.drive.m_distance == 0);
    }

    void Stop() override
    {
        m_main.drive.OverrideInputs(0, 0);
    }

    int IdealSpeedFromDistance(int distance)
    {
        const unsigned int point2 = 1600;
        const unsigned int speed2 = 2;
        const unsigned int point1 = 50;
        const unsigned int speed1 = 50;
        const unsigned int point0 = 30;

        unsigned int distanceAbs = abs(distance);
        int idealSpeed; // gyro ticks per second
        if (distanceAbs > point2)
            idealSpeed = speed1 + speed2 * (point2 - point1);
        else if (distanceAbs > point1)
            idealSpeed = speed1 + speed2 * (distanceAbs - point1);
        else if (distanceAbs > point0)
            idealSpeed = speed1;
        else
            idealSpeed = 0;

        if (distance < 0)
            idealSpeed = -idealSpeed;
        return idealSpeed;
    }

    bool ShouldStop() override
    {
        if (m_distanceToMove <= 27) // 1/2"
            return true;

        unsigned int distance = m_main.drive.m_distance;
        int error = m_distanceToMove - int(distance);

        // positive means counter-clockwise
        PositionInfo info = GetTracker().LatestPosition(false /*clicks*/);
        int actualSpeed = 1000 * (info.leftSpeed + info.rightSpeed);
        int idealSpeed = IdealSpeedFromDistance(error);
        if (!m_forward)
            actualSpeed = -actualSpeed; // make it positive

        if (idealSpeed == 0 && (abs(actualSpeed) <= 18 || error < 0))
        {
            if (abs(error) > 30)
                ReportStatus("MoveExact stop! Error: %d\n", error);
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
        int power = 0;
        if (error < 0 || idealSpeed == 0)
            power = -18 + idealSpeed / 60 + diff / 50; // Stopping!
        else if (idealSpeed != 0)
            power = 18 + idealSpeed * (25 + idealSpeed / 500) / 2000 + diff / 70; // Moving forward

        // If robot stopped, or about to stop (and far from target), then give it a boost
        // Friction is too high for formular above to start moving with right speed - it's structured
        // to be stable around desired speed.
        if (actualSpeed == 0 || (abs(actualSpeed) < 20 && error > 50))
        {
            power += maxSpeed / 2;
        }

        // Start slowly, for better accuracy
        if (distance < 50 && power > 40)
            power = 40;

        if (power > maxSpeed)
            power = maxSpeed;

        // ReportStatus("MoveExact: %d %d %d %d %d\n", error, actualSpeed, idealSpeed, diff, power);

        if (!m_forward)
            power = -power;
        m_power = (power + m_power) / 2;

        m_main.drive.OverrideInputs(m_power, 0);
        return false;
    }

  protected:
    static const int maxSpeed = 85;
    int m_power = 0;
    int m_distanceToMove = 0;
    bool m_forward = true;
};


template<typename TMoveAction>
struct MoveExactWithLineCorrectionAction : public TMoveAction
{
    MoveExactWithLineCorrectionAction(int fullDistance, unsigned int distanceAfterLine, int angle)
        : TMoveAction(fullDistance),
          m_main(GetMain()),
          m_distanceAfterLine(distanceAfterLine),
          m_angle(angle)
    {
        Assert(distanceAfterLine >= 0);
        m_main.lineTrackerLeft.Reset();
        m_main.lineTrackerRight.Reset();
    }

    bool ShouldStop() override
    {
        int shouldHaveTravelled = TMoveAction::m_distanceToMove - (int)m_distanceAfterLine;        

        // Adjust distance based only on one line tracker going over line.
        // This might be useful in cases where second line tracker will never cross the line.
        // Like when driving to climb platform.
        // If we hit line with both trackers, we will re-calibrate distance based on that later on.
        if (!m_adjustedDistance)
        {
            unsigned int distance = 0;
            if (m_main.lineTrackerLeft.HasWhiteLine(shouldHaveTravelled))
            {
                distance = m_main.lineTrackerLeft.GetWhiteLineDistance(false/*pop*/);
                m_adjustedDistance = true;
            }
            if (m_main.lineTrackerRight.HasWhiteLine(shouldHaveTravelled))
            {
                distance = m_main.lineTrackerRight.GetWhiteLineDistance(false/*pop*/);
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
        else if (m_fActive && m_main.lineTrackerLeft.HasWhiteLine(shouldHaveTravelled) && m_main.lineTrackerRight.HasWhiteLine(shouldHaveTravelled))
        {
            m_fActive = false; // ignore any other lines
            unsigned int left = m_main.lineTrackerLeft.GetWhiteLineDistance(true/*pop*/);
            unsigned int right = m_main.lineTrackerRight.GetWhiteLineDistance(true/*pop*/);
            unsigned int distance = (left + right) / 2;
            
            int diff = right - left;
            // if angles are flipped, then m_angle is flipped. SetAngle() will also flip angle to get to real one.
            if (m_main.drive.IsXFlipped())
                diff = -diff;

            float angle = atan2(diff, DistanveBetweenLineSensors * 2); // left & right is double of distanve
            int angleI = angle * 180 / PositionTracker::Pi;

            ReportStatus("Double line correction: travelled: %d, Dist: %d -> %d, angle+: %d\n",
                distance, TMoveAction::m_distanceToMove - int(distance), m_distanceAfterLine, angleI);

            if (g_leverageLineTrackers)
            {
                TMoveAction::m_distanceToMove = m_distanceAfterLine + distance;
                m_main.tracker.SetAngle(m_angle - angleI);
            }
        }

        bool res = TMoveAction::ShouldStop();
        if (res && m_fActive)
        {
            ReportStatus("Line correction did not happen! Max brightness: %d,  %d\n",
                m_main.lineTrackerLeft.MinValue(), m_main.lineTrackerRight.MinValue());
        }
        return res;
    }

protected:
    Main& m_main;
    unsigned int m_distanceAfterLine;
    int m_angle;
    bool m_fActive = true;
    bool m_adjustedDistance = false;
};
