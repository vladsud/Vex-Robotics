#include "aton.h"
#include "atonFirstPos.h"

const int angleToMoveToFlags = 2;

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

    main.tracker.SetCoordinates({16, 60, -90});

    // async actions
    SetShooterAngle(true /*high*/, g_highFlagHeight);

    //
    // knock the cone
    //
    GoToCapWithBallUnderIt(distanceToCap, distanceToCap - 200, -90);

    //
    // Turn to shoot
    //
    TurnToAngle(angleToShootFlags);
    ShootTwoBalls(g_highFlagHeight, g_midFlagHeight);
    IntakeUp();

    // prepare for middle pole shooting
    main.shooter.SetDistance(main.lcd.AtonClimbPlatform ? distanceSecondFlag : g_midFlagHeightDiagonalShot);

    //
    // Climb platform if neeed
    //
    if (main.lcd.AtonClimbPlatform)
    {
        HitLowFlagWithRecovery(3200, 3700, 13 /*angleBack*/, angleToMoveToFlags);

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
        HitLowFlagWithRecovery(3200, 2800, 5 /*angleBack*/, angleToMoveToFlags);

        TurnToAngle(-48);
        ShootOneBall(true/*high*/, g_midFlagHeightDiagonalShot, true /*checkPresenceOfBall*/);

        // IntakeUp();
        // WaitForBall(1000);
        // SetShooterAngle(false/*high*/, g_highFlagHeightDiagonalShot);

        FlipCap(2100, -400, -48);

        // ShootOneBall(false/*high*/, g_highFlagHeightDiagonalShot, true /*checkPresenceOfBall*/);
    }
}
