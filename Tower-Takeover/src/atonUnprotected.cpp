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

    
    OpenTrayOnStart(300);
    /*
    Do(MoveAction(300, 80), 500);
    Do(MoveAction(-300, 80), 500);
    */

    SetIntake(127);
    const unsigned int intakeSpeed = 17;

    /*
    MoveExactWithAngle(4800, 0, intakeSpeed);
    MoveExactWithAngle(-5700, -53);
    MoveExactWithAngle(5400, -7, intakeSpeed - 5);
    MoveExactWithAngle(4300, 90+63);
    */

    // ===== GO FORWARD =====
    // BLUE
    int distance0 = 1900;
    // RED
    if (GetLcd().AtonRed)
        distance0 = 3500;
    
    MoveExactWithAngle(distance0, 0, intakeSpeed);

    // ===== Go Sideways Back =====
    // BLUE
    int distance1 = -2500;
    int angle1 = -50;
    // RED
    if (GetLcd().AtonRed)
    {
        distance1 = -5400;
        angle1 = -50;
    }
    MoveExactWithAngle(distance1, angle1);

    // ===== GO FORWARD Again =====
    int intakeSpeed2 = 19;
    // BLUE
    int distance3 = 3000;
    int angle3 = 0;
    // RED
    if (GetLcd().AtonRed)
    {
        distance3 = 5600;
        angle3 = 4;
    } 
    MoveExactWithAngle(distance3, angle3, intakeSpeed2);
    
    // ===== Get Tower Cube =====
    // BLUE
    int distance4 = 400;
    int angle4 = -35;
    // RED
    if (GetLcd().AtonRed)
    {
        distance4 = 5600;
        angle4 = 4;
    } 
    MoveExactWithAngle(distance4, angle4, intakeSpeed2);
    Wait(300);
    MoveExactWithAngle(-500, angle4, intakeSpeed2);
    
    // ===== GO STACK =====
    // BLUE
    int distance2 = 3300;
    int angle2 = 63;
    // RED
    if (GetLcd().AtonRed)
    {
        angle2 = 65;
        distance2 = 5000;
    }
    printf("%s\n", "Moving Exact With Angle and Tray");
    Wait(500);

    MoveExactWithAngleAndTray(distance2, 90 + angle2, 1500, UINT_MAX, 4000, true);
    DoTrayAction(State::TrayOut);
    // Do(MoveAction(600, 40), 500);
    // MoveExactWithAngle(900, 90+65, intakeSpeed, 1000);

    GetIntake().m_mode = IntakeMode::Hold;

    //return;

    // Out take
    // SetIntake(-20);
    // Wait(200);
    // DoTrayAction(State::TrayOut);
    
    // Push forward a bit
    // Do(MoveAction(300, 30), 500);
    
    // Move back (straight no matter the angle)
    Do(MoveAction(-2000, 60), 1000);
    
    // MoveStraight(-1800, 70, turnAngle);
    
    DoTrayAction(State::Rest);
}
