/* Name: Yilu Zhou
   Date: 5/20/2019
   Section & Side: 02

Program Name: Homework 2
*/
// This C code has three intentional syntax errors that must be corrected
//   before submitting this file for grading. Additionally, one error in 
//   the software logic exists, which should also be fixed.
//
// Forgetting to edit the personal information comments at the top will
//   result in losing some points.
//
// Your final code should be error and warning free. Deleting lines of 
//   code does not constitute a valid solution.
//
// NOTE: It is not necessary to run this code on the car, though you are 
// of course welcome to do so. If your submitted code does not compile,
// you will receive a grade of zero. To receive credit, your code must be
// free of both errors and warnings when it compiles.

// Compiler directives
#include <C8051_SDCC.h> // Include files. This file is available online
#include <stdio.h>

// Function prototypes
void Winter(void);
void Summer(void);
void Fall(void);
void Spring(void);

// Global variables
char input;						//store keyboard entry
char number;

void main(void) 	       			// Start main function
{

  	Sys_Init();   					// Initialize UART, System clock and crossbar
  	putchar(' '); 					// Do this because we tell you to 
  	printf("LITEC Homework Assignment 2\r\n\n"); // Print start message


  	while(10)      					// Begin infinite loop 
  	{
		printf("What is your favorite seasoni Troy? \r\n");
  		printf("    Enter an 'S' for Summer\r\n");
  		printf("    Enter an 'W' for Winter\r\n");
  		printf("    Enter an 'F' for Fall\r\n");
    	input = getchar();      	// Get keyboard input
		
		//check the range
		if (input == 'S')			// MANE department chosen
		{
			Summer();
		}
		else if (input == 'F')		// ECSE department chosen
		{
			Fall();
		}
		else if (input == 'W')
		{
			Winter();
		}
		else
		{
			Spring();
		}

		printf("Press any key to pick a different season \r\n");
		getchar();
  	} 	// End while loop
}   	// End main function


// Function definitions

void Summer(void)
{
	printf("Great time for hiking. \r\n");
}

void Winter(void)
{
	printf("You must like skiing. \r\n");

}

void Fall(void)
{
	printf("It must be the leaves changing color \r\n");

}

void Spring(void)
{
	printf("No surprise. \r\n");
	printf("    Everyone likes spring \r\n");
}
