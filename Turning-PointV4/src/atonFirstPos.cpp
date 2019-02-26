#include "aton.h"
#include "atonFirstPos.h"

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonFirstPos()
{
    auto &main = GetMain();

    PositionInfo info;
 
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
    IntakeUp();

    //
    // Move to lower flag
    //
    BLOCK
    {
        KeepAngle keeper(angleToMoveToFlags);
        Move(2200, 110, true /*StopOnColision */);
        Move(400, 30, true /*StopOnColision */);
        MoveStop(0); // attempt to fully stop, for more accurate back movement
    }

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
        SetShooterAngle(false /*high*/, 55, false /*checkPresenceOfBall*/);

        ReportStatus("Moving: %d\n", distance + 300);
        MoveExactWithAngle(distance, 14, false /*allowTurning*/);  // try to get further out from the wall

        bool hasBall = main.shooter.BallStatus() == BallPresence::HasBall;
        auto time = main.GetTime();
        ReportStatus("\n   Time before diagonal shot: %d, Ball: %d\n\n", time, (int)hasBall);
        bool shooting = hasBall && (time < 11000);
        if (shooting)
        {
            TurnToAngle(-28);
            ShootBall();
        }
        TurnToAngle(-91);
        MoveToPlatform(main.lcd.AtonSkills);

        if (shooting)
        {
            int time2 = main.GetTime() - time;
            if (time2 < 4000)
                printf("Took %d to shoot and climb\n", time2);
            else
                printf("!!! WARNING: Took too long (%d) to shoot and climb\n", time2);            
        }
    }
    else
    {
        ReportStatus("Not climbing platform\n");
        distance += 1400;
        ReportStatus("Moving: %d\n", distance);
        MoveExactWithAngle(distance, 5);  // try to get further out from the wall

        WaitShooterAngleToGoUp(1000);
        SetShooterAngle(true /*high*/, g_midFlagHeightDiagonalShot, true /*checkPresenceOfBall*/);

        TurnToAngle(-48);
        WaitShooterAngleToStop();
        ShootBall();

        IntakeDown();
        MoveWithAngle(1000, -48, 65);
        MoveExactWithAngle(1000, -48, false /*allowTurning*/);

        MoveWithAngle(-400, -48);
    }
}
