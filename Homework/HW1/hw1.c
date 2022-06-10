/* Yilu Zhou 
Name this program - Homework 1 */

// Include files

#include <c8051_SDCC.h>		//This file is available online
#include <stdio.h>

// Function prototypes
void get_keyboard(void);
void set_count18(void);
void set_count270(void);
void output_count(void);

// Global variables

int imax, i;   				// maximum value of count, for loop variable
unsigned int count; 		// current value of count
unsigned int input;	`	//input from keyboard

void main(void)    // start main function
{
	/* The following 2 lines MUST be the 1st 
	two executable statements in every program for our class */

    Sys_Init();    			// Initialize UART, System clock and crossbar
    putchar(' ');  			// do this because we tell you to
    
	while(1)       			// begin infinite loop
    {
		get_keyboard();
		if (imax > 0)
		{
			output_count();
		}
    }						// end while loop
}							// end main function


// Get character from keyboard, set count range
void get_keyboard(void)
{
	printf("\r\n enter 1 to count to 18 or 2 to count to 270 \r\n");
	input=getchar();       // get count value
	if (input=='1')   
	{
		set_count18();
	}
	else if (input=='2')
	{
		set_count270();
	}
	else imax=0;
}


// If keyboard input is 1, count to 18
void set_count18(void)
{
	count=18;
	imax=18;
}

// If keyboard input is 1, count to 18
void set_count270(void)
{
	count=270;
	imax=270;
}

// Output count in decimal and hexadecimal format
void output_count(void)
{
	printf("Maximum count value is %u \n\n\r ",imax);
	printf("Decimal \t hex \n\r");
	for(i=0;i<=imax;i++)
	{
		count=i;
		printf("  %u \t\t  %x \n\r", count, count); 
	}						
}