#include "aton.h"
#include "actionsMove.h"
#include "actions.h"
#include "StateMachine.h"
#include "intake.h"
#include "cubetray.h"
#include "position.h"
#include "lift.h"
#include "lcd.h"

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

    // Intake
    SetIntake(127);
    MoveStraight(5450, 85, -90);
    
    Wait(300);
    MoveStraight(4000, -85, -90);
    

    // Turn and go to zone
    int turnAngle = 46;
    if (!GetLcd().AtonRed)
        turnAngle = 45;
    TurnToAngle(turnAngle);   
    GetIntake().m_mode = IntakeMode::Hold;
    Wait(500);
    MoveStraight(1550, 80, turnAngle);

    // Correct to zone
    // Do(MoveAction(500, 80), 1000);

    // Out take
    SetIntake(-20);
    Wait(200);
    DoTrayAction(State::TrayOut);
    
    // Push forward a bit
    Do(MoveAction(300, 30), 500);
    
    // Move back
    Do(MoveAction(-1800, 70));
    // MoveStraight(-1500, 60, 60);
    
    DoTrayAction(State::Rest);
}
