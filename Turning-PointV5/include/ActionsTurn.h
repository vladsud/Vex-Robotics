#pragma once
#include "actions.h"
#include "position.h"

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

    void Stop()
    {
        m_main.drive.OverrideInputs(0, 0);
    }

    bool ShouldStop() override
    {
        // 100 points per degree of angle
        static constexpr unsigned int points[] = {100, 101,  600,  600, UINT_MAX};
        static constexpr unsigned int speeds[] = {  0,  12,   12,  240, 240};

        // positive for positive (clock-wise) turns
        int error = m_initialAngle + m_turn - GetGyroReading();
        int sign = Sign(error);

        // positive means counter-clockwise
        int actualSpeed = 1000 * GetTracker().LatestPosition(false /*clicks*/).gyroSpeed;
        int idealSpeed = SpeedFromDistances(error * 100 / GyroWrapper::Multiplier,points, speeds);

        if ((idealSpeed == 0 && abs(actualSpeed) <= 3 * GyroWrapper::Multiplier) || (m_turn * sign < 0 && abs(error) > GyroWrapper::Multiplier / 2))
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
