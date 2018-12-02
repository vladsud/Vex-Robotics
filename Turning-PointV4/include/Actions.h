#pragma once
#include "main.h"
#include "cycle.h"

#define TurnToCenter() Move(0, AtonBlueRight ? -50 : 50, 200)
#define TurnFromCenter() Move(0, AtonBlueRight ? 50 : -50, 200)

extern Main* g_main;
extern bool AtonBlueRight;

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

struct ActionNeverEnding : public Action
{
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
    Move(int distance, int forward, int turn = 0) : m_distanceToMove(distance), m_forward(forward), m_turn(turn) {}
    void Start() override { g_main->drive.OverrideInputs(m_forward, m_turn); }
    bool ShouldStop() override  { return g_main->drive.m_distance >= m_distanceToMove; }
};

struct ShooterAngle : public Action
{
    Flag m_flag;
    int m_distanceToShoot;
    ShooterAngle(Flag flag, int distance) : m_flag(flag), m_distanceToShoot(distance) {}
    void Start() override
    {
        g_main->shooter.SetFlag(m_flag);
        g_main->shooter.SetDistance(m_distanceToShoot);
    }
};

struct ShootBall : public Action
{
    void Start() override { motorSet(shooterPort, shooterMotorSpeed); }
    bool ShouldStop() override  { return g_main->m_count >= 200; }
    void Stop() override
    {
        motorSet(shooterPort, 0);
        g_main->shooter.SetFlag(Flag::Loading);
    }
};

