#include <stdio.h>
#include <stdlib.h>
#include <c8051_SDCC.h>
#include <i2c.h>

// Define constant
#define PCA_START 28614          // The start value for the 20ms period.
#define DRIVE_MOTOR_NEUT 2765    // The PW for the drive motor to stop.
#define STEERING_SERVO_NEUT 2730 // The PW for the steering servo to be neutral.

// Global variables
unsigned int DISTANCE = 0;                     // The distance in cm read by the ultrasonic sensor.
unsigned int HEADING = 0;                      // The value for returning degrees from the compass.
const unsigned char RANGER_ADDRESS = 0xE0;     // The address of the ultrasonic ranger.
const unsigned char COMPASS_ADDRESS = 0xC0;    // The address of the compass.
const unsigned char POT_ADDRESS = 4;           // The address of POT.
unsigned char RANGER_DATA[2];                  // The variable to store the raw data from ranger.
unsigned char COMPASS_DATA[2];                 // The variable to store the raw data from the compass.
unsigned int DRIVE_MOTOR_PW = 0;               // The current PW for the drive motor.
unsigned int STEERING_SERVO_PW = 0;            // The current PW for the steering servo.
unsigned int PCA_COUNTER = 0;                  // The variable to count the PCA overflows.
unsigned char AD_VALUE = 0;                    // The data from POT.
unsigned int DRIVE_MOTOR_MAX = 3505;           // The PW for the drive motor to do full forward.
unsigned int DRIVE_MOTOR_MIN = 2028;           // The PW for the drive motor to do full reverse.
unsigned int STEERING_SERVO_LEFT = 2230;       // The PW for the steering servo to be left.
unsigned int STEERING_SERVO_RIGHT = 3230;      // The PW for the steering servo to be right.
unsigned char OUTPUT_MODE = 1;                 // The format of the output info.
unsigned int TARGET_HEADING = 0;               // The target heading for the car.
unsigned int TARGET_DRIVE_MOTOR_GAIN = 740;    // The target gain of the drive motor.
unsigned int TARGET_STEERING_SERVO_GAIN = 500; // The target gain of the steering servo.
int HEADING_ERROR = 0;                         // The error in the heading.
unsigned int TIME = 0;                         // The counter to count the time.
unsigned int RANGER_READ_COUNTER = 150;        // The flag to set the ranger to be read after 3s.

// Component address
__sbit __at 0x94 POT;    // The address of the potentiometer.
__sbit __at 0xB6 SS;     // The address of the slide switch.
__sbit __at 0xDF CF;     // The address of ultrasonic ranger overflow flag.
__sbit __at 0xB0 BILED0; // The address of BILED0.
__sbit __at 0xB1 BILED1; // The address of BILED1.

// C8051 initialization funtion.
void Port_Init(void);
void PCA_Init(void);
void XBR0_Init(void);
void Interrupt_Init(void);
void SMB_Init(void);
void ADC_Init(void);
void PCA_ISR(void) __interrupt 9;

// Ultrasonic Ranger and Drive Motor functions
void Drive_Motor(void);
void Speed_Controller(void);
void Ping_Ranger(void);
void Read_Ranger(void);
void Drive_Motor_Init(void);

// Compass and Steering Servo functions
void Steering_Servo(void);
void Direction_Controller(void);
void Read_Compass(void);
void Steering_Servo_Init(void);

// POT functions
void Read_AD_Input(void);
void POT_Reader(void);

// Keypad functions
void Read_Keypad(void);

// UI and data record functions
void Flight_Recorder(void);

// Other functions
unsigned int Is_SS_On(void);
void Turn_BILED_Green(void);
void Turn_BILED_Red(void);
void Turn_BILED_Off(void);
void Reset_PCA_Counter(void);
void Wait_For_1s(void);
void Troubleshooter(void);

void main(void)
{
    // The initialization part of the program.
    Sys_Init();
    putchar(' ');
    Port_Init();
    PCA_Init();
    XBR0_Init();
    Interrupt_Init();
    SMB_Init();
    ADC_Init();

    Turn_BILED_Off();
    printf("Embedded Control Flight Recorder\r\n");
    Drive_Motor_Init();
    Steering_Servo_Init();
    //Troubleshooter(); // Should be deleted after the circuit testing is completed.
    while (1)
    {
        if (!SS)
        {
            // When the slide switch is On.
            Reset_PCA_Counter(); // Reset the PCA_COUNTER to 0.
            Turn_BILED_Green();
            DRIVE_MOTOR_PW = DRIVE_MOTOR_MAX; // Set the drive motor PW to forward.
            //record data
            Drive_Motor(); // drive the drive motor.

            RANGER_READ_COUNTER = 0; // Reset the ranger read flag.
            while (!SS)
            {
                Direction_Controller();
                Steering_Servo();
                Drive_Motor();
                if (PCA_COUNTER > 150 || RANGER_READ_COUNTER > 150)
                {
                    Speed_Controller();
                    Reset_PCA_Counter();
                }
            }
            DRIVE_MOTOR_PW = DRIVE_MOTOR_NEUT;
            STEERING_SERVO_PW = STEERING_SERVO_NEUT;
            Drive_Motor();
            Steering_Servo();
        }
        else
        {
            // When the slide switch is Off, use the keypad to read the desired gain and heading
            Read_Keypad(); // Read the input from the keypad
            POT_Reader();
            DRIVE_MOTOR_PW = DRIVE_MOTOR_NEUT;
            Drive_Motor();
            STEERING_SERVO_PW = STEERING_SERVO_NEUT;
            Steering_Servo();
            Reset_PCA_Counter();
            while (SS)
                ;
            lcd_clear();
            lcd_print("SS is on. Execute the code.");
            // Set drive motor and steering servo to neutral position.
            RANGER_READ_COUNTER = 0;
        }
    }
}

void Drive_Motor(void)
{
    // This function will drive the DRIVE_MOTOR.
    PCA0CPL2 = 0xFFFF - DRIVE_MOTOR_PW;        // Set the low byte of CEX2.
    PCA0CPH2 = (0xFFFF - DRIVE_MOTOR_PW) >> 8; // Set the high byte of CEX2.
}

void Speed_Controller(void)
{
    // This function will assign the correct DRIVE_MOTOR_PW according to the DISTANCE.
    if (DISTANCE < 20)
    {
        RANGER_READ_COUNTER = 0; // hold the ranger read flag to 0 to stop speed change.
        // If the distance is smaller than 20cm, change the direction of the drive motor.
        if (DRIVE_MOTOR_PW > DRIVE_MOTOR_NEUT)
        {
            DRIVE_MOTOR_PW = DRIVE_MOTOR_MIN;
            Turn_BILED_Green();
            //TARGET_HEADING += 1800;
        }
        else
        {
            DRIVE_MOTOR_PW = DRIVE_MOTOR_MAX;
            Turn_BILED_Red();
        }
    }
}

void Ping_Ranger(void)
{
    // This function ping the ranger for later data read.
    RANGER_DATA[0] = 0x51;                             // Read the data in cm
    i2c_write_data(RANGER_ADDRESS, 0, RANGER_DATA, 1); // write one byte of data to reg 0 at addr, read in cm.
}

void Read_Ranger(void)
{
    // This function read the distance from the ranger.
    i2c_read_data(RANGER_ADDRESS, 2, RANGER_DATA, 2);                // Read the data from the ultrasonic ranger.
    DISTANCE = ((unsigned int)RANGER_DATA[0] << 8 | RANGER_DATA[1]); // Transfer 2 bytes of data to the distance in cm.
    Ping_Ranger();
}

void Drive_Motor_Init(void)
{
    // This function will set the drive motor to be neutral for about 1s.
    DRIVE_MOTOR_PW = DRIVE_MOTOR_NEUT;
    Drive_Motor(); // Turn the drive motor on to neutral.
    Wait_For_1s();
}

void Steering_Servo(void)
{
    // This function will control the steering servo.
    PCA0CPL0 = 0xFFFF - STEERING_SERVO_PW;
    PCA0CPH0 = (0xFFFF - STEERING_SERVO_PW) >> 8; //update servo command.
}

void Steering_Servo_Init(void)
{
    // This function will set the drive motor to be neutral for about 1s.
    STEERING_SERVO_PW = STEERING_SERVO_NEUT;
    Steering_Servo(); // Turn the drive motor on to neutral.
    Wait_For_1s();
}

void Direction_Controller(void)
{
    // This function will read the compass and set the PW for the steering servo.
    Read_Compass();
    HEADING_ERROR = TARGET_HEADING - HEADING; //calculate error
    if (HEADING_ERROR > 1800)                 //if error larger than simi circle
    {
        HEADING_ERROR -= 3600;
    }                               //turn by opposite direction
    else if (HEADING_ERROR < -1800) //if error larger than simi circle
    {
        HEADING_ERROR += 3600;
    } //turn by opposite direction
    if (DRIVE_MOTOR_PW < DRIVE_MOTOR_NEUT)
    {
        HEADING_ERROR = 0 - HEADING_ERROR;
    }
    STEERING_SERVO_PW = HEADING_ERROR * 5 / 11 + STEERING_SERVO_NEUT; // 500 / 1800 = 3/8
    if (STEERING_SERVO_PW > STEERING_SERVO_RIGHT)
    {
        STEERING_SERVO_PW = STEERING_SERVO_RIGHT; //limit SERVO_PW to PW_RIGHT
    }
    if (STEERING_SERVO_PW < STEERING_SERVO_LEFT)
    {
        STEERING_SERVO_PW = STEERING_SERVO_LEFT; //limit SERVO_PW to PW_LEFT
    }
}

void Read_Compass(void)
{
    // This function will read the data from the compass.
    i2c_read_data(COMPASS_ADDRESS, 2, COMPASS_DATA, 2);                 // Read 2 byte starting at REG 2.
    HEADING = (((unsigned int)COMPASS_DATA[0] << 8) | COMPASS_DATA[1]); // Convert the data to decimal format.
}

void Read_AD_Input(void)
{
    // This function will read the voltage input and output the AD_value.
    AMX1SL = POT_ADDRESS;    // Set P1.4 as the analog input for ADC1
    ADC1CN = ADC1CN & ~0x20; // Clear the "Conversion Completed" flag
    ADC1CN = ADC1CN | 0x10;  // Initiate A/D conversion
    while ((ADC1CN & 0x20) == 0x00)
        ;            // Wait for conversion to complete
    AD_VALUE = ADC1; // Set the AD_VALUE.
}

void POT_Reader(void)
{
    // This function will use the AD_VALUE and change the max and min of DRIVE MOTOR PW.
    Read_AD_Input();                                                                           // Read the data from POT.
    DRIVE_MOTOR_MAX = DRIVE_MOTOR_NEUT + (TARGET_DRIVE_MOTOR_GAIN * 10 / 255 * AD_VALUE / 10); // Calculate the max of drive motor.
    DRIVE_MOTOR_MIN = DRIVE_MOTOR_NEUT - (TARGET_DRIVE_MOTOR_GAIN * 10 / 255 * AD_VALUE / 10); // Calculate the min of drive motor.
    STEERING_SERVO_RIGHT = STEERING_SERVO_NEUT + (TARGET_STEERING_SERVO_GAIN * 10 / 255 * AD_VALUE / 10);
    // Calculate the right PW for the steering servo.
    STEERING_SERVO_LEFT = STEERING_SERVO_NEUT - (TARGET_STEERING_SERVO_GAIN * 10 / 255 * AD_VALUE / 10);
    // Calculate the left PW for the steering servo.
    printf("%d,%d,%d,%d.\r\n", DRIVE_MOTOR_MAX, DRIVE_MOTOR_MIN, STEERING_SERVO_RIGHT, STEERING_SERVO_LEFT);
}

void Read_Keypad(void)
{
    lcd_clear();
    lcd_print("Enter required data.");
    lcd_clear();
    // This function will read the input from the keypad.
    lcd_print("Desired direction to east pm 30 degree.\n");
    TARGET_HEADING = kpd_input(1); // Get the target heading from the keypad.
    lcd_clear();
    while (TARGET_HEADING < 600 || TARGET_HEADING > 1200)
    {
        // Prompt the user to change the input value if it is not satisfied.
        lcd_print("Desired direction is in east pm 30 degree.\n");
        TARGET_HEADING = kpd_input(1);
        lcd_clear();
    }
    printf("TARGET HEADING: %d.\r\n", TARGET_HEADING);
    lcd_print("Set the Steering servo gain.\n");
    TARGET_STEERING_SERVO_GAIN = kpd_input(1); // Get the TARGET_STEERING_SERVO_GAIN from the keypad
    printf("Steering servo gain: %d.\r\n", TARGET_STEERING_SERVO_GAIN);
    lcd_clear();
    lcd_print("Set the drive motor gain.\n");
    TARGET_DRIVE_MOTOR_GAIN = kpd_input(1); // Get the TARGET_DRIVE_MOTOR_GAIN from the keypad
    printf("Drive motor gain: %d.\r\n", TARGET_DRIVE_MOTOR_GAIN);
    lcd_clear();
    lcd_print("Turn SS on to run the code.");
}

void Flight_Recorder(void)
{
    // This function will output the data to the PuTTY.
    // If the OUTPUT_MODE is 1, it will be output to .csv mode.
    // Else, user-friendly mode.
    if (OUTPUT_MODE != 1)
    {
        // Output to user-friendly UI.
        printf("***BEGIN***\r\n");
        printf("TIME: %d.\r\n", TIME);
        printf("DISTANCE: %d, HEADING: %d.\r\n", DISTANCE, HEADING);
        printf("AD_VALUE: %d.\r\n", AD_VALUE);
        printf("DRIVE_MOTOR_PW: %d, STEERING_SERVO_PW: %d.\r\n", DRIVE_MOTOR_PW, STEERING_SERVO_PW);
        printf("TARGET HEADING: %d.\r\n", TARGET_HEADING);
        printf("****END****\r\n");
    }
    else
    {
        // Output to .csv format
        printf("%d,%d,%d,%d,%d,%d,%d\r\n", TIME, DISTANCE, HEADING, AD_VALUE, DRIVE_MOTOR_PW, STEERING_SERVO_PW, TARGET_HEADING);
    }
}

unsigned int Is_SS_On(void)
{
    // This function will return the status of the slide switch.
    if (!SS)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void Turn_BILED_Green(void)
{
    // This function will turn BILED Green
    BILED0 = 0;
    BILED1 = 1;
}

void Turn_BILED_Red(void)
{
    // This function will turn BILED Red
    BILED0 = 1;
    BILED1 = 0;
}

void Turn_BILED_Off(void)
{
    // This function will turn BILED off
    BILED0 = 1;
    BILED1 = 1;
}

void Port_Init(void)
{
    P1MDIN &= 0xEF;  // Set P1.4 to analog input.
    P1MDOUT &= 0xEF; // Set P1.4 to input mode.
    P1MDOUT |= 0x0D; // Set P1.0, P1.2, P1.3 to output.
    P1 |= ~0xEF;     // Set P1.4 to high impedance.
    P3MDOUT &= 0xBF; // Set P3.6 to input mode.
    P3MDOUT |= 0x03; // Set P3.0, P3.1 to output mode.
    P3 |= ~0xBF;     // Set P3.6 to high impedance.
}

void PCA_Init(void)
{
    PCA0MD = 0x81;    // Enable SYSCLK/12 and enable interrupt.
    PCA0CPM0 = 0xC2;  // Enable CCM0 17bit PWM.
    PCA0CPM2 = 0xC2;  // Enable CCM2 16bit PWM.
    PCA0CN = 0x40;    // Enable PCA counter.
    PCA0 = PCA_START; // For 20ms period.
}

void XBR0_Init(void)
{
    XBR0 = 0x27; // Configure crossbar with UART, SPI, SMBus, and CEX channels.
}

void Interrupt_Init(void)
{
    EA = 1;       // Enable general interrupt.
    EIE1 |= 0x08; // Enable PCA overflow interrupts.
}

void SMB_Init()
{
    SMB0CR = 0x93; // Set the clock frequency to be 100kHz.
    ENSMB = 1;     // Enable SMB.
}

void ADC_Init(void)
{
    REF0CN = 0x03;  // Configure ADC1 to use VREF.
    ADC1CN = 0x80;  // Set a gain of 1.
    ADC1CF |= 0x01; // Enable ADC1.
}

void PCA_ISR(void) __interrupt 9
{
    TIME++;                // Increment the time.
    PCA_COUNTER++;         // Increment the PCA_COUNTER.
    RANGER_READ_COUNTER++; // Increment the RANGER_READ_COUNTER.
    if (!SS)
    {
        Read_Compass();
        //Direction_Controller();
    }
    if (TIME % 10 == 0 && !SS)
    {
        // Output the data to PuTTY every 1s.
        Read_Ranger();
        Flight_Recorder();
    }
    if (CF)
    {
        CF = 0;           // Clear overflow flag.
        PCA0 = PCA_START; // Start count for 20 ms period.
    }
    PCA0CN &= 0x40; // Handle other PCA0 overflows.
}

void Reset_PCA_Counter(void)
{
    // This function reset the PCA_COUNTER.
    PCA_COUNTER = 0;
}

void Wait_For_1s(void)
{
    // WARNING: USING THIS FUNCTION WILL CLEAR PCA_COUNTER.
    // Wait for 1s.
    Reset_PCA_Counter();
    while (PCA_COUNTER < 51)
        ; // Wait for 1s.
    Reset_PCA_Counter();
}

void Troubleshooter(void)
{
    unsigned int Test = 0;
    // This is the test function for the circuit.
    // First test the BILED.
    Turn_BILED_Off();
    Turn_BILED_Green();
    Wait_For_1s();
    Turn_BILED_Red();
    Wait_For_1s();
    Turn_BILED_Off();

    // Then test the drive motor.
    Reset_PCA_Counter();
    DRIVE_MOTOR_PW = DRIVE_MOTOR_MAX;
    while (PCA_COUNTER < 51)
    {
        Drive_Motor();
    }
    Reset_PCA_Counter();
    DRIVE_MOTOR_PW = DRIVE_MOTOR_NEUT;
    while (PCA_COUNTER < 51)
    {
        Drive_Motor();
    }
    Reset_PCA_Counter();
    DRIVE_MOTOR_PW = DRIVE_MOTOR_MIN;
    while (PCA_COUNTER < 51)
    {
        Drive_Motor();
    }
    Reset_PCA_Counter();
    DRIVE_MOTOR_PW = DRIVE_MOTOR_NEUT;
    while (PCA_COUNTER < 51)
    {
        Drive_Motor();
    }
    Reset_PCA_Counter();

    // Then test the steering servo
    STEERING_SERVO_PW = STEERING_SERVO_LEFT;
    while (PCA_COUNTER < 51)
    {
        Steering_Servo();
    }
    Reset_PCA_Counter();
    STEERING_SERVO_PW = STEERING_SERVO_NEUT;
    while (PCA_COUNTER < 51)
    {
        Steering_Servo();
    }
    Reset_PCA_Counter();
    STEERING_SERVO_PW = STEERING_SERVO_RIGHT;
    while (PCA_COUNTER < 51)
    {
        Steering_Servo();
    }
    Reset_PCA_Counter();
    STEERING_SERVO_PW = STEERING_SERVO_NEUT;
    while (PCA_COUNTER < 51)
    {
        Steering_Servo();
    }
    Reset_PCA_Counter();

    // Try to test the ranger and compass.
    DISTANCE = 0;
    HEADING = 0;
    Read_Ranger();
    printf("DISTANCE: %d.\r\n", DISTANCE);
    DISTANCE = 0;
    Read_Compass();
    printf("HEADING: %d.\r\n", HEADING);
    HEADING = 0;

    // Try to read the POT
    Read_AD_Input();
    printf("AD_VALUE: %d.\r\n", AD_VALUE);

    // Test the LCD screen
    lcd_clear();
    lcd_print("LCD TESTING.\n");
    Wait_For_1s();
    lcd_clear();

    // Test the keypad
    lcd_print("Input test code.\n");
    Test = kpd_input(1);
    printf("TEST: %d.\r\n", Test);
    lcd_clear();
    Reset_PCA_Counter();
}