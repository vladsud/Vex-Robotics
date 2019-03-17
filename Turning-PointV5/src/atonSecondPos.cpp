#include "aton.h"

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonSecondPos()
{
    GetMain().tracker.SetCoordinates({16.5 - 1, 33 + 48 - 1, -30});

   // Shoot the ball
    SetShooterAngle(true, 108, false /*checkPresenceOfBall*/);
    WaitShooterAngleToStop();
    Wait(1000);
    ShootBall();

    if (GetMain().lcd.AtonClimbPlatform)
    {
        Wait(6000);
        MoveExactWithAngle(700, -30);
        TurnToAngle(-90);
        Wait(200);

        MoveToPlatform(false /*twoPlatforms*/, -90);
    }
    else
    {
        TurnToAngle(-100);
        BLOCK
        {
            GoToCapWithBallUnderIt(distanceToCap+150, -200 /*distanceBack*/, -100 /*angle*/);
        }

        TurnToAngle(-10);
        SetShooterAngle(false, 108, false /*checkPresenceOfBall*/);
        IntakeUp();
        WaitShooterAngleToStop();
        Wait(5000);
        ShootBall();

        MoveExactFastWithAngle(500, -70);

        /*
        TurnToAngle(-140);
        IntakeDown();
        Move(1400, 45);
        MoveExact(-500);
        */
    }
    
}
