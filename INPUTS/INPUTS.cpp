#include "INPUTS.h"
void INPUTS::begin(uint16_t freq, int calibrate){
    printf("Initializing Inputs...\n");
    uint32_t mem_fd;
    uint32_t *iram;
    uint32_t *ctrl;
    mem_fd = open("/dev/mem", O_RDWR|O_SYNC|O_CLOEXEC);
    pwm = (struct pwm*) mmap(0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, RCOUT_PRUSS_RAM_BASE);
    iram = (uint32_t*) mmap(0, 0x2000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, RCOUT_PRUSS_IRAM_BASE);
    ctrl = (uint32_t*) mmap(0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, RCOUT_PRUSS_CTRL_BASE);
    close(mem_fd);
    *ctrl = 0;
    std::memcpy(iram, PRUcode, sizeof(PRUcode));
    *ctrl |= 2;
    if(number_motors+number_servos>8){
        printf("Cannot have more than 8 motors/servos in this board... Please change your hardware setup!\n");
        while(1);}
    if(check_repeated_ch()==1){
        printf("Careful!!! You have repeated channels!\n");
        while(1);}
    set_freq(0xFFFFFFFF, freq);
    zero_servos();
    if(calibrate==1)calibrate_motors();
    disarm();
    printf("Inputs Initalized!\n");
    sleep(1);
}
void INPUTS::set_freq(uint32_t chmask, uint16_t freq_hz){
    uint8_t i;
    uint32_t tick = TICK_PER_S / freq_hz;
    for(i = 0; i < PWM_CHAN_COUNT; i++) {
        if(chmask & (1U << i)) {
            pwm->channel[i].time_t = tick;
        }
    }
}
void INPUTS::write_ch(uint8_t ch, uint16_t period_us)
{
    if(ch < PWM_CHAN_COUNT) {
            pwm->channel[ch].time_high = TICK_PER_US * period_us;
   }
}
void INPUTS::enable_ch(uint8_t ch)
{
    if(ch < PWM_CHAN_COUNT) {
        pwm->channelenable |= 1U << ch;
    }
}
void INPUTS::write_motors(float motors[]){
    for(int i=0;i<number_motors;i++){
        motors[i]=limit_us_motor(motors[i]);
        write_ch(motor_channels[i],(uint16_t)motors[i]+min_us_motors);
    }
}
void INPUTS::write_servos(float servos[]){
    for(int i=0;i<number_servos;i++){
        servos[i]=limit_us_servo(servos[i]);
        write_ch(servo_channels[i],(uint16_t)servos[i]+min_us_servos);
    }
}
void INPUTS::write(float motors[],float angles[]){
    write_motors(motors);
    write_servo_angles(angles);
}
float INPUTS::limit_us_motor(float us){
    if(us>range_us_motors)us=(float)range_us_motors;
    if(us<0)us=(float)0;
    return us;
}
float INPUTS::limit_us_servo(float us){
    if(us>range_us_servos)us=(float)range_us_servos;
    if(us<0)us=0;
    return us;
}
void INPUTS::calibrate_motors(){
    float MOTORS[8];
    for(int i=0;i<number_motors;i++)MOTORS[i]=range_us_motors;
    write_motors(MOTORS);
    for(int i=0;i<number_motors;i++)enable_ch(motor_channels[i]);
    sleep(3);
}
void INPUTS::disarm_motors(){
    float MOTORS[8];
    for(int i=0;i<number_motors;i++)MOTORS[i]=0;
    write_motors(MOTORS);
    for(int i=0;i<number_motors;i++)enable_ch(motor_channels[i]);
}
void INPUTS::zero_servos(){
    float ANGLES[8];
    for(int i=0;i<number_servos;i++)ANGLES[i]=zero_angle;
    write_servo_angles(ANGLES);
    for(int i=0;i<number_servos;i++)enable_ch(servo_channels[i]);
}
void INPUTS::write_servo_angles(float angles[]){
    float SERVOS[8];
    for(int i=0;i<number_servos;i++){
        angles[i]-=angle_offsets[i];                //Adjust offset
        angles[i]=limit_servo_angle(angles[i]);
        SERVOS[i]=convert_angle_to_us(angles[i]);
        if(reverse_servo_angle[i])SERVOS[i]=reverse_servo(SERVOS[i]);
    }
    write_servos(SERVOS);
}
void INPUTS::disarm(){
    disarm_motors();
    zero_servos();
}
float INPUTS::reverse_servo(float us){
    us=range_us_servos-us;
    return us;
}
float INPUTS::convert_angle_to_us(float angle){
    float us=angle*range_us_servos/range_angles_servos;
    return us;
}
float INPUTS::limit_servo_angle(float angle){
    if(angle>max_angles_servos)angle=max_angles_servos;
    if(angle<min_angles_servos)angle=min_angles_servos;
    return angle;
}
int INPUTS::check_repeated_ch(){
    int repeated=0;
    for(int i=0;i<number_motors;i++){
        for(int j=0;j<number_motors;j++){
            if(motor_channels[i]==motor_channels[j]&&i!=j){
                repeated=1;
                printf("Repeated motor %d with motor %d\n",motor_channels[i],motor_channels[j]);}
        }
        for(int j=0;j<number_servos;j++){
            if(motor_channels[i]==servo_channels[j]){
                repeated=1;
                printf("Repeated motor %d with servo %d\n",motor_channels[i],servo_channels[j]);}
        }
    }
    for(int i=0;i<number_servos;i++){
        for(int j=0;j<number_motors;j++){
            if(servo_channels[i]==motor_channels[j]){
                repeated=1;
                printf("Repeated servo %d with motor %d\n",servo_channels[i],motor_channels[j]);}   
        }
        for(int j=0;j<number_servos;j++){
            if(servo_channels[i]==servo_channels[j]&&i!=j){
                repeated=1;
                printf("Repeated servo %d with servo %d\n",servo_channels[i],motor_channels[j]);}
        }
    }
    return repeated;
}
