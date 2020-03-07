#include "aton.h"
#include "actionsMove.h"
#include "actions.h"
#include "StateMachine.h"
#include "intake.h"
#include "cubetray.h"
#include "position.h"
#include "lift.h"
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

    /*
    Do(MoveAction(1000, 80));
    Do(MoveAction(-1000, 80));
    */

    OpenTrayOnStart(500);
    SetIntake(127);

    const unsigned int intakeSpeed = 35;
    const unsigned int intakeSpeedSlow = 28;

    MoveExactWithAngle(1000, 0, intakeSpeed);
    
    MoveExactWithAngle(800, -90, intakeSpeed, 1000);
    // Wait(100);

    MoveExactWithAngle(2000, -25, intakeSpeed);

    MoveExactWithAngle(3100, 90, intakeSpeedSlow);

    MoveExactWithAngle(1900, 180);

    bool SafeMode = false;
    const int angle = 180-40;
    const unsigned distance = 1000;
    if (SafeMode) {
        MoveExactWithAngle(distance, angle, UINT_MAX, 1000);
    } else {
        MoveExactWithAngleAndTray(distance, angle, 1000, UINT_MAX, 1000);
    }

    GetIntake().m_mode = IntakeMode::Hold;

    FinishTrayOut(timeBegin);
}
 