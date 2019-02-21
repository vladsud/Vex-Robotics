#pragma once
#include <math.h>
#include "actions.h"
#include "position.h"

struct MoveAction : public Action
{
    MoveAction(int distance, int forward, bool stopOnCollision = false)
        : m_forward(forward),
          m_turn(0),
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
        
        // You should either turn on spot, or move forward with small turning.
        // Doing something else is likely slower, and less accurate.
        Assert(forward == 0 || abs(forward) > abs(m_turn));

        m_main.drive.ResetTrackingState();
        m_main.drive.OverrideInputs(m_forward, m_turn);

        // important for m_lastDistance!
        Assert(m_main.drive.m_distance == 0);
    }

    bool ShouldStop() override
    {
        unsigned int distance = m_main.drive.m_distance;
        if (distance > m_lastDistance)
            m_speed = (m_speed + (distance - m_lastDistance)) / 2;
        if (m_maxSpeed < m_speed)
            m_maxSpeed = m_speed;

        // Collision?
        if (m_stopOnCollision && m_maxSpeed >= 10 && m_speed < m_maxSpeed / 2)
        {
            ReportStatus("   Collision detected: speed=%d maxspeed=%d, distance=%d\n", m_speed, m_maxSpeed, distance);
            return true;
        }

        m_lastDistance = distance;
        return m_main.drive.m_distance >= m_distanceToMove;
    }

    void Stop() override
    {
        m_main.drive.OverrideInputs(0, 0);
    }

  protected:
    unsigned int m_distanceToMove;
    int m_forward;
    float m_turn;
    int m_speed = 0;
    int m_maxSpeed = 0;
    bool m_stopOnCollision;
    unsigned int m_lastDistance = 0;
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
            ReportStatus("MoveToPlatform: Slow down: diff = %d/%d, max  = %d, min = %d\n", m_diff, distance - (int)m_lastDistance, m_diffMax, m_diffMin);
            m_diffMin = m_diff;
            m_fIsLow = true;
            if (m_slowCount == 0)
            {
                m_main.drive.OverrideInputs(95, 0);
                m_distanceFirstHit = distance;
            }
        }

        if (m_fIsLow && distance >= m_distanceFirstHit + 1100 * 10)
        {
            ReportStatus("MoveToPlatform: Stop \n");
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
    unsigned int m_distance = 0;
    bool m_waitForStop;
    bool m_first = true;

    MoveTimeBasedAction(int speed, int time, bool waitForStop)
        : MoveAction(50000, speed),
          m_time(time),
          m_waitForStop(waitForStop)
    {
    }
    bool ShouldStop()
    {
        unsigned int distance = m_main.drive.m_distance;
        unsigned int timeElapsed = m_main.GetTime() - m_timeStart;
        if (m_waitForStop && timeElapsed > 20 && m_distance == distance)
        {
            ReportStatus("MoveTimeBased: Time to stop: %d\n", timeElapsed);
            return true;
        }
        m_distance = distance;
        m_first = false;
        if (timeElapsed >= m_time)
        {
            ReportStatus("MoveTimeBased: timed-out: %d\n", m_time);
            return true;
        }
        return false;
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


struct MoveExactWithLineCorrectionAction : public MoveExactAction
{
    MoveExactWithLineCorrectionAction(int fullDistance, unsigned int distanceAfterLine, int angle)
        : MoveExactAction(fullDistance),
          m_main(GetMain()),
          m_distanceAfterLine(distanceAfterLine),
          m_angle(angle)
    {
        Assert(distanceAfterLine > 0);
        m_main.lineTrackerLeft.Reset();
        m_main.lineTrackerRight.Reset();
    }

    bool ShouldStop() override
    {
        // Adjust distance based only on one line tracker going over line.
        // This might be useful in cases where second line tracker will never cross the line.
        // Like when driving to climb platform.
        // If we hit line with both trackers, we will re-calibrate distance based on that later on.
        if (!m_adjustedDistance)
        {
            int distance = 0;
            if (m_main.lineTrackerLeft.HasWhiteLine())
            {
                distance = m_main.lineTrackerLeft.GetWhiteLineDistance(false/*pop*/);
                m_adjustedDistance = true;
            }
             if (m_main.lineTrackerRight.HasWhiteLine())
            {
                distance = m_main.lineTrackerRight.GetWhiteLineDistance(false/*pop*/);
                m_adjustedDistance = true;
            }
            if (m_adjustedDistance)
            {
                ReportStatus("Line correction: Dist: %d -> %d\n", m_distanceToMove - int(distance), m_distanceAfterLine);
               // m_distanceToMove = m_distanceAfterLine + distance;
            }
        }
        else if (m_fActive && m_main.lineTrackerLeft.HasWhiteLine() && m_main.lineTrackerRight.HasWhiteLine())
        {
            m_fActive = false; // ignore any other lines
            unsigned int left = m_main.lineTrackerLeft.GetWhiteLineDistance(true/*pop*/);
            unsigned int right = m_main.lineTrackerRight.GetWhiteLineDistance(true/*pop*/);
            unsigned int distance = m_main.drive.m_distance;
            
            int diff = right - left;
            if (!m_forward)
                diff = -diff;
            // if angles are flipped, then m_angle is flipped. SetAngle() will also flip angle to get to real one.
            if (m_main.drive.IsXFlipped())
                diff = -diff;

            float angle = atan2(diff, DistanveBetweenLineSensors * 2); // left & right is double of distanve
            int angleI = angle * 180 / PositionTracker::Pi;

            ReportStatus("Line correction: Dist: %d -> %d\n, angle+: %d", m_distanceToMove - int(distance), m_distanceAfterLine, angleI);

            // m_distanceToMove = m_distanceAfterLine + distance;
            // m_main.tracker.SetAngle(m_angle + angleI);

        }

        return MoveExactAction::ShouldStop();
    }

protected:
    Main& m_main;
    unsigned int m_distanceAfterLine;
    int m_angle;
    bool m_fActive = true;
    bool m_adjustedDistance = false;
};
