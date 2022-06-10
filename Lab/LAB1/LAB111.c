/* 
Name: Z Jin
Section: 02
Date: 5/22/2019
File name: LAB113.c
Program description: This is the code for Lab 1-1, part 1.
*/

#include <c8051_SDCC.h> // include files. This file is available online on LMS
#include <stdio.h>

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Port_Init(void);          // Initialize ports for input and output
void Set_outputs(void);        // function to set output bits
int Slide_Switch_Status(void); // Check the status of the slide switch.
int Pushbutton1_Status(void);  // Check the status of pushbutton 1.
int Pushbutton2_Status(void);  // Check the status of pushbutton 2.
void Status_Report(void);

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

// Outputs: 1
__sbit __at 0xB6 LED0;   // LED0, associated with Port 3 Pin 6 (Pin 31)
__sbit __at 0xB3 BILED0; // BILED0, associated with Port 3 Pin 3 (Pin 36)
__sbit __at 0xB4 BILED1; // BILED1, associated with Port 3 Pin 4 (Pin 33)
__sbit __at 0xB7 BUZZER; // Buzzer, associated with Port 3 Pin 7 (Pin 32)

// Inputs: 0
__sbit __at 0xA0 SS;  // Slide switch, associated with Port 2 Pin 0 (Pin 29)
__sbit __at 0xB0 PB1; // Push button 1, associated with Port 3, Pin 0 (Pin 38)
__sbit __at 0xB1 PB2; // Push button 2, associated with Port 3, Pin 1 (Pin 37)

//***************
// Main program

void main(void)
{
    Sys_Init();   // System Initialization
    putchar(' '); // the quote fonts may not copy correctly into SiLabs IDE
    Port_Init();  // Initialize ports 2 and 3

    while (1) // infinite loop
    {
        // main program manages the function calls
        Set_outputs();
    }
}

//***************
/* Port_Init - Initializes Ports 2 and 3 in the desired modes for input and output */

void Port_Init(void)
{
    // Port 3
    P3MDOUT |= 0xD8; // Set Port 3 output pins to push-pull mode.
    P3MDOUT &= 0xFC; // Set Port 3 input pins to open drain mode.
    P3 |= ~0xFC;     // Set Port 3 input pins to high impedance state.

    // Port 2
    P2MDOUT &= 0xFE; // Set Port 2 input pins to open drain mode
    P2 |= ~0xFE;    // Set Port 2 input pins to high impedance state.
}

//***************
// Set outputs:
//    The following code is incomplete, lighting an LED depending
//    on the state of the slide switch.

/*
Group Member 1
1. When the Slide switch is ‘off’ (input is a HIGH voltage), the LED is on, all other outputs are ‘off’
2. When the Slide switch is ‘on’, the LED is off.
3. When the Slide switch is ‘on’ and only Pushbutton 1 are pushed, the BiLED is green
4. When the Slide switch is ‘on’ and only Pushbutton 2 is pushed, the BiLED is red
5. When the Slide switch is ‘on’ and both Pushbuttons are pushed, the Buzzer is on
*/

void Set_outputs(void)
{
    Status_Report();

    if (!Slide_Switch_Status())
    {
		printf("\r LED is on \n");
        LED0 = 0;   // Turn on LED.
        BILED0 = 0; // Turn off the BILED.
        BILED1 = 0;
        BUZZER = 1; // Turn off the buzzer.
    }
    else
    {
        LED0 = 1;   // Turn off LED.
        BILED0 = 0; // Turn off the BILED.
        BILED1 = 0;
        BUZZER = 1; // Turn off the buzzer.
        if (Pushbutton1_Status() && !Pushbutton2_Status())
        {
			printf("\r BILED is GREEN \n");
            BILED1 = 1; // Green.
        }
        if (!Pushbutton1_Status() && Pushbutton2_Status())
        {
			printf("\r BILED is RED \n");
            BILED0 = 1; // RED.
        }
        if (Pushbutton1_Status() && Pushbutton2_Status())
        {
			printf("\r Buzzer is ON \n");
            BUZZER = 0; // Turn on buzzer.
        }
    }
}

int Slide_Switch_Status(void)
{
    /*
    This function is used to check the status of slide switch.
    Return 1 if the slide switch is ON.
    Return 0 is the slide switch is OFF.
    */
    if (!SS) // If the slide switch is ON
    {
        return 1;
    }
    else // If the slide switch is OFF
    {
        return 0;
    }
}

int Pushbutton1_Status(void)
{
    /*
    This function is used to check the status of the pushbutton 1.
    Return 1 if the button is pushed.
    Return 0 if the button is not pushed.
    */
    if (!PB1) // If the pushbutton 1 is pushed
    {
        return 1;
    }
    else // If the pushbutton 1 is not pushed.
    {
        return 0;
    }
}

int Pushbutton2_Status(void)
{
    /*
    This function is used to check the status of the pushbutton 2.
    Return 1 if the button is pushed.
    Return 0 if the button is not pushed.
    */
    if (!PB2) // If the pushbutton 2 is pushed.
    {
        return 1;
    }
    else // If the pushbutton 2 is not pushed.
    {
        return 0;
    }
}

void Status_Report(void)
{
    if (Slide_Switch_Status())
    {
        printf("\r *Slide Switch is ON. \n");
    }
    else
    {
        printf("\r *Slide Switch is OFF. \n");
    }
    if (Pushbutton1_Status())
    {
        printf("\r *Pushbutton 1 is ACTIVATED. \n");
    }
    else
    {
        printf("\r *Pushbutton 1 is DEACTIVATED. \n");
    }
    if (Pushbutton2_Status())
    {
        printf("\r *Pushbutton 2 is ACTIVATED. \n");
    }
    else
    {
        printf("\r *Pushbutton 2 is DEACTIVATED. \n");
    }
}