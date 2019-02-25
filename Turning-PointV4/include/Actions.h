#pragma once
#include "main.h"
#include "cycle.h"
#include "logger.h"

void SetShooterAngle(bool hightFlag, int distance, bool checkPresenceOfBall);
inline void IntakeUp() { GetMain().intake.SetIntakeDirection(Intake::Direction::Up); }
inline void IntakeDown() { GetMain().intake.SetIntakeDirection(Intake::Direction::Down); }
inline void IntakeStop() { GetMain().intake.SetIntakeDirection(Intake::Direction::None); }

struct Action
{
    unsigned int m_timeStart;
    Main &m_main = GetMain();

    Action() { m_timeStart = m_main.GetTime(); }
    virtual bool ShouldStop() { return true; }
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

struct WaitShooterAngleToStopAction : public Action
{
    bool ShouldStop() override { return !m_main.shooter.IsMovingAngle(); }
    const char* Name() override { return "WaitShooterAngleToStopAction"; }
};

struct WaitShooterAngleToGoUpAction : public Action
{
    unsigned int m_wait;
    WaitShooterAngleToGoUpAction(unsigned int maxWait)
        : m_wait(maxWait)
    {
    }

    bool ShouldStop() override
    {
        if (m_main.GetTime() - m_timeStart >= m_wait)
        {
            ReportStatus("   WaitShooterAngleToGoUp: timed out: %d\n", m_wait);
            return true;
        }
        if (m_main.shooter.GetFlagPosition() != Flag::Loading)
        {
            ReportStatus("   WaitShooterAngleToGoUp: got ball: time=%d\n", m_main.GetTime() - m_timeStart);
            return true;
        }
        return false;
    }

    const char* Name() override { return "WaitShooterAngleToGoUpAction"; }
};

struct ShootBallAction : public Action
{
    ShootBallAction()
    {
        Assert(!m_main.shooter.IsShooting());
        if (m_main.shooter.GetFlagPosition() != Flag::Loading)
        {
            m_main.shooter.OverrideSetShooterMode(true /*shooting*/);
            Assert(m_main.shooter.IsShooting());
        }
    }
    bool ShouldStop() override { return !m_main.shooter.IsShooting(); }
    void Stop() override
    {
        // This should be not needed, but might be needed in the future (if we add safety in the form of time-based shooter off)
        m_main.shooter.SetFlag(Flag::Loading);
    }
    const char* Name() override { return "ShootBallAction"; }
};
