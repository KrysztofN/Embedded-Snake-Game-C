#include <REGX52.H>
// Neccesary includes
#define LED P0_0

unsigned char code Tab[] = {0xEF,0xDF,0xBF,0x7F};
volatile unsigned char bdata Port = 0;
volatile bit Bylem = 0;

sbit K1 = Port^3;
sbit K2 = Port^2;
sbit K3 = Port^1;

void Init(void) {
    SCON  = 0x50;    
    T2CON = 0x38;    
    RCAP2L = TL2 = 0xDC;
    RCAP2H = TH2 = 0xFF;     
    TR2 = 1;         
    P3_4 = 0;
    TI = 0;
}

void Send_Key(unsigned char key) {
    SBUF = key;
    while(TI != 1);  
    TI = 0;          
}

void sleep(unsigned char time){
		unsigned char i,j;
		for(i = 0; i < 123; i++){
				for(j = 0; j <= time; j++) {;}
		}
}

void blink_n_times(unsigned char times){
		unsigned char i;
		for(i=0; i< times; i++){
				LED = 0;
				sleep(20);
				LED = 1;
				sleep(20);
		}
}

void main(void) {
    unsigned char ind = 0;
		LED = 1;
	
    Init();
    
    while(1) {
        P2 = Tab[ind];
        Port = P2;
        
        if ((K1 & K2 & K3) == 0) {
            if (Bylem == 0) {
                P3_4 = 1;
                
                switch(Port) {
										case 0xE7:
												Send_Key('1');
												blink_n_times(1);
												break;
										case 0xEB: 
												Send_Key('L');
                        Send_Key('2');
                        break;
										case 0xED:
												Send_Key('3');
												blink_n_times(3);
												break;
                    case 0xD7:  
												Send_Key('L');
                        Send_Key('4');
                        break;
										case 0xDB:
												Send_Key('L');
												Send_Key('5');
												break;
                    case 0xDD:  
												Send_Key('L');
                        Send_Key('6');
                        break;
										case 0xB7:
												//Send_Key('7');
												blink_n_times(7);
												break;
                    case 0xBB:  
												Send_Key('L');
                        Send_Key('8');
                        break;
										case 0xBD:
												//Send_Key('9');
												blink_n_times(9);
												break;
										case 0x7B:
												Send_Key('L');
												Send_Key('0');
												break;
										case 0x77:
												Send_Key('L');
                        Send_Key('*');
                        break;
										case 0x7D:
												Send_Key('L');
                        Send_Key('#');
                        break;
                }
                
                Bylem = 1;
                P3_4 = 0;
            }
        } else {
            ind++;
            Bylem = 0;
            if (ind == 4) {
                ind = 0;
            }
        }
    }
}