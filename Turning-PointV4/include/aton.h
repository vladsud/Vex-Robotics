#pragma once

#include "actions.h"
#include "actionsMove.h"
#include "actionsTurn.h"
#include "main.h"
#include "cycle.h"
#include "atonFirstPos.h"


#define BLOCK

void RunAtonFirstPos();
void RunAtonSecondPos();
void MoveToPlatform(bool twhoPlatforms);

enum class AtonMode
{
    Regular,
    Skills,
#ifndef OFFICIAL_RUN
    TestRun,
    ManualAuto,
    ManualSkill,
#endif
};
extern AtonMode g_mode;

void Do(Action &&action);

void RunSuperSkills();

void MoveExactWithAngle(int distance, int angle);
void MoveWithAngle(int distance, int angle, int speed = 85);
void GoToCapWithBallUnderIt(int distance = 0);
void ShootTwoBalls(int midFlagHeight = g_midFlagHeight, int highFlagHeight = g_highFlagHeight);
void GetBallUnderCapAndReturn();
void TurnToFlagsAndShootTwoBalls();
void MoveToLowFlag();
void TurnToAngleIfNeeded(int angle);
void Do(Action &&action);

inline void Move(int distance, int forward = 85, bool stopOnCollision = false)
{
    Do(MoveAction(distance, forward, stopOnCollision));
}

inline void MoveExact(int distance)
{
    Do(MoveExactAction(distance));
}

inline void MoveTimeBased(int speed, int time, bool waitForStop)
{
    Do(MoveTimeBasedAction(speed, time, waitForStop));
}

inline void ShootBall()
{
    Do(ShootBallAction());
}

inline void Wait(unsigned int duration)
{
    Do(WaitAction(duration));
}

inline void TurnToAngle(int turn)
{
    Do(TurnPrecise(turn * GyroWrapper::Multiplier - GetGyroReading()));
}

inline void WaitShooterAngleTo()
{
    Do(WaitShooterAngleToStopAction());
}

inline void WaitShooterAngleToGoUp(unsigned int wait)
{
    Do(WaitShooterAngleToGoUpAction(wait));
}

inline void WaitShooterAngleToStop()
{
    Do(WaitShooterAngleToStopAction());
}