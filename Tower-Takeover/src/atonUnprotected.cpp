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
    ReportStatus(Log::Info, "Unprotected aton\n");

    auto timeBegin = GetTime();
    GetTracker().SetCoordinates({16, 60+24, 0});

    // NOTE: Replace MoveStraight() with MoveExactWithAngle()

    /*
    OpenTrayOnStart();

    Do(MoveAction(300, 80), 500);
    Do(MoveAction(-300, 80), 500);
    */

    SetIntake(127);
    const unsigned int intakeSpeed = 35;

    MoveExactWithAngle(4800, 0, intakeSpeed);
    MoveExactWithAngle(-6300, -40);
    MoveExactWithAngle(5300, 0, intakeSpeed);
    MoveExactWithAngle(4000, 90+65);
    // Do(MoveAction(400, 90), 1000);
    // MoveExactWithAngle(900, 90+65, intakeSpeed, 1000);

    GetIntake().m_mode = IntakeMode::Hold;


    //return;

    // Out take
    SetIntake(-20);
    Wait(200);
    DoTrayAction(State::TrayOut);
    
    // Push forward a bit
    // Do(MoveAction(300, 30), 500);
    
    // Move back (straight no matter the angle)
    Do(MoveAction(-2000, 60), 1000);
    // MoveStraight(-1800, 70, turnAngle);
    
    DoTrayAction(State::Rest);
}
