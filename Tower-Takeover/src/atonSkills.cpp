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

    //MoveExactWithAngle()
   // OpenTrayOnStart();

    // NOTE: Replace MoveStraight() with MoveExactWithAngle()

    // Stacking first cube
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
    
}
