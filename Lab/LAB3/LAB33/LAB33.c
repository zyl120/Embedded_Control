#include <stdio.h>
#include <stdlib.h>
#include <c8051_SDCC.h>
#include <i2c.h>
#define PW_MIN 2028     // The PW for the motor to do full reverse
#define PW_MAX 3505     // The PW for the motor to do full forward
#define PW_NEUT 2765    // The PW for the motor to stop
#define PCA_START 28614 // The variable to start the PCA0 for 20ms period

// C8051 Initialization funtion.
void Port_Init(void);
void PCA_Init(void);
void XBR0_Init(void);
void Interrupt_Init(void);
void PCA_ISR(void) __interrupt 9;
void SMB_Init(void);

// Other functions related to motor and sensors.
void Driver_Motor(void);
void Ping_Ranger(void);
unsigned int Read_Ranger(void);
unsigned char Is_SS_On(void);

// The global variables
const unsigned char RANGER_ADDR = 0xE0; // The address of the ultrasonic ranger.
unsigned int MOTOR_PW = 0;              // The PW of the motor.
unsigned int PCA_COUNTER = 0;           // The variable to count the overflow of PCA0.
unsigned char READ_COUNTER = 0;         // The variable to signal the microcontroller to read the sensor.
unsigned int DISTANCE = 0;              // The calculated distance from the sensor.
unsigned char DATA[2];                  // The container for the data from ultrasonic ranger.
__sbit __at 0xDF CF;                    // The address of ultrasonic ranger overflow flag.
__sbit __at 0xB6 SS;                    // The address of the slide switch.

// main function
void main(void)
{
    Sys_Init();
    putchar(' ');
    Port_Init();
    SMB_Init();
    XBR0_Init();
    PCA_Init();
    Interrupt_Init();

    printf("Embedded Control Drive Motor Control\r\n"); //print beginning message
    MOTOR_PW = PW_NEUT;                                 // Set the motor to neutral.
    PCA0CPL2 = 0xFFFF - MOTOR_PW;                       // Set the low byte for the CEX2.
    PCA0CPH2 = (0xFFFF - MOTOR_PW) >> 8;                // Set the high byte for the CEX2.
    while (PCA_COUNTER < 51)
        printf("%d",PCA_COUNTER);
    PCA_COUNTER = 0; // Reset the PCA0 counter to 0.
    while (1)
    {
        // Begin infinite loop
        if (Is_SS_On() && READ_COUNTER == 1)
        {
			printf("SS is On.\r\n");
            // The case when the slide switch is ON
            // When 80ms has passed
            // First part: Read the data from the sensor
            DISTANCE = Read_Ranger(); // Save the calculated distance to DISTANCE.
            READ_COUNTER = 0;         // Reset the READ_COUNTER.
            // Second part: control the motor
            Driver_Motor();
            printf("********************************\r\n");
            printf("DISTANCE is %d.\r\n", DISTANCE);
        }
        else if (Is_SS_On())
        {
            PCA0CPL2 = 0xFFFF - MOTOR_PW; // Set the low byte for the CEX2.
            PCA0CPH2 = (0xFFFF - MOTOR_PW) >> 8;
        }
        else
        {
			printf("SS is Off.\r\n");
            // The case when the slide switch is OFF
            MOTOR_PW = PW_NEUT;
            PCA0CPL2 = 0xFFFF - MOTOR_PW; // Set the low byte for the CEX2.
            PCA0CPH2 = (0xFFFF - MOTOR_PW) >> 8;
        }
    }
}

void Port_Init()
{
    P1MDOUT = 0x0D;  // Set output pin for CEX2 in push_pull mode.
    P3MDOUT &= 0xBF; // Set the input pin (P3.6) for the SS.
    P3 |= ~0xBF;     // Set high impedance for the SS.
}

void PCA_Init()
{
    PCA0MD = 0x81;    // Enable SYSCLK/12 and enable interrupt.
    PCA0CPM2 = 0xC2;  // Enable CCM2 16bit PWM.
    PCA0CN = 0x40;    // Enable PCA counter.
    PCA0 = PCA_START; // for 20ms period
}

void XBR0_Init()
{
    XBR0 = 0x27; // Configure crossbar with UART, SPI, SMBus, and CEX channels
}

void Interrupt_Init()
{
    EA = 1;       // Enable general interrupt
    EIE1 |= 0x08; // Enable PCA overflow interrupts
}

void SMB_Init()
{
    SMB0CR = 0x93; // Set the clock frequency to be 100kHz
    ENSMB = 1;     // Enable SMB
}

void PCA_ISR(void) __interrupt 9
{
    PCA_COUNTER++; // Increment the PCA_COUNTER.
    if (PCA_COUNTER % 4 == 0)
    {
        READ_COUNTER = 1;
    }
    if (CF)
    {
        CF = 0;           // Clear overflow flag
        PCA0 = PCA_START; // Start count for 20 ms period
    }
    PCA0CN &= 0x40; // handle other PCA0 overflows
}

void Driver_Motor()
{
    int ERROR = 0;
    const int k = 21; // Temp proportional gain.
    if (DISTANCE > 80)
    {
        // When the distance is larger than 80cm, full reverse.
        MOTOR_PW = PW_MIN;
    }
    else if (DISTANCE <= 10)
    {
        // When the distance is smaller or equal to 10cm, full forward.
        MOTOR_PW = PW_MAX;
    }
    else
    {
        ERROR = 45 - DISTANCE;
        printf("Error:%d\r\n", ERROR);
        MOTOR_PW = k * ERROR + PW_NEUT; // Set the MOTOR_PW as needed.
    }
    PCA0CPL2 = 0xFFFF - MOTOR_PW;        // Set the low byte for CEX2.
    PCA0CPH2 = (0xFFFF - MOTOR_PW) >> 8; // Set the high byte for CEX2.
    printf("MOTOR_PW: %d.\r\n", MOTOR_PW);
}

void Ping_Ranger()
{
    DATA[0] = 0x51;                          // Read the data in cm
    i2c_write_data(RANGER_ADDR, 0, DATA, 1); // write one byte of data to reg 0 at addr, read in cm.
}

unsigned int Read_Ranger()
{
    unsigned int i = 0;
    unsigned int RANGE = 0;
	printf("dis:%d", DISTANCE);
    i2c_read_data(RANGER_ADDR, 2, DATA, 2);         // Read the data from the ultrasonic ranger.
    RANGE = ((unsigned int)DATA[0] << 8 | DATA[1]); // Transfer 2 bytes of data to the distance in cm.
    Ping_Ranger();
    return RANGE; // return the range calculated from the read data.
}

unsigned char Is_SS_On()
{
    if (SS == 1)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}