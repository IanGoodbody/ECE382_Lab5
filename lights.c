/*
 * remoteInterface.c
 *
 *  Created on: Nov 9, 2014
 *      Author: Ian Goodbody
 *	  Function:	The main function for the basic funcitonality which requires
 *				the a remote turn the lights on and off on the MSP430 launchpad chip. 
 */

#include <msp430g2553.h>
#include "remoteInterface.h"


//unsigned char packetReadFlags;

void initMSP430(void)
{
	IFG1 = 0;	// Disable interrupt flag and turn off watchdog timer
	WDTCTL = WDTPW + WDTHOLD;

	BCSCTL1 = CALBC1_8MHZ;
	DCOCTL = CALDCO_8MHZ;

	P1DIR |= BIT0 | BIT6;	// Set up LED pins
	P1OUT &= ~(BIT0 | BIT6);

	initIRSensor();	// Function in the remoteInterface.c file

	_enable_interrupt();

	TACTL &= ~TAIE; // Timer reset interuupt disabled, it serves no function for the program
}

int main(void)
{
	initMSP430();

	remoteCode packet; 

	while(1)
	{
		if(packetReadFlags & ERROR_FLG)
		{
			resetSensor();	// If the signal is bad, reset the sensor flags and clear the buffer
		}
		else if(packetReadFlags & PACKET_IN_FLG) 
		{
			packet = translatePattern(); 
			switch(packet)
			{
			case BUTTON_1: // Red LED on, Green LED off
				P1OUT |= BIT0; 
				P1OUT &= ~BIT6;
				break;
			case BUTTON_2: // Red LED off, Green LED on
				P1OUT |= BIT6;
				P1OUT &= ~BIT0;
				break;
			case BUTTON_3: // Red LED on, Green LED off
				P1OUT |= BIT6;
				P1OUT |= BIT0;
				break;
			case BUTTON_4: // Red LED off, Green LED off
				P1OUT &= ~BIT6;
				P1OUT &= ~BIT0;
				break;
			}
			resetSensor();
		}
	}
}
