#pragma once
#include <stdint.h>
#include "PRU.h"
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#define RCOUT_PRUSS_RAM_BASE 0x4a302000
#define RCOUT_PRUSS_CTRL_BASE 0x4a324000
#define RCOUT_PRUSS_IRAM_BASE 0x4a338000
#define PWM_CHAN_COUNT 12
class INPUTS{
    private:
        static const uint32_t TICK_PER_US = 200;
        static const uint32_t TICK_PER_S = 200000000;
        struct pwm {
            volatile uint32_t channelenable;
            struct {
            volatile uint32_t time_high;
            volatile uint32_t time_t;
            } channel[PWM_CHAN_COUNT];
        };
        volatile struct pwm *pwm;
        uint8_t number_motors=4;
        uint8_t number_servos=4;
        uint8_t motor_channels[8]={0,1,2,3};
        uint8_t servo_channels[8]={4,5,6,7};
        uint16_t max_us_motors=2000;
        uint16_t min_us_motors=1000;
        uint16_t max_us_servos=2250;
        uint16_t min_us_servos=750;
        float max_angles_servos=180;
        float min_angles_servos=0;
        float range_angles_servos=max_angles_servos-min_angles_servos;
        float zero_angle=range_angles_servos/2;
        float angle_offsets[8]={0,0,0,0,0,0,0,0};
        uint8_t reverse_servo_angle[8]={0,0,0,1,0,0,0,0};
        int check_repeated_ch();
        float limit_us_motor(float us);
        float limit_us_servo(float us);
        float limit_servo_angle(float angle);
        float convert_angle_to_us(float angle);
        float reverse_servo(float us);
        void enable_ch(uint8_t ch);
        
    public:
        void begin(uint16_t freq, int calibrate);
        void set_freq(uint32_t chmask, uint16_t freq_hz);
        void write_ch(uint8_t ch, uint16_t period_us);
        uint16_t range_us_motors=max_us_motors-min_us_motors;
        uint16_t range_us_servos=max_us_servos-min_us_servos;
        void disarm_motors();
        void zero_servos();
        void disarm();
        void write(float motors[], float servos[]);
        void write_motors(float motors[]);
        void write_servos(float servos[]);
        void write_servo_angles(float angles[]);
        void calibrate_motors();
};
