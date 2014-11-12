/*
 * remoteInterface.c
 *
 *  Created on: Nov 9, 2014
 *      Author: Ian Goodbody
 *    Function: The functions here are used to implement the remote interface functionality.
 *				It provides functions to initiate the sensor pins and timer, translate timing
 *				signals into 1's and 0's, reset the sensor in the event of an error, and 
 *				provides the interrupt service routine to read in each signal
 */

#include <msp430g2553.h>
#include "remoteInterface.h"

/*
 * translatePattern(): translates the values in the packet times array to 1's and 0's based
 * on the threshold values defined in the the file header.
 *
 * Returns: the a 32 bit number contining the code from the remote input
 *			0 if an error occured in translation
 */
remoteCode translatePattern()
{
	remoteCode packet = 0x00000000; // Initialize the return value as zero
	remoteCode bitMask = 0x00000001; // The bit mask wich allows the loop to rotate through all 32 bits

	int i;
	for(i = SIGNAL_LENGTH-1; i >= 0; i--)
	{ // Rotate from LSB to MSB and either set or clear the bits according to the timing signal;
		if(signalTimes[i] >= DATA_1_HIGH_MIN && signalTimes[i] <= DATA_1_HIGH_MAX)
		{
			packet |= bitMask;
		}
		else if (signalTimes[i] >= DATA_0_HIGH_MIN && signalTimes[i] <= DATA_0_HIGH_MAX)
		{
			packet &= ~bitMask; // The return signal is only necessary for redundancy's sake
		}
		else
		{
			// Packet timing did not match either a high or low signal, throw the error null.
			return 0x00000000;
		}
		bitMask = bitMask << 1; // Rotate the masking bit 1 towards the MSB
	}
	return packet;
}

/*
 * initIRSensor(): set up theh values necessary for the remote interface operation
 */
void initIRSensor()
{
	int i;
	for(i = SIGNAL_LENGTH -1; i >= 0; i--)
	{
		// Initialize the timing array to all zeros
		signalTimes[i] = 0x00000000;
	}

	// Configure pin 2.6 I/O
	P2SEL  &= ~BIT6;
	P2SEL2 &= ~BIT6;
	P2DIR &= ~BIT6;

	// Configure timer
	TAR = 0x0000;
	TA0CCR0 = 0xFFFF;
	TACTL &= ~TAIFG;
	TACTL = ID_3 | TASSEL_2 | MC_1;

	// Configure pin 2.6 packet reading and interrupts
	signalTimesWriteIndex = SIGNAL_LENGTH - 1;
	packetReadFlags = 0x00;
	P2IES |= BIT6;
	P2IFG &= ~BIT6;
	P2IE |= BIT6;
}

/*
 * resetSensor(): resets the remote interface flags, pins, and array in the event of an error 
 */ 
void resetSensor()
{
	signalTimesWriteIndex = SIGNAL_LENGTH - 1;
	packetReadFlags = 0x00;
	P2IE |= BIT6;
	P2IES |= BIT6;
	P2IFG &= ~BIT6;
}

/*
 * __interrupt void pinChange(void): a gargantuine method which records the high timer coutns,
 * and checks that the different control signals, start and end, and the low half waves are within
 * the timing threshold
 */
#pragma vector = PORT2_VECTOR			// This is from the MSP430G2553.h file
__interrupt void pinChange (void)
{
	//TACTL |= TAIE;
	unsigned int counts = TAR; // Store the timer value and reset the coutner
	TAR = 0x0000;	// Worry about storing the value later

	if(P2IN & BIT6) // Pin 2.6 is high, rising edge trigger
	{
		if(packetReadFlags & READ_PKT_FLG) // Reading Data
		{
			if(counts > DATA_LOW_MAX || counts < DATA_LOW_MIN) // If the low signal has an unexpected durration
			{
				packetReadFlags |= ERROR_FLG;
				packetReadFlags &= ~(READ_PKT_FLG | PACKET_IN_FLG | START_LOW_TRIG_FLG);
			}
			else
			{
				P2IES ^= BIT6; // Toggle the edge select
			}
		}
		else if (counts >= START_LOW_MIN && counts <= START_LOW_MAX)
		{
			packetReadFlags |= START_LOW_TRIG_FLG;
			P2IES ^= BIT6;
		}
		else
		{
			packetReadFlags |= ERROR_FLG;
			packetReadFlags &= ~(READ_PKT_FLG | PACKET_IN_FLG | START_LOW_TRIG_FLG);
		}
	}
	else // Pin 2.6 is low, falling edge trigger
	{
		if(packetReadFlags & READ_PKT_FLG)
		{
			if(counts >= STOP_HIGH_MIN && counts <= STOP_HIGH_MAX) // The end high signal has been read in set flags appropriately
			{
				packetReadFlags |= PACKET_IN_FLG;
				packetReadFlags &= ~READ_PKT_FLG;
				P2IE &= ~BIT6;
			}
			else // Store the pulse width to process later
			{
				signalTimes[signalTimesWriteIndex--] = counts;
				P2IES ^= BIT6; // Toggle the edge select
			}
		}
		else if (packetReadFlags & START_LOW_TRIG_FLG)
		{
			if(counts >= START_HIGH_MIN && counts <= START_HIGH_MAX)
			{
				packetReadFlags |= READ_PKT_FLG;
				packetReadFlags &= ~START_LOW_TRIG_FLG;
				P2IES ^= BIT6;
			}
			else
			{
				packetReadFlags |= ERROR_FLG;
				packetReadFlags &= ~(READ_PKT_FLG | PACKET_IN_FLG | START_LOW_TRIG_FLG);
			}
		}
		else
		{
			P2IES ^= BIT6;
		//	packetReadFlags |= ERROR_FLG;
		//	packetReadFlags &= ~(READ_PKT_FLG | PACKET_IN_FLG | START_LOW_TRIG_FLG);
		}
	}
	//TACTL &= ~TAIE;
	P2IFG &= ~BIT6; // Toggle interrupt flag
}

#pragma vector = TIMER0_A1_VECTOR
__interrupt void timerOverflow (void)
{
	//resetSensor();
	TACTL &= ~TAIFG;
}
