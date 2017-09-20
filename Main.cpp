#include "COMS/COMS.h"
#include "INPUTS/INPUTS.h"
#include "DATALOG/DATALOG.h"
#include "RECEIVER/RECEIVER.h"
#include "INS/INS.h"
#include "Timing.h"
COMS coms;
INPUTS inputs;
RECEIVER receiver;
DATALOG datalog;
INS ins;
int main(int argc, char *argv[]){
    coms.begin(4);                  //Print data every 5th attempt (around 5 times sampling time) You can call it as 0.1/sampling_time
    inputs.begin(400,0);            //Start inputs @ 400 Hz, ESC calibration (0=No,1=Yes)
    receiver.begin(0);              //Start receiver with no calibration
    datalog.begin();                //Start datalogging
    ins.begin(sampling_time,0);       //Start INS, Accelerometer calibration (0=No,1=Yes)
    timer_begin();                  //Start timer interrupt
    while(rc_get_state()!=EXITING){
        clock_gettime(CLOCK_REALTIME,&real_time_clock);
        while(execute==0);
        execute=0;
        ins.update();
        receiver.read();
        datalog.open();
        if(system_status(receiver.status,receiver.channels)){
            float motors[4]={0,0,0,0};          //Put your control system here and assign something like control.motors to be a float array. 
                                                //Signals must be between 0 and 1000.
                                                //You can also control servos with inputs.write_servos(servos) (0-1000)
                                                //Or with inputs.write(motors,angles) wheree angles signal must be between 0-180.
                                                //You can also reverse this signal, so 180 is 0.
            inputs.write_motors(motors);
        datalog.write(ins.data,ins.data,ins.data,ins.data,ins.data,ins.data,
            ins.data,ins.data,ins.data,timing);
        }else{
            inputs.disarm();
            datalog.close();
        }
        ins.read_extra();
        coms.send(ins.data,ins.data,ins.data,ins.data,ins.data,ins.data);
        check_exec_time();
        sleep(1);                           //Don't remove this sleep cause you will lock the CPU and lose all connectivity "ssh,etc..."
    }
    rc_power_off_imu();
    rc_power_off_barometer();
    rc_cleanup();
    return 0;
}
