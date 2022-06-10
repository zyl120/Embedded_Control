/*
Name: Yilu Zhou
Section: 02
Side: 9B
Date: 5/29/2019

Assignment description:
    Count the number of times a button is pressed and and the subset number of
    presses where the button is held down for at least 2 seconds during a 20
    second interval.

File name: HW6_Yilu_Zhou.c
Description:
    The code designed for an input on P2.4. Timer0 should be configured for 13bit counting triggered by SYSCLK.
*/

#include <c8051_SDCC.h> // include files. This file is available online
#include <stdio.h>

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Port_Init(void);      //Port Initialization
void Timer_Init(void);     //Initialize Timer 0
void Interrupt_Init(void); //Initialize interrupts
void Timer0_ISR(void) __interrupt 1;

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------

unsigned int counts;         // overflow tracking variable
unsigned char input;         // input variable
unsigned int debounce_count; // The variable to track the debounce time.
unsigned int pressed_count;  // The variable to count the pressed time.
unsigned int button_count;   // The variable to count the number the button was pushed.
unsigned int button_2s;      // The variable to count the number the button was pushed for at least 2s.

__sbit __at 0xA4 PB; // Pushbutton on Port 2 pin 4

//***************
void main(void)
{
    Sys_Init();   // System Initialization
    putchar(' '); // the quote fonts may not copy correctly into SiLabs IDE
    Port_Init();
    Interrupt_Init();
    Timer_Init(); // Initialize Timer 0
    while (1)     /* the following loop contains the button pressing/tracking code */
    {
        printf("Enter a keyboard character to begin \r\n");
        input = getchar();
        counts = 0;
        debounce_count = 0;
        pressed_count = 0;
        button_count = 0;
        button_2s = 0; // Initialize the variables.
        printf("Push the button as many times as you like in 20 seconds \r\n");
        TR0 = 1;               // Start timer 0.
        while (counts < 54000) //
        {
            if (!PB)
            {
                while (debounce_count < 54)
                    ;               // Wait for debounce time (20ms).
                debounce_count = 0; // Reset the debounce count.
                while (!PB)
                    ;           // Wait for the release.
                button_count++; // Increment the button count.
                pressed_count = debounce_count;
                debounce_count = 0; // Reset the bounce count.
                while (debounce_count < 54)
                    ; // Wait for debounce time (20ms).
                if (pressed_count > 5400)
                {
                    button_2s++; // Count the number when the button is pressed for more than 2s.
                }
            }
        }
        printf("\n\rThere are %d button press in total. \n\r", button_count);
        printf("\n\rThere are %d button pressed no less than 2s. \n\r", button_2s);
    }
}

//***************

void Port_Init(void)
{
    P2MDOUT &= 0xEF; // Set the P2.4 for output
    P2 |= ~0xEF;     // Turn on high impedance for P2.4
}

//***************

void Interrupt_Init(void)
{
    IE |= 0x02; // enable Timer0 Interrupt request
    EA = 1;     // enable global interrupts
}

//***************
void Timer_Init(void)
{
    // Timer0 should be configured for 13bit counting triggered by SYSCLK.
    CKCON |= 0x08; // Timer0 uses SYSCLK as source. 0000 1000
    TMOD &= 0xF0;  // clear the 4 least significant bits. 1111 0000
    TMOD |= 0x00;  // TMOD use mode 0. 0000 0000
    TR0 = 0;       // Stop Timer0
    TL0 = 0;       // Clear low byte of register T0
    TH0 = 0;       // Clear high byte of register T0
}

//***************
void Timer0_ISR(void) __interrupt 1
{
    counts++;         //increment the global variable 'counts'.
    debounce_count++; // Increment the pushbutton time count.
}
