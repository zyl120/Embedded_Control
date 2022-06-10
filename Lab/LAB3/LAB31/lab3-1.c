#include <stdio.h>
#include <c8051_SDCC.h>
#include <i2c.h>
#define PW_MIN 2028
#define PW_MAX 3502
#define PW_NEUT 2765
//-----------------------------------------------------------------------------
// 8051 Initialization Functions
//-----------------------------------------------------------------------------
void Port_Init(void);
void PCA_Init(void);
void XBR0_Init(void);
void Interrupt_Init(void);
void Drive_Motor(void);
void PCA_ISR(void) __interrupt 9;
unsigned int MOTOR_PW = 0;           // The PW to control the motor.
unsigned int PCA_COUNTER = 0;        // the variable to count the PCA overflow.

__sbit __at 0xDF CF; // PCA0 COUNTER OVERFLOW FLAG
//-----------------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------------
void main(void)
{
    // initialize board
    Sys_Init();
    putchar(' ');
    Port_Init();
    XBR0_Init();
    
    PCA_Init();
    Interrupt_Init();
    //print beginning message
    printf("Embedded Control Drive Motor Control\r\n");
    //set initial value
    MOTOR_PW = PW_NEUT;
    PCA0CPL2 = 0xFFFF - MOTOR_PW;
    PCA0CPH2 = (0xFFFF - MOTOR_PW) >> 8;
    while (PCA_COUNTER < 51)
        ; // Wait for 1s (20ms * 50 = 1s)
    PCA_COUNTER = 0;
    while (1)
    {
        printf("PCA_COUNTER: %d\r\n", PCA_COUNTER);
        Drive_Motor();
    }
}
//-----------------------------------------------------------------------------
// Drive_Motor
//-----------------------------------------------------------------------------
//
// Vary the pulsewidth based on the user input to change the speed
// of the drive motor.
//
void Drive_Motor()
{
    char input;
    printf("press f to increase, s to decrease.\r\n");
    //wait for a key to be pressed
    input = getchar();
    if (input == 'f') //if 'f' is pressed by the user
    {
        if (MOTOR_PW < PW_MAX)
            MOTOR_PW = MOTOR_PW + 10; //increase the steering pulsewidth by 10
    }
    else if (input == 's') //if 's' is pressed by the user
    {
        if (MOTOR_PW > PW_MIN)
            MOTOR_PW = MOTOR_PW - 10; //decrease the steering pulsewidth by 10
    }
    PCA0CPL2 = 0xFFFF - MOTOR_PW;
    PCA0CPH2 = (0xFFFF - MOTOR_PW) >> 8;
    printf("MOTOR_PW: %d, \r\n", MOTOR_PW);
}
//-----------------------------------------------------------------------------
// Port_Init
//-----------------------------------------------------------------------------
//
// Set up ports for input and output
//
void Port_Init()
{
    P1MDOUT = 0x0D; //set output pin for CEX2 in push-pull mode
}
//-----------------------------------------------------------------------------
// Interrupt_Init
//-----------------------------------------------------------------------------
//
// Set up ports for input and output
//
void Interrupt_Init()
{
    EA = 1;      // Enable general interrupt
    EIE1 |= 0x08; // Enable PCA overflow interrupts
}
//-----------------------------------------------------------------------------
// XBR0_Init
//-----------------------------------------------------------------------------
//
// Set up the crossbar
//
void XBR0_Init()
{
    XBR0 = 0x27; //configure crossbar with UART, SPI, SMBus, and CEX channels
    //as in worksheet
}
//-----------------------------------------------------------------------------
// PCA_Init
//-----------------------------------------------------------------------------
//
// Set up Programmable Counter Array
//
void PCA_Init(void)
{
    PCA0MD = 0x81;   //Enable SYSCLK/12 and enable interrupt.
    PCA0CPM2 = 0xC2; // Enable CCM2 16bit PWM.
    PCA0CN = 0x40;   // Enable PCA counter.
    PCA0 = 28614;  // for 20ms period
}
//-----------------------------------------------------------------------------
// PCA_ISR
//-----------------------------------------------------------------------------
//
// Interrupt Service Routine for Programmable Counter Array Overflow Interrupt
//
void PCA_ISR(void) __interrupt 9
{
    PCA_COUNTER++;
    if (CF)
    {
        CF = 0;          // Clear overflow flag
        PCA0 = 28614; // (or PCA0 = 0x2800) Start count for 20 ms period
    }
    PCA0CN &= 0x40; // handle other PCA0 overflows
    // reference to the sample code in Example 4.5 -Pulse Width Modulation implemented using
    //the PCA(Programmable Counter Array), p.50 in Lab Manual.
}