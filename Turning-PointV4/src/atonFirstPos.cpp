#include "aton.h"
#include "atonFirstPos.h"
/*
const int distanceToCap = 1900;
const int angleToShootFlags = -5;
// distances in inches, the higher the number - the lower is the angle
const int g_midFlagHeight = 55; // 55
const int g_highFlagHeight = 55;

const int angleToMoveToFlags = 0;
const int distanceFlagsToPlatform = -4150;

// used only in recovery mode, if we hit something
const float inchesToPlatform = 80.0;

// used when shooting middle post when not climbing platform
const unsigned int g_midFlagHeightDiagonalShot = 65;
*/

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void GoToCapWithBallUnderIt(int distance)
{
    if (distance == 0)
        distance = distanceToCap;
    Do(Move(distance - 300));
    IntakeUp();
    Do(Move(300, 20));
    Do(Wait(50));
    Do(MoveTimeBased(-18, 500, true /*waitForStop*/)); // attempt to fully stop, for more accurate back movement

    // we have hard time picking up the ball, so wait
    Do(Wait(100));
}

void GetBallUnderCapAndReturn()
{
    auto &main = GetMain();

    KeepAngle keeper(-90);
    unsigned int distance = main.drive.m_distanceFromBeginning;

    GoToCapWithBallUnderIt();

    distance = main.drive.m_distanceFromBeginning - distance;
    ReportStatus("Move back: %d\n", distance);
    Do(MoveExact(-distance)); // 1800 ?
    IntakeStop();
}

void ShootTwoBalls(int midFlagHeight, int highFlagHeight)
{
    ReportStatus("Shooting 2 balls\n");
    SetShooterAngle(true /*high*/, midFlagHeight, false /*checkPresenceOfBall*/);
    Do(WaitShooterAngleToStop());
    Do(ShootBall());
    IntakeUp();
    GetMain().shooter.SetDistance(highFlagHeight);
    // wait for it to go down & start moving up
    Do(WaitShooterAngleToGoUp(g_mode == AtonMode::Skills ? 2000 : 1500));
    SetShooterAngle(false /*high*/, highFlagHeight, true /*checkPresenceOfBall*/);
    Do(WaitShooterAngleToStop());
    Do(ShootBall());
    IntakeUp();
}

void TurnToFlagsAndShootTwoBalls()
{
    Do(TurnToAngle(angleToShootFlags));
    ShootTwoBalls();
}

void MoveToLowFlag()
{
    KeepAngle keeper(angleToMoveToFlags);

    Do(Move(2200, 85, true /*StopOnColision */));
    Do(Move(200, 30, true /*StopOnColision */));
    Do(MoveTimeBased(0, 500, true /*waitForStop*/)); // attempt to fully stop, for more accurate back movement
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
        Do(TurnToAngle(CalcAngleToPoint(18, 84) + 180));
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

        Do(TurnToAngle(-90));
        MoveToPlatform(g_mode == AtonMode::Skills); //  || g_mode == AtonMode::ManualAuto);
    }
    else
    {
        ReportStatus("Not climbing platform\n");
        distance -= distanceFlagsToPlatform / 2;
        ReportStatus("Moving: %d\n", distance);
        MoveExactWithAngle(distance, 3);  // try to get further out from the wall

        Do(TurnToAngle(-55));
        SetShooterAngle(false /*high*/, g_midFlagHeightDiagonalShot, true /*checkPresenceOfBall*/);
        Do(WaitShooterAngleToGoUp(5000));
        Do(ShootBall());

        Do(TurnToAngle(-60));

    }
}
