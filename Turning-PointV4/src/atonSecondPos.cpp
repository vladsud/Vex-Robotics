#include "aton.h"

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonSecondPos()
{
    GetMain().tracker.SetCoordinates({16.5 - 1, 33 + 48 - 1, -30});

    // give some time for other robot to get out of the way
    Do(Wait(6000));

    // Shoot the ball
    Do(ShooterAngle(false, 108, false /*checkPresenceOfBall*/));
    Do(ShootBall());

    if (GetMain().lcd.AtonClimbPlatform)
    {
        Do(Move(700, 50));
        Do(Wait(1000));
        Do(Turn(-60));
        Do(Wait(200));
        Do(Move(400, 85));
        Do(MoveToPlatform(3250, 85));
    }
}
