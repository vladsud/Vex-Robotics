#include "aton.h"

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonSecondPos()
{
    GetMain().tracker.SetCoordinates({16.5 - 1, 33 + 48 - 1, -90});
    GoToCapWithBallUnderIt(distanceToCap, 400 /*distanceBack*/, -90 /*angle*/, -90);

    if (GetMain().lcd.AtonClimbPlatform)
    {
        TurnToAngle(0);
        MoveToPlatform(false /*twoPlatforms*/, 0);  
        MoveStop();
        TurnToAngle(-15);
        Wait(8000);
        //ShootTwoBalls(distanceSecondAtonFromPlatform, true, false);
        ShootOneBall(true, distanceSecondAtonFromPlatform, true, false);
        ShootOneBall(false, distanceSecondAtonFromPlatform, false, false);
    }
    else
    {
        TurnToAngle(-7);
        Wait(7000);
        ShootOneBall(true, distanceSecondAtonFromPlatform, true, false);
        ShootOneBall(false, distanceSecondAtonFromPlatform, false, false);
        // Prep for the game!
        TurnToAngle(95);
        MoveExactWithAngle(distanceToCap, 95);
        TurnToAngle(0);
    }
    

    // OLD CODE
#if 0
    GetMain().tracker.SetCoordinates({16.5 - 1, 33 + 48 - 1, -30});

    SetShooterAngle(false, distanceSecondAton);
    Wait(1000);
    ShootOneBall(false /*high*/, distanceSecondAton);

    if (GetMain().lcd.AtonClimbPlatform)
    {
        IntakeStop();
        Wait(6000);
        IntakeUp();
        MoveExactWithAngle(3600, -30);
        TurnToAngle(-90);

        MoveToPlatform(false /*twoPlatforms*/, -90);
    }
    else
    {
        TurnToAngle(-100);
        GoToCapWithBallUnderIt(distanceToCap+200, 600 /*distanceBack*/, -100 /*angle*/, -100);

        TurnToAngle(-10);
        SetShooterAngle(false, distanceSecondAton);
        IntakeUp();
        Wait(2000);
        WaitShooterAngleToStop();
        Wait(1000);
        ShootBall();

        MoveExactFastWithAngle(1500, -70);

        /*
        TurnToAngle(-140);
        IntakeDown();
        Move(1400, 45);
        MoveExact(-500);
        */
    }
#endif
}
