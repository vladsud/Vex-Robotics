#include "main.h"


void initialize(){

  leftDriveEncoder = encoderInit(leftDriveEncoderTopPort, leftDriveEncoderBotPort, true);
  rightDriveEncoder = encoderInit(rightDriveEncoderTopPort, rightDriveEncoderBotPort, true);

  //analogCalibrate(spinnerPotPort);
  //analogCalibrate(anglePotPort);
}
