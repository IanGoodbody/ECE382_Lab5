/*
 * remoteInterface.h
 *
 *  Created on: Nov 9, 2014
 *      Author: C16Ian.Goodbody
 */

#ifndef REMOTEINTERFACE_H_
#define REMOTEINTERFACE_H_

// Define remote code constants
	// Remote codes are read LSB sent first adn MSB sent last
#define BUTTON_0 	0xF50AFF00
#define BUTTON_1 	0xFF00FF00
#define BUTTON_2 	0xFE01FF00
#define BUTTON_3 	0xFD02FF00
#define BUTTON_4 	0xFC03FF00
#define BUTTON_5 	0xFB04FF00
#define BUTTON_6 	0xFA05FF00
#define BUTTON_7 	0xF906FF00
#define BUTTON_8 	0xF807FF00
#define BUTTON_9 	0xF708FF00

#define UP_BUTTON 		0xEF10FF00
#define DOWN_BUTTON 	0xEE11FF00
#define LEFT_BUTTON 	0xED12FF00
#define RIGHT_BUTTON 	0xEC13FF00
#define POWER_BUTTON	0xF30EFF00
#define ENTER_BUTTON	0xF10EFF00

typedef unsigned long remoteCode;
typedef unsigned short pulseTime;

// Define remote code timing constatns. Values are given in terms of timer counts using (Give clock conditions)

#define START_LOW_MAX 12500 //9008
#define START_LOW_MIN 8828

#define START_HIGH_MAX 7000 //4475
#define START_HIGH_MIN 4312

#define DATA_1_LOW_MAX 1000 //712
#define DATA_1_LOW_MIN 486

#define DATA_1_HIGH_MAX 2200 //1713
#define DATA_1_HIGH_MIN 1523

#define DATA_LOW_MAX 1000 //712	// The most extremem of the nearly identical low half-wave tiems
#define DATA_LOW_MIN 486	// These are valid for all data signals as well as the stop signal half-wave

#define DATA_0_HIGH_MAX 800//570
#define DATA_0_HIGH_MIN 450

#define STOP_HIGH_MAX 50000//39426
#define STOP_HIGH_MIN 39242

// Define the bits for the data reading flags
#define ERROR_FLG 			0x08
#define START_LOW_TRIG_FLG 	0x04
#define READ_PKT_FLG 		0x02
#define PACKET_IN_FLG 		0x01

// Misclaneous deffinitions

#define TRUE 	0x01
#define FALSE 	0x00

	/*
	 * The signal refered to here is the timing data for all the high and low pulses in a single array.
	 * The length is then the 32 data signal waves plus the 2 overhead waves, multiplied by 2 half wave
	 * signals per wave to give a total length of 68.
	 */
#define SIGNAL_LENGTH 32

/*
 * Implementation notes:
 *
 * The interface will passively collect data signals thorugh interrupts. It will require a number of global variables
 * including a readingSignal flag, a signalRecieved flag, and an array timer coutns that will contain
 * the signal. The interface will also contain a function to translate that array into an unsigned 32 bit number or
 * remoteCode for use by the program.
 */

pulseTime signalTimes[SIGNAL_LENGTH];
int signalTimesWriteIndex;
unsigned char packetReadFlags;

remoteCode translatePattern();
void initIRSensor();
void resetSensor();

__interrupt void pinChange (void);
__interrupt void timerOverflow (void);

#endif /* REMOTEINTERFACE_H_ */
