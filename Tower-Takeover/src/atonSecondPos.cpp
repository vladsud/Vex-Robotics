#include "aton.h"
#include "atonFirstPos.h"
#include "pros/motors.h"

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonSecondPos()
{
    auto &main = GetMain();
    auto timeBegin = main.GetTime();

    main.tracker.SetCoordinates({0, 0, 0});


    //MoveExactWithAngle(1000, 90, false);

    Do(Initialization());
    Do(Reset());

    
    GetMain().sm.intakeOverride = true;
    motor_move(intakeLeftPort, 127);
    motor_move(intakeRightPort, -127);

    Do(MoveAction(6000, 70));

    motor_move(intakeLeftPort, 0);
    motor_move(intakeRightPort, 0);

    //Wait(1000);

    TurnToAngle(-150);

    Do(MoveAction(4000, 100));
    Do(Unload());

    Do(MoveAction(2000, -30));
    
    GetMain().sm.intakeOverride = false;
}
