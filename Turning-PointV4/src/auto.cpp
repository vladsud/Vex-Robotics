//Created by Jason Zhang, Feburary 9, 2018
/** @file auto.c
 * @brief File for autonomous code
 *
 * This file should contain the user autonomous() function and any functions related to it.
 *
 * Any copyright is dedicated to the Public Domain.
 * http://creativecommons.org/publicdomain/zero/1.0/
 *
 * PROS contains FreeRTOS (http://www.freertos.org) whose source code may be
 * obtained from http://sourceforge.net/projects/freertos/files/ or on request.
 */

#include "cycle.h"
#include "actions.h"


// *** WARNING ***
// Always define it for competation!!!! 
// #define OFFICIAL_RUN

bool g_manualAuto = false;

bool g_testRun = true;
bool g_autonomousSmartsOn = true;
bool g_manualSmarts = true;

bool PrintDiagnostics(Diagnostics diag)
{
#ifndef OFFICIAL_RUN
    switch (diag)
    {
        case Diagnostics::General:
            return true;
        case Diagnostics::Position:
            // return true;
        default:
            return false;
    }
#else // OFFICIAL_RUN
    return false;
#endif
}

bool isAuto()
{
#ifndef OFFICIAL_RUN
    if (g_manualAuto)
	    return true;
#endif // OFFICIAL_RUN
    return isAutonomous();
}

bool SmartsOn()
{
#ifndef OFFICIAL_RUN
    if (g_manualSmarts)
        return true;
#endif
    return g_autonomousSmartsOn && isAuto();
}

bool g_runAutonomous = false;
Action* g_actions[100];
size_t g_actionSize = 0;

#define AddActions(actions) do {\
    memmove((char*)(g_actions + g_actionSize), (char*)actions, sizeof(actions)); \
    g_actionSize += CountOf(actions); \
} while (false)

void autonomous()
{
    // Safety net: run autonomous only once!
    // Siable second run, in case manual auto was still in place when running on competition.
	if (g_runAutonomous)
    {
		g_manualAuto = false;
        return;
    }
    g_runAutonomous = true;
    if (!isAutonomous())
		delay(2000);

    if (PrintDiagnostics(Diagnostics::Autonomous))
        printf("\n*** Autonomous: Start ***\n\n");

    Main& main = GetMain();
    g_actionSize = 0;

// This brings all key actions.
// It has to be part of function, not global scope, dur to Pros not initializing static/global variables
#include "ActionLists.h"

    if (GetMain().lcd.AtonFirstPos)
    {
        AddActions(g_actionsFirstPos);
        if (GetMain().lcd.AtonClimbPlatform)
            AddActions(g_ParkFromFirstPos);
        else
            AddActions(g_knockConeFirstPos);
    }
    else
    {
        AddActions(g_ShootFromSecondPos);
        if (GetMain().lcd.AtonClimbPlatform)
            AddActions(g_ParkFromSecondPos);
        else
            AddActions(g_knockConeSecondPos);
    }

#ifndef OFFICIAL_RUN
    // Debugging code - should not run in real autonomous
    if (g_testRun && !isAutonomous())
    {
        g_actionSize = 0;
        AddActions(g_testDrive); // g_testAngles
    }
#endif // !OFFICIAL_RUN

    g_actions[g_actionSize] = new EndOfAction();

    // all system update their counters, like distance counter.
	main.UpdateWithoutWaiting();

    if (PrintDiagnostics(Diagnostics::Autonomous))
        printf("Auto: First \n\n");

    Action** currentAction = g_actions;
    (*currentAction)->StartCore();

	while (true)
	{
		main.Update();

        while ((*currentAction)->ShouldStop())
        {
            (*currentAction)->Stop();
            if (PrintDiagnostics(Diagnostics::Autonomous))
                printf("\nAuto: Next\n\n");
            currentAction++;
            (*currentAction)->StartCore();
        }
	}

    if (PrintDiagnostics(Diagnostics::Autonomous))
        printf("\n *** Auto: Exit ***\n\n");
}
