/*
 * remoteInterface.c
 *
 *  Created on: Nov 9, 2014
 *      Author: C16Ian.Goodbody
 */

#include <msp430g2553.h>
#include "remoteInterface.h"


//unsigned char packetReadFlags;

void initMSP430(void)
{
	IFG1 = 0;
	WDTCTL = WDTPW + WDTHOLD;

	BCSCTL1 = CALBC1_8MHZ;
	DCOCTL = CALDCO_8MHZ;

	P1DIR |= BIT0 | BIT6;
	P1OUT &= ~(BIT0 | BIT6);

	initIRSensor();

	_enable_interrupt();

	TACTL &= ~TAIE;
}

int main(void)
{
	initMSP430();

	remoteCode packet;

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
			case BUTTON_1:
				P1OUT |= BIT0;
				P1OUT &= ~BIT6;
				break;
			case BUTTON_2:
				P1OUT |= BIT6;
				P1OUT &= ~BIT0;
				break;
			case BUTTON_3:
				P1OUT |= BIT6;
				P1OUT |= BIT0;
				break;
			case BUTTON_4:
				P1OUT &= ~BIT6;
				P1OUT &= ~BIT0;
				break;
			}
			resetSensor();
		}
	}
}
