#pragma once

#include "actions.h"
#include "actionsMove.h"
#include "actionsTurn.h"
#include "main.h"
#include "cycle.h"

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
#endif
};
extern AtonMode g_mode;

void Do(Action &&action);

void RunSuperSkills();

void MoveExactWithAngle(int distance, int angle);
void MoveWithAngle(int distance, int angle, int speed = 85);
void GoToCapWithBallUnderIt(int distance = 0);
void ShootTwoBalls();
void GetBallUnderCapAndReturn();
void TurnToFlagsAndShootTwoBalls();
void MoveToLowFlag();
void TurnToAngleIfNeeded(int angle);
void Do(Action &&action);
