#include "main.h"

void initialize()
{

  leftDriveEncoder = encoderInit(LEFTDRIVEENCODERTOPPORT, LEFTDRIVEENCODERBOTPORT, true);
  rightDriveEncoder = encoderInit(RIGHTDRIVEENCODERTOPPORT, RIGHTDRIVEENCODERBOTPORT, true);

  //analogCalibrate(spinnerPotPort)s;
  //analogCalibrate(anglePotPort);

}
