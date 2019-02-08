#include "aton.h"

const unsigned int distanceFromWall = 380;

void HitWallAndRecalibrate(int angle, int distanceForward, int distanceNext, bool calibrateAngle = true)
{
    Assert(distanceForward * distanceNext <= 0); // should have different signs - positive & negative

    TurnToAngleIfNeeded(angle);

    ReportStatus("Hit wall: a=%d d1=%d d2=%d\n", angle, distanceForward, distanceNext);

    KeepAngle keeper(angle);
    Do(Move(distanceForward, 85, true /*StopOnColision */));
    // ReportStatus("   2nd move\n");
    //Do(Move(3000 * Sign(distanceForward), 20, true /*StopOnColision */));
    ReportStatus("   2rd move\n");
    Do(MoveTimeBased(20 * Sign(distanceForward), 5000, true /*waitForStop*/)); // attempt to fully stop, for more accurate back movement
    Do(Wait(100));

    if (calibrateAngle)
        GetMain().tracker.SetAngle(angle);

    Do(MoveExact(distanceNext));
}

void RunSuperSkills()
{
    auto &main = GetMain();
    main.tracker.SetCoordinates({16, 60+24, -90});

    // Pick up ball
    GoToCapWithBallUnderIt();
    MoveExactWithAngle(-1930, -90);

    // Move in front of flag
    MoveExactWithAngle(2600, 0);

    // Recalibrate angle
    HitWallAndRecalibrate(-90, -100, distanceFromWall);

    // Shooting 2 balls at first row
    TurnToFlagsAndShootTwoBalls();

    // Hit low flag and come back
    MoveToLowFlag();
    MoveExactWithAngle(-2800, 2);

    // Shoot middle pole
    Do(TurnToAngle(-90));
    HitWallAndRecalibrate(-90, -100 , 0);
    MoveExactWithAngle(2050, -90);
    Do(TurnToAngle(-25));
    ShootTwoBalls();

    // pick up ball under cap
    Do(TurnToAngle(-90));
    GoToCapWithBallUnderIt(400);

    //Flip cap
    MoveExactWithAngle(-350, -90);
    Do(Wait(300));
    IntakeDown();
    MoveExactWithAngle(1150, -90);

    //Flip cap 2
    MoveExactWithAngle(-1650, -90);
    IntakeDown();
    MoveWithAngle(600, 0);
    MoveWithAngle(600, 0, 18); // slow down a bit
    MoveExactWithAngle(250, 0);

    //Low flag 2
    MoveExactWithAngle(1300, -90);
    HitWallAndRecalibrate(0, 500, -1600);

    //Cap 3
    Do(TurnToAngle(-90));
    IntakeDown();
    MoveWithAngle(1300, -90);
    MoveWithAngle(1200, -90, 18); // slow down a bit
    MoveExactWithAngle(250, -90);

    //Low flag 3, shoot
    MoveExactWithAngle(-200, -90);
    HitWallAndRecalibrate(0, 1000, -1850);
    Do(TurnToAngle(-10));
    ShootTwoBalls();

    //Climb
    MoveExactWithAngle(-2500, 30);
    Do(TurnToAngle(-270));
    MoveToPlatform(true);    
}
