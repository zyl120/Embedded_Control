/* Sample code for Lab 3.1. This program can be used to test the steering servo */
#include <c8051_SDCC.h>
#include <stdio.h>
#include <stdlib.h>
#include <i2c.h>
//-----------------------------------------------------------------------------
// 8051 Initialization Functions
//-----------------------------------------------------------------------------
//these are from lab3.1
void Port_Init(void);
//these are from lab3.2
void SMB_Init(void);
//these are shared by lab3.1 and lab3.2
void PCA_Init(void);
void XBR0_Init();
void Interrupt_Init(void);
void PCA_ISR(void) __interrupt 9;
//-----------------------------------------------------------------------------
// Initialization Global Variables
//-----------------------------------------------------------------------------
//these are from lab3.1
unsigned int PW_CENTER = 2695; //center point for pw
unsigned int PW_RIGHT = 3170;  //the rightside limitation for pw
unsigned int PW_LEFT = 2220;   //the leftside limitation for pw
unsigned int SERVO_PW = 0;     //current PW
unsigned int count = 0;        //counting for overflow to produce 1 sec delay
//these are shared by lab3.1 and lab3.2
unsigned int PW_START = 28614; //set a 20ms period
//these are from lab3.2
unsigned int ReadCompass(void); //return a int for angle
unsigned int h_count;           //how many times has overflowed
unsigned int new_heading = 0;   //boolean for to read or not
//these are from lab3.3

unsigned int Desired_Heading; //desired heading
signed int error;             //error
unsigned int Actual_Heading;  //actual heading
//PCA 0 COUNTER OVERFLOW FLAG
__sbit __at 0xDF CF;
__sbit __at 0xB7 SS;
//-----------------------------------------------------------------------------
// Main Function
//-----------------------------------------------------------------------------
void main(void)
{
    // initialize board
    Sys_Init();
    putchar(' '); //the quotes in this line has been formated correctly
    SMB_Init();
    XBR0_Init();
    PCA_Init();
    Interrupt_Init();
    Port_Init();
    // initialize every thing
    while (count <= 50) //50 overflows is 1 sec delay
        ;               //wait for 1 sec
    while (1)           //start while loop
    {
        if (new_heading) //	if switch is on and 40 ms has passed
        {
            if (SS)
            {
                Actual_Heading = ReadCompass();                  //let Actuall_Heading carry angle value
                error = Desired_Heading - Actual_Heading;        //calculate error
                printf("error before adjust is %d \r\n", error); //print out error before adjust
                if (error > 1800)                                //if error larger than simi circle
                {
                    error -= 3600;
                }                       //turn by opposite direction
                else if (error < -1800) //if error larger than simi circle
                {
                    error += 3600;
                }                                               //turn by opposite direction
                printf("error after adjust is %d \r\n", error); //print out error after adjust
                SERVO_PW = error * 3 / 8 + PW_CENTER;           // 675 / 1800 = 3/8
                printf("SERVO_PW is %d \r\n", SERVO_PW);        //print servo before adjust
                if (SERVO_PW > PW_RIGHT)
                {
                    SERVO_PW = PW_RIGHT; //limit SERVO_PW to PW_RIGHT
                }
                if (SERVO_PW < PW_LEFT)
                {
                    SERVO_PW = PW_LEFT; //limit SERVO_PW to PW_LEFT
                }
                new_heading = 0; //start new 40 ms period
                PCA0CPL0 = 0xFFFF - SERVO_PW;
                PCA0CPH0 = (0xFFFF - SERVO_PW) >> 8;                   //update servo command
                printf("limited SERVO_PW is %d \r\n", SERVO_PW);       //print
                printf("derised heading is %d \r\n", Desired_Heading); //every
                printf("actual heading is %u \r\n", Actual_Heading);   //thing
            }
            else //if ss is off
            {
                SERVO_PW = PW_CENTER; //set wheel to center
                printf("SS is off.\r\n");
                PCA0CPL0 = 0xFFFF - SERVO_PW;
                PCA0CPH0 = (0xFFFF - SERVO_PW) >> 8; //update servo command
                while (!SS)                          //wait till ss is on
                    ;
            }
        }
    }
}
//-----------------------------------------------------------------------------
// Port_Init
//-----------------------------------------------------------------------------
//
// Set up ports for input and output
//
void Port_Init()
{
    P1MDOUT |= 0x0D; //set output pin for CEX0 in push-pull mode
    P3MDOUT &= 0x7F;
    P3 |= ~0x7F;
}
//-----------------------------------------------------------------------------
// SMB_Init
//-----------------------------------------------------------------------------
//
// Set up SMBus
//
void SMB_Init()
{
    SMB0CR = 0x93; //set SCL to 100KHz

    ENSMB = 1; //Enable SMBus
}
//-----------------------------------------------------------------------------
// XBR0_Init
//-----------------------------------------------------------------------------
//
// Set up the crossbar
//
void XBR0_Init()
{
    XBR0 = 0x27; //configure the crossbar as directed in the labor manual
}
//-----------------------------------------------------------------------------
// PCA_Init
//-----------------------------------------------------------------------------
//
// Set up Programmable Counter Array
//
void PCA_Init(void)
{
    PCA0MD = 0x81;   //enable SYSCLK/12 and enable interrupt
    PCA0CPM0 = 0xC2; //CCM0 16bit
    PCA0CN = 0x40;   //enable PCA counter
}
//-----------------------------------------------------------------------------
// Interrupt_Init
//-----------------------------------------------------------------------------
//
// Set up ports for input and output
//
void Interrupt_Init()
{
    EA = 1;       //Enable interrupts
    EIE1 |= 0x08; //Enable PCA overflow interrupts
}
//-----------------------------------------------------------------------------
// PCA_ISR
//-----------------------------------------------------------------------------
//
// Interrupt Service Routine for Programmable Counter Array Overflow Interrupt
//
void PCA_ISR(void) __interrupt 9
{
    // reference to the sample code in Example 4.5 -Pulse Width Modulation implemented using
    if (CF) //If PCA interrupt flag is set
    {
        CF = 0;            //Clear the overflow flag
        PCA0 = PW_START;   //Set PCA0 to PCA_START
        h_count++;         //record overflow times by 1
        if (h_count >= 10) //if two overflow has passed
        {
            new_heading = 1; //set new_heading to 1
            h_count = 0;     //set h_count to 0
        }
        PCA0CN &= 0x40; //handle other PCA interrupt sources
        count++;        //increment count calculate the 1 sec delay
    }
}

unsigned int ReadCompass(void)
{
    unsigned char addr = 0xC0;       //0xC0 is compass's address
    unsigned char Data[2];           //create an array with length of 2
    unsigned int heading;            //value for returning degrees between 0 and 3599
    i2c_read_data(addr, 2, Data, 2); //read two byte starting at reg 2
    printf("%d,%d\r\n", Data[0], Data[1]);
    heading = (((unsigned int)Data[0] << 8) | Data[1]); //set heading equals the combine of two values from reg 2
    return heading;                                     //return value in thenths degrees
}