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

AtonMode g_mode = AtonMode::TestRun; //TestRun;

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

void Do(Action &&action)
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
        delay(4000);
#endif

    Main &main = SetupMain();

    // all system update their counters, like distance counter.
    main.ResetState();
    main.UpdateAllSystems();

    ReportStatus("\n*** Autonomous: Start ***\n\n");
    auto time = main.GetTime();
    auto time2 = millis();

    if (g_mode == AtonMode::Skills)
    {
        auto &lcd = main.lcd;
        lcd.AtonBlueRight = false;
        lcd.AtonFirstPos = false;
        lcd.AtonClimbPlatform = true;
    }

    // setup coordinates
    main.tracker.FlipX(main.lcd.AtonBlueRight);
    main.drive.FlipX(main.lcd.AtonBlueRight);

#ifndef OFFICIAL_RUN
    // Debugging code - should not run in real autonomous
    if (g_mode == AtonMode::TestRun && !isAutonomous())
    {
        RunSuperSkills();
    }
    else
#endif // !OFFICIAL_RUN
    {
         // if you remove this (super skills) to run old skills, fix lcd.AtonFirstPos = true above!!!
        if (g_mode == AtonMode::Skills)
            RunSuperSkills();
        else if (main.lcd.AtonFirstPos)
            RunAtonFirstPos();
        else
            RunAtonSecondPos();
    }
    IntakeStop();

    // unused variables in final build
    UNUSED_VARIABLE(time);
    UNUSED_VARIABLE(time2);

    ReportStatus("\n*** END AUTONOMOUS ***\n\n");
    ReportStatus("Time: %d %d \n", main.GetTime() - time, int(millis() - time2));
    ReportStatus("Max Cycle Time: %d\n", (int)main.GetMaxCycleTime());

    GetLogger().Dump();

    Do(EndOfAction());
}

void SetShooterAngle(bool hightFlag, int distance, bool checkPresenceOfBall)
{
    auto &main = GetMain();
    auto flag = hightFlag ? Flag::High : Flag::Middle;
    // we disable checking for ball only for first action - shooting.
    Assert(!main.shooter.IsShooting());

    if (!checkPresenceOfBall || main.shooter.BallStatus() != BallPresence::NoBall)
    {
        main.shooter.SetFlag(flag);
        main.shooter.SetDistance(distance);
        Assert(main.shooter.GetFlagPosition() != Flag::Loading); // importatn for next ShootBall action to be no-op
    }
    else
    {
        Assert(!main.shooter.IsMovingAngle());                   // are we waiting for nothing?
        Assert(main.shooter.GetFlagPosition() == Flag::Loading); // importatn for next ShootBall action to be no-op
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
        angle = atan(x / y) * 180 / 3.14159265358;
    else if (y < 0)
        angle = 180 + atan(x / y) * 180 / 3.14159265358;
    ReportStatus("    vector = (%f, %f): angle = %d\n", x, y, angle);
    return angle;
}

void MoveToPlatform(bool twoPlatforms)
{
    ReportStatus("First platform\n");
    Do(MoveToPlatformAction(2700));

    if (twoPlatforms)
    {
        ReportStatus("Second platform\n");
        Do(MoveToPlatformAction(2000));
        Do(MoveTimeBased(-30, 500, true /*waitForStop*/));
    }
}

void MoveExactWithAngle(int distance, int angle)
{
    TurnToAngleIfNeeded(angle);
    KeepAngle keeper(angle);
    Do(MoveExact(distance));
}

void MoveWithAngle(int distance, int angle, int speed)
{
    TurnToAngleIfNeeded(angle);
    KeepAngle keeper(angle);
    Do(Move(distance, speed));
}

void TurnToAngleIfNeeded(int angle)
{
    int angleDiff = AdjustAngle(GetGyroReading() - angle * GyroWrapper::Multiplier);
    if (abs(angleDiff) > 10 * GyroWrapper::Multiplier)
        Do(TurnToAngle(angle));
}
