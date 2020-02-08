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
    GetTracker().SetCoordinates({16, 60+24, 0});

    OpenTrayOnStart(300);

    // ============= First Stack ===============
    SetIntake(127);
    // Go forward pick up row of cubes
    MoveExactWithAngle(15000,0,30); 
    // Go backward to create angle
    MoveExactWithAngle(-3000,0,30); 
    // Go forward to stack
    MoveExactWithAngle(6800,47,30, 3500); 
    // MoveExactWithAngle(-300,48,30, 3500); 

    // Stack

    // out take a bit
    SetIntake(-80);
    Wait(200);

    DoTrayAction(State::TrayOut);
    // push a bit
    // MoveExactWithAngle(1000,45,15,700);
    // go back
    // MoveExactWithAngle(-1200,46,30);
    Do(MoveAction(-1200, 30), 1000);

    // Reset tray
    DoTrayAction(State::Rest);
    SetIntake(0);
    // Smash into war
    MoveExactWithAngle(2000,0,30, 1500); 

    // ============= Stack first tower ===============
    // Reset angle
    GetGyro().SetAngle(0);
    // Back up
    MoveExactWithAngle(-1400,0,30, 1500); 
    // Start intake
    SetIntake(127);
    // Go forward to pick up cube
    MoveExactWithAngle(5000,-90, 30, 2500); 
    // Hold cube
    SetIntake(0);
    // Go to back
    MoveExactWithAngle(-1500,-90,30, 1500); 
    ArmsMid();
    MoveExactWithAngle(1500,-90,30, 1500); 
    SetIntake(-60); 
    Wait(500);
    MoveExactWithAngle(-3000,-90,30, 4000); 

    //MoveExactWithAngle()
   // OpenTrayOnStart();

    // NOTE: Replace MoveStraight() with MoveExactWithAngle()

    // Stacking first cube
    /*
    GetIntake().m_mode = IntakeMode::IntakeTower;
    MoveExactWithAngle(550,0,50); 
    Wait(100);
    TowerMid();
    MoveExactWithAngle(600,0,50); 
    SetIntake(-20); 
    Wait(1000);
    SetIntake(0);
    MoveExactWithAngle(-600,0,50); 
    TowerRest(); 
    TurnToAngle(50);
    //DoTrayAction(Rest);
    MoveExactWithAngle(-1500,50,50,3000); 
    
    float cur_gyro = GetGyroReading(); 
   
    //get next cube stack 
    SetIntake(20);
    //need quadratic acceleration here instead of stoping every time
    MoveExactWithAngle(3000,cur_gyro,50,3000); 
    MoveExactWithAngle(2000,cur_gyro,20,3000); 
    MoveExactWithAngle(3000,cur_gyro,50,3000); 
    MoveExactWithAngle(8000,cur_gyro,20,3000); 
    MoveExactWithAngle(3000,cur_gyro,45,3000); 
    MoveExactWithAngle(3000,cur_gyro,60,3000); 

    Wait(2000);
    //TurnToAngle(angle);
    
    // Go to tower
   // GetIntake().m_mode = IntakeMode::Hold;
  //  MoveStraight(4500, 80, angle);


   // DoTrayAction(State::TrayOut);
   // DoTrayAction(State::Rest);

    // Pick up cube
  //  GetIntake().m_mode = IntakeMode::IntakeTower;

  //  MoveStraight(-100, 80, 90);

    // Tower
   // TowerMid();
   // SetIntake(-60);
   // Wait(1000);

    // Get second cube
   // GetIntake().m_mode = IntakeMode::IntakeTower;
    */
}
