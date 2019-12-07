#include "aton.h"
#include "actionsMove.h"
#include "actions.h"
#include "StateMachine.h"
#include "intake.h"
#include "cubetray.h"
#include "position.h"
#include "lift.h"

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonUnprotected()
{
    printf("Unprotected aton\n");

    auto timeBegin = GetTime();
    GetTracker().SetCoordinates({16, 60+24, -90});

    OpenArmsOnStart();

    // NOTE: Replace MoveStreight() with MoveExactWithAngle()

    SetIntake(127);
    MoveStreight(5000, 80, -90);

    TurnToAngle(-225);
    
    MoveStreight(4500, 80, -225);
    GetIntake().m_mode = IntakeMode::Hold;
    MoveStreight(500, 50, -225);

    DoTrayAction(State::TrayOut);
    MoveStreight(-1500, 60, -225);
    DoTrayAction(State::Rest);
}
