Action* g_actionsFirstPos[] = {
    
    new Wait(50),

    // Shoot the ball
    new ShooterAngle(false, 48, false /*checkPresenceOfBall*/),
    new ShootBall,
    new IntakeUp,

    new Wait(100),
    
    new Move(200, 45, 0),
    new Move(550, 50, g_lcd.AtonBlueRight ? 3.5 : -5), // turn left for red
    new Move(1350, 60, 0),

    new Wait(50),

    // turn closer to center, but then streighten to look dire ct at flag
    // This should allow us to look directly at flag, but also to be far away from border to turn around
    new Move(100, -35, 0),
    new Move(600, -50, g_lcd.AtonBlueRight ? 8 : -8), //turn rigth for red 
    new Move(100, -35, 0),
    new Move(1000, -45, g_lcd.AtonBlueRight ? -4 : 4), //turn rigth for red 
    new Move(350, -65, 0),
    //new Move(200, -35, 0),

    new IntakeStop,
    new Wait(50),
    new ShooterAngle(true, 48, true /*checkPresenceOfBall*/),
    new ShootBall,
    new Wait(30)
};

Action* g_knockConeFirstPos[] = {
    new TurnToCenter(),
    new Move(1650, 50),
    new Move(200, 35),
    new IntakeUp,
    new Wait(20),
    new Move(300, -45),
    new IntakeStop
};

Action* g_ParkFromFirstPos[] = {
    new IntakeUp,
    new Move(850, -50),
    new Wait(30),
    new TurnToCenter(),
    new Move(300, 65),  // first plat: 500
    new MoveToPlatform(3250, 85), // first platform: 2050

/* SECOND PLATFORM
    new Move(100, 85),
    new MoveToPlatform(3250, 85), // first platform: 2050
    new MoveTimeBased(-30, 10)
*/
};

Action* g_ShootFromSecondPos[] = {
    new Wait(1000),
    // Shoot the ball
    new ShooterAngle(false, 108, false /*checkPresenceOfBall*/),
    new ShootBall,
};

/*
Action* g_WaitInsteadOfShot[] = {
    new Wait(450)
};
*/

// FYI: It takes about 6 seconds to park
Action* g_ParkFromSecondPos[] = {
    //new ShooterAngle(false, 108, false /*checkPresenceOfBall*/),
    // give some time for other robot to get out of the way
    // FYI: It takes about 7 seconds to park
    //new Wait(400),
    //new ShootBall,
    new Move(850, 50),
    new Wait(100),
    new Turn(g_lcd.AtonBlueRight ? 45 : -45),
    new Wait(20),
    new Move(300, -65),  // first plat: 500
    new Wait(40),
    new Move(700, 85),  // first plat: 500
    new MoveToPlatform(3250, 85) // first platform: 2050
};

// NYI
Action* g_knockConeSecondPos[] = {};

#if TESTING

unsigned int TestDistance = 24;
Action*g_testAngles[] = {
    // new ShooterAngle(Flag::Loading, TestDistance, false /*checkPresenceOfBall*/),
    // new Wait(50),
    new ShooterAngle(false, TestDistance, false /*checkPresenceOfBall*/),
    new Wait(50),
    new ShooterAngle(true, TestDistance, false /*checkPresenceOfBall*/),
    new Wait(50),
    new ShooterAngle(false, TestDistance, false /*checkPresenceOfBall*/),
    new Wait(50),
};


Action*g_testDrive[] = {
    new Turn(45),
    new Wait(50),
    new Turn(45),
};

#endif