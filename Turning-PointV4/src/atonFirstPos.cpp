#include "aton.h"
#include "atonFirstPos.h"

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void GoToCapWithBallUnderIt(int distance)
{
    auto& drive = GetMain().drive;
    if (distance == 0)
        distance = distanceToCap;
    Move(distance - 350);
    unsigned int distanceTravelled = drive.m_distance;
    IntakeUp();
    Move(300, 20);
    Wait(50);
    distanceTravelled += drive.m_distance;
    MoveStop(-18); // attempt to fully stop, for more accurate back movement

    // we have hard time picking up the ball, so wait
    Wait(100);
    distanceTravelled += drive.m_distance;

    ReportStatus("GoToCapWithBallUnderIt: distance=%d, expected=%d\n", distanceTravelled, distance);
}

void GetBallUnderCapAndReturn()
{
    auto &main = GetMain();

    KeepAngle keeper(-90);
    unsigned int distance = main.drive.m_distanceFromBeginning;

    GoToCapWithBallUnderIt();

    distance = main.drive.m_distanceFromBeginning - distance;
    ReportStatus("Move back: %d\n", distance);
    MoveExact(-distance); // 1800 ?
    IntakeStop();
}

void ShootTwoBalls(int midFlagHeight, int highFlagHeight)
{
    auto &main = GetMain();
    Wait(2500);
    if (false && main.shooter.BallStatus() != BallPresence::NoBall)
    {
        ReportStatus("Shooting 2 balls\n");
        SetShooterAngle(true /*high*/, midFlagHeight, false /*checkPresenceOfBall*/);
        WaitShooterAngleToStop();
        ShootBall();
        IntakeUp();
        GetMain().shooter.SetDistance(highFlagHeight);
        // wait for it to go down & start moving up
        WaitShooterAngleToGoUp(g_mode == AtonMode::Skills ? 2000 : 1500);
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

void MoveToLowFlag()
{
#if 0
    KeepAngle keeper(angleToMoveToFlags);

    Move(2200, 85, true /*StopOnColision */);
    Move(200, 30, true /*StopOnColision */);
    MoveStop(0); // attempt to fully stop, for more accurate back movement
#endif
    HitTheWall(2400, angleToMoveToFlags);
}

void RunAtonFirstPos()
{
    PositionInfo info;
    auto &main = GetMain();

    main.tracker.SetCoordinates({16, 60, -90});

    // async actions
    SetShooterAngle(true /*high*/, g_midFlagHeight, false /*checkPresenceOfBall*/);

    //
    // knock the cone
    //
    GetBallUnderCapAndReturn();

    //
    // Turn to shoot, shoot
    //
    TurnToFlagsAndShootTwoBalls();

    //
    // Move to lower flag
    //
    MoveToLowFlag();

    //
    // figure out if we screwd up
    //
    info = GetTracker().LatestPosition(true /*clicks*/);
    ReportStatus("Hit wall: X,Y,A clicks: (%f, %f), A = %d\n", info.X, info.Y, info.gyro);

    int distance = distanceFlagsToPlatform;
    int distanceAlt;
    if (abs(info.gyro) > 15 * GyroWrapper::Multiplier || info.Y > 12.0 / PositionTracker::inchesPerClick)
    {
        ReportStatus("Recovery!!!\n");
        TurnToAngle(CalcAngleToPoint(18, 84) + 180);
        ReportStatus("   End of recovery");

        info = GetTracker().LatestPosition(true /*clicks*/);
        distanceAlt = int(info.Y - inchesToPlatform / PositionTracker::inchesPerClick) * 2;
        distance = distanceAlt;
    }
    else
    {
        distanceAlt = int(info.Y - inchesToPlatform / PositionTracker::inchesPerClick) * 2;
    }
    ReportStatus("Alternate calculation: %d (should be %d)\n", distanceAlt, distanceFlagsToPlatform);
    UNUSED_VARIABLE(distanceAlt); // unsed variable in finale build

    //
    // Climb platform if neeed
    //
    if (main.lcd.AtonClimbPlatform)
    {
        ReportStatus("Moving: %d\n", distance);
        MoveExactWithAngle(distance, 3);  // try to get further out from the wall

        TurnToAngle(-90);
        MoveToPlatform(g_mode == AtonMode::Skills); //  || g_mode == AtonMode::ManualAuto);
    }
    else
    {
        ReportStatus("Not climbing platform\n");
        distance -= distanceFlagsToPlatform / 2;
        ReportStatus("Moving: %d\n", distance);
        MoveExactWithAngle(distance, 3);  // try to get further out from the wall

        TurnToAngle(-55);
        SetShooterAngle(false /*high*/, g_midFlagHeightDiagonalShot, true /*checkPresenceOfBall*/);
        WaitShooterAngleToGoUp(5000);
        ShootBall();

        TurnToAngle(-60);

    }
}
