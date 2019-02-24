#include "aton.h"
#include "atonFirstPos.h"

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

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
        MoveToPlatform(main.lcd.AtonSkills);
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
