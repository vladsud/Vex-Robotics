#include "aton.h"
#include "atonFirstPos.h"

const unsigned int distanceSecondFlag = 70;

// used when shooting middle post when not climbing platform
const unsigned int g_midFlagHeightDiagonalShot = 65;
unsigned int g_highFlagHeightDiagonalShot = 50;

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonFirstPos()
{
    auto &main = GetMain();
    auto timeBegin = main.GetTime();

    PositionInfo info;
 
    main.tracker.SetCoordinates({16, 60, -90});

    // async actions
    SetShooterAngle(true /*high*/, g_highFlagHeight, false /*checkPresenceOfBall*/);
    // main.shooter.SetFlag(Flag::Loading);

    //
    // knock the cone
    //
    GetBallUnderCapAndReturn();
    Wait(100);

    //
    // Turn to shoot, shoot
    //
    TurnToAngle(angleToShootFlags);
    ShootTwoBalls(g_highFlagHeight, g_midFlagHeight);
    IntakeUp();

    // prepare for middle pole shooting
    main.shooter.SetDistance(main.lcd.AtonClimbPlatform ? distanceSecondFlag : g_midFlagHeightDiagonalShot);

    //
    // Move to lower flag
    //
    BLOCK
    {
        KeepAngle keeper(angleToMoveToFlags);
        Move(2200, 110, false /*StopOnColision */);
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
        ReportStatus("Moving: %d\n", distance + 500);
        MoveExactWithAngle(distance + 200, 10, false /*allowTurning*/);  // try to get further out from the wall

        bool hasBall = main.shooter.BallStatus() == BallPresence::HasBall;
        auto time = main.GetTime() - timeBegin;
        ReportStatus("\n   Time before diagonal shot: %d, Ball: %d\n\n", time, (int)hasBall);
        bool shooting = hasBall && (time < 11000);
        if (shooting)
        {
            TurnToAngle(-26);
            ShootOneBall(true/*high*/, distanceSecondFlag, false /*checkBallPresence*/);
        }
        TurnToAngle(-91);
        MoveToPlatform(main.lcd.AtonSkills);

        if (shooting)
        {
            int time2 = main.GetTime() - time - timeBegin;
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

        TurnToAngle(-48);
        ShootOneBall(true/*high*/, g_midFlagHeightDiagonalShot, true /*checkPresenceOfBall*/);

        IntakeUp();
        // WaitShooterAngleToGoUp(1000);
        // SetShooterAngle(false/*high*/, g_highFlagHeightDiagonalShot, true/*checkBall*/);

        IntakeDown();
        MoveWithAngle(1300, -48, 60);
        MoveExactWithAngle(800, -48, false /*allowTurning*/);
        MoveWithAngle(-400, -48);

        // ShootOneBall(false/*high*/, g_highFlagHeightDiagonalShot, true /*checkPresenceOfBall*/);
    }
}
