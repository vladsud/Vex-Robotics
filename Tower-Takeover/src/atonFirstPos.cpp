#include "aton.h"
#include "atonFirstPos.h"
#include "pros/motors.h"

using namespace pros;
using namespace pros::c;
// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonFirstPos()
{
    auto &main = GetMain();
    auto timeBegin = main.GetTime();

    main.tracker.SetCoordinates({16, 60+24, -90});


    //MoveExactWithAngle(1000, 90, false);

    Do(Initialization());
    Do(Reset());

    motor_move(intakeLeftPort, 127);
    motor_move(intakeRightPort, -127);

    Do(MoveAction(5000, 70));

    /*
    motor_move(intakeLeftPort, 0);
    motor_move(intakeRightPort, 0);

    Wait(1000);

    TurnToAngle(20);

    Do(MoveAction(7000, 100));
    Do(Unload());

    Do(MoveAction(2000, -30));
    */
}
 