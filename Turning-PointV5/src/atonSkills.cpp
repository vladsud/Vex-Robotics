#include "aton.h"

const unsigned int distanceFromWall = 380;
const unsigned int midSecondFlagDistance = 53;
const unsigned int highSecondFlagDistance = 55;
const unsigned int midThirdFlagDistance = 33;
const unsigned int highThirdFlagDistance = 70;


void ResetPostionAfterHittingWall(bool leftWall)
{
    const float robotHalfSize = 7.0;

    PositionInfo pos;
    Coordinates coord;
    PositionTracker& tracker = GetTracker();
    tracker.LatestPosition(false /*clicks*/);
    coord.X = pos.X;
    coord.Y = pos.Y;

    if (leftWall)
    {
        coord.angle = -90;
        coord.X = robotHalfSize;
    }
    else
    {
        coord.angle = 0;
        coord.Y = robotHalfSize;
    }

    tracker.SetCoordinates(coord);
}


void HitLowFlagWithRecovery(unsigned int distanceForward, unsigned int distanceBack, int angleBack, int angleForward)
{
    // should have different signs - positive & negative
    Assert(distanceForward > 0);
    Assert(distanceBack < 0);

    unsigned int distance = HitTheWall(distanceForward, angleForward);

    int actualAngle = GetGyro().Get();
    if (abs(actualAngle) > 5 * GyroWrapper::Multiplier || distance + 300 <= distanceForward)
    {
        unsigned int distanceAdj = 300; // min adjustment - wheels quite often spin without much movement
        if (distance + distanceAdj < distanceForward)
            distanceAdj = distanceForward - distance;
        distanceBack -= distanceAdj;

        ReportStatus("    !!! HitLowFlagWithRecovery: Recovering: a = %d, d = %d, expected d = %d, adj back = %d\n",
            actualAngle / GyroWrapper::Multiplier, distance, distanceForward, distanceAdj);
    }
    else
    {
        // ReportStatus("    Normal hit: a = %d, d = %d, expected d = %d\n", actualAngle / GyroWrapper::Multiplier, distance, distanceForward);
        ResetPostionAfterHittingWall(false /*leftWall*/);
    }

    // not using MoveExactWithAngle() here as we should avoid turning - we may get stuck
    MoveExact(-(int)distanceBack, angleBack);
}


void RunSuperSkills()
{
    auto &main = GetMain();
    main.tracker.SetCoordinates({16, 60+24, -90});

    // async actions
    SetShooterAngle(true /*high*/, g_highFlagHeight-2);

    // Pick up the first ball
    GoToCapWithBallUnderIt(distanceToCap, 2100, -90);

    // Move in front of first flags
    MoveExactWithLineCorrection(2550, 700, 0);

    // Hit the wall - recalibrate angle before shooting
    HitTheWall(-(int)distanceFromWall-80, -90);
    ResetPostionAfterHittingWall(true /*leftWall*/);
    MoveExact(distanceFromWall, -90);

    // Shooting 2 balls at first row
    TurnToAngle(-2 /*angleToShootFlags+1*/);
    ShootTwoBalls(g_highFlagHeight-2, g_midFlagHeight-5);
    IntakeUp();

    // Preset for net shooting
    main.shooter.SetDistance(highSecondFlagDistance);

ReportStatus("\nHitting 1st low flag\n");

    // Hit low flag and come back
    HitLowFlagWithRecovery(3200, 2800, 3 /*angleBack*/);

    // Recalibrate, and move to shooting position for second row of flags
    HitTheWall(-(int)distanceFromWall - 150, -90);
    ResetPostionAfterHittingWall(true /*leftWall*/);
    MoveExactWithAngle(1800, -90);
    

ReportStatus("\nShooting second pole\n");

    TurnToAngle(-24);
    // Shoot middle pole
    ShootTwoBalls(highSecondFlagDistance, midSecondFlagDistance);

    // Preset for net shooting
    main.shooter.SetDistance(midThirdFlagDistance);

    // pick up ball under cap
    TurnToAngle(-90);
    GoToCapWithBallUnderIt(600, 400, -90);
    WaitForBall(500); // wait for the ball actuall to land

    // Flip cap #1    
    IntakeDown();
    FlipCapWithLineCorrection(1300, 25, 1850, -90);

    // Flip cap 2
    FlipCap(1300, 100, 0);
    IntakeDown();

ReportStatus("\nGoing after second low flag\n");

    // Low flag 2
    MoveExactWithLineCorrection(2000, 100, -90);
    MoveExactWithAngle(-550, -90);
    
    HitLowFlagWithRecovery(1800, 1600, 0 /*angleBack*/, 0 /*angleForward*/);

    // Cap 3
    IntakeDown();
    FlipCapWithLineCorrection(2950, 2330, 300, -90);


ReportStatus("\nGoing after 3rd low flag\n");

    //Low flag 3, shoot
    IntakeUp();
    HitLowFlagWithRecovery(1700, 2200);
    TurnToAngle(-13);
    ShootOneBall(true/*high*/, midThirdFlagDistance, false /*checkBallPresence*/);


ReportStatus("\nGoing after platform\n");

    // Climb platform
    MoveExactWithAngle(-2110, 30);
    TurnToAngle(-270);
    MoveToPlatform(true, -270);
}
