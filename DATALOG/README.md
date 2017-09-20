# DATALOG
This library performs data logging into an SD card mounted in /media/SD. The code is able to log 9 arrays of different sizes separated by commas. 

An example is given in Datalog_Test where a sinusoidal signal with a given amplitude and frequency is logged on all the arrays at a given sampling time and FOR a given time. It also records any violation of the required sampling time which may be present if there is too much data to log or the sampling time is too fast.

You can call the function as ./Datalog_Test amplitude frequency sampling_time Time

Example,
chrt -r 99 ./Datalog_Test 10 1 0.02 20

Logs a 20 seconds of a sinusoidal signal of amplitude 10, frequency 1 Hz with a sampling time of 0.02 seconds at real time. 
The variable sampling_time_precision defines the limit of violation of the sampling time.
