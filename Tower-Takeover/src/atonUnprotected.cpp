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
    
    OpenTrayOnStart(300);
    /*
    Do(MoveAction(300, 80), 500);
    Do(MoveAction(-300, 80), 500);
    */

    SetIntake(127);
    const unsigned int intakeSpeed = 20;
    const unsigned int intakeSpeed2 = 20;

    // ===== GO FORWARD =====
    MoveExactWithAngle(2000, 0, intakeSpeed);

    // ===== Go Sideways Back =====
    MoveExactWithAngle(-2550, -55);

    PrintPos("Pos1");

    // ===== GO FORWARD Again =====
    MoveExactWithAngle(2500, 0, intakeSpeed2);

    bool SafeMode = false;
    if (!SafeMode) {
        int distance = 900;
        int angle = -28;
        MoveExactWithAngle(distance, angle);
        MoveExactWithAngle(-distance, angle);
    }

    // ===== GO STACK =====
    EnableConsoleLogs(Log::Automation);
    printf("Moving Exact With Angle and Tray\n");

    if (SafeMode) {
        const unsigned speedLimit = 30;
        MoveExactWithAngle(2500, 180-32, 30, 1800);
    } else {
        const unsigned speedLimit = 30;
        MoveExactWithAngleAndTray(2500, 180-32, 1000, speedLimit, 2000);
    }

    PrintPos("EndPos");

    FinishTrayOut(timeBegin);
}
