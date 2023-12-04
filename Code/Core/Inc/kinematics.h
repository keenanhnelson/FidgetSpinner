#ifndef KINEMATAICS_H_
#define KINEMATAICS_H_

#include <stdint.h>

void initKinematics();
int16_t getEncoderCnt();
void resetEncoderCnt();
float getRpm();

#endif /* KINEMATAICS_H_ */
