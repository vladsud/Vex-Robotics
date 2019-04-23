#include "aton.h"
#include "atonFirstPos.h"

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

const unsigned int distanceTillWall = 6900;
const int angleToMoveToFlags = 6;
const int angleToShootFlags = -2;
const int distanceToCap = 5100;

void RunAtonFirstPos()
{
    auto &main = GetMain();
    auto timeBegin = main.GetTime();

    main.tracker.SetCoordinates({16, 60, -90});

    // async actions
    SetShooterAngle(twoFlagsShootsHighFirst, distanceFirstAton);

    //
    // knock the cone
    //
    GoToCapWithBallUnderIt(distanceToCap, distanceToCap+200, -90, -96);

    //
    // Turn to shoot
    //
    TurnToAngle(angleToShootFlags);

    ShootTwoBalls(distanceFirstAton, true, false);
    IntakeUp();

    //
    // Climb platform if neeed
    //
    if (main.lcd.AtonClimbPlatform)
    {
        // prepare for middle pole shooting
        bool highFlag = false;
        SetShooterAngle(highFlag, distanceFirstAtonFromPlatform);

        HitLowFlagWithRecovery(distanceTillWall, 10600, 10 /*angleBack*/, angleToMoveToFlags);

        main.gyro.PrintValues();

        bool hasBall = main.shooter.BallStatus() == BallPresence::HasBall;
        auto time = main.GetTime() - timeBegin;
        ReportStatus("\n   Time before diagonal shot: %d, Ball: %d\n\n", time, (int)hasBall);
        bool shooting = hasBall && (time < 11000);
        if (shooting)
        {
            // Sometimes ball lands on top and interfeers with another ball
            UpdateIntakeFromShooter(IntakeShoterEvent::TooManyBalls);

            TurnToAngle(-31);
            ShootOneBall(highFlag, distanceFirstAtonFromPlatform, 0, true);
            ReportStatus("\n   Time after diagonal shot: %d,\n", main.GetTime() - timeBegin);
            if (main.GetTime() - timeBegin < 12500)
                ShootOneBall(!highFlag, distanceFirstAtonFromPlatform);
        }
        TurnToAngle(-90);
        MoveToPlatform(main.lcd.AtonSkills, -90);

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
        // prepare for middle pole shooting
        SetShooterAngle(twoFlagsShootsHighFirst, distanceFirstAtonDiagonalShot);

        HitLowFlagWithRecovery(distanceTillWall, 6100, 5 /*angleBack*/, angleToMoveToFlags);

        int angle = -50;
        TurnToAngle(angle);
        MoveExactFastWithAngle(1500, angle);
        ShootTwoBalls(distanceFirstAtonDiagonalShot, 0, true /*vision calibraion*/);

        IntakeDown();
        Wait(500);
        FlipCap(2900, 2000, -48);
    }
}
