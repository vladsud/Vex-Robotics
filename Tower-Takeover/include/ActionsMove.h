#pragma once
#include "main.h"

float AdjustAngle(float);

void MoveStop();
void MoveExactWithAngle(
    int distance,
    int angle,
    unsigned int speedLimit = UINT_MAX,
    unsigned int timeout = 1000000,
    bool stopOnCollision = false);
int HitTheWall(int distanceForward, int angle);
void MoveExactWithLineCorrection(int fullDistance, unsigned int distanceAfterLine, int angle);

void TurnToAngle(int turn);
void TurnToAngleIfNeeded(int angle);

void WaitAfterMoveReportDistance(int distance, bool success = true, unsigned int timeout = 0);
void MoveStraight(int distance, int power, int angle);

void MoveExactWithAngleAndTray(
    int distance,
    int angle,
    int timeUntil,
    unsigned int speedLimit,
    unsigned int timeout /*= 100000U*/);
