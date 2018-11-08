#include "main.h"


void initialize(){
  leftDriveEncoder = encoderInit(leftDriveEncoderTopPort, leftDriveEncoderBotPort, false);
  rightDriveEncoder = encoderInit(rightDriveEncoderTopPort, rightDriveEncoderBotPort, false);
  leftLiftEncoder = encoderInit(leftLiftEncoderTopPort, leftLiftEncoderBotPort, false);
  rightLiftEncoder = encoderInit(rightLiftEncoderTopPort, rightLiftEncoderBotPort, false);

  //analogCalibrate(spinnerPotPort);
  //analogCalibrate(anglePotPort);
}
