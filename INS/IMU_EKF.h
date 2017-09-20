#pragma once
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
extern void IMU_EKF(float P[16], float q[4], const float Q_info[3], const float
                   R_info[2], const float omega[3], float accel[3], float mag[3],
                   float dt, signed char ini, signed char use_mag);
