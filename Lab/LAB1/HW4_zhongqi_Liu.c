/*	Name: Zhongqi Liu
	Section: 02
	Side: 9B
	Date: 05/23/2019

	Port bits/Digital I/O: 
		3.4 3.5

	File name: hw4_zhongqi_Liu.c
	Description: Basic template for Homework 4
		This program uses two digital inputs
*/

//-----------------------------------------------------------------------------
// Header Files
//-----------------------------------------------------------------------------

#include <c8051_SDCC.h>// include files. This file is available online
#include <stdio.h>


//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Port_Init(void);	// digial port initialization 
void Check_Inputs(void);	// output after first character press

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
//
// edit sbit commands as appropriate to your assignment

__sbit __at 0xB4 PB1; //Push button 1, associated with port 3 , pin 4
__sbit __at 0xB5 PB2; //Push button 1, associated with port 3 , pin 5


//***************
void main(void)  		// 
{
	Sys_Init();      	// System Initialization
	putchar(' ');    
	Port_Init();		// port initialization function
	while(1)
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
	//edit this function as appropriate to your assignment
	P3MDOUT &= 0xE7;     //Turn on P3.4 and P3.5 for input
	P3 | = ~0xE7;   // Turn on high impedance of P3.5, 3.5
}

//***************
// edit the arguments to the if/else if statements, as needed
// use sbit labels to output digital signals after first character press
//
void Check_Inputs(void)
{
	if (!PB1 && !PB2)			// both buttons pushed
	{
		printf("Both buttons pushed \r\n");
	}
	else if (!PB1 || !PB2)		// one button pushed
	{
		printf("Only one button pushed \r\n");
	}
	else if (PB1 && PB2)		// no buttons pushed
	{
		printf("No buttons pushed \r\n");
	}

}
