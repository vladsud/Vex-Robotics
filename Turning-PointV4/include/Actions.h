#pragma once
#include "main.h"
#include "cycle.h"

extern bool g_manualAuto;
extern bool g_autonomousSmartsOn;

// Positive is counterclockwise
// Commands are from POV of looking forward
// Positive tunr - coutner-clockwise
#define TurnToCenter() Turn(GetMain().lcd.AtonBlueRight ? 90 : -90)
#define TurnFromCenter() Turn(GetMain().lcd.AtonBlueRight ? -90 : 90)
// #define TurnToCenter() Move(400, 0, GetMain().lcd.AtonBlueRight ? -50 : 50)
// #define TurnFromCenter() Move(400, 0, GetMain().lcd.AtonBlueRight ? 50 : -50)


struct Action
{
    int m_timeStart;

    virtual void StartCore()
    {
        m_timeStart = GetMain().GetTime();
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
        // Safety net: If you forgot to reset it,
        // we absolutly do not want to run another copy of atonomous after official one.
        // There is another one in autonomous()
        g_manualAuto = false;
        printf ("\n*** END AUTONOMOUS ***\n\n");
    }
    bool ShouldStop() override  { return false; }
};

struct IntakeUp : public Action
{
    void Start() override { GetMain().intake.SetIntakeMotor(-intakeMotorSpeed); }
};

struct IntakeDown : public Action
{
    void Start() override { GetMain().intake.SetIntakeMotor(intakeMotorSpeed); }
};

struct IntakeStop : public Action
{
    void Start() override { GetMain().intake.SetIntakeMotor(0); }
};

struct Wait : public Action
{
    int m_wait;
    Wait(int wait) : m_wait(wait) {}
    bool ShouldStop() override  { return GetMain().GetTime() - m_timeStart >= m_wait; }
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
        GetMain().drive.ResetEncoders();
        GetMain().drive.OverrideInputs(m_forward, m_turn);
        printf("Move start: %d, %d\n", GetMain().drive.m_distance, m_distanceToMove);
        Assert(GetMain().drive.m_distance == 0);
    }
    bool ShouldStop() override
    {
        // printf("Move: %d\n", GetMain().drive.m_distance);
        return abs(GetMain().drive.m_distance) >= m_distanceToMove;
    }
    void Stop() override
    {
        printf("Move stop\n");
        GetMain().drive.OverrideInputs(0, 0);
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
        m_lastDistance = GetMain().drive.m_distance;
    }

    bool ShouldStop() override
    {
        int distance = GetMain().drive.m_distance;
        m_diff = (m_diff + (distance - m_lastDistance)) / 2;
        // printf("Move: %d,   %d\n", GetMain().drive.m_distance, m_diff);

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

        return abs(GetMain().drive.m_distance) >= m_distanceToMove;
    }
};

struct MoveTimeBased : public Action
{
    int m_speed;
    int m_time;
    MoveTimeBased(int speed, int time) : m_speed(speed), m_time(time) {}
    void Start() override { GetMain().drive.OverrideInputs(m_speed, 0);}
    bool ShouldStop() {  return GetMain().GetTime() - m_timeStart >= m_time; }
    void Stop() override { GetMain().drive.OverrideInputs(0, 0); }
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
        m_initialAngle = GetMain().gyro.Get();
        g_autonomousSmartsOn = false;
        GetMain().drive.OverrideInputs(0, 0);
    }

    bool ShouldStop() override
    {
        if (m_turn == 0)
            return true;

        // negative for positive turns
        int error = GetMain().gyro.Get() - m_initialAngle - m_turn;
        
        // Positive for positive positive turns
        int errorDiff = (error - m_errorLast);
        m_errorLast = error;

        if (abs(error) <= GyroWrapper::Multiplier && abs(errorDiff) <= GyroWrapper::Multiplier/16)
        {
            printf("Turn stop! Error: %d, ErrorDiff: %d\n", error, errorDiff);
            GetMain().drive.OverrideInputs(0, 0);
            g_autonomousSmartsOn = true;
            return true;
        }

        int diffPart = errorDiff * 53;
        int errorPart = 7 * error;

        if (abs(errorDiff) <= GyroWrapper::Multiplier/8 && abs(error) <= GyroWrapper::Multiplier * 5)
        {
            // If we stopped, then we can't start moving.
            // Give it a kick!
            m_integral += 5*error;
        }
        else
            m_integral = 0;

        int speed = (errorPart + diffPart + m_integral) / GyroWrapper::Multiplier;
        
        printf("Error: %d, Speed adj: (%d, %d)  Speed: %d, integral: %d, initial angle: %d\n", error, errorPart, diffPart, speed, m_integral, m_initialAngle);

        const int maxSpeed = 45;
        if (speed > maxSpeed)
            speed = maxSpeed;
        else if (speed < -maxSpeed)
            speed = -maxSpeed;

        GetMain().drive.OverrideInputs(0, speed);
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
        Assert(!GetMain().shooter.IsShooting());
    }
    void Start() override
    {
        if (!m_checkPresenceOfBall || GetMain().shooter.BallStatus() != BallPresence::NoBall)
        {
            GetMain().shooter.SetFlag(m_flag);
            GetMain().shooter.SetDistance(m_distanceToShoot);
            Assert(GetMain().shooter.GetFlagPosition() != Flag::Loading); // importatn for next ShootBall action to be no-op
        }
        else
        {
            Assert(!GetMain().shooter.IsMovingAngle()); // are we waiting for nothing?
            Assert(GetMain().shooter.GetFlagPosition() == Flag::Loading); // importatn for next ShootBall action to be no-op
        }
    }
    bool ShouldStop() override { return !GetMain().shooter.IsMovingAngle(); }
    void Stop() override { printf("Shooter angle stop\n"); }
};

struct ShootBall : public Action
{
    void Start() override
    {
        print("Shoot Ball\n");
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
        print("Shoot Ball: Done\n");
        GetMain().shooter.SetFlag(Flag::Loading);
    }
};

