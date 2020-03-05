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

void PrintPos(const char *name)
{
    // auto pos = GetTracker().LatestPosition();
    // printf("%s: angle = %f, XY = (%f, %f)\n", name, pos.angle, pos.X, pos.Y);
}

void RunAtonUnprotected()
{
    ReportStatus(Log::Info, "Unprotected aton\n");

    auto timeBegin = GetTime();
    auto& tracker = GetTracker();
    tracker.SetCoordinates({16, 60+24, 0});
    
    OpenTrayOnStart(300);
    /*
    Do(MoveAction(300, 80), 500);
    Do(MoveAction(-300, 80), 500);
    */

    SetIntake(127);
    const unsigned int intakeSpeed = 30;
    const unsigned int intakeSpeed2 = 30;

    // ===== GO FORWARD =====
    MoveExactWithAngle(2700, 0, intakeSpeed);

    // ===== Go Sideways Back =====
    MoveExactWithAngle(-3100, -42);

    PrintPos("Pos1");

    // ===== GO FORWARD Again =====
    MoveExactWithAngle(3400, 0, intakeSpeed2);
    
    // ===== Get Tower Cube =====
    /*
    MoveExactWithAngle(400, -35, intakeSpeed2);
    MoveExactWithAngle(-500, -35, intakeSpeed2);
    */

    // ===== GO STACK =====
    printf("%s\n", "Moving Exact With Angle and Tray");
    EnableConsoleLogs(Log::Automation);
    MoveExactWithAngleAndTray(3000, 180-18, 1500, UINT_MAX, 4000, true);

    PrintPos("EndPos");
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
    Do(MoveAction(-1000, 65), 1000);
    
    // MoveStraight(-1800, 70, turnAngle);
    
    DoTrayAction(State::Rest);
}
