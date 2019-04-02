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
void MoveToPlatform(bool twhoPlatforms, int angle);
void Do(Action &&action, unsigned int timeout = 100000);
void RunSuperSkills();
void MoveExactWithAngle(int distance, int angle, bool allowTurning = true);
void MoveExactFastWithAngle(int distance, int angle);
void GoToCapWithBallUnderIt(int distance, unsigned int distanceBack, int angle);
void ShootOneBall(bool high, int distance, bool checkBallPresence = true);
void ShootTwoBalls(int distance);
void TurnToFlagsAndShootTwoBalls();
void MoveToLowFlag();
void TurnToAngleIfNeeded(int angle);
void MoveExactWithLineCorrection(int fullDistance, unsigned int distanceAfterLine, int angle);
void HitLowFlagWithRecovery(unsigned int distanceForward, unsigned int distanceBack, int angleBack = 0, int angleForward = 0);
unsigned int HitTheWall(int distanceForward, int angle);
void FlipCap(unsigned int distance, unsigned int distaneBack, int angle);
void FlipCapWithLineCorrection(unsigned int distance, unsigned int afterLine, unsigned int distaneBack, int angle);
void WaitAfterMove();
void WaitAfterMoveReportDistance(int distance);


inline void Wait(unsigned int duration)
{
    Do(WaitAction(duration));
}

inline void MoveExact(int distance, int angle)
{
    Do(MoveExactAction(distance, angle));
    WaitAfterMoveReportDistance(distance);
}

inline void ShootBall()
{
    Do(ShootBallAction());
}

inline void TurnToAngle(int turn)
{
    Do(TurnPrecise(turn * GyroWrapper::Multiplier - GetGyroReading()));
    WaitAfterMove();
}

inline void WaitForBall(unsigned int wait)
{
    Do(WaitForBallAction(wait));
}

inline void WaitShooterAngleToStop(unsigned int maxTime = 1000)
{
    Do(WaitShooterAngleToStopAction(maxTime));
}

inline void MoveStop()
{
    Do(StopAction(), 500 /*timeout*/);
}
