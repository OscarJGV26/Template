#include "INS.h"
int INS::begin(double ts,int cal_accel){
    printf("Initializing INS..\n");
    dt=(float)ts;
    if(rc_initialize()){                        //Initialize robotics cape
                fprintf(stderr,"ERROR: failed to run rc_initialize(), are you root?\n");
                return -1;
        }
    rc_cpu_freq_t rc_cpu_freq;
    rc_cpu_freq=FREQ_1000MHZ;                  
    if(rc_set_cpu_freq(rc_cpu_freq)){           //Set CPU frequency at maximum
            printf("rc_set_cpu_freq failed \n");
            return -1;
    }
    printf("CPU Frequency = ");rc_print_cpu_freq();printf("\n");
    rc_imu_config_t conf = rc_default_imu_config();
    conf.enable_magnetometer=1;
    conf.accel_dlpf=ACCEL_DLPF_20;
    conf.gyro_dlpf=GYRO_DLPF_20;
    printf("Initializing IMU...\n");
    if(rc_initialize_imu(&imu, conf)){
            printf("rc_initialize_imu_failed\n");
            return -1;
    }
    int calibrated_accel=read_accel_calibration();
    if(cal_accel==1)calibrated_accel=0;
    while(calibrated_accel==0){
        calibrated_accel=calibrate_accel();
        if(calibrated_accel==0)printf("Accelerometer calibration wrong! Can't let you fly with this...\n");
        else printf("Accelertometer calibration accepted!\n");
    }
    save_accel_calibration();
    sleep(1);
    read();
    reset();
    printf("IMU Initialized!\n");
    printf("Initializing barometer...\n");
    if(rc_initialize_barometer(BMP_OVERSAMPLE_16,BMP_FILTER_16)<0){
		fprintf(stderr,"ERROR: rc_initialize_barometer failed\n");
		return -1;
    }
    printf("Barometer Initialized!\n");
    zero_references();
    read_extra();
    zero_baro_ref();
    printf("INS Initialized!\n");
    return 0;
}
void INS::reset(){
    IMU_EKF(Pcov,quat,Q_info,R_info,gyro_rad,accel,mag,dt,1,1);
}
void INS::read_extra(){
    //Read sensors without predictable timing
    //Read barometer
    rc_read_barometer();
    //Read GPS
    
    //Read Ultrasonic
    
    //Store data
    baro_altitude=rc_bmp_get_altitude_m()-baro_ref_altitude;
}
void INS::read(){
    //Read sensors with predictable timing
    //Read IMU(One-Shot)
    rc_read_gyro_data(&imu);
    rc_read_accel_data(&imu);
    rc_read_mag_data(&imu);
    //Store data (Used for datalogging and coms)
    for(int i=0;i<3;i++){
        data[i+3]=imu.gyro[i];
        data[i+6]=(imu.accel[i]-accel_bias[i])*accel_scaler[i];
        data[i+9]=imu.mag[i];
        gyro_rad[i]=imu.gyro[i]*DEG_TO_RAD;
        accel[i]=data[i+6];
        mag[i]=imu.mag[i];
    }
}
void INS::update(){
    read();             //Read sensors available with predictable timing (IMU)
    correct_order_n_orientation();          //Correct orientation and order of the data
    IMU_EKF(Pcov,quat,Q_info,R_info,gyro_rad,accel,mag,dt,0,1);     //Run IMU EKF
    quat_to_euler();            //Convert quaternion to euler and store it
}
void INS::print(){
    printf("Euler = ");
    for(int i=0;i<3;i++)printf("%f, ",data[i]);
    printf("Gyro = ");
    for(int i=0;i<3;i++)printf("%f, ",data[i+3]);
    printf("Accel = ");
    for(int i=0;i<3;i++)printf("%f, ",data[i+6]);
    printf("Mag = ");
    for(int i=0;i<3;i++)printf("%f, ",data[i+9]);
    printf("Quat = ");
    for(int i=0;i<4;i++)printf("%f, ",quat[i]);
    printf("\n");
}
void INS::correct_order_n_orientation(){
    float temp=data[4];
    data[4]=-data[3];
    data[3]=temp; 
    data[5]=-data[5];
    temp=data[7];
    data[7]=-data[6];
    data[6]=temp;
}
void INS::quat_to_euler(){
  data[0]=-asin(2.0f * (quat[1] * quat[3] - quat[0] * quat[2]))*RAD_TO_DEG;    //Roll is -Pitch
  data[1]=-atan2(2.0f * (quat[0] * quat[1] + quat[2] * quat[3]), quat[0] * quat[0] - quat[1] * quat[1] - quat[2] * quat[2] + quat[3] * quat[3])*RAD_TO_DEG;    //Pitch is -Roll
  data[2]=-atan2(2.0f * (quat[1] * quat[2] + quat[0] * quat[3]), quat[0] * quat[0] + quat[1] * quat[1] - quat[2] * quat[2] - quat[3] * quat[3])*RAD_TO_DEG;    //Yaw is -Yaw
}
int INS::calibrate_accel(){
    printf("Resetting accelerometer calibration values and calibrating...\n");
    //Reset calibration (0 bias - scaler 1)
    float avg[3];
    float max[3];
    // float min[3];
    for(int i=0;i<3;i++){
        accel_bias[i]=0;
        accel_scaler[i]=1;
        avg[i]=0;
        max[i]=0;
        // min[i]=0;
    }
    int N=10;               //Number of samples to take
    char ch;
    printf("Place the IMU/Vehicle horizontal and press enter...\n");
    scanf("%c",&ch);
    for(int i=0;i<N;i++){
        read();
        avg[0]+=accel[0];
        avg[1]+=accel[1];
        max[2]+=accel[2];
        usleep(20000);
    }
    accel_bias[0]=avg[0]/(float)N;
    accel_bias[1]=avg[1]/(float)N;
    printf("Place the IMU/Vehicle on its X axis and press enter...\n");
    scanf("%c",&ch);
    printf("Place the IMU/Vehicle on its Y axis and press enter...\n");
    scanf("%c",&ch);
    printf("Place the IMU/Vehicle on its -X axis and press enter...\n");
    scanf("%c",&ch);
    printf("Place the IMU/Vehicle on its -Y axis and press enter...\n");
    scanf("%c",&ch);
    printf("Place the IMU/Vehicle upside down and press enter...\n");
    scanf("%c",&ch);
    
    return check_accel_calibration();
}
int INS::read_accel_calibration(){ 
    FILE *f;
    if((f=fopen("accel_calibration.txt", "r"))==NULL){
        printf("No acceel calibration file found... Calibration required!\n");
        return 0;
    }
    float value;
    int i=0;
    fscanf (f, "%f\n", &value);    
    while (!feof (f))
    {  
        if(i<3)accel_bias[i]=value;
        else if(i<6)accel_scaler[i-3]=value;
        i++;
        fscanf (f, "%f\n", &value);      
    }
    if(i<6)accel_scaler[i-3]=value;
    return check_accel_calibration();
}
void INS::save_accel_calibration(){
    FILE *f;
    f=fopen("accel_calibration.txt","w");
    for(int i=0;i<3;i++)fprintf(f,"%f\n",accel_bias[i]);
    for(int i=0;i<3;i++)fprintf(f,"%f\n",accel_scaler[i]);
    fclose(f);
	printf("Calibration Saved!\n");
}
int INS::check_accel_calibration(){
    printf("Current Calibration\n");
    for(int i=0;i<3;i++){
        printf("%f,%f\n", accel_bias[i],accel_scaler[i]);
        if(accel_bias[i]>1||accel_bias[i]<-1||accel_scaler[i]>1.3||accel_scaler[i]<0.7){
            printf("Carefull... Needs calibration!\n");
            return 0;
        }
    }
    return 1;
}
void INS::zero_baro_ref(){
    baro_ref_altitude=baro_altitude;
}
void INS::zero_ultrass_ref(){
    ultrass_ref_altitude=0;
}
void INS::zero_references(){
    ultrass_ref_altitude=0;
    baro_ref_altitude=0;
}
