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
        Move(700, 50);
        Wait(1000);
        TurnToAngle(-90);
        
        Wait(200);
        Move(400);
        MoveToPlatform(false /*twoPlatforms*/);
    }
    else
    {
        TurnToAngle(-100);
        BLOCK
        {
            KeepAngle keeper(-100);
            GoToCapWithBallUnderIt(distanceToCap+150, -100);
            Wait(500);
            MoveExact(-100);
        }

        TurnToAngle(-10);
        SetShooterAngle(false, 108, false /*checkPresenceOfBall*/);
        IntakeUp();
        WaitShooterAngleToStop();
        Wait(5000);
        ShootBall();

        TurnToAngle(-70);
        Move(500, -60);

        /*
        TurnToAngle(-140);
        IntakeDown();
        Move(1400, 45);
        MoveExact(-500);
        */
    }
    
}
