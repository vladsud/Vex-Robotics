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
bool Do(Action &&action, unsigned int timeout = 100000);
bool Do(Action &action, unsigned int timeout = 100000);
void RunSuperSkills();
void MoveExactWithAngle(int distance, int angle, bool allowTurning = true);
void MoveExactFastWithAngle(int distance, int angle, bool stopOnHit = false);
void GoToCapWithBallUnderIt(int distance, unsigned int distanceBack, int angle, int angleBack);
void ShootOneBall(bool high, int distance, unsigned int extraDelay = 0, bool visionMove = false, bool visionAngle = false);
void ShootTwoBalls(int distance, bool visionMove = false, bool visionAngle = false);
void TurnToFlagsAndShootTwoBalls();
void MoveToLowFlag();
void TurnToAngleIfNeeded(int angle);
void MoveExactWithLineCorrection(int fullDistance, unsigned int distanceAfterLine, int angle);
void HitLowFlagWithRecovery(unsigned int distanceForward, unsigned int distanceBack, int angleBack = 0, int angleForward = 0);
unsigned int HitTheWall(int distanceForward, int angle);
void FlipCap(unsigned int distance, unsigned int distaneBack, int angle);
void FlipCapWithLineCorrection(unsigned int distance, unsigned int afterLine, unsigned int distaneBack, int angle);
void WaitAfterMove(unsigned int timeout = 0);
void WaitAfterMoveReportDistance(int distance, unsigned int timeout = 0);


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
    Do(ShootBallAction(), 600);
}

inline void TurnToAngle(int turn)
{
    const auto mult = GyroWrapper::Multiplier;
    auto angle = turn * mult - GetGyroReading();
    Do(TurnPrecise(angle));
    WaitAfterMove();
    if (abs(turn * mult - GetGyroReading()) >= mult/2)
        ReportStatus("!!! Turn Error: (x10) curr angle = %d, desired angle = %d\n", GetGyroReading() * 10 / mult, 10 * turn);
}

inline void WaitForBall(unsigned int wait)
{
    Do(WaitForBallAction(), wait);
}

inline void WaitShooterAngleToStop(unsigned int maxTime = 1000)
{
    Do(WaitShooterAngleToStopAction(), maxTime);
}

inline void MoveStop()
{
    Do(StopAction(), 500 /*timeout*/);
}
