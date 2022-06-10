/*  Names: Yilu Zhou
    Section: 02
    Date: 5/22/2019
    File name: lab1-1
    Program description:
*/
#include <c8051_SDCC.h> // include files. This file is available online
#include <stdio.h>

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Port_Init(void);        // Initialize ports for input and output
int SlideSwitchStatus(void); // function that checks the Slide switch
int Pushbutton1Status(void); // function that checks PushButton 1
int Pushbutton2Status(void); // function that checks PushButton 2
void Set_outputs(void);      // function to set output bits
void StatusReport(void);     // Report the status of input devices.

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
    P3MDOUT &= 0xFC; // Set Port 3 input pins to open drain mode.
    P3MDOUT |= 0xD8; // Set Port 3 output pins to push-pull mode.
    P3 |= ~0xFC;     // Set Port 3 input pins to high impedance state.

    // Port 2
    P2MDOUT &= 0xFE; // Set Port 2 input pins to open drain mode
    P2 |= ~0xFE;     // Set Port 2 input pins to high impedance state.
}

//***************
// Set outputs:
//    The following code is incomplete, lighting an LED depending
//    on the state of a single pushbutton.

/*
 * Group Member 3
1. When the Slide switch is ‘off’ (input is a HIGH voltage), all outputs are off
2. When the Slide switch is ‘on’, the LED is on
3. When the Slide switch is ‘on’ and both Pushbuttons are pushed, the Buzzer is turned on
4. When the Slide switch is ‘on’ and only Pushbutton 1 is pushed, the BiLED is red
5. When the Slide switch is ‘on’ and only Pushbutton 2 is pushed, the BiLED is green
 */

void Set_outputs(void)
{
    StatusReport();

    if (!SlideSwitchStatus())
    {
        // Slide switch is OFF.
        printf("\r -All outputs are OFF. \n");
        LED0 = 1;
        BILED0 = 0;
        BILED1 = 0;
        BUZZER = 1;
    }

    if (SlideSwitchStatus())
    {
        // Slide switch is ON.
        printf("\r -LED is ON. \n");
        LED0 = 0;
    }
    else
    {
        LED0 = 1;
    }

    if (SlideSwitchStatus() && Pushbutton1Status() && Pushbutton2Status())
    {
        // Both pushbutton is pushed.
        printf("\r -Buzzer is ON. \n");
        BUZZER = 0;
    }
    else
    {
        BUZZER = 1;
    }

    if (SlideSwitchStatus() && !Pushbutton1Status() && Pushbutton2Status())
    {
        // Pushbutton 2 is pushed.
        printf("\r -BILED is GREEN. \n");
        BILED0 = 0;
        BILED1 = 1;
    }
    else if (SlideSwitchStatus() && Pushbutton1Status() && !Pushbutton2Status())
    {
        // Pushbutton 1 is pushed.
        printf("\r -BILED is RED. \n");
        BILED0 = 1;
        BILED1 = 0;
    }
    else
    {
        BILED0 = 0;
        BILED1 = 0;
    }
}

int Pushbutton1Status(void)
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

int Pushbutton2Status(void)
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


void StatusReport(void)
{
    if (SlideSwitchStatus())
    {
        printf("\r SlideSwitch is ON \n");
    }
    if (Pushbutton1Status())
    {
        printf("\r PUSHBUTTON 1 IS ON	\n");
    }
    else
    {
        printf("\r PushBUtton 1 is OFF");
    }
    if (Pushbutton2Status())
    {
        printf("\r PUSHBUTTON 2 IS ON	\n");
    }
    else
    {
        printf("\r PushBUtton 2 is OFF");
    }
}
