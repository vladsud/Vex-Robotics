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

Main* g_main;


bool AtonBlueRight = true;
bool AtonFirstPos = false;

bool AtonClimbPlatform = false;


void autonomous()
{
    Action** g_actions = nullptr;
    Main main;
    g_main = &main;

#include "ActionLists.h"

    g_actions = g_actionsNothing;
    if (AtonFirstPos)
    {
        g_actions = g_actionsFirstPos;
        if (AtonClimbPlatform)
            g_actions = g_ParkFromFirstPos;
    }
    else
    {
        if (AtonClimbPlatform)
            g_actions = g_ParkFromSecondPos;
    }



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
            printf("Next\n");
            currentAction++;
            (*currentAction)->StartCore();
        }
	}

    printf("Exit Auto\n");
}
