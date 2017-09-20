#include "RECEIVER.h"
void RECEIVER::begin(int cal){
    printf("Initializing receiver...\n"); 
    int mem_fd = open("/dev/mem", O_RDWR|O_SYNC|O_CLOEXEC);
    if (mem_fd == -1)printf("Unable to open /dev/mem");
    ring_buffer = (volatile struct ring_buffer*) mmap(0, 0x1000, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, RCIN_PRUSS_RAM_BASE);
    close(mem_fd);
    ring_buffer->ring_head = 0;
    int calibrated=read_calibration();
    if(cal==1)calibrated=0;
    while(calibrated==0){
        calibrated=calibrate();
        if(calibrated==0)printf("Receiver calibration wrong! Can't let you fly with this...\n");
        else printf("Reeceiver calibration accepted!\n");
    }
    save_calibration();
    printf("Receiver Initialized!\n");
    sleep(1);
}
void RECEIVER::read(){
    while (ring_buffer->ring_head != ring_buffer->ring_tail){
        status[0]=1;
        clock_gettime(CLOCK_REALTIME,&real_time_clock);
        rt_clock=real_time_clock.tv_sec+(double)real_time_clock.tv_nsec/1000000000.0f;
        real_period=rt_clock-timer;
        timer=rt_clock;
            if (ring_buffer->ring_tail >= NUM_RING_ENTRIES||real_period>acceptable_period){
                status[0]=0;
                printf("Receiver Malfunctioning... You have a timing problem! Period = %lf\n", real_period);
                return;
            }
	    status[0]=1;
        processPulse((ring_buffer->buffer[ring_buffer->ring_head].s1_t) / TICK_PER_US,
        (ring_buffer->buffer[ring_buffer->ring_head].s0_t) / TICK_PER_US);
        ring_buffer->ring_head = (ring_buffer->ring_head + 1) % NUM_RING_ENTRIES;
    }
    offset_channels();
    check_receiver_status();
}
void RECEIVER::check_receiver_status(){
    uint16_t extra=50;
    for(int i=0;i<number_channels;i++){
        if(raw_channels[i]>upper_limits[i]+extra||raw_channels[i]<lower_limits[i]-extra){
            status[1]=0;
            printf("Receiver Malfunctioning... Values are outside range!\n");
            return;
        }
    }
    status[1]=1;
}
void RECEIVER::processPulse(uint16_t width_1,uint16_t width_2){
    uint16_t pulse=width_1+width_2;
    if(pulse>pulse_limit||channel_counter>=number_channels)channel_counter=0;
    else{
        raw_channels[channel_counter]=pulse;
        channel_counter++;
    }
}
void RECEIVER::setMidPoints(){
    for(int i=0;i<number_channels;i++)mid_points[i]=raw_channels[i];
}
void RECEIVER::setLimits(){
		for(int i=0;i<number_channels;i++){
		    if(raw_channels[i]>upper_limits[i]&&raw_channels[i]<2100)upper_limits[i]=raw_channels[i];
		    if(raw_channels[i]<lower_limits[i]&&raw_channels[i]>900)lower_limits[i]=raw_channels[i];
		}
}
void RECEIVER::print(){
    printf("Channels\n");
    for(int i=0;i<number_channels;i++)printf("%lf, ",channels[i]);
    printf("\n");
}
void RECEIVER::print_raw(){
    printf("Raw Channels\n");
    for(int i=0;i<number_channels;i++)printf("%hu, ",raw_channels[i]);
    printf("\n");
}
void RECEIVER::print_calibration(){
    printMidPoints();
    printLimits();
}
void RECEIVER::printMidPoints(){
    printf("ChX = Mid points\n");
    for(int i=0;i<number_channels;i++){
        printf("Ch%d = %u\n",i,mid_points[i]);
    }
}
void RECEIVER::printLimits(){
    printf("ChX = Max Min\n");
    for(int i=0;i<number_channels;i++){
        printf("Ch%d = %u %u\n",i,upper_limits[i],lower_limits[i]);
    }
}
void RECEIVER::offset_channels(){
    for(int i=0;i<number_channels;i++){
        channels[i]=(float)raw_channels[i]-(float)mid_points[i];
        if(channels[i]>1100)channels[i]=1100;
        if(channels[i]<-500)channels[i]=-500;
    }
}

int RECEIVER::calibrate(){
    printf("Reseting receiver calibration values and calibrating!\n");
    for(int i=0;i<number_channels;i++){
    mid_points[i]=1100;
    upper_limits[i]=1200;
    lower_limits[i]=1100;
    }
    fd_set readfds;
    struct timeval tv;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
	select(STDIN_FILENO+1, &readfds, NULL, NULL, &tv);
    printf("Please move all channels to their maximums/minimums,\n");
	printf("then throttle down, release all sticks and press enter when ready..\n");
    while(FD_ISSET(STDIN_FILENO, &readfds)==0){
        printf("Reading... \r");
	    FD_ZERO(&readfds);
	    FD_SET(STDIN_FILENO, &readfds);
	    tv.tv_sec = 0;
	    tv.tv_usec = 20000;
		select(STDIN_FILENO+1, &readfds, NULL, NULL, &tv);
		read();
		setLimits();
    };
	setMidPoints();
    printf("Calibration finished!\n");
    char msg[10];
    fgets(msg,10,stdin);
    return check_calibration();
}
void RECEIVER::save_calibration(){
    FILE *f;
    f=fopen("mid_points_calibration.txt","w");
    for(int i=0;i<number_channels;i++)fprintf(f,"%u\n",mid_points[i]);
    fclose(f);
    f=fopen("upper_limits_calibration.txt","w");
    for(int i=0;i<number_channels;i++)fprintf(f,"%u\n",upper_limits[i]);
    fclose(f);
    f=fopen("lower_limits_calibration.txt","w");
    for(int i=0;i<number_channels;i++)fprintf(f,"%u\n",lower_limits[i]);
    fclose(f);
	printf("Calibration Saved!\n");
}
int RECEIVER::read_calibration(){
    FILE *f;
    if((f=fopen("mid_points_calibration.txt", "r"))==NULL){
        printf("No mid points calibration file found... Calibration required!\n");
        return 0;
    }
    uint16_t value;
    int i=0;
    fscanf (f, "%hu\n", &value);    
    while (!feof (f))
    {  
        if(i<number_channels)mid_points[i]=value;
        i++;
        fscanf (f, "%hu\n", &value);      
    }
        if(i<number_channels)mid_points[i]=value;
    fclose(f);
    if((f=fopen("upper_limits_calibration.txt", "r"))==NULL){
        printf("No upper limits calibration file found... Calibration required!\n");
        return 0;
    }
    i=0;
    fscanf (f, "%hu\n", &value);    
    while (!feof (f))
    {  
        if(i<number_channels)upper_limits[i]=value;
        i++;
        fscanf (f, "%hu\n", &value);     
    }
        if(i<number_channels)upper_limits[i]=value;
    fclose(f);
    if((f=fopen("lower_limits_calibration.txt", "r"))==NULL){
        printf("No lower limits calibration file found... Calibration required!\n");
        return 0;
    }
    i=0;
    fscanf (f, "%hu\n", &value);    
    while (!feof (f))
    {  
        if(i<number_channels)lower_limits[i]=value;
        i++;
        fscanf (f, "%hu\n", &value);   
    }
         if(i<number_channels)lower_limits[i]=value;
    fclose(f);
    return check_calibration();
}
int RECEIVER::check_calibration(){
    printf("Current Calibration\n");
    for(int i=0;i<number_channels;i++){
        printf("%hu,%hu,%hu\n", upper_limits[i],mid_points[i],lower_limits[i]);
        if(upper_limits[i]>2100||lower_limits[i]<900||mid_points[i]>upper_limits[i]||mid_points[i]<lower_limits[i]){
            printf("Carefull... Needs calibration!\n");
            return 0;
        }
    }
    return 1;
}
