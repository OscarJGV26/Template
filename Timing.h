#pragma once
#include <signal.h>
#include <time.h>
#define CLOCKID CLOCK_REALTIME
#define SIG SIGRTMIN
void begin(double sample_time);
static void handler(int sig, siginfo_t *si, void *uc);
long long freq_nanosecs;
timer_t timerid;
struct sigevent sev;
struct itimerspec its;
struct timespec real_time_clock;
struct timespec real_time_clock_end;
sigset_t mask;
struct sigaction sa;
double sampling_time_precision=0.001;   //Any hang above 1 ms will alert the system.
volatile int execute=1;
int system_started=0;
double real_period=0;
double execution_time=0;
double sampling_time=0.02;          //20 ms sampling time (standard 50 Hz)
double period_difference;
double rt_clock_end=0;
double rt_clock=0;
double timer=0;
float timing[2];

static void handler(int sig, siginfo_t *si, void *uc){
    execute=1;
}
void timer_begin(){
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIG, &sa, NULL);
    sigemptyset(&mask);
    sigaddset(&mask, SIG);
    sigprocmask(SIG_SETMASK, &mask, NULL);
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.sigev_value.sival_ptr = &timerid;
    timer_create(CLOCKID, &sev, &timerid);
    freq_nanosecs=(long long)(sampling_time*1000000000);
    its.it_value.tv_sec = freq_nanosecs / 1000000000;
    its.it_value.tv_nsec = freq_nanosecs % 1000000000;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
    timer_settime(timerid, 0, &its, NULL);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}
int check_period(){
    rt_clock=real_time_clock.tv_sec+(double)real_time_clock.tv_nsec/1000000000.0f;
    real_period=rt_clock-timer;
    timer=rt_clock;
    timing[0]=(float)real_period;
    period_difference=real_period-sampling_time;
    if(period_difference>sampling_time_precision||
    period_difference<-sampling_time_precision)return 1;
    else return 0;
}
int system_status(int receiver_status[], float channels[]){
    int status=1;                   //Changes status if any violation to required system status
    
    //Timing:
    if(check_period()){             //Checks that the control loop is not hanging.
        printf("Your code is hanging. Fix that!\n");
        return status=0;
    }       
    
    //Receiver checks:
    if(receiver_status[0]!=1){      //Last input was more than 50ms ago.
        printf("Your receiver seems to have disconnected!\n");
        return status=0;      
    }
    if(receiver_status[1]!=1){      //The receiver is not giving acceptable measures
        printf("Your receiver is giving bad inputs!\n");
        return status=0;   
    }
    if(channels[4]<500){            //Using channel 5 for arming/disarming
        printf("Your channel 5 is disarming me!\n");
        return status=0;    
    }
    
    //IMU checks:
    //Currently flying acro mode so no checks.    //Angles within acceptable measures   
    
    ///Initialization Sequence: 
    if(channels[2]<100&&channels[3]>100)system_started=1;         //Initialization sequence is throttle down yaw right
    if(system_started!=1){          //Checks that you have emitted an initialization sequence
        printf("Please initialize your system!\n");
        return status=0;                           
    }
    return status;
}
void check_exec_time(){
        clock_gettime(CLOCK_REALTIME,&real_time_clock_end);
        rt_clock_end=real_time_clock_end.tv_sec+(double)real_time_clock_end.tv_nsec/1000000000.0f;
        execution_time=rt_clock_end-rt_clock;
        timing[1]=(float)execution_time;
}
    
