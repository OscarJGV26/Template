#include "COMS.h"
using namespace std;
int COMS::begin(float lim){
    printf("Initializing Communications...\n");
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    server = gethostbyname(hostname);
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
	  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    serverlen = sizeof(serveraddr);
    limit=(int)lim;
    if(rc_initialize()){                        //Initialize robotics cape
                fprintf(stderr,"ERROR: failed to run rc_initialize(), are you root?\n");
                return -1;
        }
    if(rc_uart_init(1,57600,0.1)){
        printf("Error: failted to initialize uart!");
        return -1;
    }
    printf("Communications Initialized!\n");
    sleep(1);
    return 0;
}
void COMS::print_serial_float(float f){
    char * b = (char *) &f;
    for(int i=0;i<4;i++){
        buffer[index]=b[3-i];
        index++;
    }
}
void COMS::print_serial_int(int f){
    char * b = (char *) &f;
    for(int i=0;i<2;i++){
        if(index<256)buffer[index]=b[1-i];
        index++;
    }
}
void COMS::print_serial_msg(const char msg[],int size){
    for(int i=0;i<size;i++){
        if(index<256)buffer[index]=msg[i];
        index++;
    }
}
void COMS::send(float imu_data[],float motors[], float references_data[], float roll_parameters[], float pitch_parameters[], float gps_data[]){
    if(count>=limit){
        //Reset counting variables
        count=0;
        index=0;
        
        // Print Roll/Pitch/Yaw angles
        print_serial_msg("ANGL",4);
        scaler=90.0;
        checksum_int=0;
        for(int i=0;i<3;i++){
            data_int=(int)(imu_data[i]*scaler);
            print_serial_int(data_int);
            checksum_int+=data_int;
        }
        print_serial_int(checksum_int);
        
        // Print Roll/Pitch/Yaw rates
        print_serial_msg("RATE",4);
        scaler=13.0;
        checksum_int=0;
        for(int i=0;i<3;i++){
            data_int=(int)(imu_data[i+3]*scaler);
            print_serial_int(data_int);
            checksum_int+=data_int;
        }
        print_serial_int(checksum_int);
        
        // Print Roll/Pitch/Yaw References
        print_serial_msg("REFA",4);
        scaler=13.0;
        checksum_int=0;
        for(int i=0;i<3;i++){
            data_int=(int)(references_data[i]*scaler);
            print_serial_int(data_int);
            checksum_int+=data_int;
        }
        print_serial_int(checksum_int);
        
        //Print Roll/Pitch/Yaw Rate References
        print_serial_msg("REFR",4);
        scaler=13.0;
        checksum_int=0;
        for(int i=0;i<3;i++){
            data_int=(int)(references_data[i+3]*scaler);
            print_serial_int(data_int);
            checksum_int+=data_int;
        }
        print_serial_int(checksum_int);
        
        //Print Motors
        print_serial_msg("MOTO",4);
        scaler=1.0;
        checksum_int=0;
        for(int i=0;i<4;i++){
            data_int=(int)(motors[i]*scaler);
            print_serial_int(data_int);
            checksum_int+=data_int;
        }
        print_serial_int(checksum_int);
        
        //Print Acceleration
        print_serial_msg("ACCL",4);
        scaler=100.0;
        checksum_int=0;
        for(int i=0;i<3;i++){
            data_int=(int)(imu_data[i+6]*scaler);
            print_serial_int(data_int);
            checksum_int+=data_int;
        }
        print_serial_int(checksum_int);
        
        //Print Magnetometer
        print_serial_msg("MAGN",4);
        scaler=10.0;
        checksum_int=0;
        for(int i=0;i<3;i++){
            data_int=(int)(imu_data[i+9]*scaler);
            print_serial_int(data_int);
            checksum_int+=data_int;
        }
        print_serial_int(checksum_int);
        
        //Print Roll Parameters
        print_serial_msg("ROLP",4);
        scaler=10000.0;
        checksum_int=0;
        for(int i=0;i<4;i++){
            data_int=(int)(roll_parameters[i]*scaler);
            print_serial_int(data_int);
            checksum_int+=data_int;
        }
        print_serial_int(checksum_int);
        
        //Print Pitch Parameters
        print_serial_msg("PITP",4);
        scaler=10000.0;
        checksum_int=0;
        for(int i=0;i<4;i++){
            data_int=(int)(pitch_parameters[i]*scaler);
            print_serial_int(data_int);
            checksum_int+=data_int;
        }
        print_serial_int(checksum_int);
        
        //Send buffer
        sendto(sockfd, buffer, 256, 0, (struct sockaddr *)&serveraddr, serverlen);
        rc_uart_send_bytes(1,256,buffer);
    }
    else count++;
}