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
    // auto pos = GetTracker().GetCoordinates();
    // printf("%s: angle = %f, XY = (%f, %f)\n", name, pos.angle, pos.X, pos.Y);
}

void FinishTrayOut(unsigned int timeBegin)
{
    int duration = 14600 + timeBegin - GetTime();
    printf("%d for stacking!\n", duration);
    DoTrayAction(State::TrayOut, duration);
    // Do(MoveAction(600, 40), 500);
    // MoveExactWithAngle(900, 90+65, intakeSpeed, 1000);

    // Sometimes we move out too quickly, without tray really fully opened.
    duration = 14300 + timeBegin - GetTime();
    if (duration > 0)
    {
        Wait(duration);
    }

    GetIntake().m_mode = IntakeMode::Hold;

    // Move back (straight no matter the angle)
    Do(MoveAction(-1000, 85), 1000);
    
    DoTrayAction(State::Rest);
}

void RunAtonUnprotected()
{
    ReportStatus(Log::Info, "Unprotected aton\n");

    auto timeBegin = GetTime();
    auto& tracker = GetTracker();
    tracker.SetCoordinates({16, 60+24, 0});
    
    OpenTrayOnStart();
    /*
    Do(MoveAction(300, 80), 500);
    Do(MoveAction(-300, 80), 500);
    */

    SetIntake(127);
    const unsigned int intakeSpeed = 20;
    const unsigned int intakeSpeed2 = 20;

    // ===== GO FORWARD =====
    MoveExactWithAngle(1900, 0, intakeSpeed);

    // ===== Go Sideways Back =====
    MoveExactWithAngle(-2550, -55);

    PrintPos("Pos1");

    // ===== GO FORWARD Again =====
    MoveExactWithAngle(2400, 0, intakeSpeed2);

    bool SafeMode = true;
    if (!SafeMode) {
        int distance = 1000;
        int angle = -28;
        MoveExactWithAngle(distance, angle);
        MoveExactWithAngle(-distance, angle);
    }

    // ===== GO STACK =====
    // EnableConsoleLogs(Log::Automation);
    // printf("Moving Exact With Angle and Tray\n");

    const unsigned speedLimit = 20;
    const int angle = 180-32;
    const unsigned distance = 2500;
    if (SafeMode) {
        MoveExactWithAngle(distance, angle, speedLimit, 1500);
    } else {
        MoveExactWithAngleAndTray(distance, angle, 1000, speedLimit, 1500);
    }

    PrintPos("EndPos");

    FinishTrayOut(timeBegin);
}
