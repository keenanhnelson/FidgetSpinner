#ifndef REVOLUTION_SPEED_H_
#define REVOLUTION_SPEED_H_

#include <stdint.h>

void initRevolutionSpeed();
int16_t getEncoderCnt();
void resetEncoderCnt();
float getRpm();

#endif /* REVOLUTION_SPEED_H_ */
