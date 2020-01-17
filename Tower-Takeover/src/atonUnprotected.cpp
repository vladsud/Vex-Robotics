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

    // NOTE: Replace MoveStraight() with MoveExactWithAngle()

    OpenTrayOnStart();

    SetIntake(127);
    MoveStraight(5800, 45, -90);
 
    int turnAngle = 67;
    TurnToAngle(turnAngle);    
    GetIntake().m_mode = IntakeMode::Hold;
    MoveStraight(4000, 80, turnAngle);

    DoTrayAction(State::TrayOut);
    //Do(MoveAction(300, 30));
    
    Do(MoveAction(-1500, 60));
    // MoveStraight(-1500, 60, 60);
    
    DoTrayAction(State::Rest);
}
