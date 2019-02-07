#include "aton.h"

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonSecondPos()
{
    GetMain().tracker.SetCoordinates({16.5 - 1, 33 + 48 - 1, -30});

   // Shoot the ball
    SetShooterAngle(false, 108, false /*checkPresenceOfBall*/);
    Do(WaitShooterAngleToStop());
    Do(Wait(1000));
    Do(ShootBall());

    if (GetMain().lcd.AtonClimbPlatform)
    {
        Do(Wait(6000));
        Do(Move(700, 50));
        Do(Wait(1000));
        Do(Turn(-60));
        
        Do(Wait(200));
        Do(Move(400));
        MoveToPlatform(false /*twoPlatforms*/);
    }
    else
    {
        Do(TurnToAngle(-100));
        BLOCK
        {
            KeepAngle keeper(-100);
            GoToCapWithBallUnderIt();
        }

        Do(TurnToAngle(-10));
        SetShooterAngle(true, 108, false /*checkPresenceOfBall*/);
        IntakeUp();
        Do(WaitShooterAngleToStop());
        Do(Wait(5000));
        Do(ShootBall());

        Do(TurnToAngle(-70));
        Do(Move(500, -60));

        /*
        Do(TurnToAngle(-140));
        IntakeDown();
        Do(Move(1400, 45));
        Do(MoveExact(-500));
        */
    }
    
}
