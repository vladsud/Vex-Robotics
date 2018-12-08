#pragma once
#include "main.h"
#include "cycle.h"

extern Main* g_main;
extern bool g_manualAuto;
extern bool g_smartsOn;

// Positive is counterclockwise
// Commands are from POV of looking forward
// Positive tunr - coutner-clockwise
#define TurnToCenter() Turn(g_lcd.AtonBlueRight ? 90 : -90)
#define TurnFromCenter() Turn(g_lcd.AtonBlueRight ? -90 : 90)
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
    void Start() override
    {
        g_manualAuto = false;
        printf ("\n*** END AUTONOMOUS ***\n\n");
    }
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
    int m_distanceToMove, m_forward;
    float m_turn;
    Move(int distance, int forward, float turn = 0)
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
        g_main->drive.ResetEncoders();
        g_main->drive.OverrideInputs(m_forward, m_turn);
        printf("Move start: %d, %d\n", g_main->drive.m_distance, m_distanceToMove);
        Assert(g_main->drive.m_distance == 0);
    }
    bool ShouldStop() override
    {
        // printf("Move: %d\n", g_main->drive.m_distance);
        return abs(g_main->drive.m_distance) >= m_distanceToMove;
    }
    void Stop() override
    {
        printf("Move stop\n");
        g_main->drive.OverrideInputs(0, 0);
    }
};

struct MoveToPlatform : public Move
{
    int m_lastDistance = 0;
    int m_diff = 50;
    int m_slowCount = 0;
    bool m_fIsLow = false;

    MoveToPlatform(int distance, int forward) : Move(distance, forward) {}

    void Start() override
    {
        Move::Start();
        m_lastDistance = g_main->drive.m_distance;
    }

    bool ShouldStop() override
    {
        int distance = g_main->drive.m_distance;
        m_diff = (m_diff + (distance - m_lastDistance)) / 2;
        // printf("Move: %d,   %d\n", g_main->drive.m_distance, m_diff);

        if (m_diff <= 8)
        {
            if (!m_fIsLow)
                printf ("SLOW\n");
            //if (!m_fIsLow && m_slowCount == 1)
            //    return true;
            m_fIsLow = true;
        }
        if (m_fIsLow && m_diff >= 14)
        {
            printf ("NOT SLOW\n");
            m_fIsLow = false;
            m_slowCount++;
            if (m_slowCount == 2)
                return true;
        }
        m_lastDistance = distance;

        return abs(g_main->drive.m_distance) >= m_distanceToMove;
    }
};

struct MoveTimeBased : public Action
{
    int m_speed;
    int m_time;
    MoveTimeBased(int speed, int time) : m_speed(speed), m_time(time) {}
    void Start() override { g_main->drive.OverrideInputs(m_speed, 0);}
    bool ShouldStop() {  return g_main->m_count >= m_time + m_count; }
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
        g_smartsOn = false;
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
            g_smartsOn = true;
            return true;
        }

        int diffPart = errorDiff * 53;
        int errorPart = 6 * error;

        if (abs(errorDiff) <= GyroWrapper::Multiplier/8 && abs(error) <= GyroWrapper::Multiplier * 5)
        {
            // If we stopped, then we can't start moving.
            // Give it a kick!
            m_integral += 2*error;
        }
        else
            m_integral = 0;

        int speed = (errorPart + diffPart + m_integral) / GyroWrapper::Multiplier;
        
        printf("Error: %d, Speed adj: (%d, %d)  Speed: %d, integral: %d, initial angle: %d\n", error, errorPart, diffPart, speed, m_integral, m_initialAngle);

        const int maxSpeed = 40;
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
        printf("Shooter angle start\n");
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
    void Stop() override { printf("Shooter angle stop\n"); }
};

struct ShootBall : public Action
{
    void Start() override
    {
        print("Shoot Ball\n");
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
        print("Shoot Ball: Done\n");
        g_main->shooter.SetFlag(Flag::Loading);
    }
};

