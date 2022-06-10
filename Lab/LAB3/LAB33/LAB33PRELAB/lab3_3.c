#include <c8051_SDCC.h>
#include <stdio.h>
#include <stdlib.h>
#include <i2c.h>
//-----------------------------------------------------------------------------
// Function Prototypes
//-----------------------------------------------------------------------------
void Port_Init(void);
void PCA_Init (void);
void XBR0_Init();
void Interrupt_Init(void);
void SMB_Init(void);
void Compass();
void Ranger();
unsigned int ReadRanger();
unsigned int ReadCompass();
void wait();
void PCA_ISR(void) __interrupt 9;
unsigned int ReadLight();
void Lightfunction(void);
//Global variables
unsigned int count;
unsigned int range;
unsigned char RangeData[2];  //array to store ranger data
unsigned char CompassData[2]; //array to store compass data
unsigned int r_count = 0;     //count overflows for ranger
unsigned int h_count = 0;	  //count overflows for compass
unsigned int new_heading = 0;
unsigned int new_range = 0;
unsigned char RangeAddr = 0xE0;  // the address of the ranger is 0xE0
unsigned char CompAddr = 0xC0; // the address of the sensor, 0xC0 for the compass
unsigned char CompAddr;
unsigned int heading;
unsigned char LightData[2];
unsigned int light;
__sbit __at 0xB6 SS0;
__sbit __at 0xB7 SS1;
__sbit __at 0xB5 SS2;
void main(void)
{
	Sys_Init();
    putchar(' '); //the quotes in this line may not format correctly
    Port_Init();  //initialize PortI/O
    XBR0_Init();  //Initialize crossbar
    PCA_Init();   //initialize PCA
	Interrupt_Init();  //initialize interrupts
	SMB_Init();
	while b   (!SS0)
			Ranger();
	while (!SS1)
			Compass();
	while (!SS2)
			Lightfunction();
}

//-----------------------------------------------------------------------------
// Port_Init
//-----------------------------------------------------------------------------
//
// Set up ports for input and output
//
void Port_Init()
{
    P1MDOUT|= 0x0D;  //set output pin for CEX0 or CEX2 or CEX3 in push-pull mode
	P3MDOUT &= 0x1F;
	P3 |= ~0x1F; /* Send logic 1 to input pin P3.0 for impedance */
}

//-----------------------------------------------------------------------------
// XBR0_Init
//-----------------------------------------------------------------------------
//
// Set up the crossbar
//
void XBR0_Init()
{
    XBR0 = 0x27;  //configure crossbar as directed in the laboratory

}
//-----------------------------------------------------------------------------
// PCA_Init
//-----------------------------------------------------------------------------
//
// Set up Programmable Counter Array
//
void PCA_Init(void)
{
    // reference to the sample code in Example 4.5 -Pulse Width Modulation 
    // implemented using the PCA (Programmable Counter Array), p. 50 in Lab Manual.
	PCA0MD = 0x81; //enable CF interrupt and use SYSCLK/12
	PCA0CPM0 = 0xC2;
	PCA0CPM2 = 0xC2;
	PCA0CPM3 = 0xC2;     //CCM0 and CCM2 in 16-bit compare mode
	PCA0CN = 0x40;         //enable PCA counter
}

void Interrupt_Init(void)
{
	EIE1 |= 0x08;		   //enable PCA interrupt
	EA = 1;               //enable global interrupts
}

void PCA_ISR(void) __interrupt 9
{
	if(CF)
	{
		CF = 0;
		PCA0 = 28671;
		h_count++;
		r_count++;
		if(r_count >= 4)
		{
			new_range = 1; //4 overflows is about 80 ms
			r_count = 0;
		}
		else if(h_count >= 2)
		{
			new_heading = 1;  //2 overflows is about 40ms
			h_count = 0;
		}
	}
	PCA0CN &= 0xC0;                    // handle other PCA interrupt services
	count++;                          // reset overflow counter
}

void SMB_Init(void)
{
	SMB0CR = 0x93; //set SCL to 100KHz
	ENSMB = 1; //bit 6 of SMB0CN, enable the SMBus
}

unsigned int ReadRanger()
{
	RangeData[0] = 0;
	i2c_read_data(RangeAddr,2 ,RangeData, 2);  // read two bytes, starting at reg 2
	range = (((unsigned int)RangeData[0] << 8) | RangeData[1]);
	return range; 
}

unsigned int ReadCompass()
{
	i2c_read_data(CompAddr,2,CompassData,2);  // read two byte, starting at reg 2 
	heading =(((unsigned int)CompassData[0] << 8) | CompassData[1])/10;  //combine the two values, heading has units of 1/10 of a degree 
	return heading;  // the heading returned in degrees between 0 and 3599
}

unsigned int ReadLight()
{
	LightData[0]=0;
	i2c_read_data(RangeAddr,1,LightData,1);  // read one byte, starting at reg 1 
	light =(((unsigned int)LightData[0]));  
	return light;  // the heading returned in degrees between 0 and 3599
}
//function to wait
void wait()
{
	count = 0;
	while(count<6);  //wait 5 counts , about 100 ms
}

void Lightfunction()
{	
	unsigned int PW_CENTER = 18432;
	unsigned int PW_MIN = 1843;
	unsigned int PW_MAX = 35023; 
	unsigned int PW=0;
	
	if(new_range)
	{
		light=ReadLight();
		LightData[0] = 0x51;  // write 0x51 to reg 0 of the ranger:
		i2c_write_data(RangeAddr, 0, LightData, 1); // write one byte of data to reg 0 at addr
		new_range = 0;  //reset the 80ms flag
		printf("\r\nLight = %d \n\r", light);
	}
	//The LED is brightest if an object if the light sensor returns a value less than 40.
	if (light < 40)
		PW=PW_MIN;

	//¨C The LED is dimmest if an object if the light sensor returns a value less than 215.
	if (light > 215)
		PW=PW_MAX;
	//¨C Use your calibrated brightest and dimmest pulsewidth values from laboratory 3.1.
	if	(40<light && light<215)
		PW=(light-128)*65+PW_CENTER;
	//¨C The pulse width varies linearly from brightest (PWLEDmax) to dimmest (PWLEDmin) as the light sensor varies from 40 to 215
	PCA0CP3 = 0xFFFF - PW;
}
void Ranger()
{
	unsigned int PW_CENTER = 2765;
	unsigned int PW_MIN = 2027;
	unsigned int PW_MAX = 3502; 
	unsigned int x;
	unsigned int PW=0;
	if(new_range)
	{
		range = ReadRanger();
		RangeData[0] = 0x51;  // write 0x51 to reg 0 of the ranger:
		i2c_write_data(RangeAddr, 0, RangeData, 1); // write one byte of data to reg 0 at addr
		new_range = 0;  //reset the 80ms flag
	}
	//The motor has full power forward if an object is 10 cm or less above the car.
	if (range<=10)
	{
		PW=PW_MAX;
		printf("\rfull power forward\n");
	}
	//The motor is neutral when the object is 40-50 cm above the car.
	if (40<range<50)
	{
		PW=PW_CENTER;
		printf("\rneutral\n");
	}
	//The motor has full power reverse if the closest object is more than 90 cm from the car.
	if (range>90)
	{
		PW=PW_MIN;
		printf("\rfull power reverse\n");
	}
	//The pulse width varies linearly for distances from 10 cm to 40 cm, between max forward and neutral pulsewidths.
	if (40>range>10)
	{
		x=(range-10)/30;
		PW=(PW_MAX-PW_CENTER)*x+PW_CENTER;
		printf("\rthe motor is %d of max forward power\n",x);
	}
	//The pulse width varies linearly for distances from 50 cm to 90 cm, between neutral and maximum reverse.
	if (90>range>50)
	{
		x=(range-50)/40;
		PW=PW_CENTER-(PW_CENTER-PW_MIN)*x;
		printf("\rthe motor is %d of max reverse power\n",x);
	}
	printf("\rthe object is %d cm away and the car is moving %d pw\n",range,PW);
	PCA0CPL2 = 0xFFFF - PW;
	PCA0CPH2 = (0xFFFF - PW) >> 8;
	
}

void Compass()
{
	//Assume a desired heading, make this a variable so it can be changed later, but fix the value for now. For example:
	unsigned int desired_heading = 900;
	unsigned int error;
	unsigned int PW_CENTER = 2765;
	unsigned int PW_MIN = 1659;
	unsigned int PW_MAX = 3871; 
	unsigned int PW=0;
	unsigned int temp_servo_pw;
	if(new_heading)
	{
		heading = ReadCompass();
		new_heading = 0; //reset the 40 ms flag
	}
	wait(); //wait 5 counts
	error = desired_heading - heading;
	if (error > 1800)
		error -= 3600;
	if (error < -1800)
		error += 3600;
	temp_servo_pw = (error)/4 + PW_CENTER;
	printf("\rHeading = %d degrees, \n", heading);      // print heading value
	printf("%d",temp_servo_pw);
	PCA0CPL0 = 0xFFFF - temp_servo_pw;
	PCA0CPH0 = (0xFFFF - temp_servo_pw) >> 8;
}
