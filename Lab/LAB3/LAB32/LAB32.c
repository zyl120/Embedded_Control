#include <stdio.h>
#include <stdlib.h>
#include <c8051_SDCC.h>
#include <i2c.h>

void SMB_Init(void);
void PCA_Init(void);
void XBR0_Init(void);
void Interrupt_Init(void);
void PCA_ISR(void) __interrupt 9;
void Ping_Ranger(void);
unsigned int Read_Ranger(void);

unsigned int PCA_COUNTER = 0;         // The variables to count the PCA0 overflows
const unsigned int PCA_START = 28614; // for 20ms period.
unsigned int READ_COUNTER = 0;
// The variable to signal the microcontrolller to read data from the ranger.
unsigned int DISTANCE; // The calculated distance from the data.
unsigned char Data[2];
const unsigned char addr = 0xE0; // The address of ultrasonic ranger.
__sbit __at 0xDF CF;             // The PCA0 overflow flag

void main(void)
{
    Sys_Init();
    putchar(' ');
    SMB_Init();
    XBR0_Init();
    Interrupt_Init();
    PCA_Init();
    while (1)
    {
        if (READ_COUNTER)
        {
            DISTANCE = Read_Ranger();
            printf("The distance is: %d cm.\r\n", DISTANCE);
            READ_COUNTER = 0; // Reset READ_COUNTER to 0.
        }
    }
}
void SMB_Init()
{
    SMB0CR = 0x93; // Set the clock frequency to be 100kHz
    ENSMB = 1;     // Enable SMB
}

void XBR0_Init()
{
    XBR0 = 0x27; //configure crossbar with UART, SPI, SMBus, and CEX channels
    //as in worksheet
}

void Interrupt_Init()
{
    EA = 1;       // Enable general interrupt
    EIE1 |= 0x08; // Enable PCA overflow interrupts
}

void PCA_Init()
{
    PCA0MD = 0x81; //Enable SYSCLK/12 and enable interrupt.
    PCA0CN = 0x40; // Enable PCA counter.
}

void PCA_ISR(void) __interrupt 9
{
    PCA_COUNTER++; // Increment the PCA_COUNTER.
    if (CF)
    {
        if (PCA_COUNTER >= 4)
        {
            READ_COUNTER++; // Count for 80ms and read data from the ranger.
            PCA_COUNTER = 0;
        }
        CF = 0;           // Clear overflow flag
        PCA0 = PCA_START; // Start count for 20 ms period
    }
    PCA0CN &= 0x40; // handle other PCA0 overflows
}

void Ping_Ranger()
{
    Data[0] = 0x51;                   // Read the data in cm
    i2c_write_data(addr, 0, Data, 1); // write one byte of data to reg 0 at addr
}

unsigned int Read_Ranger()
{
    unsigned int range = 0;          // range should have the same value as the DISTANCE.
    i2c_read_data(addr, 2, Data, 2); // Read data from the ranger.
    range = ((unsigned int)Data[0] << 8 | Data[1]); // Transfer 2 bytes of data to one number.
    Ping_Ranger();
    return range;                                   // Return the read range.
}
