/*  Name: Z Jin
    Section: 02
    Date: 2019/5/23
    File name: Lab1-1.c
    Program description: Lab 1-1
*/
/*
  This program is incomplete. Part of the code is provided as an example. You 
  need to modify the code, adding code to satisfy the stated requirements. Blank 
  lines have also been provided at some locations, indicating an incomplete line.
*/
#include <c8051_SDCC.h> // include files. This file is available online on LMS
#include <stdio.h>

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Port_Init(void);   // Initialize ports for input and output
void Set_outputs(void); // function to set output bits
//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
__sbit __at 0xB6 LED0;   // LED0, associated with Port 3 Pin 6
__sbit __at 0xB3 BL0;    // BILED0, associated with P3,3
__sbit __at 0xB4 BL1;    // BILED1, associated with P3,4
__sbit __at 0xB7 BUZZER; // Buzzer, associated with p3,7
__sbit __at 0xA0 SS;     // Slide switch, associated with Port 2 Pin 0
__sbit __at 0xB0 PB1;    // Push button 1, associated with Port 3, Pin 0
__sbit __at 0xB1 PB2;    // Push button 2, associated with P3.1

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
    P3MDOUT = P3MDOUT | 0xD8; // set Port 3 output pins to push-pull mode (fill in the blank)
    P3MDOUT = P3MDOUT & 0xFC; // set Port 3 input pins to open drain mode (fill in the blank)
    P3 = ~0xFC;               // set Port 3 input pins to high impedance state (fill in the blank)

    // Port 2
    P2MDOUT &= 0xFE;
    P2 |= ~0xFE;
    //
    //
}

//***************
// Set outputs:
//    The following code is incomplete, lighting an LED depending
//    on the state of the slide switch.

/*
member 2 de
void Set_outputs(void)
{
    if (!SS)        // if Slide Switch activated (On position)
    {
		if(PB1&&PB2) // if no Push button is pushed
		{BL0 = 1;	// BL is not green
		 BL1 = 1;	// BL is not red
        printf("\r Slide switch is on and no Push button is pushed   \n");
		if(!PB1&&PB2)	// if Push button 1 is pushed
		{
			BUZZER = 0; // BUZZER is on
			printf("\r Pushbutton 1 is pushed   \n");
		}
		if(!PB2&&PB1)	// if Push button 2 is pushed
		{
			LED0 = 0;   // turn on LED0
			printf("\r Pushbutton 2 is pushed   \n");
		}
		if(!PB2&&!PB1)	// if Push button 1 and 2 is pushed
		{
		BL1 = 0;	// BL is red
		printf("\r Pushbutton 1 and 2 is pushed   \n");
		}
    }
    else            // if Slide Switch is not activated (Off position)
    {
        BL0 = 0;   // BL is green
        printf("\r Slide switch is off   \n");	
    }
}
*/

void Set_outputs(void)
{
    if (!SS) // if Slide Switch activated (On position)
    {
        LED0 = 0; // turn on LED0
        printf("\r Slide switch is on    \n");
        if (!PB1 && PB2) // if Push button 1 is pushed
        {
            BL1 = 1; // BL is not red
            BL0 = 0; // BL is green
            printf("\r Pushbutton 1 is pushed   \n");
        }
        if (!PB2 && PB1) // if Push button 2 is pushed
        {
            BL0 = 1; // BL is red
            BL1 = 0; // BL is not green
            printf("\r Pushbutton 2 is pushed   \n");
        }
        if (!PB2 && !PB1) // if Push button 1 and 2 is pushed
        {
            BUZZER = 0; // BUZZER is on
            printf("\r Pushbutton 1 and 2 is pushed   \n");
        }
    }
    else // if Slide Switch is not activated (Off position)
    {
        LED0 = 1; // turn off LED0
        printf("\r Slide switch is off   \n");
    }
}