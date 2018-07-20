/*
 * roadfighter.c
 *David Anaya
 *
 *Custom Lab Project - Racing Game
 *
 *Nokia LCD: Used LittleBuster Nokia 5110 LCD Library
 *-https://github.com/LittleBuster/avr-nokia5110
 *drawImage() function taken from josephfortune's Nokia 5110 Library 
 -https://github.com/josephfortune/NokiaLCD
 *
 *SNES controller: readcontroller() was made with help from /u/Crumster's code on Reddit. Modified to work for my use
 *-https://www.reddit.com/r/avr/comments/2fcslw/reading_a_snesnes_controller/?st=ji9vz0ug&sh=ac14ec7f
 *
 *LED Matrix: Help from LED Matrix Lab 
 *
 * 
 * 
 */ 


#include <avr/io.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include "timer.h"
#include "io.c"

#include "nokia5110.c"
//#include "NokiaLCD.c"

#define POS_MIN 2
#define POS_MAX 31

//pins
#define LATCH   PORTA0
#define CLOCK   PORTA1
#define DATA    PORTD0

//Bit position of button
#define btn_B 0
#define btn_Y 1
#define btn_SELECT 2
#define btn_START 3
#define btn_UP 4*
#define btn_DOWN 5
#define btn_LEFT 6
#define btn_RIGHT 7
#define btn_A 8
#define btn_X 9
#define btn_L 10
#define btn_R 11
//button bit order 0-15 --> B, Y, SELECT, START, UP, DOWN, LEFT, RIGHT, A, X, L, R, NA, NA, NA, NA


enum Movement_SM {Move_Start, Move_Wait, Move_LEFT, Move_RIGHT} move_state;
	
enum Scroll_SM {Scroll_Start, Scroll_Wait, Scroll_GO} scroll_state;

enum Enemy_SM {Enemy_Start, Enemy_SPAWN, Enemy_DESPAWN} enemy_state;
	
enum GameLogic_SM {Game_Start, Game_Play, Game_Over} game_state;
	
enum Display_SM {Disp_Start, Disp_Title, Disp_HiScore, Disp_Count, Disp_Display} disp_state;

uint16_t data = 0x0000; //16 bits of controller data

unsigned char carPos = POS_MIN; //Sets the Y position. X never changes 
unsigned char roadPos = 0;
unsigned char enemyPOSx = 0;
unsigned char enemyPOSy = 0;
unsigned char MOVE_FLAG = 0;
unsigned char SCROLL_FLAG = 0;
unsigned char START_FLAG = 0;
unsigned char ENEMY_FLAG = 0;
unsigned char HIT_FLAG = 0;
unsigned short time = 0;
unsigned char cnt_down = 0x04;
uint8_t score = 0;
uint8_t hiScore = 0;
unsigned char i, j, k, a, b;
unsigned char lanePos[]  = {76, 60, 44, 28, 12};

//84x48 

uint8_t car[] = {14, 24,
	0xfe, 0x07, 0xe0, 0xfe, 0x07, 0xe0, 0x7f, 0xff, 0xfc, 0x7f,
	0xff, 0xfc, 0x7c, 0x78, 0xff, 0x7c, 0x78, 0x7f, 0x7c, 0x78,
	0x7f, 0x7c, 0x78, 0x7f, 0x7c, 0x78, 0x7f, 0x7c, 0x78, 0xff,
	0x7f, 0xff, 0xfc, 0x7f, 0xff, 0xfc, 0xfe, 0x07, 0xe0, 0xfe,
	0x07, 0xe0,
};

uint8_t enemyCar[] = {14, 24,
	0x3e, 0x00, 0x7c, 0xff, 0xff, 0xfe, 0xff, 0xff, 0xff, 0xe1,
	0xfc, 0xff, 0xe1, 0xfc, 0x7f, 0xe1, 0xfc, 0x7f, 0xe1, 0xfc,
	0x7f, 0xe1, 0xfc, 0x7f, 0xe1, 0xfc, 0x7f, 0xe1, 0xfc, 0x7f,
	0xe1, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x3e,
	0x00, 0x7c,
};

uint8_t lane[] = { 2, 8,
	0xff, 0xff, };

uint8_t laneTest[] = { 2, 24,
	0xff, 0x00, 0xff,  0xff, 0x00, 0xff
};

uint8_t edge[] = { 1, 84,
	0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	0xf0,};

uint8_t start[] = {22, 5,
	0x08, 0xf8, 0x08, 0x00, 0xb0, 0x48, 0x48, 0xf8, 0x00, 0xf0,
	0x28, 0x28, 0xf0, 0x00, 0x08, 0xf8, 0x08, 0x00, 0x48, 0xa8,
	0xa8, 0x90, };
	
uint8_t title[] = { 43, 23,
	0x33, 0x00, 0x00, 0x04, 0x80, 0x00, 0x04, 0x80, 0x00, 0x04,
	0x80, 0x00, 0x3f, 0x80, 0x00, 0x3f, 0x80, 0x00, 0x00, 0x00,
	0xfc, 0x20, 0x81, 0x02, 0x20, 0x81, 0x02, 0x22, 0x81, 0x02,
	0x22, 0x81, 0x02, 0x3f, 0x81, 0xfe, 0x3f, 0x81, 0xfe, 0x00,
	0x00, 0x00, 0x00, 0x81, 0xfc, 0x00, 0x80, 0x12, 0x3f, 0x80,
	0x12, 0x3f, 0x80, 0x12, 0x00, 0x80, 0x12, 0x00, 0x01, 0xfe,
	0x3f, 0x81, 0xfc, 0x02, 0x00, 0x00, 0x02, 0x00, 0xfc, 0x02,
	0x01, 0x02, 0x3f, 0x81, 0x02, 0x3f, 0x81, 0x02, 0x00, 0x01,
	0x02, 0x3d, 0x81, 0xfe, 0x20, 0x80, 0xfc, 0x20, 0x80, 0x00,
	0x3f, 0x81, 0xbc, 0x1f, 0x00, 0x22, 0x00, 0x00, 0x22, 0x3e,
	0x80, 0x22, 0x3e, 0x80, 0x22, 0x00, 0x01, 0xfe, 0x00, 0x41,
	0xfe, 0x00, 0x40, 0x00, 0x00, 0x40, 0x00, 0x02, 0x40, 0x00,
0x02, 0x40, 0x00, 0x3f, 0xc0, 0x00, 0x3f, 0xc0, 0x00, };


uint8_t cnt_3[] = { 4, 6,
0x68, 0x94, 0x94, 0x84, };

uint8_t cnt_2[] = { 4, 6,
0x88, 0x94, 0xa4, 0xe4, };

uint8_t cnt_1[] = { 3, 6,
0xfc, 0x08, 0x10, };

uint8_t cnt_GO[] = { 18, 8,
	0xbf, 0xbf, 0x00, 0x00, 0x00, 0x7c, 0x82, 0x82, 0x82, 0xfe,
0x7c, 0x00, 0xe2, 0xa2, 0xa2, 0x82, 0xfe, 0x7c, };
 
uint8_t gameover[] = { 25, 17,
	0x00, 0x21, 0x00, 0xd8, 0x25, 0x00, 0x24, 0x25, 0x00, 0x24,
	0x3f, 0x00, 0xfc, 0x3f, 0x00, 0xfc, 0x00, 0x00, 0x00, 0x3f,
	0x00, 0x84, 0x01, 0x00, 0x94, 0x02, 0x00, 0x94, 0x01, 0x00,
	0xfc, 0x3f, 0x00, 0xfc, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x3c,
	0x3e, 0x00, 0x40, 0x09, 0x00, 0xc0, 0x09, 0x00, 0x7c, 0x3f,
	0x00, 0x3c, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x7c, 0x38, 0x80,
	0x82, 0x28, 0x80, 0x82, 0x28, 0x80, 0x82, 0x20, 0x80, 0xfe,
0x3f, 0x80, 0x7c, 0x1f, 0x00, };


//52x2

void readController(){
	data = 0x0000;

	//pull latch to read new data
	PORTA |= (1 << LATCH);
	PORTA |= (1 << CLOCK);
	_delay_us(12);
	PORTA &= ~(1 << LATCH);
	_delay_us(6);

	//read bits
	for(int i = 0; i < 16; i++){
		PORTA |= (1 << CLOCK);
		_delay_us(6);
		data |= ((PIND & 1) << i);
		PORTA &= ~(1 << CLOCK);
		_delay_us(6);
	}
	
	data = ~data;
}

void Movement(){
	readController();
	
	switch(move_state){
		case Move_Start:	move_state = Move_Wait;
		
		break;
		
		case Move_Wait:		if(!START_FLAG){
								move_state = Move_Wait;
							}
							else if((data & (1 << btn_RIGHT))){
								move_state = Move_RIGHT;
							}
							else if((data & (1 << btn_LEFT))){
								move_state = Move_LEFT;
							}
							else{
								move_state = Move_Wait;
							}
		
		break;
		
		case Move_RIGHT:	if(data & (1 << btn_RIGHT)){
								move_state = Move_RIGHT;
								}
							else{
								move_state = Move_Wait;
							}
		
		break;
		
		case Move_LEFT:		if(data & (1 << btn_LEFT)){
								move_state = Move_LEFT;
							}
							else{
								move_state = Move_Wait;
							}
		
		break;
		
		default:			move_state = Move_Start;
		break;
			
	}
	
	switch(move_state){
		case Move_Start:
		
		break;
		
		case Move_Wait:		MOVE_FLAG = 0;
		
		break;
		
		case Move_RIGHT:	if(carPos < POS_MAX){
								if(carPos == POS_MAX - 1){
									++carPos;
								}
								else{
									carPos += 2;
								}
								
							}
							MOVE_FLAG = 1;
		break;
		
		case Move_LEFT:		if(carPos > POS_MIN){
								if(carPos == POS_MIN + 1){
									--carPos;
								}
								else{
									carPos -= 2;
								}		
							}
							MOVE_FLAG = 1;
		break;
		
		default:
		break;
		
	}
	
	
}


void Scroll(){
	readController();
	
	switch(scroll_state){
		case Scroll_Start:		scroll_state = Scroll_Wait;
		
		break;
		
		case Scroll_Wait:		if(data & (1 << btn_B) || data & (1 << btn_L) ){
									scroll_state = Scroll_GO;
								}
								else{
									scroll_state = Scroll_Wait;
								}
		
		break;
		
		
		case Scroll_GO:			if(data & (1 << btn_B) || data & (1 << btn_L)){
									scroll_state = Scroll_GO;
								}
								else{
									scroll_state = Scroll_Wait;
								}
		
		break;
		
		default:				scroll_state = Scroll_Start;
		break;
		
		
	}
	
	switch(scroll_state){
		case Scroll_Start:
		
		break;
		
		case Scroll_Wait:	SCROLL_FLAG = 0;
							if(enemyPOSx < 60 && ENEMY_FLAG){
								enemyPOSx += 2;
							}
							else{
								enemyPOSx = 60;
							}
		
		break;
		
		
		case Scroll_GO:		if(i < sizeof(lanePos)/sizeof(lanePos[0]) - 1){
								++i;
							}
							else {
								i = 0;
							}
							if(j < sizeof(lanePos)/sizeof(lanePos[0]) - 1){
								++j;
							}
							else {
								j = 0;
							}
							
							if(enemyPOSx > 0){
								enemyPOSx -= 2;
							}
							else{
								enemyPOSx = 60;
							}
							
							SCROLL_FLAG = 1;
		break;
		
		default:
		break;
		
		
	}
	
	
}

void Enemy(){
	static unsigned char i = 0;
	
	switch(enemy_state){
		case Enemy_Start:		enemy_state = Enemy_DESPAWN;		
		
		break;
		
		case Enemy_DESPAWN:		if(i > 15){
									enemy_state = Enemy_SPAWN;
									i = 0;
									uint16_t tmp = rand();
									enemyPOSx = 60;
									//enemyPOSy = tmp % 2 ? 30 : 5;
									enemyPOSy = rand() % 26 + 5;
									ENEMY_FLAG = 1;
								}
								else{
									enemy_state = Enemy_DESPAWN;
								}
		
		break;
		
		case Enemy_SPAWN:		if((!SCROLL_FLAG && enemyPOSx >=60) || i >= 30){
									enemy_state = Enemy_DESPAWN;
									ENEMY_FLAG = 0;
									i = 0;
								}
								else{
									enemy_state = Enemy_SPAWN;
								}
		
		break;
		
		default:				enemy_state = Enemy_Start;
		break;
		
	}
	
	switch(enemy_state){
		case Enemy_Start:		
		
		break;
		
		case Enemy_DESPAWN:		if(SCROLL_FLAG){
									++i;
								}
								else{
									i = 0;
								}	
								srand(time / 3);
								ENEMY_FLAG = 0;
								
						
		break;
		
		case Enemy_SPAWN:		if(SCROLL_FLAG){
									++i;
								}
								else{
									i = 0;
								}
								ENEMY_FLAG = 1;
		
		break;
		
		default:
		break;
		
	}
	
}

void GameLogic(){
	switch(game_state){
		case Game_Start:	MOVE_FLAG = 0;
							SCROLL_FLAG = 0;
							START_FLAG = 0;
							ENEMY_FLAG = 0;
							HIT_FLAG = 0;
							carPos = POS_MIN;
							score = 0;
							cnt_down = 0x04;
							time = 0;
							game_state = Game_Play;
		break;
		
		case Game_Play:		if(HIT_FLAG){
								game_state = Game_Over;
							}
							else{
								game_state = Game_Play;
							}
		
		break;
		
		case Game_Over:		game_state = Game_Over;
		
		break;
		
		default:			game_state = Game_Start;
		break;		
	}
	
	switch(game_state){
		case Game_Start:
		
		break;
		
		case Game_Play:		if(START_FLAG){
								PORTA &= 0x7F;
							}
							
							if(enemyPOSx == 0){
								++score;
							}
							
							//Checks if cars hit
							for(unsigned char w = carPos; w < carPos + 15; ++w){
								if(enemyPOSx <= 24 && enemyPOSy == w){
									HIT_FLAG = 1;
								}
							}
		
		break;
		
		case Game_Over:		hiScore = eeprom_read_byte((uint8_t*)46);
							if(score > hiScore){
								hiScore = score;
								eeprom_update_byte((uint8_t*)46, hiScore);
							}
							_delay_ms(7000);
							scroll_state = Scroll_Start;
							move_state = Move_Start;
							disp_state = Disp_Start;
							enemy_state = Enemy_Start;
							game_state = Game_Start;
							enemyPOSx = POS_MAX;
							enemyPOSy = 60;
							HIT_FLAG = 0;
							
		break;
		
		default:
		break;
	}
}
	

void Display(){
	readController();
	static unsigned short disp_cnt = 0;
	char scr_str[16] = "Score:";
	char hscr_str[16] = "Hi-Score:";
	char score_str[128];
	char hscore_str[128];
	
	switch(disp_state){
		case Disp_Start:	disp_state = Disp_Title;
		
		break;
		
		case Disp_Title:	if(data & (1 << btn_START)){
								disp_state = Disp_Count;
								PORTA |= 0x80;
							}
							else if(data & (1 << btn_SELECT)){
								disp_state = Disp_HiScore;
							}
							else{
								disp_state = Disp_Title;
							}
		
		break;
		
		case Disp_HiScore:	if(data & (1 << btn_B)){
								disp_state = Disp_Title;
							}
							else{
								disp_state = Disp_HiScore;
							}
		
		break;
		
		case Disp_Count:	if(cnt_down == 1){
								disp_state = Disp_Display;
								PORTA &= 0x7F;
								START_FLAG = 1;
								cnt_down = 0x04;
							}
							else{
								disp_state = Disp_Count;
							}
		
		break;
		
		case Disp_Display:	disp_state = Disp_Display;
							
		
		break;
		
		default:			disp_state = Disp_Start;
		break;
		
	}
	
	switch(disp_state){
		case Disp_Start:
		
		break;
		
		case Disp_Title:		nokia_lcd_clear();
								nokia_lcd_drawImage(title, 52, 2);
								nokia_lcd_drawImage(start, 25, 14);
								nokia_lcd_render();
								LCD_ClearScreen();
								++time;
		break;
		
		case Disp_HiScore:		hiScore = eeprom_read_byte((uint8_t*)46);
								utoa(hiScore, hscore_str, 10);
								strcat(hscr_str, hscore_str);
								LCD_ClearScreen();
								LCD_DisplayString(1, hscr_str);
		break;
		
		case Disp_Count:		nokia_lcd_clear();
								nokia_lcd_drawImage(lane, lanePos[0], 23);
								nokia_lcd_drawImage(lane, lanePos[1], 23);
								nokia_lcd_drawImage(lane, lanePos[2], 23);
								nokia_lcd_drawImage(lane, lanePos[3], 23);
								nokia_lcd_drawImage(lane, lanePos[4], 23);
								
								nokia_lcd_drawImage(edge, 0, 0);
								nokia_lcd_drawImage(edge, 0, 46);
								nokia_lcd_drawImage(car, 0, carPos);
								
		
								if(disp_cnt >= 20){
									--cnt_down;
									disp_cnt = 0;
								}
								else{
									++disp_cnt;
								}
								if(cnt_down == 0x04){
									nokia_lcd_drawImage(cnt_3, 56, 30);
								}
								else if(cnt_down == 0x03){
									nokia_lcd_drawImage(cnt_2, 56, 30);
								}
								else if(cnt_down == 0x02){
									nokia_lcd_drawImage(cnt_1, 56, 30);
								}
								else if(cnt_down == 0x01){
									nokia_lcd_drawImage(cnt_GO, 56, 25);
								}
									
								nokia_lcd_render();
		
		break;
		
		
		case Disp_Display:		nokia_lcd_clear();
								if(cnt_down > 0){
									nokia_lcd_set_cursor(70, 30);
									nokia_lcd_write_char(cnt_down + '0', 1);	
									--cnt_down;	
								}
								
								if(HIT_FLAG){
									nokia_lcd_drawImage(gameover, 48, 11);
									nokia_lcd_render();
									return;
								}
								
								
								if(SCROLL_FLAG){
									nokia_lcd_drawImage(lane, lanePos[i], 23);
									nokia_lcd_drawImage(lane, lanePos[j], 23);
								}
								else{
									nokia_lcd_drawImage(lane, lanePos[0], 23);
									nokia_lcd_drawImage(lane, lanePos[1], 23);
									nokia_lcd_drawImage(lane, lanePos[2], 23);
									nokia_lcd_drawImage(lane, lanePos[3], 23);
									nokia_lcd_drawImage(lane, lanePos[4], 23);
								}
								
								if(ENEMY_FLAG){
									nokia_lcd_drawImage(enemyCar, enemyPOSx, enemyPOSy);
								}
								
							
								nokia_lcd_drawImage(edge, 0, 0);
								nokia_lcd_drawImage(edge, 0, 46);
								nokia_lcd_drawImage(car, 0, carPos);
								
								nokia_lcd_render();
								
								utoa(score, score_str, 10);
								strcat(scr_str, score_str);
								LCD_ClearScreen();
								LCD_DisplayString(1, scr_str);
								
								++time;
								
								if(data & (1 << btn_L) && data & (1 << btn_R) && data & (1 << btn_A) && data & (1 << btn_SELECT) ){ //Resets Game
									scroll_state = Scroll_Start;
									move_state = Move_Start;
									disp_state = Disp_Start;
									enemy_state = Enemy_Start;
									game_state = Game_Start;
								}
		
		break;
		
		default:
		break;
	}
	
	
	
	
}

int main(void)
{
	DDRA = 0x8F; PORTA = 0x70;
	//PORTA &= ~(1 << LATCH) & ~(1 << CLOCK); //sets latch and clock low
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0x00; PORTD = 0xFF;
	
	scroll_state = Scroll_Start;
	move_state = Move_Start;
	disp_state = Disp_Start;
	enemy_state = Enemy_Start;
	game_state = Game_Start;
	
	enemyPOSx = 60;
	
	//eeprom_update_byte((uint8_t*)46, 0);
	
	LCD_init();
	
	nokia_lcd_init();
	nokia_lcd_clear();
	
	TimerSet(50);
	TimerOn();
	
    while(1)
    {		
		Movement();
		Scroll();
		GameLogic();
		Enemy();
		Display();
			
		while(!TimerFlag){}
		
		TimerFlag = 0;
		
    }
}