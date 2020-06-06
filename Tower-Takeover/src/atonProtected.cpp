#include "aton.h"
#include "actionsMove.h"
#include "actions.h"
#include "intake.h"
#include "position.h"
#include "lcd.h"


#include "pros/motors.h"

using namespace pros;
using namespace pros::c;

// WARNING:
// All coordinates and gyro-based turns are from the POV of RED (Left) position
// For Blue (right) automatic transformation happens

void RunAtonProtected()
{
    ReportStatus(Log::Info, "Protected aton\n");

    auto timeBegin = GetTime();
    GetTracker().SetAngle(0);
}
 