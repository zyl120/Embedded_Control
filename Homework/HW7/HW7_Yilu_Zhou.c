/*	Name: Yilu Zhou
	Section: 02
	Side: 9B
	Date: 5/30/2019

	Gain: 4
	Port pin: Port 1.7

	File name: HW7_Yilu_Zhou.c
	Description: This program reads the analog input as port P1.7 and output
    the A/D conversion result and the measured voltage in mV by using the A/D
    value.
*/

#include <c8051_SDCC.h> // include files. This file is available online
#include <stdio.h>

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void ADC_Init(void);
void Port_Init(void);
unsigned char read_AD_input(unsigned char pin_number);

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

unsigned char AD_value;
unsigned char input;
unsigned int voltage;

//***************
void main(void)
{
    Sys_Init();   // System Initialization
    putchar(' '); // the quote fonts may not copy correctly into SiLabs IDE
    Port_Init();
    ADC_Init();
    printf("Start \r\n");
    while (1)
    {
        printf("enter key to read A/D input \r\n");
        input = getchar();
        AD_value = read_AD_input(7);                         // Get the A/D value;
        voltage = (0.6 / 255) * AD_value * 1000;             // Covert the input voltage from A/D.
        printf("A/D conversion result is %d\r\n", AD_value); // print the A/D conversion result.
        printf("Input voltage is %d mV\r\n", voltage);       // print the input voltage.
    }
}

//
// add the initialization code needed for the ADC1
//
void ADC_Init(void)
{
    REF0CN = 0x03;  // Set Vref to use internal ref. Voltage 2.4V.
    ADC1CN = 0x80;  // Enable A/D converter.
    ADC1CF |= 0x03; // Set A/D converter gain to 4.
}
//
// function that completes an A/D conversion
//
unsigned char read_AD_input(unsigned char pin_number)
{
    AMX1SL = pin_number; //Set P1.n as the analog input for ADC1.
    ADC1CN &= ~0x20;     //Clear the convertion completed flag
    ADC1CN |= 0x10;      //Initiate A/D convertion.
    while ((ADC1CN & 0x20) == 0x00)
        ;        // Wait for the conversion to be completed.
    return ADC1; // Return the digital value in ADC1 register.
}

//
// add Port initialization code
//
void Port_Init(void)
{
    P1MDIN &= 0x7F;  // Set P1.7 for analog input
    P1MDOUT &= 0x7F; // Set P1.7 to open drain
    P1 |= ~0x7F;     // send logic 1 to input pin P1.7
}
