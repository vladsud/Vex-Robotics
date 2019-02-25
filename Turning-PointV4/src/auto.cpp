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
#include "battery.h"

enum class AtonMode
{
    Regular,
#ifndef OFFICIAL_RUN
    TestRun,
    ManualSkill,
#endif
};

#ifndef OFFICIAL_RUN
AtonMode g_mode = AtonMode::TestRun;
#endif

const  bool g_leverageLineTrackers = true;

// Turn Auton off incase battery isn't plugged in
bool g_enabled = true;

// Variables not to touch - control actual autonomous mode
bool g_autonomousSmartsOn = true;
bool g_manualSmarts = false;

bool g_alreadyRunAutonomous = false;

bool isAuto()
{
#ifndef OFFICIAL_RUN
    if (g_mode == AtonMode::TestRun)
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

void Do(Action &&action)
{
    // auto time = millis();
    while (!action.ShouldStop())
    {
        GetMain().Update();
    }
    action.Stop();
    // ReportStatus("action time (%s): %ld\n", action.Name(), millis() - time);
}

void autonomous()
{
    Battery bat;
    float mp = bat.GetMainPower();
    float ep = bat.GetExpanderPower();
    ReportStatus("Main Battery Level: %.2f\n" , mp);
    ReportStatus("Expander Battery Level: %.2f\n", ep);
    if (ep > 6.0f && mp > 6.0f)
    {
        g_enabled = true;
    } 
    else 
    {
        g_enabled = false;
    }

    if (g_enabled)
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

        auto &lcd = main.lcd;
        if (lcd.AtonSkills)
        {
            lcd.AtonBlueRight = false;
            lcd.AtonFirstPos = false;
            lcd.AtonClimbPlatform = true;
        }

        // setup coordinates
        main.tracker.FlipX(main.lcd.AtonBlueRight);
        main.drive.FlipX(main.lcd.AtonBlueRight);

#ifndef OFFICIAL_RUN
        // Debugging code - should not run in real autonomous
        if (g_mode == AtonMode::ManualSkill && isAutonomous())
            RunSuperSkills();
        else if (g_mode == AtonMode::TestRun && !isAutonomous())
            RunSuperSkills();
        else
#endif // !OFFICIAL_RUN
        {
            // if you remove this (super skills) to run old skills, fix lcd.AtonFirstPos = true above!!!
            if (lcd.AtonSkills)
                RunSuperSkills();
            else if (lcd.AtonFirstPos)
                RunAtonFirstPos();
            else
                RunAtonSecondPos();
        }
        IntakeStop();

        // unused variables in final build
        UNUSED_VARIABLE(time);
        UNUSED_VARIABLE(time2);

        ReportStatus("\n*** END AUTONOMOUS ***\n\n");
        printf("Time: %d %d \n", main.GetTime() - time, int(millis() - time2));
        printf("Max Cycle Time: %d\n", (int)main.GetMaxCycleTime());

        GetLogger().Dump();

        Do(EndOfAction());
    }
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
    // ReportStatus("First platform\n");
    Do(MoveToPlatformAction(2700));

    if (twoPlatforms)
    {
        // ReportStatus("Second platform\n");
        Do(MoveToPlatformAction(2250));
        MoveStop(-30);
    }
}

void MoveExactWithAngle(int distance, int angle, bool allowTurning /*= true*/)
{
    if (allowTurning)
        TurnToAngleIfNeeded(angle);
    KeepAngle keeper(angle);
    MoveExact(distance);
}

void MoveWithAngle(int distance, int angle, int speed)
{
    TurnToAngleIfNeeded(angle);
    KeepAngle keeper(angle);
    Move(distance, speed);
}

void TurnToAngleIfNeeded(int angle)
{
    int angleDiff = AdjustAngle(GetGyroReading() - angle * GyroWrapper::Multiplier);
    if (abs(angleDiff) > 10 * GyroWrapper::Multiplier)
        TurnToAngle(angle);
}

void MoveExactWithLineCorrection(int fullDistance, unsigned int distanceAfterLine, int angle)
{
    TurnToAngleIfNeeded(angle);
    KeepAngle keeper(angle);
    Do(MoveExactWithLineCorrectionAction<MoveExactAction>(fullDistance, distanceAfterLine, angle));
}

void MoveWithLineCorrection(int fullDistance, unsigned int distanceAfterLine, int angle)
{
    TurnToAngleIfNeeded(angle);
    KeepAngle keeper(angle);
    Do(MoveExactWithLineCorrectionAction<MoveAction>(fullDistance, distanceAfterLine, angle));
}

unsigned int HitTheWall(int distanceForward, int angle)
{
    Assert(distanceForward != 0);
    Drive& drive = GetMain().drive;

    TurnToAngleIfNeeded(angle);

    ReportStatus("Hitting wall: a=%d d1=%d\n", angle, distanceForward);

    int distance = distanceForward * 80 / 100 - Sign(distanceForward) * 300;

    if (distance * distanceForward <= 0)
        distance = Sign(distanceForward) * 100;

    KeepAngle keeper(angle);

    Move(distance, 85, true /*StopOnColision */);
    unsigned int distanceTravelled = drive.m_distance;

    // attempt to fully stop, for more accurate back movement
    int time = distance > 1000 ? 300 : 100;
    MoveTimeBased(12 * Sign(distanceForward), time, true /*waitForStop*/);
    distanceTravelled += drive.m_distance;

    MoveTimeBased(25 * Sign(distanceForward), 2000, true /*waitForStop*/);
    distanceTravelled += drive.m_distance;

    // MoveTimeBased(30 * Sign(distanceForward), 100, false /*waitForStop*/);
    Wait(300);
    distanceTravelled += drive.m_distance;

    ReportStatus("   HitTheWall: Actually travelled: %d out of %d\n", distanceTravelled, distanceForward);

    return distanceTravelled;
}

void GoToCapWithBallUnderIt(int distance)
{
    auto& drive = GetMain().drive;
    if (distance == 0)
        distance = distanceToCap;
    Move(distance - 400, 110);
    unsigned int distanceTravelled = drive.m_distance;
    IntakeUp();
    Move(300, 20);
    // Wait(50);
    distanceTravelled += drive.m_distance;
    MoveStop(-18); // attempt to fully stop, for more accurate back movement

    // we have hard time picking up the ball, so wait
    // Wait(100);
    distanceTravelled += drive.m_distance;

    ReportStatus("GoToCapWithBallUnderIt: distance=%d, expected=%d\n", distanceTravelled, distance);
}

void GetBallUnderCapAndReturn()
{
    auto &main = GetMain();

    KeepAngle keeper(-90);
    unsigned int distance = main.drive.m_distanceFromBeginning;

    GoToCapWithBallUnderIt();

    distance = main.drive.m_distanceFromBeginning - distance + 200;
    ReportStatus("Move back: %d\n", distance);
    MoveExact(-distance); // 1800 ?
    IntakeStop();
}

void ShootTwoBalls(int midFlagHeight, int highFlagHeight)
{
    auto &main = GetMain();
    if (main.shooter.BallStatus() != BallPresence::NoBall)
    {
        ReportStatus("Shooting 2 balls\n");
        SetShooterAngle(true /*high*/, midFlagHeight, false /*checkPresenceOfBall*/);
        WaitShooterAngleToStop();
        ShootBall();
        IntakeUp();
        GetMain().shooter.SetDistance(highFlagHeight);
        // wait for it to go down & start moving up
        WaitShooterAngleToGoUp(main.lcd.AtonSkills ? 2000 : 1500);
        SetShooterAngle(false /*high*/, highFlagHeight, true /*checkPresenceOfBall*/);
        WaitShooterAngleToStop();
        ShootBall();
    }
    IntakeUp();
}

void TurnToFlagsAndShootTwoBalls()
{
    TurnToAngle(angleToShootFlags);
    ShootTwoBalls();
}
