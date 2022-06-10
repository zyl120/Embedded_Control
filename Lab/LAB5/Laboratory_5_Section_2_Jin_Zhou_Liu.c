#include <stdio.h>
#include <stdlib.h>
#include <c8051_SDCC.h>
#include <i2c.h>

// Define constant
#define PCA_START 28614          // The start value for the 20ms period.
#define DRIVE_MOTOR_NEUT 2765    // The PW for the drive motor to stop.
#define STEERING_SERVO_NEUT 2845 // The PW for the steering servo to be neutral.

// Global variables
const unsigned char ACCEL_ADDRESS = 0x3A;      // The address of the accel.
const unsigned char POT_ADDRESS = 4;           // The address of POT.
unsigned char ACCEL_DATA[4];                   // The variable to store the raw data from ranger.
int DRIVE_MOTOR_PW = 0;                        // The current PW for the drive motor.
int STEERING_SERVO_PW = 0;                     // The current PW for the steering servo.
unsigned int PCA_COUNTER = 0;                  // The variable to count the PCA overflows.
int DRIVE_MOTOR_MAX = 3505;                    // The PW for the drive motor to do full forward.
int DRIVE_MOTOR_MIN = 2028;                    // The PW for the drive motor to do full reverse.
int STEERING_SERVO_LEFT = 2305;                // The PW for the steering servo to be left.
int STEERING_SERVO_RIGHT = 3340;               // The PW for the steering servo to be right.
unsigned char OUTPUT_MODE = 1;                 // The format of the output info.
unsigned int TARGET_DRIVE_MOTOR_GAIN = 740;    // The target gain of the drive motor.
unsigned int TARGET_STEERING_SERVO_GAIN = 500; // The target gain of the steering servo.
unsigned int TIME = 0;                         // The counter to count the time.
int AVG_GX = 0;                                // The average acceleration on x-axis.
int AVG_GY = 0;                                // The average acceleration on y-axis.
int GX = 0;                                    // The value of acceleration on x-axis.
int OLD_GX = 0;                                // The old GX value.
int OLD_GY = 0;                                // The old GY value.
int GY = 0;                                    // The value of acceleration on y-axis.
int KS = 0;                                    // The steering feedback gain.
int KDX = 0;                                   // The x-axis drive feedback gain.
int KDY = 0;                                   // The y-axis drive feedback gain.
unsigned char SPEED_FLAG = 0;                  // The indicator to control the speed.

// Component address
__sbit __at 0x94 POT;    // The address of the potentiometer.
__sbit __at 0xB6 SS0;    // The address of the slide switch 0.
__sbit __at 0xB7 SS1;    // The address of the slide switch 1.
__sbit __at 0xB0 BILED0; // The address of BILED0.
__sbit __at 0xB1 BILED1; // The address of BILED1.

// C8051 initialization funtion.
void Port_Init(void);
void PCA_Init(void);
void XBR0_Init(void);
void Interrupt_Init(void);
void SMB_Init(void);
void PCA_ISR(void) __interrupt 9;

// Ultrasonic Ranger and Drive Motor functions
void Drive_Motor(void);
void Speed_Controller(void);
void Drive_Motor_Init(void);

// Acceleration detector and Steering Servo functions.
void Steering_Servo(void);
void Direction_Controller(void);
void Steering_Servo_Init(void);

// Read the data from the accel.
void Read_Accel(void);

// Keypad functions
void Read_Keypad(void);

// UI and data record functions
void Flight_Recorder(void);

// Other functions
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
    Accel_Init_C(); // Initial accelerometer.
    Turn_BILED_Red();
    printf("Embedded Control Lab 5\r\n");
    Drive_Motor_Init();
    Steering_Servo_Init();
    //Troubleshooter(); // Should be deleted after the circuit testing is completed.
    while (1)
    {
        if (!SS0)
        {
            lcd_clear();
            lcd_print("SS is on. Execute the code.");
            // When the slide switch is On.
            Reset_PCA_Counter(); // Reset the PCA_COUNTER to 0.
            Turn_BILED_Green();
            //Read_Accel();
            //record data
            while (!SS0)
            {
                if (SPEED_FLAG != 0)
                {
                    Speed_Controller();
                    Drive_Motor();
                    Direction_Controller();
                    Steering_Servo();
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
            DRIVE_MOTOR_PW = DRIVE_MOTOR_NEUT;
            Drive_Motor();
            STEERING_SERVO_PW = STEERING_SERVO_NEUT;
            Steering_Servo();
            Reset_PCA_Counter();
            while (SS0)
                ;
            lcd_clear();
            lcd_print("SS is on. Execute the code.");
            // Set drive motor and steering servo to neutral position.
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
    int TEMP = 0;
    // This function will assign the correct DRIVE_MOTOR_PW according to the DISTANCE.
    if (!SS1)
    {
        TEMP = (KDY * GY) - (KDX * abs(GX));
        //printf("down.");
        // Go down the ramp
        DRIVE_MOTOR_PW = DRIVE_MOTOR_NEUT - TEMP;
    }
    else
    {
        TEMP = (KDY * GY) - (KDX * abs(GX));
        //printf("up.");
        DRIVE_MOTOR_PW = DRIVE_MOTOR_NEUT + TEMP;
        if (DRIVE_MOTOR_PW > DRIVE_MOTOR_NEUT)
        {
            DRIVE_MOTOR_PW = DRIVE_MOTOR_NEUT;
        }
        //Go up the ramp.
    }
    Turn_BILED_Green();
    if (DRIVE_MOTOR_PW > DRIVE_MOTOR_MAX)
    {
        DRIVE_MOTOR_PW = DRIVE_MOTOR_MAX;
    }
    else if (DRIVE_MOTOR_PW < DRIVE_MOTOR_MIN)
    {
        DRIVE_MOTOR_PW = DRIVE_MOTOR_MIN;
    }
    if (DRIVE_MOTOR_PW > DRIVE_MOTOR_NEUT - 30 && DRIVE_MOTOR_PW < DRIVE_MOTOR_NEUT + 30)
    {
        DRIVE_MOTOR_PW = DRIVE_MOTOR_NEUT;
        Turn_BILED_Red();
    }
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
    if (!SS1)
    {
        // Go down the ramp.
        STEERING_SERVO_PW = STEERING_SERVO_NEUT + KS * GX;
    }
    else
    {
        // Go up the ramp
        STEERING_SERVO_PW = STEERING_SERVO_NEUT - KS * GX;
    }
    if (STEERING_SERVO_PW > STEERING_SERVO_RIGHT)
    {
        STEERING_SERVO_PW = STEERING_SERVO_RIGHT; //limit SERVO_PW to PW_RIGHT
    }
    if (STEERING_SERVO_PW < STEERING_SERVO_LEFT)
    {
        STEERING_SERVO_PW = STEERING_SERVO_LEFT; //limit SERVO_PW to PW_LEFT
    }
}

void Read_Keypad(void)
{
    lcd_clear();
    lcd_print("Enter required data.");
    lcd_clear();
    // This function will read the input from the keypad.
    lcd_print("Desired KS:\n");
    KS = kpd_input(1); // Get the steering feedback from the keypad.
    lcd_clear();
    printf("KS %d.\r\n", KS);
    lcd_print("Set x-axis drive feedback gain:\n");
    KDX = kpd_input(1); // Get the TARGET_STEERING_SERVO_GAIN from the keypad
    printf("KDX: %d.\r\n", KDX);
    lcd_clear();
    lcd_print("Set y-axis drive feedback gain:\n");
    KDY = kpd_input(1); // Get the TARGET_DRIVE_MOTOR_GAIN from the keypad
    printf("KDY: %d.\r\n", KDY);
    lcd_clear();
    lcd_print("Turn SS1 on to go up, turn SS1 off to go down.\n");
    while (SS0)
        ; // Wait for the SS0 to be turned on.
    lcd_clear();
    lcd_print("Running code.");
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
        printf("DRIVE_MOTOR_PW: %d, STEERING_SERVO_PW: %d.\r\n", DRIVE_MOTOR_PW, STEERING_SERVO_PW);
        printf("GX: %d, GY: %d.\r\n", GX, GY);
        printf("****END****\r\n");
    }
    else
    {
        // Output to .csv format
        printf("%d,%d,%d,%d,%d\r\n", TIME, DRIVE_MOTOR_PW, STEERING_SERVO_PW, GX, GY);
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
    P3MDOUT &= 0x3F; // Set P3.6 to input mode.
    P3MDOUT |= 0x03; // Set P3.0, P3.1 to output mode.
    P3 |= ~0x3F;     // Set P3.6 to high impedance.
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

void PCA_ISR(void) __interrupt 9
{
    TIME++;        // Increment the time.
    PCA_COUNTER++; // Increment the PCA_COUNTER.
    if (TIME % 1 == 0 && !SS0)
    {
        Flight_Recorder();
    }
    if (TIME % 10 == 0 && !SS0)
    {
        Read_Accel();
    }
    if (TIME % 10 == 0 && !SS0)
    {
        lcd_clear();
        lcd_print("GX: %d, GY: %d.\n", GX, GY);
        lcd_print("DDPW: %d, SSPW: %d.\n", DRIVE_MOTOR_PW, STEERING_SERVO_PW);
        if (!SS1)
        {
            lcd_print("Go Down.\n");
        }
        else
        {
            lcd_print("Go Up.\n");
        }
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

void Read_Accel(void)
{
    unsigned char i;
    SPEED_FLAG = 0;
    AVG_GX = 0;
    AVG_GY = 0; // Reset the average value of the average GX and average GY.
    for (i = 0; i < 4; i++)
        i2c_read_data(ACCEL_ADDRESS, 0x27, ACCEL_DATA, 1);
    {
        while ((ACCEL_DATA[0] &= 0x03) != 0x03)
        {
            i2c_read_data(ACCEL_ADDRESS, 0x27, ACCEL_DATA, 1); // Wait for the 2Lsbits to be high.
        }
        i2c_read_data(ACCEL_ADDRESS, 0x28 | 0x80, ACCEL_DATA, 4);
        AVG_GX += ((ACCEL_DATA[1] << 8) >> 4);
        ; //combine the two values
        AVG_GY += ((ACCEL_DATA[3] << 8) >> 4);
        ; //combine the two values
    }
    GX = AVG_GX / 4; // Set the global GX to the average of 4 measurement.
    GY = AVG_GY / 4; // Set the global GY to the average of 4 measurement.
    if (GX >= -16 && GX <= 16)
        GX = 0;
    if (GY >= -16 && GY <= 16)
        GY = 0;
    if (GX != OLD_GX)
    {
        OLD_GX = GX;
        SPEED_FLAG = 1;
    }
    if (GY != OLD_GY)
    {
        OLD_GY = GY;
        SPEED_FLAG = 1;
    }
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

    printf("SS0: %d, SS1: %d.\r\n", SS0, SS1);
    Wait_For_1s();
    printf("SS0: %d, SS1: %d.\r\n", SS0, SS1);
    Wait_For_1s();
    printf("SS0: %d, SS1: %d.\r\n", SS0, SS1);
}