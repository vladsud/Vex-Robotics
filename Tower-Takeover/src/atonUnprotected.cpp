#include "aton.h"
#include "actionsMove.h"
#include "actions.h"
#include "intake.h"
#include "position.h"
#include "lcd.h"

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void PrintPos(const char *name)
{
    GetTracker().PrintPos(Log::Automation);
}

void RunAtonUnprotected()
{
    ReportStatus(Log::Info, "Unprotected aton\n");

    auto timeBegin = GetTime();
    auto& tracker = GetTracker();
    tracker.SetCoordinates({16, 60+24, 0});   
}
