#pragma once

#include "actions.h"
#include "actionsMove.h"
#include "actionsTurn.h"
#include "main.h"
#include "cycle.h"

void RunAtonFirstPos();
void RunAtonSecondPos();

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
void DoCore(Action&& action);

#ifdef OFFICIAL_RUN
#  define Do(x) DoCore(x)
#else //OFFICIAL_RUN 
#  define Do(x)                                         \
    do {                                                \
        if (PrintDiagnostics(Diagnostics::Autonomous))  \
            puts("Next action started");                \
        DoCore(x);                                      \
    } while(false)
#endif //OFFICIAL_RUN 

