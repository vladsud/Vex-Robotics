#pragma once
#include "actions.h"
#include "position.h"
#include <cmath>

#define Turn(x) TurnPrecise((x)*GyroWrapper::Multiplier)
#define TurnToPoint(x, y) TurnToAngle(CalcAngleToPoint(x, y))

int CalcAngleToPoint(double x, double y);

struct TurnPrecise : public Action
{
    const char* Name() override { return "TurnPrecise"; }

    TurnPrecise(int turn)
        : m_turn(AdjustAngle(turn))
    {
        // ReportStatus("TurnPrecise: %d, curr=%d\n", m_turn / GyroWrapper::Multiplier, GetGyroReading() / GyroWrapper::Multiplier);

        m_main.drive.ResetTrackingState();
        m_initialAngle = GetGyroReading();
    }

    int IdealSpeedFromDistance(int distance)
    {
        const unsigned int point2 = 60 * GyroWrapper::Multiplier;
        const unsigned int speed2 = 4;
        const unsigned int point1 = 6 * GyroWrapper::Multiplier;
        const unsigned int speed1 = 12 * GyroWrapper::Multiplier;
        const unsigned int point0 = GyroWrapper::Multiplier;

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
        if (abs(m_turn) <= GyroWrapper::Multiplier)
            return true;

        // positive for positive (clock-wise) turns
        int error = m_initialAngle + m_turn - GetGyroReading();
        int sign = Sign(error);

        // positive means counter-clockwise
        int actualSpeed = 1000 * GetTracker().LatestPosition(false /*clicks*/).gyroSpeed;
        int idealSpeed = IdealSpeedFromDistance(error);

        if ((idealSpeed == 0 && abs(actualSpeed) <= 3 * GyroWrapper::Multiplier) || (m_turn * sign < 0 && abs(error) > GyroWrapper::Multiplier / 2))
        {
            if (abs(error) >= GyroWrapper::Multiplier)
                ReportStatus("   Turn stop! Error: error=%d turn=%d\n", error, m_turn);
            m_main.drive.OverrideInputs(0, 0);
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
            power = -Sign(actualSpeed) * 12;
        else
            power = sign * 26 + (idealSpeed / 40 + diff / 2) / GyroWrapper::Multiplier;

        // Sometimes we hit the wall and there is not enough power to turn
        // Attempt to fix this condition
        if (errorAbs > 45 * GyroWrapper::Multiplier && abs(actualSpeed) < 5 * GyroWrapper::Multiplier)
        {
            // ReportStatus("   Turn: turning on super charge\n");
            power *= 3;
        }

        int maxSpeed = 100;
        if (errorAbs <= 3 * GyroWrapper::Multiplier)
            maxSpeed = 25;

        if (power > maxSpeed)
            power = maxSpeed;
        else if (power < -maxSpeed)
            power = -maxSpeed;

        m_power = power; // + m_power) / 2;

        // ReportStatus("Turn: %d %d %d %d\n", error, idealSpeed, actualSpeed, power);
        m_main.drive.OverrideInputs(0, -m_power);
        return false;
    }

  protected:
    int m_turn;

  private:
    int m_initialAngle;
    int m_power = 0;
};
