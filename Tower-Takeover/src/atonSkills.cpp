#include "aton.h"
#include "actionsMove.h"
#include "actions.h"
#include "StateMachine.h"
#include "intake.h"
#include "cubetray.h"
#include "position.h"
#include "lift.h"

#include "pros/motors.h"
void RunSuperSkills()
{
    ReportStatus(Log::Info, "Skillz aton\n");

    auto timeBegin = GetTime();
    GetTracker().SetCoordinates({16, 60+24, -90});

    // Pushe cubes away
    MoveStraight(500, 60, -90);
    MoveStraight(-500, 60, -90);

    // Initialize
    OpenTrayOnStart();

    // NOTE: Replace MoveStraight() with MoveExactWithAngle()

    // Intaking
    SetIntake(127);
    Wait(500);

    // Go forward get cubes
    MoveStraight(7000, 80, -90);

    // Avoid pole --> could decide to get 9th cube or not
    MoveStraight(1000, 80, -80);
    MoveStraight(1000, 80, -100);

    // Go forward and get cubes
    MoveStraight(7000, 80, -90);

    // Turn toward tower
    int angle = -180;
    TurnToAngle(angle);
    
    // Go to tower
    GetIntake().m_mode = IntakeMode::Hold;
    MoveStraight(4500, 80, angle);

    // Stack
    DoTrayAction(State::TrayOut);

    // TODO: add quick time out
    MoveStraight(150, 30, angle);
    
    MoveStraight(-1500, 60, angle);

    DoTrayAction(State::Rest);

    // Go back
    MoveStraight(-4500, 80, angle);

    // Go to tower
    MoveStraight(4500, 80, 90);

    // Pick up cube
    GetIntake().m_mode = IntakeMode::IntakeTower;

    MoveStraight(-100, 80, 90);

    // Tower
    TowerMid();
    SetIntake(-60);
    Wait(1000);

    // Get second cube
    GetIntake().m_mode = IntakeMode::IntakeTower;
    
    MoveStraight(-4500, 80, -90);

    MoveStraight(4500, 80, 0);

    // Tower second cube
    TowerMid();
    SetIntake(-60);
    Wait(1000);

    // Move back
    MoveStraight(-1000, 80, 0);
    
    
}
