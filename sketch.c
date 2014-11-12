/*
 * sketch.c
 *
 *  Created on: Nov 10, 2014
 *      Author: C16Ian.Goodbody
 */

#include <msp430g2553.h>
#include "remoteInterface.h"

#define BLACK 0xFF

extern void init();
extern void initNokia();
extern void clearDisplay();
extern void drawBlock(unsigned char row, unsigned char col, unsigned char coloredByte);

void main() {

	unsigned char	x, y, color;
	remoteCode packet;

	// === Initialize system ================================================
	IFG1=0; /* clear interrupt flag1 */
	WDTCTL=WDTPW+WDTHOLD; /* stop WD */

	init();
	initNokia();
	clearDisplay();
	x=4;		y=4;
	color = BLACK;
	drawBlock(y,x, color);

	initIRSensor();
	_enable_interrupt();
	TACTL &= ~TAIE;

	while(1)
	{
		if(packetReadFlags & ERROR_FLG)
		{
			resetSensor();
		}
		else if(packetReadFlags & PACKET_IN_FLG)
		{
			packet = translatePattern();
			switch(packet)
			{
			case UP_BUTTON:
				if(y > 0) y--;
				break;
			case DOWN_BUTTON:
				if(y < 7) y++;
				break;
			case LEFT_BUTTON:
				if(x > 0) x--;
				break;
			case RIGHT_BUTTON:
				if (x < 11) x++;
				break;
			case ENTER_BUTTON:
				color = ~color;
				break;
			}
			drawBlock(y,x, color);
			resetSensor();
		}
	}
}
