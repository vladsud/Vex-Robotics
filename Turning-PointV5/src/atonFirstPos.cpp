#include "aton.h"
#include "atonFirstPos.h"

const int angleToMoveToFlags = 3;

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

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
    GoToCapWithBallUnderIt(distanceToCap, distanceToCap + 300, -90);

    //
    // Turn to shoot
    //
    TurnToAngle(angleToShootFlags);
    ShootTwoBalls(distanceFirstAton);
    IntakeUp();

    // prepare for middle pole shooting
    bool highFlag = false;
    SetShooterAngle(highFlag, main.lcd.AtonClimbPlatform ? distanceFirstAtonFromPlatform : distanceFirstAtonDiagonalShot);

    //
    // Climb platform if neeed
    //
    if (main.lcd.AtonClimbPlatform)
    {
        HitLowFlagWithRecovery(6700, 10500, 13 /*angleBack*/, angleToMoveToFlags);

        bool hasBall = main.shooter.BallStatus() == BallPresence::HasBall;
        auto time = main.GetTime() - timeBegin;
        ReportStatus("\n   Time before diagonal shot: %d, Ball: %d\n\n", time, (int)hasBall);
        bool shooting = hasBall && (time < 11000);
        if (shooting)
        {
            TurnToAngle(-26);
            ShootOneBall(highFlag, distanceFirstAtonFromPlatform, false /*checkBallPresence*/);
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
        HitLowFlagWithRecovery(6700, 2800, 5 /*angleBack*/, angleToMoveToFlags);

        TurnToAngle(-48);
        ShootOneBall(highFlag, distanceFirstAtonDiagonalShot, true /*checkPresenceOfBall*/);

        // IntakeUp();
        // WaitForBall(1000);
        // SetShooterAngle(!highFlag, distanceFirstAtonDiagonalShot);

        FlipCap(2100, -400, -48);

        // ShootOneBall(false/*high*/, distanceFirstAtonDiagonalShot, true /*checkPresenceOfBall*/);
    }
}
