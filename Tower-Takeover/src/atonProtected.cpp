#include "aton.h"
#include "actionsMove.h"
#include "actions.h"
#include "StateMachine.h"
#include "intake.h"
#include "cubetray.h"
#include "position.h"
#include "lift.h"

#include "pros/motors.h"

using namespace pros;
using namespace pros::c;

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonProtected()
{
    ReportStatus(Log::Info, "Protected aton\n");

    auto timeBegin = GetTime();
    GetTracker().SetCoordinates({16, 60+24, -90});

    /*
    Do(MoveAction(1000, 80));
    Do(MoveAction(-1000, 80));
    */



/*


    OpenTrayOnStart();

    // NOTE: Replace MoveStraight() with MoveExactWithAngle()

    SetIntake(127);
    // Wait(500);
    MoveStraight(5000, 90, -90);

    int turnAngle = -240;
    TurnToAngle(turnAngle);
    
    MoveStraight(4000, 90, turnAngle);
    //MoveStraight(1500, 50, -210);

    GetIntake().m_mode = IntakeMode::Hold;

    //Wait(500);

    MoveStraight(700, 50, turnAngle);

    DoTrayAction(State::TrayOut);

    // TODO: add quick time out
    Do(MoveAction(150, 30), 1000);
    
    MoveStraight(-1500, 60, turnAngle);
    DoTrayAction(State::Rest);




    */
}
 