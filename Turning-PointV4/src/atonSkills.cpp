#include "aton.h"

const unsigned int distanceFromWall = 380;

unsigned int HitWallAndRecalibrate(int distanceForward, int distanceBack, int angle, bool calibrateAngle = true)
{
    Assert(distanceForward != 0);
    Assert(distanceForward * distanceBack <= 0); // should have different signs - positive & negative
    Drive& drive = GetMain().drive;

    TurnToAngleIfNeeded(angle);

    ReportStatus("Hitting wall: a=%d d1=%d d2=%d\n", angle, distanceForward, distanceBack);

    int distance = distanceForward * 9 / 10 - Sign(distanceForward) * 250;
    if (distance * distanceForward <= 0)
        distance = Sign(distanceForward) * 100;

    KeepAngle keeper(angle);

    Do(Move(distance, 85, true /*StopOnColision */));
    unsigned int distanceTravelled = drive.m_distance;

    Do(MoveTimeBased(20 * Sign(distanceForward), 5000, true /*waitForStop*/)); // attempt to fully stop, for more accurate back movement
    Do(Wait(100));
    distanceTravelled += drive.m_distance;

    ReportStatus("   Actually travelled: %d\n", distanceTravelled);

    if (calibrateAngle)
        GetMain().tracker.SetAngle(angle);

    Do(MoveExact(distanceBack));

    return distanceTravelled;
}


void HitLowFlagWithRecovery(unsigned int distanceForward, int distanceBack, int angleBack = 0)
{
    const int angle = 0;
    Assert(distanceBack < 0); // should have different signs - positive & negative

    unsigned int distance = HitWallAndRecalibrate(distanceForward, 0 /*distanceBack*/, angle, false /*calibrateAngle*/);

    int actualAngle = GetGyro().Get();
    if (abs(actualAngle) > 6 * GyroWrapper::Multiplier || distance + 300 <= distanceForward)
    {
        ReportStatus("    !!! HitLowFlagWithRecovery: Recovering: a = %d, d = %d, expected d = %d\n", actualAngle, distance, distanceForward);
        if (distance < distanceForward)
            distanceBack -= (distanceForward - distance);
    }
    else
    {
        ReportStatus("    Normal hit: Recovering: a = %d, d = %d, expected d = %d\n", actualAngle, distance, distanceForward);
        GetMain().tracker.SetAngle(angle);
    }

    MoveExactWithAngle(distanceBack, angleBack);
}


void RunSuperSkills()
{
    auto &main = GetMain();
    main.tracker.SetCoordinates({16, 60+24, -90});

    // Pick up the first ball
    GoToCapWithBallUnderIt();
    MoveExactWithAngle(-1930, -90);

    // Move in front of first flags
    MoveExactWithAngle(2600, 0);

    // Recalibrate angle
    HitWallAndRecalibrate(-(int)distanceFromWall-50, distanceFromWall, -90);

    // Shooting 2 balls at first row
    TurnToFlagsAndShootTwoBalls();



    ReportStatus("\nHitting 1st low flag\n");

    // Hit low flag and come back
    HitLowFlagWithRecovery(2750, -2800, 3 /*angleBack*/);

    // Recalibrate, and move to shooting position for second row of flags
    HitWallAndRecalibrate(-(int)distanceFromWall - 50, 0, -90);
    MoveExactWithAngle(2050, -90);
    Do(TurnToAngle(-25));



    ReportStatus("\nShooting second pole\n");

    // Shoot middle pole
    ShootTwoBalls();

    // pick up ball under cap
    Do(TurnToAngle(-90));
    GoToCapWithBallUnderIt(400);

    // Flip cap #1
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



    ReportStatus("\nGoing after second low flag\n");

    // Low flag 2
    MoveExactWithAngle(1400, -90);
    HitLowFlagWithRecovery(1200, -1600);

    //Cap 3
    Do(TurnToAngle(-90));
    IntakeDown();
    MoveWithAngle(1300, -90);
    MoveWithAngle(1200, -90, 18); // slow down a bit
    MoveExactWithAngle(250, -90);



    ReportStatus("\nGoing after 3rd low flag\n");

    //Low flag 3, shoot
    MoveExactWithAngle(-200, -90);
    HitLowFlagWithRecovery(1600, -1850);
    Do(TurnToAngle(-20));
    ShootTwoBalls();



    ReportStatus("\nGoing after platform\n");

    // Climb platform
    MoveExactWithAngle(-2600, 30);
    Do(TurnToAngle(-270));
    MoveToPlatform(true);    
}
