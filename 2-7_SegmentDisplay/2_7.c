#include <REGX52.H>

#define PATTERN_LENGTH 8


unsigned char code nums[] = {0x3F,0x06,0x5B,0x4F,0x66,
                            0x6D,0x7D,0x07,0x7F,0x6F};

unsigned char xdata Ones _at_ 0xFD00;
unsigned char xdata Tens _at_ 0xFE00;
volatile unsigned char oness = 0;
volatile unsigned char tenss = 0;
volatile unsigned char num = 0;
volatile unsigned char highest_score = 0;
volatile bit should_send_highest = 0;
bit waiting_for_score = 0;
bit run_simulation1 = 0;
bit run_simulation2 = 0;
bit current_pattern = 0;
volatile unsigned char patternIndex = 0;

const unsigned char code PATTERN1_ONES[PATTERN_LENGTH] = {
    0x00,  
    0x00,  
    0x00,  
    0x08,  
    0x0C,  
    0x06,  
    0x03,  
    0x01   
};

const unsigned char code PATTERN1_TENS[PATTERN_LENGTH] = {
    0x20,  
    0x30,  
    0x18, 
    0x08,  
    0x00,  
    0x00,  
    0x00,  
    0x01   
};

const unsigned char code PATTERN2_ONES[PATTERN_LENGTH] = {
    0x00,
		0x00,
		0x00,
		0x01,
		0x03,
		0x06,
		0x0C,
		0x08
};
const unsigned char code PATTERN2_TENS[PATTERN_LENGTH] = {
    0x10,
		0x30,
		0x21,
		0x01,
		0x00,
		0x00,
		0x00,
		0x08
};

void ISR_INT0 (void) interrupt 0 {
    oness++; 
    if(oness == 10) {
        oness = 0;
        tenss++;  
        if(tenss == 10) {
            tenss = 0;
        }
    }
}

unsigned char max(unsigned char num1, unsigned char num2){
		return num1 > num2? num1 : num2;
}

void sendHighestScore(void){
		P3_4 = 1;
		SBUF = 'H';  
    while(!TI);      
    TI = 0; 
	
    SBUF = highest_score + '0';  
    while(!TI);      
    TI = 0; 
		P3_4 = 0;
}

void ISR_Serial (void) interrupt 4 {
    if (RI == 1) {
        if(waiting_for_score) {
            num = SBUF - '0';
						if(num > 9) {
								oness = num % 10;
								tenss = num / 10;
						} else {oness = num ;}
            waiting_for_score = 0;
        }
        else if(SBUF == 'S') {
            waiting_for_score = 1;
        }
        else if(SBUF == 'E') {
            highest_score = max(highest_score, num);
            should_send_highest = 1;
						tenss = 0;
						oness = 0;
        } else if(SBUF == '1'){
					current_pattern = 0;
					run_simulation1 = 1;
				} else if(SBUF == '3'){
					current_pattern = 1;
					run_simulation2 = 1;
				}
        RI = 0;
    }
}

void Init()
  {
    // konfiguracja portu szeregowego
    SCON  = 0x50; // REN  = 1
    T2CON = 0x38; // TCLK = 1
                  // RCLK = 1
                  // TR2  = 0
                  // EXEN2= 1
    RCAP2L = TL2 = 0xDC;
    RCAP2H = TH2 = 0xFF;     
    TR2 = 1;

    // konfiguracja przerwa?
    IT0 = 1;
    EX0 = 1;
    ES  = 1; // w??czenie IRQ portu szeregowego
    EA  = 1;
    P3_4 = 0;
  }
void refreshDisplay(void) {
    if(patternIndex < PATTERN_LENGTH) {
        if(!current_pattern) {
            // Pattern 1
            Ones = PATTERN1_ONES[patternIndex];
            Tens = PATTERN1_TENS[patternIndex];
        } else {
            // Pattern 2
            Ones = PATTERN2_ONES[patternIndex];
            Tens = PATTERN2_TENS[patternIndex];
        }
    }
}

void sleep(unsigned int time) {
	unsigned int i;
	for(i = 0; i < time; i++) {
	TMOD |= 0x01;
	TH0 = 0xFC;
	TL0 = 0x66;
	TR0 = 1;
	while(!TF0) {
	refreshDisplay();
	}
	TR0 = 0;
	TF0 = 0;
	}
}

void displayPattern1(void) {
    patternIndex = 0;  
    
    while(patternIndex < PATTERN_LENGTH) {
        refreshDisplay();
        sleep(7);
        patternIndex++;
    }
    
    patternIndex = 0; 
}

void displayPattern2(void) {
    patternIndex = 0;  
    
    while(patternIndex < PATTERN_LENGTH) {
        refreshDisplay();
        sleep(7);
        patternIndex++;
    }
    
    patternIndex = 0; 
}

void main(void)
  {
    Init();
    while(1) {
				Ones = nums[oness];
				Tens = nums[tenss];
				if(should_send_highest) {
            sendHighestScore();
            should_send_highest = 0;
        }
				if(run_simulation1){
						displayPattern1();
						run_simulation1 = 0;
				}
				if(run_simulation2){
						displayPattern2();
						run_simulation2 = 0;
				}

		}
  }