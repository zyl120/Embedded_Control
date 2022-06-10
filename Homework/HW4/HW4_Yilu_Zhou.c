/*	Name: Yilu Zhou
    Section: 02
    Side: 9B
    Date: 5/23/2019

    Port bits/Digital I/O: 
        3.0, 3.1

    File name: HW4_Yilu_Zhou.c
    Description: Basic template for Homework 4
        This program uses two digital inputs
*/

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------

#include <c8051_SDCC.h> // include files. This file is available online
#include <stdio.h>

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Port_Init(void);    // digial port initialization
void Check_Inputs(void); // output after first character press

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
//
// edit sbit commands as appropriate to your assignment

__sbit __at 0xB0 PB1; // Push button 1, associated with Port 3, Pin 0 (Pin 38)
__sbit __at 0xB1 PB2; // Push button 2, associated with Port 3, Pin 1 (Pin 37)

//***************
void main(void) //
{
    Sys_Init(); // System Initialization
    putchar(' ');
    Port_Init(); // port initialization function
    while (1)
    {
        printf("Press a key to check which buttons are pushed \r\n");
        getchar();
        Check_Inputs();
    }
}

//***************
// configure P2MDOUT or P3MDOUT
//
void Port_Init(void)
{
    P3MDOUT &= 0xFC; // Turn on P3.0, P3.1 for input.
    P3 |= ~0xFC; // Turn on high impedance of P3.0, 3.1.
}

//***************
// edit the arguments to the if/else if statements, as needed
// use sbit labels to output digital signals after first character press
//
void Check_Inputs(void)
{
    if (!PB1 && !PB2) // both buttons pushed
    {
        printf("Both buttons pushed \r\n");
    }
    else if (!PB1 || !PB2) // one button pushed
    {
        printf("Only one button pushed \r\n");
    }
    else if (PB1 && PB2) // no buttons pushed
    {
        printf("No buttons pushed \r\n");
    }
}
