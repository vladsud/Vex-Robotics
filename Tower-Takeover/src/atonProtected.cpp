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
    const unsigned int intakeSpeed = 105;

    MoveExactWithAngle(1200, 0, intakeSpeed);
    Wait(300);
    
    MoveExactWithAngle(1000, 90, intakeSpeed);

    MoveExactWithAngle(900, 45, intakeSpeed);

    MoveExactWithAngle(1700, 0, intakeSpeed);

    const unsigned int turnAngle = -180-55;

    MoveExactWithAngle(3300, -90, intakeSpeed);

    MoveExactWithAngle(2300, -180, intakeSpeed);

    GetIntake().m_mode = IntakeMode::Hold;

    MoveExactWithAngleAndTray(900, -180 + 45, 1000,  intakeSpeed, 3000, true);

    DoTrayAction(State::TrayOut);
    
    Do(MoveAction(-1500, 30), 1000);

    DoTrayAction(State::Rest);


}
 