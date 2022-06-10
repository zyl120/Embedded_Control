/*	Names: Yilu Zhou
    Section: 02
    Side: 9B
    Date: 5/26/2019

    Size of counter: 13bit

    Trigger: SYSCLK/12

    File name: HW5_Yilu_Zhou.c
    Description: Basic template for Homework 5
        This program waits 3 seconds after a keyboard input and then prints the
        number of overflows that occurred. Then wait for 5 more seconds and
        then prints the number of overflows.
*/

#include <c8051_SDCC.h> // include files. This file is available online
#include <stdio.h>

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Timer_Init(void);     // Initialize Timer 0
void Interrupt_Init(void); //Initialize interrupts
void Timer0_ISR(void) __interrupt 1;

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

unsigned char input;
unsigned int counts = 0;

//***************
void main(void)
{
    Sys_Init(); // System Initialization
    putchar(' ');
    Interrupt_Init();
    Timer_Init(); // Initialize Timer 0

    while (1) /* the following loop waits until a key is pressed on the
                 keyboard and prints the number of overflows that occurr
                 in two time intervals */
    {
        printf("Hit any key on the keyboard to start \r\n");
        input = getchar();
        TMR0 = 0;   // clear the high and low byte of Timer0
        counts = 0; // Reset the counts.
        TR0 = 1;    // start Timer0.
        while (counts < 675)
            ;                                           // for a 3 second delay
        printf("3 seconds have passed. \r\n");          // print that the time has elapsed
        printf("number of overflows: %d \r\n", counts); // print number of overflows using count variable
        counts = 0;                                     // Reset the counts.
        while (counts < 1125)
            ;                                           // for a 5 second delay
        printf("5 seconds have passed. \r\n");          // print that the time has elapsed
        printf("number of overflows: %d \r\n", counts); // print number of overflows using count variable
    }
}

//***************

void Interrupt_Init(void)
{
    ET0 = 1;    // enable Timer0 Interrupt request using sbit variable
    IE |= 0x80; // enable global interrupts using bit masking
}
//***************
void Timer_Init(void)
{

    CKCON &= 0xF7; // Timer0 uses SYSCLK/12
    TMOD &= 0xF0;  // clear the 4 least significant bits
    TMOD |= 0x00;  // Timer0 mode setting. 13bits -> 0000 0000
    TR0 = 0;       // Stop Timer0
    TL0 = 0;       // Clear low byte of register T0
    TH0 = 0;       // Clear high byte of register T0
}

//***************
void Timer0_ISR(void) __interrupt 1
{
    counts++;
}
