/*
Names: Yilu Zhou
Section: 02
Date: 5/25/2019
File name: LAB1-2.c
Description: This program demonstrates the use of T0 interrupts. The code
will count the number of T0 timer overflows that occur while a slide switch
is in the ON position.
*/

#include <c8051_SDCC.h> // include files. This file is available online
#include <stdio.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Port_Init(void);                // Initialize ports for input and output
void Timer_Init(void);               // Initialize Timer 0
void Interrupt_Init(void);           // Initialize interrupts
void Timer0_ISR(void) __interrupt 1; // Timer0 interrupt
void random(void);                   // Generate a random number
unsigned char IsPB1Pushed(void);     // The status of PB1
unsigned char IsPB2Pushed(void);     // The status of PB2
unsigned char IsSSOn(void);          // The status of slide switch
void LEDController(void);            // Control the on/off of the LED0 and 1 depends on the random number
void ResultController(void);         // Recored the inputs of the pushbuttons and light the BILED
void TurnBILEDGreen(void);           // Let the BILED be green
void TurnBILEDRed(void);             // Let the BILED be red
void TurnBILEDOff(void);             // Turn off the BILED
void TurnLED0On(void);               // Turn on LED0
void TurnLED0Off(void);              // Turn off LED0
void TurnLED1On(void);               // Turn on LED1
void TurnLED1Off(void);              // Turn off LED1

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
__sbit __at 0xB3 BILED1; // BILED1, associated with Port 3 Pin 3 (Pin 36)
__sbit __at 0xB4 BILED2; // BILED2, associated with Port 3 Pin 4 (Pin 33)
__sbit __at 0xB6 LED0;   // LED0, associated with Port 3 Pin 6 (Pin 31)
__sbit __at 0xB5 LED1;   // LED1, assocaited with Port 3 Pin 5 (Pin 34)

__sbit __at 0xA0 SS;  // Slide Switch associated with Port 2 Pin 0 (Pin 29)
__sbit __at 0xB0 PB1; // Push button 1, associated with Port 3, Pin 0 (Pin 38)
__sbit __at 0xB1 PB2; // Push button 2, associated with Port 3, Pin 1 (Pin 37)

__sbit __at 0x8C TR0; // Timer 0 on/off

unsigned int Counts = 0;          // Record the number of overflows.
unsigned char Wins = 0;           // Record the number of the wins.
unsigned char CurrentRandom = 0;  // Record the results from random.
unsigned char PreviousRandom = 0; // Record the previous random number.
unsigned char Tries = 0;          // Record the number of tries.

//***************
void main(void)
{
    Sys_Init();  // System Initialization
    Port_Init(); // Initialize ports 2 and 3
    Interrupt_Init();
    Timer_Init(); // Initialize Timer 0

    putchar(' '); // the quote fonts may not copy correctly into SiLabs IDE
    printf("Start\r\n");
    while (1)
    {
        TurnBILEDRed();
        TurnLED0Off();
        TurnLED1Off();
        TR0 = 0; // Pause until the slide switch is on.
        while (IsSSOn())
        {
            for (; Tries < 10; Tries++)
            {
                LEDController(); // Use the LEDController to control the on/off of the LED based on the CurrentRandom.
                printf("NM$L\r\n");
                TMR0 = 0; // Clear the counts on the Timer0.
                TR0 = 1;  // Turn timer0 on.
                while (Counts < 337)
                {
                    printf("%d\r\n", Counts);
                    while (!IsSSOn())
                        //Assert slide switch on.;               // Wait for 1 second.
                        ;
                }
                ResultController(); // Use ResultController to determine the user inputs.
                TR0 = 0;            // Turn off Timer0.
                Counts = 0;         // Reset the counts of Timer0.
            }
            printf("You have scored %d wins, switch off and on to play again\r\n", Wins);
            Wins = 0;
            TurnLED0Off();
            TurnLED1Off();
            TurnBILEDOff();
            while (IsSSOn())
                ;
            printf("CXK\r\n");
            while (!IsSSOn())
                ;
            Tries = 0;
        }
    }
}

//***************
void Port_Init(void)
{
    // Port 3
    P3MDOUT &= 0xFC; // Set Port 3 input pins to open drain mode. 1111 1100
    P3MDOUT |= 0xF8; // Set Port 3 output pins to push-pull mode. 1111 1000
    P3 |= ~0xFC;     // Set Port 3 input pins to high impedance state.

    // Port 2
    P2MDOUT &= 0xFE; // Set Port 2 input pins to open drain mode
    P2 |= ~0xFE;     // Set Port 2 input pins to high impedance state.
}

void Interrupt_Init(void)
{
    IE |= 0x02; // enable Timer0 Interrupt request (by masking)
    EA = 1;     // enable global interrupts (by sbit)
}
//***************
void Timer_Init(void)
{
    CKCON |= 0x08; // Timer0 uses SYSCLK as source. 0000 1000
    TMOD &= 0xF0;  // clear the 4 least significant bits. 1111 0000
    TMOD |= 0x01;  // Timer0 in mode 1 0000 0001
    TR0 = 0;       // Stop Timer0
    TMR0 = 0;      // Clear high & low byte of T0
}

//***************
void Timer0_ISR(void) __interrupt 1
{
    // add interrupt code here, in this lab, the code will increment the
    // global variable 'counts'
    Counts++; // Increment the Counts variable.
}

void random(void)
{
    CurrentRandom = 0;
    while (1)
    {
        CurrentRandom = rand() % 3; // Generate a random number first.
        if (CurrentRandom != PreviousRandom)
        {
            // When the new random number is not the same as the previous one,
            // break the while loop.
            PreviousRandom = CurrentRandom;
            // Set the PreviousRandom the same as the CurrentRandom.
            break;
        }
    }
    return;
}

void LEDController(void)
{
    TurnLED0Off();
    TurnLED1Off();
    random();
    if (CurrentRandom == 0)
    {
        printf("%d: TurnLED0ON\r\n.", CurrentRandom);
        TurnLED0On();
        return;
    }
    if (CurrentRandom == 1)
    {
        printf("%d: TurnLED1ON\r\n.", CurrentRandom);
        TurnLED1On();
        return;
    }
    if (CurrentRandom == 2)
    {
        printf("%d: TurnLED01ON\r\n.", CurrentRandom);
        TurnLED0On();
        TurnLED1On();
        return;
    }
}

void ResultController(void)
{
    if (CurrentRandom == 0)
    {
        if (IsPB1Pushed() && !IsPB2Pushed())
        {
            TurnBILEDGreen();
            Wins++;
        }
        else
        {
            TurnBILEDRed();
        }
        return;
    }
    if (CurrentRandom == 1)
    {
        if (!IsPB1Pushed() && IsPB2Pushed())
        {
            TurnBILEDGreen();
            Wins++;
        }
        else
        {
            TurnBILEDRed();
        }
        return;
    }
    if (CurrentRandom == 2)
    {
        if (IsPB1Pushed() && IsPB2Pushed())
        {
            TurnBILEDGreen();
            Wins++;
        }
        else
        {
            TurnBILEDRed();
        }
        return;
    }
    TurnBILEDRed();
    return;
}

unsigned char IsPB1Pushed(void)
{
    if (!PB1) // When PB1 is pushed.
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

unsigned char IsPB2Pushed(void)
{
    if (!PB2) // When PB2 is pushed.
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

unsigned char IsSSOn(void)
{
    if (!SS) // When the slide switch is on.
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void TurnBILEDGreen(void)
{
    printf("GREEN\r\n");
    BILED1 = 0;
    BILED2 = 1;
}

void TurnBILEDRed(void)
{
    printf("RED\r\n");
    BILED1 = 1;
    BILED2 = 0;
}

void TurnBILEDOff(void)
{
    BILED1 = 0;
    BILED2 = 0;
}

void TurnLED0On(void)
{
    LED0 = 0;
}

void TurnLED0Off(void)
{
    LED0 = 1;
}

void TurnLED1On(void)
{
    LED1 = 0;
}

void TurnLED1Off(void)
{
    LED1 = 1;
}