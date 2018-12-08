#pragma once
#include "main.h"
#include "cycle.h"

extern Main* g_main;

#define TurnToCenter() Turn(-90)
#define TurnFromCenter() Turn(90)
// #define TurnToCenter() Move(400, 0, g_lcd.AtonBlueRight ? -50 : 50)
// #define TurnFromCenter() Move(400, 0, g_lcd.AtonBlueRight ? 50 : -50)


struct Action
{
    int m_count;

    virtual void StartCore()
    {
        m_count = g_main->m_count;
        Start();
    }

    virtual void Start() {}
    virtual bool ShouldStop() { return true; }
    virtual void Stop() {}
};

// Action that does nothing.
using NoOp = Action;

using GyroOff = NoOp;
using GyroOn = NoOp;


struct EndOfAction : public Action
{
    void Start() override { printf ("\n*** END AUTONOMOUS ***\n\n"); }
    bool ShouldStop() override  { return false; }
};

struct IntakeUp : public Action
{
    void Start() override { g_main->intake.SetIntakeMotor(-intakeMotorSpeed); }
};

struct IntakeDown : public Action
{
    void Start() override { g_main->intake.SetIntakeMotor(intakeMotorSpeed); }
};

struct IntakeStop : public Action
{
    void Start() override { g_main->intake.SetIntakeMotor(0); }
};

struct Wait : public Action
{
    int m_wait;
    Wait(int wait) : m_wait(wait) {}
    bool ShouldStop() override  { return g_main->m_count >= m_count + m_wait; }
};

struct Move : public Action
{
    int m_distanceToMove, m_forward, m_turn;
    Move(int distance, int forward, int turn = 0)
      : m_distanceToMove(distance),
        m_forward(forward),
        m_turn(turn)
    {
        // You should either turn on spot, or move forward with small turning.
        // Doing something else is likely slower, and less accurate.
        Assert(forward == 0 || abs(forward) > abs(turn));
    }
    void Start() override
    {
        g_main->drive.OverrideInputs(m_forward, m_turn);
        Assert(g_main->drive.m_distance == 0);
    }
    bool ShouldStop() override  { return abs(g_main->drive.m_distance) >= m_distanceToMove; }
    void Stop() override {g_main->drive.OverrideInputs(0, 0);}
};

struct Turn : public Action
{
    Turn(int turn)
        : m_turn(turn * GyroWrapper::Multiplier)
    {
        m_errorLast = -m_turn;
    }

    void Start() override
    {
        m_initialAngle = g_main->gyro.Get();
        g_main->drive.OverrideInputs(0, 0);
    }

    bool ShouldStop() override
    {
        if (m_turn == 0)
            return true;

        // negative for positive turns
        int error = g_main->gyro.Get() - m_initialAngle - m_turn;
        
        // Positive for positive positive turns
        int errorDiff = (error - m_errorLast);
        m_errorLast = error;

        if (abs(error) <= GyroWrapper::Multiplier/2 && abs(errorDiff) <= GyroWrapper::Multiplier/16)
        {
            printf("Turn stop! Error: %d, ErrorDiff: %d\n", error, errorDiff);
            g_main->drive.OverrideInputs(0, 0);
            return true;
        }

        int diffPart = errorDiff * 53;
        int errorPart = 6 * error;

        if (abs(errorDiff) <= GyroWrapper::Multiplier/8 && abs(error) <= GyroWrapper::Multiplier * 5)
        {
            // If we stopped, then we can't start moving.
            // Give it a kick!
            m_integral += error;
        }
        else
            m_integral = 0;

        int speed = (errorPart + diffPart + m_integral) / GyroWrapper::Multiplier;
        
        printf("Error: %d, Speed adj: (%d, %d)  Speed: %d, integral: %d, initial angle: %d\n", error, errorPart, diffPart, speed, m_integral, m_initialAngle);

        const int maxSpeed = 35;
        if (speed > maxSpeed)
            speed = maxSpeed;
        else if (speed < -maxSpeed)
            speed = -maxSpeed;

        g_main->drive.OverrideInputs(0, speed);
        return false;
    }
private:
    int m_turn;
    int m_initialAngle;
    int m_errorLast = 0;
    int m_integral = 0;
};

struct ShooterAngle : public Action
{
private:
    Flag m_flag;
    int m_distanceToShoot;
    bool m_checkPresenceOfBall;
public:
    ShooterAngle(bool hightFlag, int distance, bool checkPresenceOfBall)
      : m_flag(hightFlag ? Flag::High : Flag::Middle),
        m_distanceToShoot(distance),
        m_checkPresenceOfBall(checkPresenceOfBall)
    {
        // we disable checking for ball only for first action - shooting.
        Assert(m_flag != Flag::Loading);
        Assert(!g_main->shooter.IsShooting());
    }
    void Start() override
    {
        if (!m_checkPresenceOfBall || g_main->shooter.BallStatus() != BallPresence::NoBall)
        {
            g_main->shooter.SetFlag(m_flag);
            g_main->shooter.SetDistance(m_distanceToShoot);
            Assert(g_main->shooter.GetFlagPosition() != Flag::Loading); // importatn for next ShootBall action to be no-op
        }
        else
        {
            Assert(!g_main->shooter.IsMovingAngle()); // are we waiting for nothing?
            Assert(g_main->shooter.GetFlagPosition() == Flag::Loading); // importatn for next ShootBall action to be no-op
        }
    }
    bool ShouldStop() override { return !g_main->shooter.IsMovingAngle(); }
};

struct ShootBall : public Action
{
    void Start() override
    {
        Assert(!g_main->shooter.IsShooting());
        if (g_main->shooter.GetFlagPosition() != Flag::Loading)
        {
            g_main->shooter.OverrideSetShooterMode(true/*shooting*/);
            Assert(g_main->shooter.IsShooting());
        }
    }
    bool ShouldStop() override  { return !g_main->shooter.IsShooting(); }
    void Stop() override
    {
        // This should be not needed, but might be needed in the future (if we add safety in the form of time-based shooter off)
        g_main->shooter.SetFlag(Flag::Loading);
    }
};

