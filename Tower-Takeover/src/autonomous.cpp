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

#include "aton.h"
#include "actionsMove.h"
#include "main.h"
#include "actions.h"
#include "drive.h"
#include "cycle.h"
#include "forwards.h"

#include "pros/misc.h"

using namespace pros::c;

// #define OFFICIAL_RUN

enum class AtonMode
{
    Regular,
    SkillsInManual,
#ifndef OFFICIAL_RUN
    TestRun,
#endif
};

AtonMode g_mode = AtonMode::Regular;

const  bool g_leverageLineTrackers = true;

// Variables not to touch - control actual autonomous mode
const bool g_autonomousSmartsOn = true;
const bool g_manualSmarts = false;

bool g_alreadyRunAutonomous = false;


bool isAuto()
{
#ifndef OFFICIAL_RUN
    if (g_mode == AtonMode::TestRun)
        return true;
#endif // OFFICIAL_RUN
    if (g_mode == AtonMode::SkillsInManual)
        return true;
    return competition_is_autonomous();
}


bool SmartsOn()
{
    // if we enable smarts in manual, then then have to be on in autonomous
    Assert(!g_manualSmarts || g_autonomousSmartsOn);
    if (g_manualSmarts)
        return true;
    return g_autonomousSmartsOn && isAuto();
}

bool ShouldBailOutOfAutonomous()
{
    return (g_mode == AtonMode::Regular && !competition_is_autonomous());
}

// Scans digital buttons on joystick
bool joystickGetDigital(pros::controller_id_e_t id, pros::controller_digital_e_t button)
{
    bool result = (controller_get_digital(id, button) == 1);

    // if we are running autonous code in non-autonomous mode, then allow user to bail out.
    // this is very useful to run autonomous skills in manual skills mode.
    if (result && g_mode != AtonMode::Regular)
    {
        ReportStatus(Log::Warning, "\n!!! Cancelling autonomous mode, running opControl()!\n");
        Assert(isAuto());
        Assert(!competition_is_autonomous());
        g_mode = AtonMode::Regular;
        Assert(!isAuto());
    }
    return result;
}

struct EndOfAction : public Action
{
    bool ShouldStop() override { return false; }
};

void autonomous()
{
    ReportStatus(Log::Info, "Main Battery Level: %.2f\n" , battery_get_capacity());

    // Safety net: run autonomous only once!
    // In case manual auto was still in place when running on competition.
    if (g_alreadyRunAutonomous && !competition_is_autonomous())
    {
        ReportStatus(Log::Error, "\n!!! Safety: running second aton, switching to manaul mode!\n");
        g_mode = AtonMode::Regular;
        return;
    }

    g_alreadyRunAutonomous = true;

#ifndef OFFICIAL_RUN
    if (g_mode == AtonMode::TestRun)
        delay(4000);
#endif

    ReportStatus(Log::Info, "\n*** Autonomous: Start ***\n\n");

    Main &main = SetupMain();
    auto time = main.GetTime();
    auto time2 = millis();

    // all system update their counters, like distance counter.
    main.ResetState();

    auto &lcd = main.lcd;

    if (g_mode == AtonMode::SkillsInManual)
        lcd.AtonSkills = true;

    if (lcd.AtonSkills)
    {
        lcd.AtonRed = false;
        lcd.AtonProtected = false;
    }

    // setup coordinates
    main.tracker.FlipX(main.lcd.AtonRed);
    main.drive.FlipX(main.lcd.AtonRed);
    //main.vision.SetFlipX(main.lcd.AtonRed);

#ifndef OFFICIAL_RUN
    // Debugging code - should not run in real autonomous
    if (g_mode == AtonMode::TestRun && !competition_is_autonomous())
    {
        // lcd.AtonClimbPlatform = false;
        // lcd.AtonProtected = false;

        RunAtonProtected();
        // RunAtonUnprotected();
        // RunSuperSkills();
    }
    else
#endif // !OFFICIAL_RUN
    {
        // if you remove this (super skills) to run old skills, fix lcd.AtonProtected = true above!!!
        if (lcd.AtonSkills)
            RunSuperSkills();
        else if (lcd.AtonProtected)
            RunAtonProtected();
        else
            RunAtonUnprotected();
    }
    // unused variables in final build
    UNUSED_VARIABLE(time);
    UNUSED_VARIABLE(time2);

    ReportStatus(Log::Info, "*** END AUTONOMOUS ***\n\n");
    // ReportStatus(Log::Info, "Time: %d %d \n", main.GetTime() - time, int(millis() - time2));

    Do(EndOfAction());
}

void WaitAfterMoveReportDistance(int distance, bool success, unsigned int timeout)
{
    if (success)
        WaitAfterMove(timeout);
    unsigned int error = abs(distance - GetDrive().GetDistance());
    if (error >= 50 || !success)
        ReportStatus(Log::Warning, "MoveExact (or equivalent) big Error: %d, distance travelled: %d, expected: %d\n", error, GetDrive().GetDistance(), distance);
}
