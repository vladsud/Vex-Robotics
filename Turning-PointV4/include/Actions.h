#pragma once
#include "main.h"
#include "cycle.h"
#include "logger.h"

// Positive is counterclockwise
// Commands are from POV of looking forward
// Positive turn - coutner-clockwise
#define TurnToCenter() TurnToAngle(-90)
// #define TurnToCenter() Turn(-90)

void ShooterSetAngle(bool hightFlag, int distance, bool checkPresenceOfBall);

struct Action
{
    unsigned int m_timeStart;

    Action() { m_timeStart = GetMain().GetTime(); }
    virtual bool ShouldStop() { return true; }
    virtual void Stop() {}
};

// Action that does nothing.
using NoOp = Action;


struct EndOfAction : public Action
{
    EndOfAction()
    {
        ReportStatus("\n*** END AUTONOMOUS ***\n\n");
        ReportStatus ("Time: %d\n", m_timeStart);
        GetLogger().Dump();
    }
    bool ShouldStop() override  { return false; }
};

struct IntakeUp : public Action
{
    IntakeUp() { GetMain().intake.SetIntakeDirection(Intake::Direction::Up); }
};

struct IntakeDown : public Action
{
    IntakeDown() { GetMain().intake.SetIntakeDirection(Intake::Direction::Down); }
};

struct IntakeStop : public Action
{
    IntakeStop() { GetMain().intake.SetIntakeDirection(Intake::Direction::None); }
};

struct Wait : public Action
{
    unsigned int m_wait;
    Wait(int wait) : m_wait(wait) {}
    bool ShouldStop() override  { return GetMain().GetTime() - m_timeStart >=  m_wait; }
};

struct WaitShooterAngleToStop : public Action
{
    bool ShouldStop() override { return !GetMain().shooter.IsMovingAngle(); }
};

struct WaitShooterAngleToGoUp : public Action
{
    unsigned int m_wait;
    WaitShooterAngleToGoUp(unsigned int maxWait)
        : m_wait(maxWait)
    {
    }

    bool ShouldStop() override
    {
        if (GetMain().GetTime() - m_timeStart >=  m_wait)
            return true;
        auto& shooter = GetMain().shooter;
        return !shooter.IsMovingAngle() && shooter.GetFlagPosition() != Flag::Loading;
    }
};

struct ShooterAngle : public WaitShooterAngleToStop
{
    ShooterAngle(bool hightFlag, int distance, bool checkPresenceOfBall)
    {
        ShooterSetAngle(hightFlag, distance, checkPresenceOfBall);
    }
};

struct ShootBall : public Action
{
    ShootBall()
    {
        Assert(!GetMain().shooter.IsShooting());
        if (GetMain().shooter.GetFlagPosition() != Flag::Loading)
        {
            GetMain().shooter.OverrideSetShooterMode(true/*shooting*/);
            Assert(GetMain().shooter.IsShooting());
        }
    }
    bool ShouldStop() override  { return !GetMain().shooter.IsShooting(); }
    void Stop() override
    {
        // This should be not needed, but might be needed in the future (if we add safety in the form of time-based shooter off)
        GetMain().shooter.SetFlag(Flag::Loading);
    }
};

