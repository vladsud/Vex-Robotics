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

void DoCore(Action &&action);

#ifdef OFFICIAL_RUN
#define Do(x) DoCore(x)
#else //OFFICIAL_RUN
#define Do(x)                                          \
    do                                                 \
    {                                                  \
        if (PrintDiagnostics(Diagnostics::Autonomous)) \
            puts("Next action started");               \
        DoCore(x);                                     \
    } while (false)
#endif //OFFICIAL_RUN

// All streight movements will follow gyro angle as long as this bject is on thw statck
struct KeepAngle
{
    KeepAngle(int angle) { GetMain().drive.StartTrackingAngle(angle); }
    ~KeepAngle() { GetMain().drive.StopTrackingAngle(); }
};
