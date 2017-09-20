#include "INS.h"
#include "Timing.h"
#include <stdio.h>
#include <unistd.h>
#include <math.h>

INS ins;

int main(int argc, char *argv[]){
    double sampling_time;
    sscanf(argv[1],"%lf",&sampling_time);
    ins.begin(sampling_time,0);
    timer_begin(sampling_time);
    int count=0;
    double sampling_time_precision=0.001;  
    double writting_time=0;
    double avg_writting_time=0;
    double sum_writting_time=0;
    double period_difference;
    long i=0;
    while(rc_get_state()!=EXITING){
        clock_gettime(CLOCK_REALTIME,&real_time_clock);
        rt_clock=real_time_clock.tv_sec+(double)real_time_clock.tv_nsec/1000000000.0f;
        real_period=rt_clock-timer;
        timer=rt_clock;
        while(execute==0);
        execute=0;
        i++;
        clock_gettime(CLOCK_REALTIME,&real_time_clock_begin);
        ins.read();
        clock_gettime(CLOCK_REALTIME,&real_time_clock_end);
        rt_clock_begin=real_time_clock_begin.tv_sec+(double)real_time_clock_begin.tv_nsec/1000000000.0f;
        rt_clock_end=real_time_clock_end.tv_sec+(double)real_time_clock_end.tv_nsec/1000000000.0f;
        period_difference=real_period-sampling_time;
        if(period_difference>sampling_time_precision||period_difference<-sampling_time_precision){
            printf("\nDifference detected = %10.10f\n",period_difference);
            count++;
        }
        writting_time=rt_clock_end-rt_clock_begin;
        sum_writting_time+=writting_time;
        avg_writting_time=sum_writting_time/i;
        ins.print();
        printf("Printing..., Real Period = %lf, Average Function Time = %lf, Detected sampling time violations = %d \n", real_period, avg_writting_time,count-2);
        sleep(1);
    }
    printf("Program Terminated!");
}
