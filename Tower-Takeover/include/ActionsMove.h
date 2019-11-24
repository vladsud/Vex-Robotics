#pragma once

void MoveStop();
void MoveWithFixedPower(int distance, int power);
void MoveExact(int distance, int angle);
void MoveExactFastWithAngle(int distance, int angle, bool stopOnHit = false);
void MoveExactWithAngle(int distance, int angle, bool allowTurning = true);
unsigned int HitTheWall(int distanceForward, int angle);
void MoveExactWithLineCorrection(int fullDistance, unsigned int distanceAfterLine, int angle);

void TurnToAngle(int turn);
int CalcAngleToPoint(double x, double y);
#define TurnToPoint(x, y) TurnToAngle(CalcAngleToPoint(x, y))
void TurnToAngleIfNeeded(int angle);

void WaitAfterMoveReportDistance(int distance, unsigned int timeout = 0);
void MoveStreight(int distance, int power, int angle);
