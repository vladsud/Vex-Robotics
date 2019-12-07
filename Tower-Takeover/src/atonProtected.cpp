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
    printf("Protected aton\n");

    auto timeBegin = GetTime();
    GetTracker().SetCoordinates({16, 60+24, -90});

    // NOTE: Replace MoveStreight() with MoveExactWithAngle()

    OpenArmsOnStart();

    SetIntake(127);
    MoveStreight(6000, 60, -90);

    TurnToAngle(60);    
    GetIntake().m_mode = IntakeMode::Hold;
    MoveStreight(5000, 80, 60);

    DoTrayAction(State::TrayOut);
    MoveStreight(-1500, 60, 60);
    DoTrayAction(State::Rest);
}
 