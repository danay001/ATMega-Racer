/*
 * LED_Matrix.c
 *
 * Created: 5/29/2018 4:37:08 PM
 *  Author: Anaya
 */ 


#include <avr/io.h>
#include <util/delay.h>

// ====================
// SM1: DEMO LED matrix
// ====================
enum SM1_States {sm1_start, sm1_wait, sm1_first, sm1_second, sm1_third, sm1_green} state;
	
unsigned char tmpB = 0x00;
	
void SM1_Tick() {

	// === Local Variables ===
	static unsigned char column_val = 0x00; // sets the pattern displayed on columns
	static unsigned char column_sel = 0xFF; // grounds column to display pattern
	
	tmpB = PINB & 0x01;
	
	// === Transitions ===
	switch (state) {
		case sm1_start:		state = sm1_first;
		
		break;
		
		case sm1_wait:		if(tmpB == 0x01){
								state = sm1_start;
							}
							else{
								state = sm1_wait;
							}
		break;
		
		case sm1_first:    state = sm1_second;
		
		break;
		
		case sm1_second:	state = sm1_third;
		
		break;
		
		case sm1_third:		state = sm1_green;
		
		break;
		
		case sm1_green:		state = sm1_wait;
		
		break;
		default:   	        state = sm1_wait;
		break;
	}
	
	// === Actions ===
	switch (state) {
		case sm1_start:		column_val = 0x00;
							column_sel = 0xFF;
							PORTD = 0x00;
							PORTA = column_val;
							PORTC = column_sel;
							
		break;
		
		case sm1_wait:		PORTD = 0x00;
							PORTA = 0x00;
							PORTC = 0x00;
		break;
		
		
		case sm1_first:		column_val = 0xC0;
							column_sel = 0xFF;
							PORTA = column_val;
							PORTC = column_sel;
		
		break;
		
		case sm1_second:	column_val = 0xF0;
							column_sel = 0xFF;
							PORTA = column_val;
							PORTC = column_sel;
		
		break;
		
		case sm1_third:		column_val = 0xFC;
							column_sel = 0xFF;
							PORTA = column_val;
							PORTC = column_sel;
		
		break;
		
		case sm1_green:		column_val = 0xFF;
							column_sel = 0xFF;
							PORTC = 0x00;
							PORTA = column_val;
							PORTD = column_sel;
		
		break;
		default:   	        break;
	}
	
	//PORTA = column_val; // PORTA displays column pattern
	//PORTC = column_sel; // PORTB selects column to display pattern

};


int main(void)
{
	DDRA = 0xFF; PORTA = 0x00;
	DDRB = 0x00; PORTB = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	state = sm1_wait;
	
    while(1)
    {
		SM1_Tick();
		_delay_ms(5000);
        //TODO:: Please write your application code 
    }
}