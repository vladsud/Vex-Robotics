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

using namespace pros::c;

enum class AtonMode
{
    Regular,
    SkillsInManual,
#ifndef OFFICIAL_RUN
    TestRun,
    ManualSkill,
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

void StartSkillsinManual()
{
    g_mode = AtonMode::SkillsInManual;
    Assert(isAuto());
}


void Do(Action &&action, unsigned int timeout /* = 100000 */)
{
    // auto time = millis();
 
    auto& main = GetMain();
    while (!action.ShouldStop())
    {
        if (main.GetTime() - action.m_timeStart >= timeout)
        {
            ReportStatus("!!! TIME-OUT: %s: %d", action.Name(), timeout);
            break;
        }

        main.Update();

        // Check if we have bailed out of autonomous mode in manual skills (some key was pressed on joystick)
        if (g_mode == AtonMode::Regular && !competition_is_autonomous())
        {
            ReportStatus("\n!!! Switching to manual mode!\n");
            Assert(!isAuto());
            operatorControl(); // this does not return!
            ReportStatus("\n!!! Error: Should never get back from opControl()!\n");
        }
    }

    if (timeout <= 15000)
    {
        ReportStatus("%s's time: %d / %d", action.Name(), main.GetTime() - action.m_timeStart, timeout);
    }

    action.Stop();
    // ReportStatus("action time (%s): %ld\n", action.Name(), millis() - time);
}

// Scans digital buttons on joystick
bool joystickGetDigital(pros::controller_id_e_t id, pros::controller_digital_e_t button)
{
    bool result = controller_get_digital(id, button);

    // if we are running autonous code in non-autonomous mode, then allow user to bail out.
    // this is very useful to run autonomous skills in manual skills mode.
    if (result && g_mode != AtonMode::Regular)
    {
        ReportStatus("\n!!! Cancelling autonomous mode, running opControl()!\n");
        Assert(isAuto());
        Assert(!competition_is_autonomous());
        g_mode = AtonMode::Regular;
        Assert(!isAuto());
    }
    return result;
}

void autonomous()
{
    float mp = GetMainPower();
    ReportStatus("Main Battery Level: %.2f\n" , mp);
    if (mp <= 10.0f)
    {
        ReportStatus("\nERROR: LOW OR NO BATTERY\n");
        return;
    }

    // Safety net: run autonomous only once!
    // In case manual auto was still in place when running on competition.
    if (g_alreadyRunAutonomous && !competition_is_autonomous())
    {
        ReportStatus("\n!!! Safety: running second aton, switching to manaul mode!\n");
        g_mode = AtonMode::Regular;
        return;
    }

    g_alreadyRunAutonomous = true;

#ifndef OFFICIAL_RUN
    if (g_mode == AtonMode::TestRun)
        delay(4000);
#endif

    ReportStatus("\n*** Autonomous: Start ***\n\n");

    Main &main = SetupMain();
    auto time = main.GetTime();
    auto time2 = millis();

    // all system update their counters, like distance counter.
    main.ResetState();
    main.UpdateAllSystems();

    auto &lcd = main.lcd;

    if (g_mode == AtonMode::SkillsInManual)
        lcd.AtonSkills = true;

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
    if (g_mode == AtonMode::ManualSkill && competition_is_autonomous())
        RunSuperSkills();
    else if (g_mode == AtonMode::TestRun && !competition_is_autonomous())
    {
        // lcd.AtonClimbPlatform = false;
        RunAtonFirstPos();
        // RunAtonSecondPos();
        // RunSuperSkills();
    }
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


void MoveToPlatform(bool twoPlatforms, int angle)
{
    // ReportStatus("First platform\n");
    Do(MoveToPlatformAction(2700, angle));

    if (twoPlatforms)
    {
        // ReportStatus("Second platform\n");
        Do(MoveToPlatformAction(2250, angle));
        MoveStop();
    }
}


void MoveExactWithAngle(int distance, int angle, bool allowTurning /*= true*/)
{
    if (allowTurning)
        TurnToAngleIfNeeded(angle);
    MoveExact(distance, angle);
}

void MoveExactFastWithAngle(int distance, int angle)
{
    TurnToAngleIfNeeded(angle);
    Do(MoveExactFastAction(distance, angle));
    WaitAfterMoveReportDistance(distance);
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
    Do(MoveExactWithLineCorrectionAction<MoveExactAction>(fullDistance, distanceAfterLine, angle));
    WaitAfterMove();
}


unsigned int HitTheWall(int distanceForward, int angle)
{
    TurnToAngleIfNeeded(angle);
    Do(MoveHitWallAction(distanceForward, angle), 1500 + abs(distanceForward) / 1000 /*trimeout*/);
    WaitAfterMove();

    unsigned int distance = GetMain().drive.m_distance;
    ReportStatus("   HitTheWall: Actually travelled: %d out of %d\n", distance, distanceForward);
    return distance;
}


void GoToCapWithBallUnderIt(int distance, unsigned int distanceBack, int angle)
{
    IntakeUp();
    MoveExactFastWithAngle(distance, angle);
    distanceBack = distanceBack + GetMain().drive.m_distance - distance;
    MoveExactWithAngle(-(int)distanceBack, angle);
}

void FlipCap(unsigned int distance, unsigned int distanceBack, int angle)
{
    TurnToAngleIfNeeded(angle);
    IntakeDown();
    Do(MoveFlipCapAction(distance, angle));
    WaitAfterMoveReportDistance(distance);
    distanceBack = distanceBack + GetMain().drive.m_distance - distance;
    MoveExact(-(int)distanceBack, angle);
}

void FlipCapWithLineCorrection(unsigned int distance, unsigned int afterLine, unsigned int distaneBack, int angle)
{
    IntakeDown();
    TurnToAngleIfNeeded(angle);
    Do(MoveExactWithLineCorrectionAction<MoveFlipCapAction>(distance, afterLine, angle));
    WaitAfterMove();
    // can't use this adjustment - we do not know how far we went after line...
    // distanceBack = distanceBack + GetMain().drive.m_distance - distance;
    MoveExactWithAngle(-(int)distaneBack, angle);
}

void ShootOneBall(bool high, int distance, bool checkBallPresence)
{
    auto &main = GetMain();
    IntakeStop();

    ReportStatus("Waiting for angle to go up: %ld\n", millis());
    WaitShooterAngleToGoUp(2000);
    if (main.shooter.BallStatus() != BallPresence::NoBall)
    {
        SetShooterAngle(high, distance, checkBallPresence);
        ReportStatus("Waiting for angle to stop: %ld\n", millis());
        WaitShooterAngleToStop(main.lcd.AtonSkills ? 4000: 1000);
        ReportStatus("Shooting: %ld\n", millis());
        Wait(300);
        ShootBall();
    }
    IntakeUp();
}

void ShootTwoBalls(int highFlagDistance, int midFlagDistance)
{
    ReportStatus("Shooting 2 balls\n");
    ShootOneBall(true /*high*/, highFlagDistance, false /*checkPresenceOfBall*/);
    ShootOneBall(false /*high*/, midFlagDistance, true /*checkPresenceOfBall*/);
    IntakeUp();
}


// give some time for robot to completely stop
void WaitAfterMove()
{
    // Not enough time in "main" atonomous
    auto& lcd = GetMain().lcd;
    unsigned int timeout = lcd.AtonSkills || !lcd.AtonFirstPos || !lcd.AtonClimbPlatform ? 100 : 50;
    Do(WaitTillStopsAction(), timeout);
}

void WaitAfterMoveReportDistance(int distance)
{
    WaitAfterMove();
    unsigned int error = abs((int)abs(distance) - (int)GetMain().drive.m_distance);
    if (error >= 20)
        ReportStatus("MoveExact (or equivalent) big Error: %d\n", error);
}
