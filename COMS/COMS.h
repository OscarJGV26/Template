#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
extern "C" {
        #include <rc_usefulincludes.h>
        #include <roboticscape.h>
}
using namespace std;
#define hostname "192.168.137.1"
#define portno 14550
class COMS{
    private:
        int bus=1;
        int baudrate=57600;
        int count=0;
        int limit=6;
        int data_int=0;
        float data_float=0;
        void print_serial_float(float f);
        void print_serial_int(int f);
        void print_serial_msg(const char msg[],int size);
        float checksum_float=0;
        int checksum_int=0;
        float scaler=1;
        int sockfd,serverlen;
        struct hostent *server;
        struct sockaddr_in serveraddr;
        char buffer[256];
        int buffer_size=256;
        int index=0;
    public:
        int begin(float lim);
        void send(float imu_data[],float motors[],float reference_data[], float roll_parameters[], float pitch_parameters[], float gps_data[]);
};