/*	Author: Dumitru Chiriac lab 11
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "bit.h"
#include "keypad.h"
#include "io.h"
#include <stdlib.h>
#endif


volatile unsigned char TimerFlag = 0;

unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;

void TimerOn(){
        TCCR1B = 0x0B;
        OCR1A = 125;
        TIMSK1 = 0x02;
        TCNT1 = 0;

        _avr_timer_cntcurr = _avr_timer_M;
        SREG |= 0x80;

}

void TimerISR() {
        TimerFlag = 1;
}

ISR(TIMER1_COMPA_vect){
        _avr_timer_cntcurr--;
        if(_avr_timer_cntcurr == 0) {
                TimerISR();
                _avr_timer_cntcurr = _avr_timer_M;
        }
}


void TimerSet(unsigned long M){
        _avr_timer_M = M;
        _avr_timer_cntcurr = _avr_timer_M;
}

typedef struct task {
        int state;
        unsigned long period;
        unsigned long elapsedTime;
        int (*TickFct)(int);
} task;

task tasks[2];
const unsigned char tasksNum = 2;
const unsigned long pG = 100;
const unsigned long p1 = 100;
const unsigned long p2 = 100;

//variables for CSM
unsigned char x;
unsigned char temp;
unsigned char player = '>';
unsigned char playerIndex = 2;
unsigned char obstacle = '#';
unsigned char gOver = 0;
unsigned char obstacleIndex = 1;
unsigned char init[] = "D to start";
unsigned char r;

//sms
enum Lights { L_Start, L_disp};
int Tick_L(int state);

enum Update {U_Start, U_Disp, U_Pause, U_Menu, U_GO};
int Tick_U(int state);

void RUN() { //changed name from timerISR to avoid conflictiong things
        unsigned char i;
        for (i = 0; i < tasksNum; i++){
                if (tasks[i].elapsedTime >= tasks[i].period){
                        while (!TimerFlag);
                        tasks[i].state = tasks[i].TickFct(tasks[i].state);
                        tasks[i].elapsedTime = 0;
                        TimerFlag = 0;
                }
                tasks[i].elapsedTime += pG;
        }
}

int main(void) {
	
	unsigned char i = 0;
        //task 1
        tasks[i].state = L_Start;
        tasks[i].period = p1;
        tasks[i].elapsedTime = tasks[i].period;
        tasks[i].TickFct = &Tick_L;
        i++;

	//task 2
        tasks[i].state = U_Start;
        tasks[i].period = p2;
        tasks[i].elapsedTime = tasks[i].period;
        tasks[i].TickFct = &Tick_U;
   	i++;

	
	TimerSet(pG);
	TimerOn();

	DDRA = 0xFF; PORTA = 0x00;
     	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;
	DDRD = 0xFF; PORTD = 0x00;
	LCD_init();
	LCD_DisplayString(1, init);
    	while (1) {
		RUN();
    	}
    	return 1;
}

int Tick_L(int state){

	switch(state){
		case L_Start: state = L_disp; break;
		case L_disp: state = L_disp; break;
		default: state = L_Start; break;
	}

	switch(state){
		case L_disp:
			x = GetKeypadKey();
  		        switch(x){
                        	case '\0': PORTB = 0x1F; temp = x; break;
                        	case '1': PORTB = 0x01; temp = x; break;
	                        case '2': PORTB = 0x02; temp = x;break;
       		                case '3': PORTB = 0x03; temp = x;break;
                        	case '4': PORTB = 0x04; temp = x;break;
                        	case '5': PORTB = 0x05; temp = x;break;
                        	case '6': PORTB = 0x06; temp = x;break;
                        	case '7': PORTB = 0x07; temp = x;break;
                        	case '8': PORTB = 0x08; temp = x;break;
                        	case '9': PORTB = 0x09; temp = x;break;
                        	case 'A': PORTB = 0x0A; temp = x;break;
                        	case 'B': PORTB = 0x0B; temp = x;break;
                        	case 'C': PORTB = 0x0C; temp = x;break;
                        	case 'D': PORTB = 0x0D; temp = x;break;
                        	case '*': PORTB = 0x0E; temp = x;break;
                        	case '0': PORTB = 0x00; temp = x;break;
                        	case '#': PORTB = 0x0F; temp = x;break;
                        	default: PORTB = 0x1B; break;
                	}
			break;
		default: break;

	}
	return state;
}

int Tick_U (int state){

	switch(state){
		case U_Start: state = U_Menu; break;
		case U_Menu:
			if (x == 'D'){
				LCD_ClearScreen();
				LCD_Cursor(2);
				LCD_WriteData(player);
				LCD_Cursor(1);
				playerIndex = 2;
				state = U_Disp;
			}
			else
				state = U_Menu;
			break;
		case U_Disp: state = (x == 'D') ? U_Pause : U_Disp;
			    if (gOver) state = U_GO; break;
		case U_Pause: state = (x == 'D') ? U_Disp : U_Pause; break;
		case U_GO: state = (x == 'D') ? U_Start : U_GO; break;
	}

	switch(state){
		case U_Start: gOver = 0; break;
		case U_Menu: LCD_DisplayString(1, init); break;
		case U_Disp: 
			//obstacle
		       	r = rand() % 2;
			if (r ==  0 && (obstacleIndex == 1 || obstacleIndex == 17)){
				LCD_Cursor(16);
				LCD_WriteData(obstacle);
				LCD_Cursor(1);
				LCD_WriteData(' ');
				LCD_Cursor(17);
				LCD_WriteData(' ');
				obstacleIndex = 16;
			}
			else if (r == 1 && (obstacleIndex == 1 || obstacleIndex == 17)){
				LCD_Cursor(32);
                                LCD_WriteData(obstacle);
                                LCD_Cursor(1);
                                LCD_WriteData(' ');
                                LCD_Cursor(17);
                                LCD_WriteData(' ');
				obstacleIndex = 32;
			}
			else{
				LCD_Cursor(obstacleIndex);
				LCD_WriteData(' ');
				LCD_Cursor(obstacleIndex - 1);
				LCD_WriteData(obstacle);
				obstacleIndex--;
			}			

			//player
			if (x == 'A'){
				LCD_Cursor(2);
				LCD_WriteData(player);
				LCD_Cursor(18);
				LCD_WriteData(' ');
				LCD_Cursor(1);
				playerIndex = 2;	
	
			}
			else if (x == 'B'){
				LCD_Cursor(2);
                                LCD_WriteData(' ');
                                LCD_Cursor(18);
                                LCD_WriteData(player);
				LCD_Cursor(17);
				playerIndex = 18;
			}

			//crash
			if (playerIndex == obstacleIndex){
				gOver = 1;
			}
			break;
		case U_GO:
			gOver = 1;
			LCD_ClearScreen();
			LCD_DisplayString(1, "Game Over!");
			break;
	}
	return state;
}
