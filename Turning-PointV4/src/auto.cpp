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


AtonMode g_mode = AtonMode::Regular;

// Variables not to touch - control actual autonomous mode
bool g_autonomousSmartsOn = true;
bool g_manualSmarts = false;
bool g_alreadyRunAutonomous = false;


bool isAuto()
{
#ifndef OFFICIAL_RUN
    if (g_mode == AtonMode::ManualAuto || g_mode == AtonMode::TestRun)
            return true;
#endif // OFFICIAL_RUN
    return isAutonomous();
}

bool SmartsOn()
{
    // if we enable smarts in manual, then then have to be on in autonomous
    Assert(!g_manualSmarts || g_autonomousSmartsOn);
    if (g_manualSmarts)
        return true;
    return g_autonomousSmartsOn && isAuto();
}

void SetSkillSelection(bool skills)
{
    if (skills)
        g_mode = AtonMode::Skills;
    else
        g_mode = AtonMode::Regular;
}

void DoCore(Action&& action)
{
    while (!action.ShouldStop())
    {
        GetMain().Update();
    }
    action.Stop();
}


void autonomous()
{
#ifndef OFFICIAL_RUN
    // Safety net: run autonomous only once!
    // Siable second run, in case manual auto was still in place when running on competition.
    if (g_alreadyRunAutonomous && !isAutonomous())
    {
        g_mode = AtonMode::Regular;
        return;
    }
    g_alreadyRunAutonomous = true;
    if (!isAutonomous())
        delay(2000);
#endif

    ReportStatus("\n*** Autonomous: Start ***\n\n");
    ReportStatus ("Time: %d\n", GetMain().GetTime());

    Main& main = SetupMain();

    // all system update their counters, like distance counter.
    main.UpdateAllSystems();

#ifndef OFFICIAL_RUN
    // Debugging code - should not run in real autonomous
    if (g_mode == AtonMode::TestRun && !isAutonomous())
    {
        Do(MoveToPlatform(3650, 85));
        Do(MoveTimeBased(30, 100));
        ReportStatus("Second platform\n");
        Do(MoveToPlatform(3350, 85));
        Do(MoveTimeBased(40, 200));
        Do(MoveTimeBased(-30, 100));
        Do(EndOfAction());
        return;
    }
#endif // !OFFICIAL_RUN

    if (g_mode == AtonMode::Skills)
    {
        auto& lcd = GetMain().lcd;
        lcd.AtonBlueRight = false;
        lcd.AtonFirstPos = true;
        lcd.AtonClimbPlatform = true;
    }

    // setup coordinates
    GetMain().tracker.FlipX(GetMain().lcd.AtonBlueRight);
    GetMain().drive.FlipX(GetMain().lcd.AtonBlueRight);

    if (GetMain().lcd.AtonFirstPos)
        RunAtonFirstPos();
    else
        RunAtonSecondPos();

    Do(EndOfAction());

    if (PrintDiagnostics(Diagnostics::Autonomous))
        printf("\n *** Auto: Exit ***\n\n");
}


void ShooterSetAngle(bool hightFlag, int distance, bool checkPresenceOfBall)
{
    auto flag = hightFlag ? Flag::High : Flag::Middle;
    // we disable checking for ball only for first action - shooting.
    Assert(!GetMain().shooter.IsShooting());

   if (!checkPresenceOfBall || GetMain().shooter.BallStatus() != BallPresence::NoBall)
   {
       GetMain().shooter.SetFlag(flag);
       GetMain().shooter.SetDistance(distance);
       Assert(GetMain().shooter.GetFlagPosition() != Flag::Loading); // importatn for next ShootBall action to be no-op
   }
   else
   {
       Assert(!GetMain().shooter.IsMovingAngle()); // are we waiting for nothing?
       Assert(GetMain().shooter.GetFlagPosition() == Flag::Loading); // importatn for next ShootBall action to be no-op
   }
}

int CalcAngleToPoint(double x, double y)
{
    PositionInfo info = GetTracker().LatestPosition(false /*clicks*/);
    ReportStatus("TurnToPoint: To = (%f, %f), current = (%f, %f)\n", x, y, info.X, info.Y);
    x = info.X - x;
    y = info.Y - y;
    int angle = 0;
    if (y > 0)
        angle = atan(x/y) * 180 / 3.14159265358;
    else if (y < 0)
        angle = 180 + atan(x/y) * 180 / 3.14159265358;
    ReportStatus("    vector = (%f, %f): angle = %d\n", x, y, angle);
    return angle;
}