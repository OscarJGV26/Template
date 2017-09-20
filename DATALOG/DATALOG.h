#pragma once
#include <fstream>
#include <stdio.h>
#include <unistd.h>
using namespace std;
class DATALOG{
    private:
        ofstream myfile;        //File.
        const char * file = "/media/SD/FlightData.txt";
        int precision=4;
        int counter=0;          //Counter for the times called.
        int limit=100;          //close after calling close 100 times consecutively
    public:
        void begin();
        void open();
        void close();
        void write(float imu_data[], float motors[],float references[],float mat1[], float mat2[], float mat3[],float gain1[],float gain2[], float gain3[],float timing[]);
        void read();
};