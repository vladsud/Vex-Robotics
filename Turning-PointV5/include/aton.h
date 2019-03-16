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
void Do(Action &&action);
void RunSuperSkills();
void MoveExactWithAngle(int distance, int angle, bool allowTurning = true);
void MoveWithAngle(int distance, int angle, int speed = 85);
void GoToCapWithBallUnderIt(int distance, int angle = -90);
void ShootOneBall(bool high, int distance, bool checkBallPresence = true);
void ShootTwoBalls(int highFlagDistance, int midFlagDistance);
void GetBallUnderCapAndReturn();
void TurnToFlagsAndShootTwoBalls();
void MoveToLowFlag();
void TurnToAngleIfNeeded(int angle);
void Do(Action &&action);
void MoveExactWithLineCorrection(int fullDistance, unsigned int distanceAfterLine, int angle);
void MoveWithLineCorrection(int fullDistance, unsigned int distanceAfterLine, int angle);

unsigned int HitTheWall(int distanceForward, int angle);

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

inline void WaitShooterAngleToGoUp(unsigned int wait)
{
    Do(WaitShooterAngleToGoUpAction(wait));
}

inline void WaitShooterAngleToStop(unsigned int maxTime = 1000)
{
    Do(WaitShooterAngleToStopAction(maxTime));
}

inline void MoveStop(int power)
{
    MoveTimeBased(power, 500, true /*waitForStop*/);
}
