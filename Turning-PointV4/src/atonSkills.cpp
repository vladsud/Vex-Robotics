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

//#if 0

    // Pick up ball
    GoToCapWithBallUnderIt();
    MoveExactWithAngle(-1930, -90);

    // Move in front of flag
    MoveExactWithAngle(2600, 0);

    // Recalibrate angle
    //HitWallAndRecalibrate(-90, -75, distanceFromWall);

    // Shooting 2 balls at first row
    TurnToFlagsAndShootTwoBalls();

    // Hit low flag and come back
    MoveToLowFlag();
    MoveExactWithAngle(-2850, 0);

    //Drop second ball if any
    IntakeDown();
    Do(Wait(1500));
    IntakeUp();

    // Pick up ball from cap
    //HitWallAndRecalibrate(-90, -75, 0);
    Do(TurnToAngle(-90));
    GoToCapWithBallUnderIt(500 /*additionalDistance*/);
    MoveExactWithAngle(-400, -90);

    //Turn and shoot
    Do(TurnToAngle(-20));
    ShootTwoBalls();

    //Flip cap
    Do(TurnToAngle(-90));
    IntakeDown();
    MoveExactWithAngle(1200, -90);

    //Flip cap 2
    MoveExactWithAngle(-2000, -90);
    IntakeDown();
    Do(TurnToAngle(0));
    MoveExactWithAngle(1250, 0);
    MoveExactWithAngle(-1250, 0);

    //Low flag 2
    MoveExactWithAngle(2250, -47);

//#endif

    HitWallAndRecalibrate(0, 600, -1600 /*-2800*/);

#if 0
    //Come back
    HitWallAndRecalibrate(-90, -1800, distanceFromWall);

    //Climb
    Do(TurnToAngle(-180));
    MoveExactWithAngle(1450, -180);
    Do(TurnToAngle(-90));
    MoveToPlatform(true);
#endif

    //Cap 3
    Do(TurnToAngle(-90));
    IntakeDown();
    MoveExactWithAngle(3100, -90);
    MoveExactWithAngle(-550, -90);

    //Low flag 3
    Do(TurnToAngle(0));
    HitWallAndRecalibrate(0, 700, -2650 /*-2800*/);

    //Climb
    Do(TurnToAngle(-90));
    MoveExactWithAngle(1100, -90);
    Do(TurnToAngle(-180));
    MoveExactWithAngle(1500, -180);
    IntakeStop();
    Do(TurnToAngle(-270));
    MoveToPlatform(true);
    
}