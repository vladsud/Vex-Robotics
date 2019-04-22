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
#include "pros/rtos.h"

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

bool Do(Action &&action, unsigned int timeout /* = 100000 */)
{
    auto time = millis();
    bool timedout = false;

    auto& main = GetMain();
    while (!action.ShouldStop())
    {
        if (main.GetTime() - action.m_timeStart >= timeout)
        {
            timedout = true;
            ReportStatus("!!! TIME-OUT: %s: %d\n", action.Name(), timeout);
            break;
        }

        main.Update();

        // Check if we have bailed out of autonomous mode in manual skills (some key was pressed on joystick)
        if (g_mode == AtonMode::Regular && !competition_is_autonomous())
        {
            ReportStatus("\n!!! Switching to manual mode!\n");
            Assert(!isAuto());
            opcontrol(); // this does not return!
            ReportStatus("\n!!! Error: Should never get back from opControl()!\n");
        }
    }

    action.Stop();
    if (!timedout && false)
    {
        if (timeout <= 15000)
            ReportStatus("%s took %ld ms (time-out: %d)\n", action.Name(), millis() - time, timeout);
        else
            ReportStatus("%s took %ld ms\n", action.Name(), millis() - time);
    }

    return !timedout;
}

// Scans digital buttons on joystick
bool joystickGetDigital(pros::controller_id_e_t id, pros::controller_digital_e_t button)
{
    bool result = (controller_get_digital(id, button) == 1);

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
    ReportStatus("Main Battery Level: %.2f\n" , battery_get_capacity());

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
    if (main.lcd.AtonBlueRight)
        ReportStatus("Flipping coordinates\n");
    main.tracker.FlipX(main.lcd.AtonBlueRight);
    main.drive.FlipX(main.lcd.AtonBlueRight);
    main.vision.SetFlipX(main.lcd.AtonBlueRight);

#ifndef OFFICIAL_RUN
    // Debugging code - should not run in real autonomous
    if (g_mode == AtonMode::TestRun && !competition_is_autonomous())
    {
        // MoveExactWithAngle(6000, 10, false);
        // lcd.AtonClimbPlatform = false;
        // lcd.AtonFirstPos = false;
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


void SetShooterAngle(bool hightFlag, int distance)
{
    auto &main = GetMain();
    auto flag = hightFlag ? Flag::High : Flag::Middle;
    // we disable checking for ball only for first action - shooting.
    Assert(!main.shooter.IsShooting());

    main.shooter.SetFlag(flag);
    main.shooter.SetDistance(distance);
}


void MoveToPlatform(bool twoPlatforms, int angle)
{
    // ReportStatus("First platform\n");
    Do(MoveToPlatformAction(5200, angle));

    if (twoPlatforms)
    {
        // ReportStatus("Second platform\n");
        Do(MoveToPlatformAction(5200, angle));
        MoveStop();
    }
}


void MoveExactWithAngle(int distance, int angle, bool allowTurning /*= true*/)
{
    if (allowTurning)
        TurnToAngleIfNeeded(angle);
    MoveExact(distance, angle);
}

void MoveExactFastWithAngle(int distance, int angle, bool stopOnHit)
{
    TurnToAngleIfNeeded(angle);
    Do(MoveExactFastAction(distance, angle, stopOnHit));
    WaitAfterMoveReportDistance(distance, 500);
}

void TurnToAngleIfNeeded(int angle)
{
    int angleDiff = AdjustAngle(GetGyroReading() - angle * GyroWrapper::Multiplier);
    if (abs(angleDiff) > 8 * GyroWrapper::Multiplier)
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
    Do(MoveHitWallAction(distanceForward, angle), 1000 + abs(distanceForward) /*trimeout*/);
    WaitAfterMove();

    unsigned int distance = GetMain().drive.m_distance;
    return distance;
}


void GoToCapWithBallUnderIt(int distance, unsigned int distanceBack, int angle)
{
    IntakeUp();
    Do(MoveExactFastAction(distance, angle, true /*stopOnHit*/));
    BLOCK
    {
        // GyroFreezer freezer(GetMain().gyro);
        // WaitAfterMoveReportDistance(distance, 200);
    }
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
    ReportStatus("Driving back: %d\n", distanceBack);
    MoveExactFastWithAngle(-(int)distanceBack, angle);
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

void ShootOneBall(bool high, int distance, unsigned int extraDelay, bool visionMove, bool visionAngle)
{
    auto &main = GetMain();
    GyroFreezer freezer(main.gyro);

    SetShooterAngle(high, distance);
    if (main.shooter.BallStatus() != BallPresence::HasBall)
    {
        ReportStatus("No ball, waiting\n");
        IntakeUp();
        WaitForBall(1000);
    }

    if (main.shooter.BallStatus() != BallPresence::NoBall)
    {
        IntakeStop();
        WaitShooterAngleToStop(main.lcd.AtonSkills ? 4000: 1000);
        if (extraDelay > 0)
            Wait(extraDelay);
        
        bool shoot = true;
        if (visionMove || visionAngle)
        {
            shoot = !Do(ShootWithVisionAction(visionMove, visionAngle), 300);
            ReportStatus("Shooting with vision: %d\n", shoot);
        }
        if (shoot)
            ShootBall();
    }
    else
    {
        ReportStatus("No ball, akip ahooting\n");
    }
    
    IntakeUp();
}

void ShootTwoBalls(int distance, bool visionMove, bool visionAngle)
{
    ReportStatus("Shooting 2 balls\n");
    ShootOneBall(twoFlagsShootsHighFirst, distance, 0, visionMove, visionAngle);
    ShootOneBall(!twoFlagsShootsHighFirst, distance, 50, visionMove, visionAngle);
}


// give some time for robot to completely stop
void WaitAfterMove(unsigned int timeout)
{
    // Not enough time in "main" atonomous
    auto& lcd = GetMain().lcd;
    if (timeout == 0)
        timeout = lcd.AtonSkills || !lcd.AtonFirstPos || !lcd.AtonClimbPlatform ? 500 : 200;
    // Do(WaitTillStopsAction(), timeout);
}

void WaitAfterMoveReportDistance(int distance, unsigned int timeout)
{
    WaitAfterMove(timeout);
    unsigned int error = abs((int)abs(distance) - (int)GetMain().drive.m_distance);
    if (error >= 50)
        ReportStatus("MoveExact (or equivalent) big Error: %d, distance travelled: %d, expected: %d\n", error, GetMain().drive.m_distance, distance);
}
