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
    GetTracker().SetAngle(0);

    // NOTE: Replace MoveStraight() with MoveExactWithAngle()

    
    OpenTrayOnStart(300);
    /*
    Do(MoveAction(300, 80), 500);
    Do(MoveAction(-300, 80), 500);
    */

    SetIntake(127);
    const unsigned int intakeSpeed = 43;

    /*
    MoveExactWithAngle(4800, 0, intakeSpeed);
    MoveExactWithAngle(-5700, -53);
    MoveExactWithAngle(5400, -7, intakeSpeed - 5);
    MoveExactWithAngle(4300, 90+63);
    */

    // ===== GO FORWARD =====
    // BLUE
    int distance0 = 3700;
    // RED
    if (GetLcd().AtonRed)
        distance0 = 3500;
    MoveExactWithAngle(distance0, 0, intakeSpeed);

    // ===== Go Sideways Back =====
    // BLUE
    int distance1 = -5700;
    int angle1 = -68;
    // RED
    if (GetLcd().AtonRed)
    {
        distance1 = -5400;
        angle1 = -50;
    }
    MoveExactWithAngle(distance1, angle1);

    // ===== GO FORWARD Again =====
    // BLUE
    int distance3 = 5600;
    int angle3 = -3;
    // RED
    if (GetLcd().AtonRed)
    {
        distance3 = 5600;
        angle3 = 4;
    }
    MoveExactWithAngle(distance3, angle3, intakeSpeed - 5);
    
    // ===== GO STACK =====
    // BLUE
    int distance2 = 5100;
    int angle2 = 63;
    // RED
    if (GetLcd().AtonRed)
    {
        angle2 = 65;
        distance2 = 5000;
    }
    MoveExactWithAngle(distance2, 90+angle2);


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
