#pragma once
extern "C" {
        #include <rc_usefulincludes.h>
        #include <roboticscape.h>
}
#include "IMU_EKF.h"
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
class INS{
    private:
        float dt=0.02;
        float airspeed_zero;
        float ultrass_ref_altitude;
        float baro_ref_altitude;
        float gps_ref_altitude;
        float accel_bias[3]={0,0,0};
        float accel_scaler[3]={1,1,1};
        void quat_to_euler();
        void correct_order_n_orientation();
        int read_accel_calibration();
        int calibrate_accel();
        int check_accel_calibration();
        void reset();
        void save_accel_calibration();
        void zero_baro_ref();
        void zero_ultrass_ref();
        void zero_references();
    public:
        float Pcov[16];
        float quat[4];
        float Q_info[3]={0.00001,10,5};
        float R_info[2]={0.1,1};
        float gyro_rad[3];
        float accel[3];
        float mag[3];
        float linear_accel[3];
        float gravity[3];
        float position[3];
        float gps_data[3];
        float baro_altitude;
        float ultrass_altitude;
        float airspeed;
        rc_imu_data_t imu;
        float data[18];
        int begin(double ts, int cal_accel);
        void read();
        void read_extra();
        void update();
        void print();
};
