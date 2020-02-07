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
    GetTracker().SetCoordinates({16, 60+24, -90});

    /*
    Do(MoveAction(1000, 80));
    Do(MoveAction(-1000, 80));
    */


    OpenTrayOnStart(300);

    // NOTE: Replace MoveStraight() with MoveExactWithAngle()

    SetIntake(127);
    const unsigned int intakeSpeed = 43;

    MoveExactWithAngle(4000, -90, intakeSpeed);
    MoveExactWithAngle(2000, -90, intakeSpeed - 20);

    const unsigned int turnAngle = -180-55;

    MoveExactWithAngle(5000, turnAngle, intakeSpeed);

    GetIntake().m_mode = IntakeMode::Hold;

    MoveExactWithAngle(1200, turnAngle, intakeSpeed, 3000);

    DoTrayAction(State::TrayOut);
    
    Do(MoveAction(-1500, 30), 1000);

    DoTrayAction(State::Rest);




}
 