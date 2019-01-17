#include "aton.h"

int g_midFlagHeight = 69;
int g_highFlagHeight = 7;

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonFirstPos()
{
    PositionInfo info;

    GetMain().tracker.SetCoordinates({16, 60, -90});

    // async actions
    ShooterSetAngle(true /*high*/, g_midFlagHeight, false /*checkPresenceOfBall*/);

    //
    // knock the cone
    //
    Do(Move(50, 40));
    Do(Move(1300, 85));
    Do(IntakeUp());
    Do(Move(200, 20));
    Do(Wait(300));
    Do(MoveTimeBased(-18, 100)); // attempt to fully stop, for more accurate back movement
    Do(MoveExact(-2110));
    Do(IntakeStop());

    //
    // Turn to shoot
    //
    Do(TurnToAngle(-8));
    info = GetTracker().LatestPosition(false /*clicks*/);
    ReportStatus("Shooting: X,Y,A inches: (%f, %f), A = %d\n", info.X, info.Y, info.gyro);

    //
    // Shoot the ball
    //
    Do(ShooterAngle(true /*high*/, g_midFlagHeight, false /*checkPresenceOfBall*/));
    Do(ShootBall());
    Do(IntakeUp());
    // wait for it to go down & start moving up
    GetMain().shooter.SetDistance(g_highFlagHeight);
    Do(WaitShooterAngleToGoUp(g_mode == AtonMode::Skills ? 5000 : 1500)); 
    Do(ShooterAngle(false /*high*/, g_highFlagHeight, false /*checkPresenceOfBall*/));
    Do(ShootBall());

    //
    // Move to lower flag
    //
    Do(TurnToAngle(GetMain().lcd.AtonBlueRight ? 5 : 7));
    Do(IntakeUp());
    Do(Move(100, 35, 0, false /*StopOnColision */));
    Do(Move(2400, 85, 0, true /*StopOnColision */));
    Do(Wait(400));

    //
    // figure out if we screwd up
    //
    int distance = -4030;
    info = GetTracker().LatestPosition(true /*clicks*/);
    printf("Hit wall: X,Y,A clicks: (%f, %f), A = %d\n", info.X, info.Y, info.gyro);

    if (abs(info.gyro) > 15 * GyroWrapper::Multiplier || info.Y > 12.0 / PositionTracker::inchesPerClick)        
    {
            puts("Recovery!!!");
            Do(Wait(300));
            Do(Move(100, -35, 0, false /*StopOnColision */));
            Do(Wait(300));
            puts("   Turning");
            Do(TurnToAngle(CalcAngleToPoint(18, 84) + 180));
            puts("   End of recovery");

            info = GetTracker().LatestPosition(true /*clicks*/);
            printf("After recovery: X,Y,A clicks: (%f %f), A = %d\n", info.X, info.Y, info.gyro);
            distance = int(info.Y - 77.0 / PositionTracker::inchesPerClick) * 2;
    }
    else
    {
            int distanceAlt = int(info.Y - 80.0 / PositionTracker::inchesPerClick) * 2;
            printf("Alternate calculation: %d (should be %d)\n", distanceAlt, distance);
    }

    //
    // Climb platform if neeed
    //
    if (GetMain().lcd.AtonClimbPlatform)
    {
        printf("Moving: %d\n", distance);
        Do(MoveExact(distance));
    
        Do(IntakeStop()); // sometimes it turns off due to ball shaking - very annoying
        Do(Turn(-90));

        ReportStatus("First platform\n");
        Do(MoveToPlatform(3650, 85));

        if (g_mode != AtonMode::Skills)
        {
            Do(MoveTimeBased(30, 100));
        }
        else
        {
            // SECOND PLATFORM
            ReportStatus("Second platform\n");
            Do(MoveToPlatform(3350, 85));
            Do(MoveTimeBased(40, 200));
            Do(MoveTimeBased(-30, 100));
        }
    }
    else
    {
        ReportStatus("Not climbing platform\n");
        distance = distance / 2;
        printf("Moving: %d\n", distance);
        Do(MoveExact(distance));
    }
    Do(IntakeStop());
}