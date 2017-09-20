#include "INPUTS.h"
#include "Timing.h"
#include <stdio.h>
#include <math.h>
INPUTS inputs;

int main(int argc, char *argv[]){
    inputs.begin(400,0);
    double amplitude,frequency,sampling_time;
    sscanf(argv[1],"%lf",&amplitude);
    sscanf(argv[2],"%lf",&frequency);
    sscanf(argv[3],"%lf",&sampling_time);
    timer_begin(sampling_time);
    double test_signal;
    double pi=3.141592653;
    double omega=frequency*2*pi;
    float motors[8];
    float angles[8];
    int count=0;
    double sampling_time_precision=0.001;  
    double writting_time=0;
    double avg_writting_time=0;
    double sum_writting_time=0;
    double period_difference;
    long i=0;
    while(1){
        while(execute==0);
        i++;
        test_signal=amplitude*sin(omega*i*sampling_time);
        for(int j=0;j<8;j++){
            motors[j]=(float)test_signal*500/amplitude+500;
            angles[j]=(float)test_signal*180/2/amplitude+180/2;
        }
        clock_gettime(CLOCK_REALTIME,&real_time_clock_begin);
        inputs.write(motors,angles);
        clock_gettime(CLOCK_REALTIME,&real_time_clock_end);
        rt_clock_begin=real_time_clock_begin.tv_sec+(double)real_time_clock_begin.tv_nsec/1000000000.0f;
        rt_clock_end=real_time_clock_end.tv_sec+(double)real_time_clock_end.tv_nsec/1000000000.0f;
        period_difference=real_period-sampling_time;
        if(period_difference>sampling_time_precision||period_difference<-sampling_time_precision){
            printf("\n");
            printf("Difference detected = %10.10f\n",period_difference);
            count++;
        }
        writting_time=rt_clock_end-rt_clock_begin;
        sum_writting_time+=writting_time;
        avg_writting_time=sum_writting_time/i;
        printf("Printing..., Real Period = %lf, Average Writting Time = %lf, Detected sampling time violations = %d \r", real_period, avg_writting_time,count-2);
        sleep(1);
    }
    printf("Program Terminated!");
    
}
