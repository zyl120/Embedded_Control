// Section 02, Side 9B,10B,11B
// Z Jin, Zhongqi Liu, Yilu Zhou
// Code for lab 6
// 6/28/2019

#include <c8051_SDCC.h>
#include <stdlib.h>
#include <stdio.h>
#include <i2c.h>

const unsigned int PCA_START = 28635;         // The PCA_START value for 20ms period.
unsigned char STAGE = 0;                      // The variable to show the stage of the operation.
__xdata unsigned int KP = 0;                  // The proportional gain for the PW.
__xdata unsigned int KD = 0;                  // The derivative gain for the PW.
int CURRENT_HEADING = 0;                      // The current heading of the gondola.
int PREVIOUS_HEADING = 0;                     // The previous heading of the gondola.
__xdata int TARGET_HEADING = 4000;            // The target heading of the gondola
__xdata unsigned char RANGER_ADDRESS = 0xE0;  // The address of the ultrasonic ranger.
unsigned char RANGER_DATA[2];                 // The data from the ranger.
__xdata unsigned char COMPASS_ADDRESS = 0xC0; // The address of the compass.
unsigned char COMPASS_DATA[2];                // The data from the compass.
unsigned int PCA_COUNTER = 0;                 // The PCA counter for the overflows.
unsigned int TIME = 0;                        // The varibale to count the PCA0 overflows.
__xdata const unsigned int MAX_PW = 3502;     // The max pw for the fan.
__xdata const unsigned int NEUT_PW = 2765;    // The neutral pw for the fan.
__xdata const unsigned int MIN_PW = 2028;     // The min pw for the fan.
long int TEMP_PW = 0;                         // The temp variable to hold the PW.
__xdata unsigned int ANGLE_PW = 2765;         // The PW for the tail fan angle.
unsigned int TAIL_FAN_PW = 0;                 // The PW for the tail fan.
unsigned int LEFT_FAN_PW = 0;                 // The PW for the left fan.
unsigned int RIGHT_FAN_PW = 0;                // The PW for the right fan.
unsigned int DISTANCE = 0;                    // The distance read by the ranger.
int CURRENT_HEADING_ERROR = 0;                // The current error in the heading.
int PREVIOUS_HEADING_ERROR = 0;               // The previous heading error.
unsigned char RANGER_READ_INDICATOR = 0;      // The indicator to read the ranger.
unsigned int SEQUENCE[3];                     // The variable to store the TARGET_HEADING.
int TEMP = 0;                                 // The temp TARGET_HEADING.

void Read_Compass(void);
void Read_Ranger(void);
void Fan_Controller(void);
void Adjust_Angle(void);
void Read_Gains(void);
void Fan_Init(void);
void PCA_ISR(void) __interrupt 9;
void Port_Init(void);
void PCA_Init(void);
void SMB_Init(void);
void Interrupt_Init(void);
void XBR0_Init(void);
void Flight_Recorder(void);
void Sequence_Control(void);
void Wait_For_1s(void);

__sbit __at 0xB7 SS; // The sbit for the slide switch.

void main(void)
{
    Sys_Init(); // initialize board
    putchar(' ');
    Port_Init();
    PCA_Init();
    SMB_Init();
    Interrupt_Init();
    XBR0_Init();
    Fan_Init();
    printf("Begin Angle Adjustment.\r\n");
    Adjust_Angle();
    printf("The angle adjustment is completed.\r\n");
    Read_Gains(); // Read the target heading from the keyboard.
    Sequence_Control();
    printf("Beginning control sequence.\r\n");
    TIME = 0;
    STAGE = 0;
    while (1)
    {
        TARGET_HEADING = SEQUENCE[STAGE]; // Get the current heading from the sequence array.
    }
}

void Fan_Controller(void)
{
    // This function will control the PW for the three fans.
    PREVIOUS_HEADING = CURRENT_HEADING; //store previous heading value
    PREVIOUS_HEADING_ERROR = TARGET_HEADING - PREVIOUS_HEADING;
    if (PREVIOUS_HEADING_ERROR > 1800)
    {
        PREVIOUS_HEADING_ERROR -= 3600;
    }
    else if (PREVIOUS_HEADING_ERROR < -1800)
    {
        PREVIOUS_HEADING_ERROR += 3600;
    }
    Read_Compass(); //Read new heading value
    if (CURRENT_HEADING == PREVIOUS_HEADING)
    {
        // If there is no change in compass reading, do nothing.
        return;
    }
    CURRENT_HEADING_ERROR = TARGET_HEADING - CURRENT_HEADING; // Calculate the current heading error.
    if (CURRENT_HEADING_ERROR > 1800)
    {
        CURRENT_HEADING_ERROR -= 3600;
    }
    else if (CURRENT_HEADING_ERROR < -1800)
    {
        CURRENT_HEADING_ERROR += 3600;
    }
    // Calculate PW for tail fan
    TEMP_PW = NEUT_PW + (long)KP * CURRENT_HEADING_ERROR + (long)KD * (CURRENT_HEADING_ERROR - PREVIOUS_HEADING_ERROR);
    if (TEMP_PW > (long)MAX_PW)
    {
        TEMP_PW = MAX_PW;
    }
    else if (TEMP_PW < (long)MIN_PW)
    {
        TEMP_PW = MIN_PW;
    }
    if (STAGE == 1)
    {
        TEMP_PW = NEUT_PW;
    }
    TAIL_FAN_PW = (unsigned int)TEMP_PW;
    // For right fans.
    TEMP_PW = NEUT_PW - (long)KP * CURRENT_HEADING_ERROR - (long)KD * (CURRENT_HEADING_ERROR - PREVIOUS_HEADING_ERROR);
    if (TEMP_PW > (long)MAX_PW)
    {
        TEMP_PW = MAX_PW;
    }
    else if (TEMP_PW < (long)MIN_PW)
    {
        TEMP_PW = MIN_PW;
    }
    if (STAGE == 2)
    {
        TEMP_PW = NEUT_PW;
    }
    RIGHT_FAN_PW = (unsigned int)TEMP_PW;
    // For left fans.
    TEMP_PW = NEUT_PW + (long)KP * CURRENT_HEADING_ERROR + (long)KD * (CURRENT_HEADING_ERROR - PREVIOUS_HEADING_ERROR);
    if (TEMP_PW > (long)MAX_PW)
    {
        TEMP_PW = MAX_PW;
    }
    else if (TEMP_PW < (long)MIN_PW)
    {
        TEMP_PW = MIN_PW;
    }
    if (STAGE == 2)
    {
        TEMP_PW = NEUT_PW;
    }
    LEFT_FAN_PW = (unsigned int)TEMP_PW;
    // Execute the pw to the PCA0.
    PCA0CP0 = 0xFFFF - TAIL_FAN_PW;
    PCA0CP2 = 0xFFFF - LEFT_FAN_PW;
    PCA0CP3 = 0xFFFF - RIGHT_FAN_PW;
}

void Sequence_Control(void)
{
    int i = 1;
    SEQUENCE[0] = TARGET_HEADING; // Set the first variable in the SEQUENCE.
    if (SS)
    {
        for (; i < 3; i++)
        {
            TEMP = SEQUENCE[i - 1] + 1200;
            if (TEMP > 3600)
            {
                TEMP -= 3600;
            }
            SEQUENCE[i] = (unsigned)TEMP; // Increment the target heading in the next space.
        }
    }
    else
    {
        for (; i < 3; i++)
        {
            TEMP = SEQUENCE[i - 1] - 1200; // Decrement the target heading in the next space.
            if (TEMP < 0)
            {
                TEMP += 3600;
            }
            SEQUENCE[i] = (unsigned)TEMP;
        }
    }
}

void Adjust_Angle(void)
{
    // This function will control the angle of the fans.
    while (1)
    {
        char input;
        input = getchar();
        if (input == 'w') // thrust angle up
        {
            ANGLE_PW = ANGLE_PW + 30;
        }
        if (input == 's') // thrust angle down
        {
            ANGLE_PW = ANGLE_PW - 30;
        }
        if (input == 'x')
        {
            break;
        }
        PCA0CP1 = 0xFFFF - ANGLE_PW;
    }
    PCA0CP0 = 0xFFFF - NEUT_PW;
    PCA0CP2 = 0xFFFF - NEUT_PW;
    PCA0CP3 = 0xFFFF - NEUT_PW;
}

void Read_Gains(void)
{
    printf("Read gains from the keypads.\r\n");
    lcd_clear();
    lcd_print("KP:\n"); // Set the proportional gain.
    KP = kpd_input(1);
    lcd_clear();

    lcd_print("KD:\n"); // set direvative gain
    KD = kpd_input(1);
    lcd_clear();

    lcd_print("Please enter the TARGET HEADING:\r"); // Set the target heading.
    TARGET_HEADING = kpd_input(1);
    printf("TARGET HEADING Set To %d.\r\n", TARGET_HEADING);
    lcd_clear();
    lcd_print("Ready.");
    Wait_For_1s();
    TIME = 0;
}

void Read_Ranger(void)
{
    i2c_read_data(RANGER_ADDRESS, 2, RANGER_DATA, 2);                  // read two bytes from the ranger.
    DISTANCE = (((unsigned int)RANGER_DATA[0] << 8) | RANGER_DATA[1]); // Convert the data.
    RANGER_DATA[0] = 0x51;                                             // Ping the ranger.
    i2c_write_data(RANGER_ADDRESS, 0, RANGER_DATA, 1);
}

void Read_Compass(void)
{
    i2c_read_data(COMPASS_ADDRESS, 2, COMPASS_DATA, 2);                         // Read the data from the compass.
    CURRENT_HEADING = (((unsigned int)COMPASS_DATA[0] << 8) | COMPASS_DATA[1]); // Convert the data to readable value.
}

void Fan_Init(void)
{
    // This function will set all three fans to neut for about 1s.
    while (PCA_COUNTER < 51)
    {
        PCA0CP0 = 0xFFFF - NEUT_PW;
        PCA0CP2 = 0xFFFF - NEUT_PW;
        PCA0CP3 = 0xFFFF - NEUT_PW;
    }
    PCA_COUNTER = 0; // reset the PCA_COUNTER to 0.
}

void Port_Init(void)
{
    P0MDOUT &= 0xF3;
    P0MDOUT |= 0xF0;
    P0 |= ~0xF3;
    P1MDIN &= 0xF7;
    P1MDOUT &= 0xF7;
    P1 |= ~0xF7;
    P3MDOUT &= 0x7F;
    P3 |= ~0x7F;
}

void PCA_Init(void)
{
    PCA0CN = 0x40;   // Enable PCA counter
    PCA0MD = 0x81;   // Enable SYSCLK/12 and enable interrupt.
    PCA0CPM0 = 0xC2; // Rudder Fan on CCM0.
    PCA0CPM1 = 0xC2; // Thrust Angle on CCM1.
    PCA0CPM2 = 0xC2; // Left Fan on CCM2
    PCA0CPM3 = 0xC2; // Right Fan on CCM3.
}

void SMB_Init(void)
{
    SMB0CR = 0x93; // Set the clock frequency to be 100kHz.
    ENSMB = 1;     // Enable SMB.
}

void Interrupt_Init(void)
{
    EA = 1;       // Enable all interrupts.
    EIE1 |= 0x08; // Enable PCA0 interrupt.
}

void XBR0_Init(void)
{
    XBR0 = 0x25; // Enable XBR0 according to the lab guide.
}

void Wait_For_1s(void)
{
    // WARNING: USING THIS FUNCTION WILL CLEAR PCA_COUNTER.
    // Wait for 1s.
    PCA_COUNTER = 0;
    while (PCA_COUNTER < 51)
        ; // Wait for 1s.
    PCA_COUNTER = 0;
}

void PCA_ISR(void) __interrupt 9
{
    if (CF)
    {
        CF = 0; // clear overflow indicator
        PCA_COUNTER++;
        TIME++;
        if (PCA_COUNTER % 25 == 0)
        {
            RANGER_READ_INDICATOR = 1; // Read the data from the ranger.
        }
        if (PCA_COUNTER % 20 == 0 && TARGET_HEADING != 4000)
        {
            Flight_Recorder(); // print the data.
        }
        if (PCA_COUNTER % 1 == 0 && TARGET_HEADING != 4000)
        {
            Fan_Controller();
        }
        if (TIME % 1000 == 0)
        {
            STAGE += 1;
            if (STAGE >= 3)
            {
                STAGE = 0;
            }
        }
        PCA0 = PCA_START;
    }
    PCA0CN &= 0x40; // handle other PCA interrupt sources
}

void Flight_Recorder(void)
{
    // This function will print the data to the terminal in .csv format.
    printf("%d,%d,%d,%d,%d,%d,%d\r\n", TIME, DISTANCE, TARGET_HEADING, CURRENT_HEADING, TAIL_FAN_PW, LEFT_FAN_PW, RIGHT_FAN_PW);
}
