# Lab 5
# Remote Interfacing

## Objectives

The focus of this lab was to interface an inferred remote control with the MSP430. Basic functionality called for 
using the remote control to toggle the MSP430 launchpad LEDs while more advanced functionality required interfacing 
with both the remote control and the Nokia display to implment either the pong game or etch-a-sketch from earilier labs.

## Prelab
### Remote Reverse Engineering

The primary funciton of the pre-design work was to understand how the IR remote control sent different data signals, and 
how the IR sensor that was connected to the MSP430 would interpret these signals. The experiment used to determine how
the IR remote functioned involved connecting an IR signal sensor to the MSP430 and to a logic analizer to measure both 
the time of each compomnent of the signal wave, as well as the timer A counts for the timer setup used throughout the 
project, 1/8 prescaler, count-up, with the overflow register set to `0xFFFF`. The testing C file can be found on the course websight under lab 5. The test was
set up with the IR sensor connected as it is in the generic schematic for this lab: VCC and GND connected to the 
corresponding launchpad pins, and the output signal wired to pin 2.6.

Data was taken from 11 seperate samples by reading the waveform from the logic analyzer as well as the timer counting 
array generated by the provided code. Every test involved sampling from the waveform generated by a power button press. 

![alt text]((https://raw.githubusercontent.com/IanGoodbody/ECE382_Lab5/master/labDataFiles/Power_Wave.jpg "Example wave output")

The waveform above gives the general anatomy of the signal generated by the specific remote used. The sensor output idles
at 1 until the falling edge of the inital start wave at the trigger. This start wave stands out with a fairly long
signal durration. Following the start wave, comes a series of fiarly constant 0 pulses and 1 pulses of either a 
consistantly long or consistantly short durration. What these long or short pulses were origionally designed to 
or what order they were intended bo be given (LSB first or MSB first) has no effect on the design of this lab as long
as the standard used to analize these bits thorughout the lab were consistant. In this light the short 1 pulses were 
defined as 0 signal, and the long 1 pulses were designated as 1 signals. Data was oritionally interpreted as the MSB 
recieved first, however later analysis showed that reading the data LSB first produced a more logical pattern in bit 
codes and so that standard is represented here and in the actual implementation of the lab. The last useful wave signal
is a standard 0 pulse similar to those of the data pulses, followed by a very long high signal. This was interpreted as
the end of transmition signal.

The samples taken from the power button press were taken durring 11 seperate trials for each the timing data and the 
timer count data. For the timing data, the times that corresponded to the data signals were taken from random waveforms
within the logic analizer signal. Conversely, the timer count data was only taken from single points on the waveform.
As can be seen in the data tables below, this sampling method for the timer counts produced slightly narrower error 
that were awkward when implemented in the code. The tables below give the means, standard deviations, and error margins
for the different signal time and timer counts analyzed. The actual data is given in the xcel sheets are included with
the code.

##### Table 1: Logic Analizer Timing Samples
Values are given in miliseconds

|Measure |Start Low|Start High|Data 1 Low|Data 1 High|Data 0 Low|Data 0 High|Stop Low|Stop High|
|:------:|:-------:|:--------:|:--------:|:---------:|:--------:|:---------:|:------:|:-------:|
|Mean    | 9.024   | 4.439    | 0.605    | 1.647     | 0.594    | 0.522     | 0.619  | 39.779  |
|Std Dev | 0.020   | 0.021    | 0.016    | 0.019     | 0.019    | 0.016     | 0.024  | 0.024   |
|Max     | 9.100   | 4.519    | 0.665    | 1.719     | 0.666    | 0.581     | 0.708  | 39.868  |
|Min     | 8.949   | 4.359    | 0.544    | 1.575     | 0.521    | 0.464     | 0.529  | 39.690  |


##### Table 2: Timer Counts
Values are recorded from TAR, units of counts

|Measure |Start Low|Start High|Data 1 Low|Data 1 High|Data 0 Low|Data 0 High|Stop Low|Stop High|
|:------:|:-------:|:--------:|:--------:|:---------:|:--------:|:---------:|:------:|:-------:|
|Mean    | 8918    | 4394     | 599      | 1618      | 581      | 529       | 598    | 39334   |
|Std Dev | 24.03   | 21.78    | 30.24    | 25.32     | 12.47    | 10.72     | 19.62  | 24.69   |
|Max     | 9007    | 4475     | 712      | 1712      | 628      | 569       | 671    | 39426   |
|Min     | 8828    | 4312     | 486      | 1523      | 534      | 488       | 524    | 39242   |

##### Table 3: Button codes
Values were interpreted from the logical analizer waveforms
|Button| Hex Code|
|:-:|:-:|
|0|

The statistical analysis of the data to generate the confidence interevals took advantage of the fact that most error
distributions are approximately normal (the data sets are too small to deffinitively show a normal distribution so the 
a priori assumption must suffice). The intereval was then designed to include 99.99% of the signals that came into
the sensor, which for a normal distribuiton equates to 3.719 standard deviations on either side of the mean. The 
population standard deviation for each wave signal was estimated from the sample standard deviation. The critical 
value was found using the R script

```R
qnorm(0.9999, 0, 1)
```

## Functionality
![alt text](https://raw.githubusercontent.com/IanGoodbody/ECE382_Lab5/master/labDataFiles/L5_Schematic.jpg "Generic Schematic")

### Core Functionality

Although a set of remote interfacing code was provided, that fact that this same interface would be used in multiple
labs warrented me to create my own interfacing code. The main motivation was that by creating, and thus being 
intimately familiar with the limitations and design of the system, I should be better able to debug and modify this 
basic interface in on future projects. For the core functionality the IR sensor was attached to the the MSP430 
Launchpad as shown in the above schematic and the nokia LCD was omitted.

Modularity called for IR sensor interfacing code to exist in its own header and C files for easy exportation to other 
projects at a later date. Based off of the interfacing code provided for the lab, changes in the IR sensor output 
would be detected using an interrupt vector to add the times between each signal change into an array. In an attempt
to make the ISR short, the timer values were stored into the array using a decrementing index rather than a rotation 
(the ammount of conditionals ended up giveing the ISR a lot of code, however I maintain that it is faster than running
31 or more seperate moves for each change in the signal). In order to control how the signal was stored and to add a 
layer of error checking and robustness, a global 8 bit char was used to store 4 different control flags. The flags
indica ted if there was an error in the reading, whether the low half-wave of the start signal had been read, whether 
the entirety of the start signal had been  read, and if the end signal had been read indicating that the packet read
was complete. The ISR itself goes though a series of flag and threshold checks to make sure that the values it 
recieves match the expected values for this partuclar waveform.

The error checking functionality of this interfacing system allows the program dynamically check for errant IR signals
and to then reset the the global variables and buffer array on the fly. The timer overflow functionality ened up being
unnecessary for the consistant functioning of the program and was not utalized on this implementation although the 
shell of this functionality is included for later development and use.

Additonal functions provided in the remoteInterface.c file include an initiation method, a reset, and a function to 
convert the array signals into practical 1's and 0's. The seperation of the signal timing and the conversion give the
user some flexability in how they may want to interpret or modify the raw signal times.

The header file for the interface implementation defines all the time thresholds and button codes. These time 
thresholds had to be modified from the origional values to accomendate other perriferials on later implementaitons; 
the origional values are included in comments next to the new.

### Basic Functionality: Switching LED's

The base interfacing code was simply implemented by calling the header file. The LED switching was implmented by 
polling the signal recieved and error flags. Whenever the error flag was triggered the program called the reset 
function and resumed polling. Once the signal recieved flag was raised, the program froze all interrupts, translated 
the signal, then took appropriate action in chaning the LED configuration.

The button control effects were

|Button|Red LED|Green LED|
|:-:|:-:|:-:|
|1|ON|OFF|
|2|OFF|ON|
|3|ON|ON|
|4|OFF|OFF|

The degbugging process here was fairly straight forward. 

Checking the ISR code involved placing breakpoints to trigger
all throughout the ISR, ensuring that they triggered in the expected order, and analizing the necessary varibales 
in the CCS debug screen. Simple bugs were fixed regarding timing threshold errors and and code flow and the LEDs were
able to be switched on and off using the buttons shown above.

These first test buttons were chosen because their bit codes lined up in a logical sequence which gave a good chance 
that the codes were correct and not preterbed by translation error. Once this code was up and running it proved useful
in checking the other codes for the different buttons on the remote.

### Advanced Functionality: Etch-a-Sketch

The advanced functionality called for using the code for the remote interfacing to control an animation on the Nokia
LCD screen. We were given the option to implement either pong or the etch-a-sketch drawing program from the previous
lab. Becuase the pong game ended up as a fairly specaized program with a resulting littany of implementation querks t
the simpler ecth-a-sketch was chosen in stead. The code was implemented almost idencitcally to the LED switching 
functionality with the exception of a number of different instantiation calls and setting the pins to read from
interface with the LCD. The button function mapping is given below.

|Button|Fnction|
|:-:|:-:|
|Up| Move cursor up|
|Down| Move cursor down|
|Left| Move cursor left|
|Right| Move cursor right (bet you weren't expecting that!)|
|Enter| Toggle the cursor color|

The debugging process for the etch-a-sketch proved far more difficult than for the LED switching. Every test of the 
remote input threw an error within the ISR and prompted a reset with no update in the screen. Similar debugging steps
were used as with the LED switching, and showed that the times read by the timer counts were about 80% longer than the
the values for the threshold. It was determined that this was caused by interfering timer requirements from the 
nokia.asm file. The assembly control of the LCD screen used slightly different clock settings, which were then 
syncronized with the remote clock; however, this did not produce the desired effect. Attempts to turn on and off 
interrupt enables around the different calls to the nokia and the IR sensor, as well as attempts to reinitalize each
compoenent when they were calld also failed to create a functioning program.

Despite my best attempts to reduce the timer to the expected values, the timer counts for each debugging iteration 
remained fairly consistant. Noting the 80% increase in the mean time value, the program was eventually able to 
function by implementing the highly rigourus adjustent to the upper timer count thresholds: "go up to the nearest 
round-ish and aestically pleasing number that is slightly less than double the origional threshold". Remarkably this 
adjustment was able to get around the timer slowing caused by the addition of the Nokia functionality and the program 
worked as expected.

## Notes for Future Implementation

Although the timer rollover iterrupt is disabled for this functionality it's implementaiton in future projects 
will add a good layer of robustness to the design

The error throw and reset operations have potential pitfalls in that an error throw and reset may be able to complete 
prior to the completion of the signal reading which would result in reading the same signal twice leaving the buffer
in an awkward half filled state until multiple button presses clear it. Additionally there is no check that the 
index could go out of bounds for for the buffer array. Adding an index check alongside a fairly tight timer rollover 
interrupt could remedy this potential problem

Lastly, the timer distortion that resulted from the addition of the nokia display introduces timing inconsistincies 
based on what perriferials are attached to the MSP430. Future uses of the MSP430 where the timer output must be 
consistent will require that the times be taken and tested with the additonal perriferials attached, initialized, and
running to produce good timing data.

#### Documentaiton

C1C McPeek helped orient me with the use of the logical anaizer and testing code to pull out the relevant timing data.
