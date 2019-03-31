#pragma once
#include "main.h"
#include "cycle.h"
#include "logger.h"

#include <math.h>
#include <cstdio>

unsigned int SpeedFromDistances(unsigned int distance, const unsigned int* points, const unsigned int* speeds);
int SpeedFromDistances(int distance, const unsigned int* points, const unsigned int* speeds);

void SetShooterAngle(bool hightFlag, int distance);
inline void IntakeUp() { GetMain().intake.SetIntakeDirection(Intake::Direction::Up); }
inline void IntakeDown() { GetMain().intake.SetIntakeDirection(Intake::Direction::Down); }
inline void IntakeStop() { GetMain().intake.SetIntakeDirection(Intake::Direction::None); }

struct Action
{
    unsigned int m_timeStart;
    Main &m_main = GetMain();

    Action() { m_timeStart = m_main.GetTime(); }
    virtual bool ShouldStop() = 0; //{ return true; }
    virtual void Stop() {}
    virtual const char* Name() { return "unknown"; }
};

struct EndOfAction : public Action
{
    bool ShouldStop() override { return false; }
};

struct WaitAction : public Action
{
    unsigned int m_wait;
    WaitAction(unsigned int wait) : m_wait(wait) {}
    bool ShouldStop() override { return m_main.GetTime() - m_timeStart >= m_wait; }
};


struct WaitTillStopsAction : public Action
{
    bool ShouldStop() override
    {
        auto left = abs(GetLeftVelocity());
        auto right = abs(GetRightVelocity());
        return left <= 1 && right <= 1;
    }
};


struct WaitShooterAngleToStopAction : public Action
{
    WaitShooterAngleToStopAction(unsigned int maxTime)
      : m_maxTime(maxTime)
    {}
    bool ShouldStop() override
    {
        if (GetMain().GetTime() - m_timeStart > m_maxTime)
            return true;
         return !m_main.shooter.IsMovingAngle();
    }
    void Stop() override
    {
        ReportStatus("Waited for angle to stop: %d / %d\n", GetMain().GetTime() - m_timeStart, m_maxTime);
    }
    const char* Name() override { return "WaitShooterAngleToStopAction"; }
private:
    unsigned int m_maxTime;
};

struct WaitForBallAction : public Action
{
    unsigned int m_wait;
    WaitForBallAction(unsigned int maxWait)
        : m_wait(maxWait)
    {
    }

    bool ShouldStop() override
    {
        if (m_main.GetTime() - m_timeStart >= m_wait)
        {
            ReportStatus("   WaitForBallAction: timed out: %d\n", m_wait);
            return true;
        }
        if (m_main.shooter.BallStatus() == BallPresence::HasBall)
        {
            ReportStatus("   WaitForBallAction: got ball: time=%d\n", m_main.GetTime() - m_timeStart);
            return true;
        }
        return false;
    }

    const char* Name() override { return "WaitForBallAction"; }
};

struct ShootBallAction : public Action
{
    ShootBallAction()
    {
        Assert(!m_main.shooter.IsShooting());
        if (m_main.shooter.BallStatus() !=  BallPresence::NoBall)
        {
            m_main.shooter.OverrideSetShooterMode(true /*shooting*/);
            Assert(m_main.shooter.IsShooting());
        }
    }
    bool ShouldStop() override { return !m_main.shooter.IsShooting(); }
    const char* Name() override { return "ShootBallAction"; }
};
