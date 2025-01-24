#include<REGX52.H>

#define MAX 255
#define MIN 0
#define light1 P2_1
#define light2 P2_2
#define motor2 P2_4

volatile unsigned char data suwak2, pwm2_value;
volatile unsigned char rotations;
volatile unsigned char sent_data = 0;
volatile bit motor_serial = 0;
volatile bit waiting_for_key = 0;
bit motor_direction = 1;

void EXT0_ISR(void) interrupt 0 {
		rotations++;
}


void ISR_Serial(void) interrupt 4 {
    if (RI == 1) {
        if(SBUF == 'F'){
						waiting_for_key = 1;
				}
        RI = 0;
    }
}

void set_motor2(unsigned char percent, bit direction){
    unsigned int temp;
    P2_5 = direction;
    P2_6 = !direction;
    
    temp = (unsigned int)(percent * 254);
    temp = (temp + 50)/100;
    pwm2_value = (unsigned char)temp;
}

void init(){
    SCON = 0x50;    
    
    T2CON = 0x38;   
    RCAP2L = TL2 = 0xDC;
    RCAP2H = TH2 = 0xFF;
    
    TMOD = 0x11;    
    TH0 = 0xFF;     
    TL0 = 0xF0;
    TH1 = 0xFF;
    TL1 = 0xF0;
    
    ET1 = 1;        
    ES = 1;        
	
		IT0 = 1;
		EX0 = 1;		

    TR0 = 1;
    TR1 = 1;
    TR2 = 1;
    TI = 0;
		EA = 1;
    SM1 = 1;
    P3_4 = 0;   
		rotations = 0;    
}

void Timer1_ISR(void) interrupt 3{
    TR1 = 0;
    suwak2++;
    
    if(pwm2_value == MIN) {
        motor2 = 0;
    }
    else if(pwm2_value == MAX) {
        motor2 = 1;
    }
    else {
        motor2 = (suwak2 <= pwm2_value) ? 1 : 0;
    }
    
    if(suwak2 == MAX) suwak2 = MIN;
    
    TH1 = 0xff;
    TL1 = 0xf0;
    TR1 = 1;
}

int main(){
    init();
    set_motor2(60, motor_direction);
    light1 = 1;
		light2 = 0;
    while(1){
        if(waiting_for_key){
            motor_direction = !motor_direction;
            set_motor2(60, motor_direction);
						light1 = !light1;
						light2 = !light2;
            sent_data = 0;
						waiting_for_key = 0;
						rotations = 0;
        };
    }
    
    return 0;
}