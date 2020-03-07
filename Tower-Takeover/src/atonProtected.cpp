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
    Do(MoveAction(500, 80));
    Do(MoveAction(-500, 80));
    */

    OpenTrayOnStart();
    SetIntake(127);

    const unsigned int intakeSpeed = 25;
    const unsigned int intakeSpeedSlow = 20;

    MoveExactWithAngle(1100, 0, intakeSpeedSlow);
    
    MoveExactWithAngle(850, 90, intakeSpeedSlow, 1000);
    Wait(300); // pick up cube
    MoveExactWithAngle(-850, 90, intakeSpeed, 1000);

    MoveExactWithAngle(1300, -84, intakeSpeed);
    Wait(300); // pick up cube

    MoveExactWithAngle(-1250, -42, intakeSpeed);
    MoveExactWithAngle(1650, -90-10, intakeSpeedSlow, 2500);

    GetIntake().m_mode = IntakeMode::Hold;

    FinishTrayOut(timeBegin-100);
}
 