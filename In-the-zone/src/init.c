#include "main.h"
Encoder encoderLeft;
Encoder encoderRight;

void initializeIO() {
}


void initialize() {
  encoderLeft = encoderInit(1, 2, false);
  encoderRight = encoderInit(1, 2, false);
}
