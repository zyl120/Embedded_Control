/*
Names: Zeyu Jin, Zhongqi Liu, Yilu Zhou
Section: 02
Date: 5/30/2019
File name: Laboratory_2_Section_2_Jin_Liu_Zhou.c
Description: This program is the code for Lab 2.
*/

#include <c8051_SDCC.h> // include files. This file is available online
#include <stdio.h>
#include <stdlib.h>

// Function Prototypes
void Port_Init(void);
void Timer_Init(void);
void Interrupt_Init(void);
void ADC_Init(void);
void Timer0_ISR(void) __interrupt 1;
void Random(void);
unsigned char Read_AD_Input(void); // Read the AD value.
void POT_Reader_For_Mode2(void);   // POT for mode 2.
void POT_Reader_For_Mode3(void);   // The function to read the potentiometer and set the on-time.
void Mode_Selector(void);          // The function to select the mode of the game based on slide switches.
void Mode_1(void);                 // Set the blink pattern for mode 1
void Sequence_1(void);             // Function to control the sequence 1 of mode 1
void Sequence_2(void);             // Function to control the sequence 2 of mode 1
void Sequence_3(void);             // Function to control the sequence 3 of mode 1
void Mode_2(void);                 // The function to perform the mode 2 blink pattern
void Player1_Mode_2(void);
void Player2_Mode_2(void);
void Mode_3(void); // Set the function of mode 3
void Player1_Clockwise(void);
void Player1_Counter_Clockwise(void);
void Player2_Clockwise(void);
void Player2_Counter_Clockwise(void);
void Troubleshooter(void); // The function to check the wire connection.
void Wait_For_Targeted_Time(void);
// The function to hold the execution based on Targeted_Counter.
// Warning: Use this function will clear Timer0_Counter and stop Timer0!
void Turn_On_Top_LED(void);
void Turn_Off_Top_LED(void);
void Turn_On_Left_LED(void);
void Turn_Off_Left_LED(void);
void Turn_On_Bottom_LED(void);
void Turn_Off_Bottom_LED(void);
void Turn_On_Right_LED(void);
void Turn_Off_Right_LED(void);
void Turn_On_All_LEDs(void);
void Turn_Off_All_LEDs(void);
void Turn_BILED_Green(void);
void Turn_BILED_Red(void);
void Turn_BILED_Off(void);
void Turn_On_Timer0(void);
void Turn_Off_Timer0(void);
void Turn_On_Top_LED_For_Target_Time(void);
void Turn_On_Left_LED_For_Target_Time(void);
void Turn_On_Bottom_LED_For_Target_Time(void);
void Turn_On_Right_LED_For_Target_Time(void);
void Result_Selector(void);
unsigned char Is_PB0_Pushed(void);
unsigned char Is_PB1_Pushed(void);
unsigned char Is_PB2_Pushed(void);
unsigned char Is_PB3_Pushed(void);
unsigned char Is_SS0_On(void);
unsigned char Is_SS1_On(void);
void Reset_Timer0_Counter(void);
void Reset_All_Variables(void);
void Reset_Debounce_Counter(void);

// Global Variables
__sbit __at 0x90 POT;    // Potentiometer, P1.0 #12
__sbit __at 0xA4 SS0;    // Slide switch 0, P2.4 #24
__sbit __at 0xA5 SS1;    // Slide switch 1, P2.5 #25
__sbit __at 0xA0 PB0;    // Pushbutton 0, P2.0 #29
__sbit __at 0xA1 PB1;    // Pushbutton 1, P2.1 #30
__sbit __at 0xA2 PB2;    // Pushbutton 2, P2.2 #27
__sbit __at 0xA3 PB3;    // Pushbutton 3, P2.3 #28
__sbit __at 0xB0 LED0;   // LED 0, P3.0 #38
__sbit __at 0xB1 LED1;   // LED 1, P3.1 #37
__sbit __at 0xB2 LED2;   // LED 2, P3.2 #35
__sbit __at 0xB3 LED3;   // LED 3, P3.3 #36
__sbit __at 0xB4 BILED0; // BILED 0, P3.4 #33
__sbit __at 0xB5 BILED1; // BILED 1, P3.5 #34

unsigned int Timer0_Counter;   // Timer 0 counter
unsigned int Target_Counter;   // The number of Timer 0 counter read from analog
unsigned int Debounce_Counter; // The counter to count the debounce process.
unsigned char Potent_AD;       // A/D result
unsigned char Current_Random;  // Current random number
unsigned char Previous_Random; // Previous random number
unsigned char Current_Mode;    // Current game mode.
int Player1_Score;             // Record the score of player 1
int Player2_Score;             // Record the score of player 2
//unsigned int Button_status_Counter;

void main(void)
{
    Sys_Init();
    Port_Init();
    Interrupt_Init();
    Timer_Init();
    ADC_Init();
    putchar(' ');
    Reset_All_Variables();
    //Troubleshooter();
    while (1)
    {
        Reset_All_Variables(); // Reset all the variables for a new game.
        Mode_Selector();
        if (Current_Mode == 1)
        {
            printf("*****Enter mode 1.\r\n");
            Reset_All_Variables();
            Mode_1();
            printf("*****Exit mode 1.\r\n");
        }
        else if (Current_Mode == 2)
        {
            printf("*****Enter mode 2.\r\n");
            Reset_All_Variables();
            Mode_2();
            printf("*****Exit mode 2.\r\n");
        }
        else if (Current_Mode == 3)
        {
            printf("*****Enter mode 3.\r\n");
            Reset_All_Variables();
            Mode_3();
            printf("*****Exit mode 3.\r\n");
        }
        else
        {
            Reset_All_Variables();
            continue;
        }
        // Some code here
        Target_Counter = 1690; // Wait for 5s.
        Wait_For_Targeted_Time();
    }
}

void Port_Init(void)
{
    P1MDIN &= 0xFE;   //Set P1.0 as analog input
    P1MDOUT &= 0xFE;  // Set P1.0 as input
    P1 |= ~0xFE;      // Set P1.0 to high impedence
    P2MDOUT &= ~0x3F; // Set P2.0 - P2.5 to input
    P2 |= 0x3F;       // Turn on high impedence on P2.0 - P2.5
    P3MDOUT |= 0x3F;  // Turn on output on P3.0 - P3.5
}

void Interrupt_Init(void)
{
    IE |= 0x02; // Turn on Timer0 interrupt
    EA = 1;     // Turn on global interrupt
}

void Timer_Init(void)
{
    CKCON |= 0x08; // Timer0 uses SYSCLK as source. 0000 1000
    TMOD &= 0xF0;  // clear the 4 least significant bits. 1111 0000
    TMOD |= 0x01;  // Timer0 in mode 1 0000 0001
    TR0 = 0;       // Stop Timer0
    TMR0 = 0;      // Clear high & low byte of T0
}

void ADC_Init(void)
{
    REF0CN = 0x03;  // Congure ADC1 to use VREF
    ADC1CN = 0x80;  // Set a gain of 1
    ADC1CF |= 0x01; // Enable ADC1
}

void Timer0_ISR(void) __interrupt 1
{
    Timer0_Counter++;   // Increment Timer0 counter.
    Debounce_Counter++; // Increment Debounce counter.
}

void Random(void)
{
    while (1)
    {
        Current_Random = rand() % 4; // Generate a random number first.
        if (Current_Random != Previous_Random)
        {
            // When the new random number is not the same as the previous one,
            // break the while loop.
            Previous_Random = Current_Random;
            // Set the PreviousRandom the same as the CurrentRandom.
            break;
        }
    }
    return;
}

unsigned char Read_AD_input(void)
{
    // This function will read the voltage input and output the AD_value.
    AMX1SL = 0;              // Set P1.0 as the analog input for ADC1
    ADC1CN = ADC1CN & ~0x20; // Clear the "Conversion Completed" flag
    ADC1CN = ADC1CN | 0x10;  // Initiate A/D conversion
    while ((ADC1CN & 0x20) == 0x00)
        ;        // Wait for conversion to complete
    return ADC1; // Return digital value in ADC1 register
}

void POT_Reader_For_Mode2(void)
{
    // This function reads the AD_Value from analog input and calculate the Target_Counter.
    unsigned char AD_Value = read_AD_input();
    Target_Counter = 338 * (1 + (4 * AD_Value / 255));
    printf("*****Target Counter: %d.\r\n", Target_Counter);
}

void POT_Reader_For_Mode3(void)
{
    // This function reads the AD_Value from analog input and calculate the Target_Counter.
    unsigned char AD_Value = read_AD_input();
    Target_Counter = 85 + (253.5 * AD_Value / 255);
    printf("*****Target Counter: %d.\r\n", Target_Counter);
}

void Mode_Selector(void)
{
    Turn_Off_All_LEDs();
    Turn_BILED_Off();
    // The function to select the mode based on the slide switch status.
    if (!Is_SS0_On() && !Is_SS1_On())
    {
        Current_Mode = 0;
        return;
    }
    if (Is_SS0_On() && !Is_SS1_On())
    {
        Current_Mode = 1;
        return;
    }
    if (!Is_SS0_On() && Is_SS1_On())
    {
        Current_Mode = 2;
        return;
    }
    if (Is_SS0_On() && Is_SS1_On())
    {
        Current_Mode = 3;
        return;
    }
}

void Mode_1(void)
{
    POT_Reader_For_Mode3();
    Turn_Off_All_LEDs();
    printf("The Target_Counter is %d\r\n", Target_Counter);
    Sequence_1();
    Sequence_2();
    Sequence_3();
}

void Sequence_1(void)
{
    Turn_On_Top_LED_For_Target_Time();
    Turn_On_Right_LED_For_Target_Time();
    Turn_On_Bottom_LED_For_Target_Time();
    Turn_On_Left_LED_For_Target_Time();
    Turn_On_Top_LED_For_Target_Time();
    Turn_On_Left_LED_For_Target_Time();
    Turn_On_Bottom_LED_For_Target_Time();
    Turn_On_Right_LED_For_Target_Time();
}

void Sequence_2(void)
{
    Turn_On_Top_LED_For_Target_Time();
    Turn_On_Bottom_LED_For_Target_Time();
    Turn_On_Left_LED_For_Target_Time();
    Turn_On_Right_LED_For_Target_Time();
    Turn_On_Left_LED_For_Target_Time();
    Turn_On_Bottom_LED_For_Target_Time();
    Turn_On_Top_LED_For_Target_Time();
}

void Sequence_3(void)
{
    Turn_On_Left_LED_For_Target_Time();
    Turn_On_Right_LED_For_Target_Time();
    Turn_On_Left_LED_For_Target_Time();
    Turn_On_Right_LED_For_Target_Time();
}

void Mode_2(void)
{
    unsigned int Turns = 0;
    POT_Reader_For_Mode2();
    Turn_Off_Timer0();
    Reset_Timer0_Counter();
    for (; Turns < 3; Turns++)
    {
        Player1_Mode_2();
        Reset_Timer0_Counter();
        Reset_Debounce_Counter();
        if (Player1_Score == -1)
        {
            Turn_Off_All_LEDs();
            Turn_BILED_Off();
            Turn_Off_Timer0();
            Reset_Debounce_Counter();
            Reset_Timer0_Counter();
            return;
        }
        Player2_Mode_2();
        if (Player2_Score == -1)
        {
            Turn_Off_All_LEDs();
            Turn_BILED_Off();
            Turn_Off_Timer0();
            Reset_Debounce_Counter();
            Reset_Timer0_Counter();
            return;
        }
        Turn_Off_All_LEDs();
        Turn_BILED_Off();
        Turn_Off_Timer0();
        Reset_Debounce_Counter();
        Reset_Timer0_Counter();
    }
    Result_Selector();
}

void Player1_Mode_2(void)
{
    Turn_On_Left_LED();
    Turn_On_Timer0();

    while (Timer0_Counter < Target_Counter)
    {
        if (Is_PB1_Pushed())
        {
            Reset_Debounce_Counter();
            while (Debounce_Counter < 7)
                ; // Wait for debounce time (20ms).
            Reset_Debounce_Counter();
            while (Is_PB1_Pushed())
                ; // Wait fro the release
            ;
            Player1_Score++;
            Reset_Debounce_Counter();
            while (Debounce_Counter < 7)
                ; // Wait for debounce time (20ms).
        }
        while (Is_PB3_Pushed())
        {
            printf("*****Player 2 wins.\r\n");
            Player1_Score = -1;
            Turn_BILED_Red();
            Reset_Timer0_Counter();
            Target_Counter = 338;
            Wait_For_Targeted_Time();
            return;
        }
    }
    Turn_Off_Left_LED();
    Turn_Off_Timer0();
    Reset_Timer0_Counter();
}
void Player2_Mode_2(void)
{
    Turn_On_Right_LED();
    Turn_On_Timer0();

    while (Timer0_Counter < Target_Counter)
    {
        if (Is_PB3_Pushed())
        {
            Reset_Debounce_Counter();
            while (Debounce_Counter < 7)
                ; // Wait for debounce time (20ms).
            Reset_Debounce_Counter();
            while (Is_PB3_Pushed())
                ; // Wait fro the release
            ;
            Player2_Score++;
            Reset_Debounce_Counter();
            while (Debounce_Counter < 7)
                ; // Wait for debounce time (20ms).
        }
        while (Is_PB1_Pushed())
        {
            printf("*****Player 1 wins.\r\n");
            Player2_Score = -1;
            Turn_BILED_Green();
            Reset_Timer0_Counter();
            Target_Counter = 338;
            Wait_For_Targeted_Time();
            return;
        }
    }
    Turn_Off_Right_LED();
    Turn_Off_Timer0();
    Reset_Timer0_Counter();
}
void Mode_3(void)
{
    unsigned char Turns = 0;
    Turn_Off_All_LEDs();
    POT_Reader_For_Mode3();
    for (; Turns < 5; Turns++)
    {
        printf("*****Begin Player1 Clockwise.\r\n");
        Random();
        printf("Random: %d.\r\n", Current_Random);
        Player1_Clockwise();
        printf("*****Begin Player1 Counter-Clockwise.\r\n");
        Random();
        printf("Random: %d.\r\n", Current_Random);
        Player1_Counter_Clockwise();
        printf("*****Begin Player2 Clockwise.\r\n");
        Random();
        printf("Random: %d.\r\n", Current_Random);
        Player2_Clockwise();
        printf("*****Begin Player2 Counter-Clockwise.\r\n");
        Random();
        printf("Random: %d.\r\n", Current_Random);
        Player2_Counter_Clockwise();
    }
    Result_Selector();
}

void Player1_Clockwise(void)
{
    unsigned int Temp_Player1_Score = 0;

    Turn_On_Timer0();
    Reset_Timer0_Counter();
    if (Current_Random == 0) // Top
    {
        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();
        return;
    }
    if (Current_Random == 1) //from left
    {
        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();
        return;
    }
    if (Current_Random == 2) //from bottom
    {
        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();
        return;
    }
    if (Current_Random == 3) //from right
    {

        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();
        return;
    }
}

void Player1_Counter_Clockwise(void)
{
    unsigned int Temp_Player1_Score = 0;
    //Random();
    //printf("Random: %d.\r\n", Current_Random);
    Turn_On_Timer0();
    Reset_Timer0_Counter();
    if (Current_Random == 0) // From top
    {
        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();
        return;
    }
    if (Current_Random == 1) //From Left
    {

        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        return;
    }

    if (Current_Random == 2) //from bottom
    {

        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();

        return;
    }

    if (Current_Random == 3) //from right
    {

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();

        Temp_Player1_Score = 0;

        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player1_Score == 0)
            {
                Temp_Player1_Score++;
                Player1_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();
        return;
    }
}

void Player2_Clockwise(void)
{
    unsigned int Temp_Player2_Score = 0;
    //Random();
    //printf("Random: %d.\r\n", Current_Random);
    Turn_On_Timer0();
    Reset_Timer0_Counter();
    if (Current_Random == 0) // Top
    {
        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();
        return;
    }
    if (Current_Random == 1) //from left
    {
        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();
        return;
    }
    if (Current_Random == 2) //from bottom
    {
        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();
        return;
    }
    if (Current_Random == 3) //from right
    {

        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();
        return;
    }
}

void Player2_Counter_Clockwise(void)
{
    unsigned int Temp_Player2_Score = 0;
    //Random();
    //printf("Random: %d.\r\n", Current_Random);
    Turn_On_Timer0();
    Reset_Timer0_Counter();
    if (Current_Random == 0) // From top
    {
        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();
        return;
    }
    if (Current_Random == 1) //From Left
    {

        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        return;
    }

    if (Current_Random == 2) //from bottom
    {

        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();

        return;
    }

    if (Current_Random == 3) //from right
    {

        Turn_On_Right_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB1_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Right_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Top_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB0_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Top_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Left_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB3_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Left_LED();
        Reset_Timer0_Counter();

        Temp_Player2_Score = 0;

        Turn_On_Bottom_LED();
        while (Timer0_Counter < Target_Counter)
        {
            if (Is_PB2_Pushed() && Temp_Player2_Score == 0)
            {
                Temp_Player2_Score++;
                Player2_Score++;
            }
        }
        Turn_Off_Bottom_LED();
        Reset_Timer0_Counter();
        return;
    }
}

void Result_Selector(void)
{
    printf("*****Player1:Player2     %d:%d.\r\n", Player1_Score, Player2_Score);
    if (Player1_Score > Player2_Score)
    {
        Target_Counter = 338;
        Turn_BILED_Green();
        printf("*****Player 1 wins.\r\n");
        Wait_For_Targeted_Time();
        Turn_BILED_Off();
    }
    else if (Player1_Score < Player2_Score)
    {
        Target_Counter = 338;
        Turn_BILED_Red();
        printf("*****Player 2 wins.\r\n");
        Wait_For_Targeted_Time();
        Turn_BILED_Off();
    }
    else
    {
        printf("*****Player 1 and 2 ties.\r\n");
    }
    Reset_All_Variables();
}
void Troubleshooter(void)
{
    Reset_All_Variables();
    Target_Counter = 338;
    printf("Entering troubleshooter.\r\n");
    Turn_On_Timer0();
    Turn_On_All_LEDs();
    printf("All LEDs should be turned on.\r\n");
    Wait_For_Targeted_Time();
    Turn_Off_All_LEDs();
    Turn_Off_Timer0();
    Reset_Timer0_Counter();
    printf("If not all LEDs are turned on, Enter 1: \r\n");

    Turn_On_Bottom_LED_For_Target_Time();
    Turn_On_Left_LED_For_Target_Time();
    Turn_On_Right_LED_For_Target_Time();
    Turn_On_Top_LED_For_Target_Time();

    Turn_BILED_Green();
    printf("BILED should be Green.\r\n");
    Turn_On_Timer0();
    Wait_For_Targeted_Time();
    Reset_Timer0_Counter();
    Turn_BILED_Red();
    printf("BILED should be Red.\r\n");
    Wait_For_Targeted_Time();
    Turn_BILED_Off();
    Reset_Timer0_Counter();
    /*
    printf("Checking Pushbutton, press pushbuttons in 10s to check the status change.\r\n");
    Turn_On_Timer0();
    while (Timer0_Counter < 3380)
    {
        if (Timer0_Counter % 338 == 0)
        {
            printf("Status:\r\n");
            printf("PB0: %d\r\n", Is_PB0_Pushed());
            printf("PB1: %d\r\n", Is_PB1_Pushed());
            printf("PB2: %d\r\n", Is_PB2_Pushed());
            printf("PB3: %d\r\n", Is_PB3_Pushed());
        }
    }
    Turn_Off_Timer0();
    Reset_Timer0_Counter();*/
    printf("Checking slide switch, change the SS in 10s to check the stauts change.\r\n");
    Turn_On_Timer0();
    while (Timer0_Counter < 3380)
    {
        if (Timer0_Counter % 338 == 0)
        {
            printf("Status:\r\n");
            printf("SS0: %d\r\n", Is_SS0_On());
            printf("SS1: %d\r\n", Is_SS1_On());
        }
    }
    Turn_Off_Timer0();
    Reset_All_Variables();
    printf("Exiting Troubleshooter.\r\n");
}

void Wait_For_Targeted_Time(void)
{
    // The function to pause the execution for demand time.
    printf("Target Counter: %d\r\n", Target_Counter);
    Reset_Timer0_Counter();
    Turn_On_Timer0();
    while (Timer0_Counter < Target_Counter)
        ;
    Turn_Off_Timer0();
    Reset_Timer0_Counter();
}

void Turn_On_Top_LED(void)
{
    printf("Top LED is ON.\r\n");
    // This function will turn on LED0
    LED0 = 0;
}

void Turn_Off_Top_LED(void)
{
    printf("Top LED is OFF.\r\n");
    // This function will turn off LED0
    LED0 = 1;
}

void Turn_On_Left_LED(void)
{
    printf("Left LED is ON.\r\n");
    // This function will turn on LED1
    LED1 = 0;
}

void Turn_Off_Left_LED(void)
{
    printf("Left LED is OFF.\r\n");
    // This function will turn off LED1
    LED1 = 1;
}

void Turn_On_Bottom_LED(void)
{
    printf("Bottom LED is ON.\r\n");
    // This function will turn on LED2
    LED2 = 0;
}

void Turn_Off_Bottom_LED(void)
{
    printf("Bottom LED is OFF.\r\n");
    // This function will turn off LED2
    LED2 = 1;
}

void Turn_On_Right_LED(void)
{
    printf("Right LED is ON.\r\n");
    // This function will turn on LED3
    LED3 = 0;
}

void Turn_Off_Right_LED(void)
{
    printf("Right LED is OFF.\r\n");
    // This function will turn off LED3
    LED3 = 1;
}

void Turn_On_All_LEDs(void)
{
    // This function is used to turn on all LEDs.
    Turn_On_Top_LED();
    Turn_On_Left_LED();
    Turn_On_Bottom_LED();
    Turn_On_Right_LED();
}

void Turn_Off_All_LEDs(void)
{
    // This function is used to turn off all LEDs.
    Turn_Off_Top_LED();
    Turn_Off_Left_LED();
    Turn_Off_Bottom_LED();
    Turn_Off_Right_LED();
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

void Turn_On_Timer0(void)
{
    TR0 = 1;
}

void Turn_Off_Timer0(void)
{
    TR0 = 0;
}

void Turn_On_Top_LED_For_Target_Time(void)
{
    Turn_On_Top_LED();
    Wait_For_Targeted_Time();
    Turn_Off_Top_LED();
}

void Turn_On_Left_LED_For_Target_Time(void)
{
    Turn_On_Left_LED();
    Wait_For_Targeted_Time();
    Turn_Off_Left_LED();
}

void Turn_On_Bottom_LED_For_Target_Time(void)
{
    Turn_On_Bottom_LED();
    Wait_For_Targeted_Time();
    Turn_Off_Bottom_LED();
}

void Turn_On_Right_LED_For_Target_Time(void)
{
    Turn_On_Right_LED();
    Wait_For_Targeted_Time();
    Turn_Off_Right_LED();
}

unsigned char Is_PB0_Pushed(void)
{
    // Return the status of PB0
    if (!PB0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

unsigned char Is_PB1_Pushed(void)
{
    if (!PB1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

unsigned char Is_PB2_Pushed(void)
{
    if (!PB2)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

unsigned char Is_PB3_Pushed(void)
{
    if (!PB3)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

unsigned char Is_SS0_On(void)
{
    if (!SS0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

unsigned char Is_SS1_On(void)
{
    if (!SS1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void Reset_Timer0_Counter(void)
{
    Timer0_Counter = 0;
}

void Reset_Debounce_Counter(void)
{
    Debounce_Counter = 0;
}

void Reset_All_Variables(void)
{
    printf("All variables Reset.\r\n");
    Timer0_Counter = 0;
    Target_Counter = 0;
    Potent_AD = 0;
    Current_Random = 0;
    Previous_Random = 0;
    Current_Mode = 0;
    Debounce_Counter = 0;
    Player1_Score = 0;
    Player2_Score = 0;
}