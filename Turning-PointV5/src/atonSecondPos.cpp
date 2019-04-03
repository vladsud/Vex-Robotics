#include "aton.h"

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonSecondPos()
{
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
        BLOCK
        {
            GoToCapWithBallUnderIt(distanceToCap+200, 600 /*distanceBack*/, -100 /*angle*/);
        }

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
    
}
