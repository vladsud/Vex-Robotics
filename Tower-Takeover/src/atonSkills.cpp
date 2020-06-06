#include "aton.h"
#include "actionsMove.h"
#include "actions.h"
#include "intake.h"
#include "position.h"

#include "pros/motors.h"
void RunSuperSkills()
{
    ReportStatus(Log::Info, "Skillz aton\n");

    auto timeBegin = GetTime();
    GetTracker().SetAngle(0);

}
