#include "aton.h"


const int distanceToCap = 1900;
const int angleToShootFlags = -1;
// distances in inches, the higher the number - the lower is the angle
const int g_midFlagHeight = 55;
const int g_highFlagHeight = 55;

const int angleToMoveToFlags = 4;
const int distanceFlagsToPlatform = -4300;

// used only in recovery mode, if we hit something
const float inchesToPlatform = 80.0;

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonFirstPos()
{
    PositionInfo info;
    auto& main = GetMain();

    main.tracker.SetCoordinates({16, 60, -90});

    // async actions
    SetShooterAngle(true /*high*/, g_midFlagHeight, false /*checkPresenceOfBall*/);

    //
    // knock the cone
    //
    BLOCK
    {
        KeepAngle keeper(-90);
        unsigned int distance = main.drive.m_distanceFromBeginning;

        Do(Move(distanceToCap-250, 85));
        IntakeUp();
        Do(Move(250, 20));
        Do(Wait(100));
        Do(MoveTimeBased(-18, 500, true /*waitForStop*/)); // attempt to fully stop, for more accurate back movement

        // we have hard time picking up the ball, so wait  
        Do(Wait(100));

        distance = main.drive.m_distanceFromBeginning - distance + 50;
        ReportStatus("Move back: %d\n", distance);
        Do(MoveExact(-distance)); // 1800 ?
        IntakeStop();
    }


    //
    // Turn to shoot
    //
    Do(TurnToAngle(angleToShootFlags));
    info = GetTracker().LatestPosition(false /*clicks*/);
    ReportStatus("Shooting: X,Y,A inches: (%f, %f), A = %d\n", info.X, info.Y, info.gyro);

    //
    // Shoot the ball
    //
    SetShooterAngle(true /*high*/, g_midFlagHeight, false /*checkPresenceOfBall*/);
    Do(WaitShooterAngleToStop());
    Do(ShootBall());
    IntakeUp();
    main.shooter.SetDistance(g_highFlagHeight);
    // wait for it to go down & start moving up
    Do(WaitShooterAngleToGoUp(g_mode == AtonMode::Skills ? 5000 : 1000)); 
    SetShooterAngle(false /*high*/, g_highFlagHeight, true /*checkPresenceOfBall*/);
    Do(WaitShooterAngleToStop());
    Do(ShootBall());

    //
    // Move to lower flag
    //
    BLOCK
    {
        KeepAngle keeper(angleToMoveToFlags);

        IntakeUp();
        Do(Move(2500, 85, 0, true /*StopOnColision */));
        Do(MoveTimeBased(0, 500, true /*waitForStop*/)); // attempt to fully stop, for more accurate back movement
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
            // Do(Move(100, -35, 0, false /*StopOnColision */));
            // Do(Wait(300));
            // ReportStatus("   Turning\n");
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
        BLOCK
        {
            KeepAngle keeper(3); // try to get further out from the wall
            ReportStatus("Moving: %d\n", distance);
            Do(MoveExact(distance));
        }

        Do(TurnToAngle(-90));
        MoveToPlatform(g_mode == AtonMode::Skills); //  || g_mode == AtonMode::ManualAuto);
    }
    else
    {
        ReportStatus("Not climbing platform\n");
        distance = distanceFlagsToPlatform / 2;
        ReportStatus("Moving: %d\n", distance);
        Do(MoveExact(distance));
    }

    IntakeStop();

}
