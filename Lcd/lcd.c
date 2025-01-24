#include <REGX52.H>

#define FOOD "\xA5"
#define TOP 0
#define LEFT 1
#define RIGHT 2
#define DOWN 3
#define MAX_LENGTH 40


unsigned char data Var1, Var2, Var3;
volatile unsigned char data sent_data = 0;
volatile bit needsUpdate = 0;
unsigned char data game_start;
volatile unsigned char direction = LEFT;
volatile unsigned char row = 2;
volatile unsigned char col = 8;
volatile unsigned char food_row = 4;
volatile unsigned char food_col = 5;
volatile unsigned char snake_length = 1;
volatile unsigned char highest_score = 0;
unsigned char code snake_skins[] = {
    0xDB,  
    0xFF,  
    0xDD,  
    0xDE,  
    0xDF,  
    0xDC   
};
volatile unsigned char skin_index = 0;
bit waiting_for_highest_score = 0;
bit waiting_for_controls_transmission = 0;
bit force_end = 0;
bit stop_game = 0;

static unsigned int seed = 0;

code unsigned char max_row = 4;    
code unsigned char max_col = 16;


unsigned char data snake_rows[MAX_LENGTH];
unsigned char data snake_cols[MAX_LENGTH];

extern void LcdInit();
extern void LcdWelcome();
extern void Lcd_Cursor(char row, char column);
extern void Lcd_DisplayCharacter(char a_char);
extern void Lcd_DisplayString(char row, char column, char *string);
extern void Lcd_WriteControl(unsigned char LcdCommand);
void gameInit(void);

void ISR_Serial(void) interrupt 4 {
    if (RI == 1) {
        if (waiting_for_highest_score) {
            highest_score = SBUF - '0';
            waiting_for_highest_score = 0;
        }
        else if (SBUF == 'H') {
            waiting_for_highest_score = 1;
        }
        else if (waiting_for_controls_transmission) {
            sent_data = SBUF - '0';
            if (sent_data == -6) { // gwiazdka
                gameInit();
            } else if(sent_data == -13){ // hash
                force_end = 1;
            } else if(sent_data == 0){ // 0
                stop_game = !stop_game;
            } else if (sent_data == 5){
                skin_index = (skin_index + 1) % 6;    
            } else {
                needsUpdate = 1;
            }
            waiting_for_controls_transmission = 0;
        }
        else if (SBUF == 'L') {
            waiting_for_controls_transmission = 1;
        }
        
        
        RI = 0;
    }
}

unsigned int getRandom(unsigned char min, unsigned char max) {
    seed = (seed << 8) | TH0;
    return (seed % (max - min + 1) + min);
}

void placeFood(void) {
    unsigned char valid_position = 0;
    unsigned char attempts = 0;
    unsigned char i;
    
    while (!valid_position && attempts < 20) {
        valid_position = 1;
        food_row = getRandom(1, max_row);
        food_col = getRandom(1, max_col);
        
        // sprawdzenie czy jedzenie nie spawnuje sie na wezu
        for (i = 0; i < snake_length; i++) {
            if (food_row == snake_rows[i] && food_col == snake_cols[i]) {
                valid_position = 0;
                break;
            }
        }
        attempts++;
    }
    
    if (!valid_position) {
        food_row = 1;
        food_col = 1;
    }
}

void showFood(void) {
    Lcd_DisplayString(food_row, food_col, FOOD);
}

void delay(unsigned char time) {
    int i, j;
    for (i = 0; i < time; i++) {
        for (j = 0; j < 62; j++) {;}
    }
}

void sendScore(void) { 
		P3_4 = 1;
		SBUF = 'S';  
    while(!TI);      
    TI = 0; 
		
		delay(5);
	
    SBUF = snake_length + '0';  
    while(!TI);      
    TI = 0; 
		P3_4 = 0;
}

void signalEndGame(void) { 
		P3_4 = 1;
		SBUF = 'E';  
    while(!TI);      
    TI = 0; 
		P3_4 = 0;
}

void signalFood(void) { 
		P3_4 = 1;
		SBUF = 'F';  
    while(!TI);      
    TI = 0; 
		P3_4 = 0;
}

void Init(void) {
    SCON = 0x50;
    T2CON = 0x38;

    TMOD |= 0x01;
    TR0 = 1;

    RCAP2L = TL2 = 0xDC;
    RCAP2H = TH2 = 0xFF;
    TR2 = 1;
    IT0 = 1;
    EX0 = 1;
    ES = 1;
    EA = 1;
    TI = 0;
    P3_4 = 0;
    game_start = 0;
}

void gameInit(void) {

    row = 2;
    col = 8;
    direction = LEFT;
    snake_length = 1;
		sendScore();
    
    snake_rows[0] = row;
    snake_cols[0] = col;
    
    Lcd_WriteControl(0x01);
    Lcd_DisplayString(1, 1, "2 - UP");
    Lcd_DisplayString(2, 1, "4 - LEFT");
    Lcd_DisplayString(3, 1, "6 - RIGHT");
    Lcd_DisplayString(4, 1, "8 - DOWN");
    delay(240);
		delay(240);
    Lcd_WriteControl(0x01);
    game_start = 1;
    placeFood();
}

void checkBounds(void) {
    if (col > 16 && direction == RIGHT) col = 1;
    if (col < 1 && direction == LEFT) col = 16;
    if (row < 1 && direction == TOP) row = 4;
    if (row > 4 && direction == DOWN) row = 1;
}

void checkFoodCollision(void) {
    if (food_row == row && food_col == col) {
        placeFood();
        if (snake_length < MAX_LENGTH) {
            snake_length++;
						sendScore();
						signalFood();
						delay(10);
        }
        delay(10);
    }
}

bit checkSelfCollision(void){
		unsigned char i;
	
		for (i = 1; i < snake_length; i++) {
        if (row == snake_rows[i] && col == snake_cols[i]) {
            return 1;  // kolizja
        }
    }
    return 0;
}

void updateSnakePosition(void) {

		int i;
    for (i = snake_length - 1; i > 0; i--) {
        snake_rows[i] = snake_rows[i-1];
        snake_cols[i] = snake_cols[i-1];
    }
    
    if (direction == RIGHT) col++;
    else if (direction == LEFT) col--;
    else if (direction == DOWN) row++;
    else if (direction == TOP) row--;
    
    checkBounds();
    
    snake_rows[0] = row;
    snake_cols[0] = col;
}

void drawSnake(void) {
		unsigned char i;
    for (i = 0; i < snake_length; i++) {
        Lcd_Cursor(snake_rows[i], snake_cols[i]);
        Lcd_DisplayCharacter(snake_skins[skin_index]);
    }
}

void main(void) {
    LcdInit();
    Init();
    Lcd_DisplayString(1, 2, "* - START Game");
		Lcd_DisplayString(2, 2, "0 - STOP  Game");
		Lcd_DisplayString(3, 2, "# - END   Game");
    Lcd_WriteControl(0x0C);
	
    while (1) {
				while(stop_game){;}
					
        if (needsUpdate) {
            if (sent_data == 2 && direction != DOWN) direction = TOP;
            else if (sent_data == 4 && direction != RIGHT) direction = LEFT;
            else if (sent_data == 6 && direction != LEFT) direction = RIGHT;
            else if (sent_data == 8 && direction != TOP) direction = DOWN;
            needsUpdate = 0;
        }
        
        if (game_start) {
            Lcd_WriteControl(0x01);  
            showFood();
            updateSnakePosition();
						
						if (checkSelfCollision() || force_end) {
								force_end = 0;
								signalEndGame();
								delay(100);
                game_start = 0;  // koniec gry
                Lcd_WriteControl(0x01);  // wyczysc ekran
                Lcd_DisplayString(1, 4, "Game Over !");
								Lcd_DisplayString(2, 3, "Your Score: ");
								if(snake_length > 9){
										Lcd_DisplayCharacter((snake_length / 10) + '0');
										Lcd_DisplayCharacter((snake_length % 10) + '0');
								} else {
										Lcd_DisplayCharacter(snake_length + '0');
								}
                Lcd_DisplayString(3, 5, "Record: ");
								if(highest_score > 9){
										Lcd_DisplayCharacter((highest_score / 10) + '0');
										Lcd_DisplayCharacter((highest_score % 10) + '0');
								} else {
										Lcd_DisplayCharacter(highest_score + '0');
								}
								//Lcd_DisplayCharacter(highest_score + '0');
								Lcd_DisplayString(4, 2, "* - to restart");
                delay(100);
                continue;
            }

            checkFoodCollision();
            drawSnake();
            delay(50);
        }
    }
}