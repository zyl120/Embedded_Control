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
unsigned int Read_Ranger_0(void);  // The function to read the ranger software revision info.
unsigned int Read_Ranger_1(void);  // The function to read the ranger data in inches.
unsigned int Read_Compass_0(void); // The function to read the compass software revision info.
unsigned int Read_Compass_1(void); // The function to read the compass orientation.

unsigned int PCA_COUNTER = 0;               // The variable to count the PCA0 overflow.
unsigned int PCA_START = 28614;             // The variable to start the 20ms period.
unsigned int RANGER_READ_COUNTER = 0;       // The variable to signal the microcontroller to read the ranger data.
unsigned int COMPASS_READ_COUNTER = 0;      // The variable to signal the microcontroller to read the compass data.
unsigned int COMPASS_PRINT_COUNTER = 0;     // The variable to signal the print of the compass data.
unsigned int DISTANCE = 0;                  // The distance found by the ultrasonic ranger.
unsigned int CompassD;                      // The angle value for printing
unsigned int COMPASS_SOFTWARE_REVISION = 0; // The compass software revision info.
unsigned char RANGER_DATA[2];               // The data of the ranger
unsigned char COMPASS_DATA[2];              // The data  for compass
unsigned int RANGER_SOFTWARE_REVISION = 0;  // The ranger software revision info.
const unsigned char RANGER_ADDR = 0xE0;     // The address of ultrasonic ranger.
const unsigned char COMPASS_ADDR = 0xC0;    // The address of the compass.
__sbit __at 0xDF CF;

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
        if (RANGER_READ_COUNTER)
        {
            // If the RANGER_READ_COUNTER is not 0.
            DISTANCE = Read_Ranger_1();
            printf("The distance is: %d inches.\r\n", DISTANCE);
            RANGER_SOFTWARE_REVISION = Read_Ranger_0();
            printf("The ranger software is %d.\r\n", RANGER_SOFTWARE_REVISION);
            RANGER_READ_COUNTER = 0;
        }
        if (COMPASS_READ_COUNTER)
        {
            // If the COMPASS_READ_COMPASS is not 0.
            CompassD = Read_Compass_1();
            printf("The direction is %d.\r\n", CompassD);
            COMPASS_SOFTWARE_REVISION = Read_Compass_0();
            printf("The compass software is %d.\r\n", COMPASS_SOFTWARE_REVISION);
            COMPASS_READ_COUNTER = 0;
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
        if (PCA_COUNTER % 2 == 0)
        {
            // Signal the microcontroller to read the compass.
            COMPASS_READ_COUNTER++;
        }
        if (PCA_COUNTER % 4 == 0)
        {
            // Signal the microcontroller to read the ranger.
            RANGER_READ_COUNTER++; // Count for 80ms.
        }
        if (PCA_COUNTER % 5 == 0)
        {
            // Signal the microcontroller to print the data from the compass.
            COMPASS_PRINT_COUNTER++;
            PCA_COUNTER = 0;
        }
        CF = 0;           // Clear overflow flag
        PCA0 = PCA_START; // Start count for 20 ms period
    }
    PCA0CN &= 0x40; // handle other PCA0 overflows
}

void Ping_Ranger()
{
    RANGER_DATA[0] = 0x50;                          // Read the RANGER_DATA in inches.
    i2c_write_data(RANGER_ADDR, 0, RANGER_DATA, 1); // write one byte of RANGER_DATA to reg 0 at RANGER_ADDR.
}

unsigned int Read_Ranger_0()
{
    i2c_read_data(RANGER_ADDR, 0, RANGER_DATA, 1); // Read the software revision from ranger.
    return RANGER_DATA[0];                         // return the ranger software revision info.
}

unsigned int Read_Ranger_1()
{
    unsigned int range = 0;
    i2c_read_data(RANGER_ADDR, 6, RANGER_DATA, 2);                // Read the ranger register 6 and 7.
    range = ((unsigned int)RANGER_DATA[0] << 8 | RANGER_DATA[1]); // Convert the two info to one number
    Ping_Ranger();
    return range;
}

unsigned int Read_Compass_0()
{
    i2c_read_data(COMPASS_ADDR, 0, COMPASS_DATA, 1); // Read the compass software revision data.
    return COMPASS_DATA[0];                          // Return the read data.
}

unsigned int Read_Compass_1()
{
    unsigned int heading;                                               //value for returning degrees between 0 and 3599
    i2c_read_data(COMPASS_ADDR, 4, COMPASS_DATA, 2);                    //read two byte starting at reg 2
    heading = (((unsigned int)COMPASS_DATA[0] << 8) | COMPASS_DATA[1]); //set heading equals the combine of two values from reg 2
    return heading;                                                     //return value in thenths degrees
}
