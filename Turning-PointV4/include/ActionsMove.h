#pragma once
#include "actions.h"
#include "position.h"

struct Move : public Action
{
    Move(unsigned int distance, int forward, float turn = 0, bool stopOnCollision = false)
      : m_distanceToMove(distance),
        m_forward(forward),
        m_turn(turn),
        m_stopOnCollision(stopOnCollision)
    {
        // You should either turn on spot, or move forward with small turning.
        // Doing something else is likely slower, and less accurate.
        Assert(forward == 0 || abs(forward) > abs(turn));

        GetMain().drive.ResetTrackingState();
        GetMain().drive.OverrideInputs(m_forward, m_turn);
        
        // important for m_lastDistance!
	    Assert(GetMain().drive.m_distance == 0);
    }

    bool ShouldStop() override
    {
        unsigned int distance = GetMain().drive.m_distance;
	    if (distance > m_lastDistance)
            m_speed = (m_speed + (distance - m_lastDistance)) / 2;
        if (m_maxSpeed < m_speed)
            m_maxSpeed = m_speed;
        
        // Collision?
        if (m_stopOnCollision && m_maxSpeed >= 10 && m_speed < m_maxSpeed / 2)
        {
            ReportStatus("Collision detected: %d %d\n", m_speed, m_maxSpeed);
            return true;
        }

        m_lastDistance = distance;
        return GetMain().drive.m_distance >= m_distanceToMove;
    }

    void Stop() override
    {
        GetMain().drive.OverrideInputs(0, 0);
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

struct MoveToPlatform : public Move
{
    int m_diff = 0;
    int m_diffMax = 0;
    int m_diffMin = 10000; // some big number
    int m_slowCount = 0;
    bool m_fIsLow = false;

    MoveToPlatform(int distance, int forward) : Move(distance, forward) {}

    bool ShouldStop() override
    {
        int distance = GetMain().drive.m_distance;
        m_diff = (m_diff + (distance - (int)m_lastDistance)) / 2;
	if (m_diffMax < m_diff)
	    m_diffMax = m_diff;

	if (GetMain().GetTime() - m_timeStart >= 200)
	{
	    if (m_diff <= m_diffMax / 2 - 2)
            {
	        if (m_diffMin > m_diff)
	            m_diffMin = m_diff;
	        if (!m_fIsLow)
		    ReportStatus("MoveToPlatform: Slow down: dist=%d diff=%d max=%d min=%d\n", distance, m_diff, m_diffMax, m_diffMin);
            m_fIsLow = true;
            }
	}
        if (m_fIsLow && (/*m_diff >= m_diffMax * 2 / 3 + 2 ||*/ m_diff >= m_diffMin * 3 / 2 + 7))
        {
	    ReportStatus("MoveToPlatform: Speed up: dist=%d diff=%d >= min(%d, %d), max=%d, min=%d\n",
			distance, m_diff, m_diffMax * 2 / 3 + 2, m_diffMin * 3 / 2 + 7, m_diffMax, m_diffMin);
            m_fIsLow = false;
            m_slowCount++;
	    m_diffMax = m_diff;
            if (m_slowCount == 2)
	    {
	        ReportStatus("MoveToPlatform: Stop (speed up): dist=%d diff=%d max=%d min=%d\n", distance, m_diff, m_diffMax, m_diffMin);
                return true;
	    }
        }
        m_lastDistance = distance;

	// sfety net - did we miss the platform?
	if (m_slowCount == 0 && !m_fIsLow && distance > 2300 && m_diff >= m_diffMax - 2)
	{
	    ReportStatus("MoveToPlatform: Safetyy stop!\n");
	    return true;
	}
	ReportStatus("MoveToPlatform: dist = %d, diff = %d, max  = %d, min = %d\n", distance, m_diff, m_diffMax, m_diffMin);

        if (GetMain().drive.m_distance >= m_distanceToMove)
	{
	    ReportStatus("MoveToPlatform: Stop based on disance!\n");
	    return true;
	}
	return false;
    }
};

struct MoveTimeBased : public Action
{
    int m_speed;
    unsigned int m_time;
    MoveTimeBased(int speed, int time)
	: m_speed(speed),
          m_time(time)
    {
	GetMain().drive.ResetTrackingState();
        GetMain().drive.OverrideInputs(m_speed, 0);
    }
    bool ShouldStop() {  return GetMain().GetTime() - m_timeStart >= m_time; }
    void Stop() override { GetMain().drive.OverrideInputs(0, 0); }
}; 

struct MoveExact : public Action
{    
    MoveExact(int distance)
      : m_distanceToMove(distance)
    {
        if (m_distanceToMove < 0)
        {
            m_forward = false;
            m_distanceToMove = -m_distanceToMove;
        }
        GetMain().drive.ResetTrackingState();
        Assert(GetMain().drive.m_distance == 0);
    }

    void Stop() override
    {
        GetMain().drive.OverrideInputs(0, 0);
    }

    unsigned int IdealSpeedFromDistance(int distance)
    {
        const int point2 = 1600;
        const int speed2 = 2;
        const int point1 = 50;
        const float speed1 = 50;
        const int point0 = 30;

        unsigned int distanceAbs = abs(distance);
        int idealSpeed; // gyro ticks per second
        if (distanceAbs > point2)
            idealSpeed = speed1 + speed2 * (point2-point1);
        else if (distance > point1)
            idealSpeed = speed1 + speed2 * (distanceAbs-point1);
        else if (distance > point0)
            idealSpeed = speed1;
        else
            idealSpeed = 0;

        if (distanceAbs < 0)
            idealSpeed = -idealSpeed;
        return idealSpeed;
    }

    bool ShouldStop() override
    {
        if (m_distanceToMove <= 27) // 1/2"
            return true;

	unsigned int distance = GetMain().drive.m_distance;
        int error = m_distanceToMove - int(distance);

        // positive means counter-clockwise
        PositionInfo info = GetTracker().LatestPosition(false /*clicks*/);
        int actualSpeed = 1000 * (info.leftSpeed + info.rightSpeed);
        if (!m_forward)
            actualSpeed = -actualSpeed;
        int idealSpeed = IdealSpeedFromDistance(error);

        if (idealSpeed == 0 && abs(actualSpeed) <= 18)
        {
            ReportStatus("MoveExact stop! Error: %d\n", error);
            GetMain().drive.OverrideInputs(0, 0);
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
        if (error < 0)
            power = -18 + idealSpeed / 60 + diff / 50;  // Stopping!
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

        // ReportStatus("MoveExact: %d %d %d %d\n", error, idealSpeed, actualSpeed, power);

        if (!m_forward)
            power = - power;
        m_power = (power + m_power) / 2;

        GetMain().drive.OverrideInputs(m_power, 0);
        return false;
    }

private:
    static const int maxSpeed = 85;
    int m_power = 0;
    int m_distanceToMove = 0;
    bool  m_forward = true;
};
