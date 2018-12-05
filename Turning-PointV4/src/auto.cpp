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

LCD g_lcd;
Main* g_main;

Action* g_actions[100];
size_t g_actionSize = 0;

bool testAgles = true;

#define AddActions(actions) do {\
    memmove((char*)(g_actions + g_actionSize), (char*)actions, sizeof(actions)); \
    g_actionSize += CountOf(actions); \
} while (false)

void autonomous()
{
    Main main;
    g_main = &main;    
    g_actionSize = 0;

// This brings all key actions.
// It has to be part of function, not global scope, dur to Pros not initializing static/global variables
#include "ActionLists.h"

    if (g_lcd.AtonFirstPos)
    {
        AddActions(g_actionsFirstPos);
        if (g_lcd.AtonClimbPlatform)
            AddActions(g_ParkFromFirstPos);
        else
            AddActions(g_knockConeFirstPos);
    }
    else
    {
        if (g_lcd.AtonShootHighFlag)
            AddActions(g_ShootFromSecondPos);
        if (g_lcd.AtonClimbPlatform)
            AddActions(g_ParkFromSecondPos);
        else
            AddActions(g_knockConeSecondPos);
    }

    // Debugging code - should not run in real autonomous
    if (testAgles && !isAutonomous())
    {
        g_actionSize = 0;
        AddActions(g_testAngles);
    }


    g_actions[g_actionSize] = new EndOfAction();
    // all system update their counters, like distance counter.
	main.Update();

    Action** currentAction = g_actions;
    (*currentAction)->StartCore();

	while (true)
	{
		main.Update();
		delay(10);

        while ((*currentAction)->ShouldStop())
        {
            (*currentAction)->Stop();
            printf("\nNext\n\n");
            currentAction++;
            (*currentAction)->StartCore();
        }
	}

    printf("Exit Auto\n");
}
