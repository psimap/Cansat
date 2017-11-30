#pragma once
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <time.h>
//Dabiri
#include <iostream>
#include <stdio.h>
//smnanphoton
#include "i2c.h"

 
//############## BMP085 ##############
const unsigned char OSS = 0;  // Oversampling Setting
// Calibration values
extern int ac1 ,ac2, ac3, b1, b2, mb, mc, md, b5;
extern unsigned int ac4, ac5, ac6;

// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void setup_BMP085(int bus)
{
	char *data;
	data = new char[2];
	
	registerRead(bus, 0x77, 0xAA, 2, data);
	ac1 = (int16_t)(data[0] << 8 | data[1]);
	
	registerRead(bus, 0x77, 0xAC, 2, data);
	ac2 = (int16_t)(data[0] << 8 | data[1]);
	
	registerRead(bus, 0x77, 0xAE, 2, data);
	ac3 = (int16_t)(data[0] << 8 | data[1]);
	
	registerRead(bus, 0x77, 0xB0, 2, data);
	ac4 = (int16_t)(data[0] << 8 | data[1]);
	
	registerRead(bus, 0x77, 0xB2, 2, data);
	ac5 = (int16_t)(data[0] << 8 | data[1]);
	
	registerRead(bus, 0x77, 0xB4, 2, data);
	ac6 = (int16_t)(data[0] << 8 | data[1]);
	
	registerRead(bus, 0x77, 0xB6, 2, data);
	b1 = (int16_t)(data[0] << 8 | data[1]);
	
	registerRead(bus, 0x77, 0xB8, 2, data);
	b2 = (int16_t)(data[0] << 8 | data[1]);
	
	registerRead(bus, 0x77, 0xBA, 2, data);
	mb = (int16_t)(data[0] << 8 | data[1]);
	
	registerRead(bus, 0x77, 0xBC, 2, data);
	mc = (int16_t)(data[0] << 8 | data[1]);
	
	registerRead(bus, 0x77, 0xBE, 2, data);
	md = (int16_t)(data[0] << 8 | data[1]);
}

// Read the uncompensated temperature value
// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C
// Read the uncompensated pressure value
// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
// Temperature -> mode -- 't' / Pressure -> mode == 'p'
float bmp085Read(int bus, char mode)
{
	if ( mode == 't')
	{
		unsigned int ut;
		char *data;
		data = new char[2];
		
		registerWrite(bus, 0x77, 0xF4, 0x2E);
	  
		// Wait at least 4.5ms
		usleep(5000);
	  
		// Read two bytes from registers 0xF6 and 0xF7
		registerRead(bus, 0x77, 0xF6, 2, data);
		  
		ut = (int16_t)(data[0] << 8 | data[1]);
		long x1, x2;
		x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
		x2 = ((long)mc << 11)/(x1 + md);
		b5 = x1 + x2;
		
		return (float)((b5 + 8)>>4)/10;
	}
	
	if ( mode == 'p')
	{
		unsigned long up, b4, b7;
		long x1, x2, x3, b3, b6, p;
	    char *data;
		data = new char[3];
	    
		// Write 0x34+(OSS<<6) into register 0xF4
		// Request a pressure reading w/ oversampling setting
		registerWrite(bus, 0x77, 0xF4, 0x34+(OSS<<6));
	  
		// Wait for conversion, delay time dependent on OSS
		usleep((2 + (3<<OSS))*1000);
		
		registerRead(bus, 0x77, 0xF6, 3, data);
	
		up = (((int16_t) data[0] << 16) | ((int16_t) data[1] << 8) | (int16_t) data[2]) >> (8-OSS);
		
		b6 = b5 - 4000;
		
		// Calculate B3
		x1 = (b2 * (b6 * b6)>>12)>>11;
		x2 = (ac2 * b6)>>11;
		x3 = x1 + x2;
		b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;
	  
		// Calculate B4
		x1 = (ac3 * b6)>>13;
		x2 = (b1 * ((b6 * b6)>>12))>>16;
		x3 = ((x1 + x2) + 2)>>2;
		b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;
	  
		b7 = ((unsigned long)(up - b3) * (50000>>OSS));
		if (b7 < 0x80000000)
			p = (b7<<1)/b4;
		else
			p = (b7/b4)<<1;
	    
		x1 = (p>>8) * (p>>8);
		x1 = (x1 * 3038)>>16;
		x2 = (-7357 * p)>>16;
		p += (x1 + x2 + 3791)>>4;
	  
		return (float)p;
	}
	
}


//############## HMC5883 ##############

// Setup sensor and set registers
void setup_HMC5883(int bus)
{
	//Register 0x00
	/*
	CRA7  CRA6    CRA5    CRA4     CRA3     CRA2     CRA1     CRA0 
	(0)   MA1(0)  MA0(0)  DO2_(1)  DO1_(0)  DO0_(0)  MS1_(0)  MS0_(0)
	_____________________________________________
	|DO2 |DO1 |DO0 |Typical Data Output Rate (Hz)|
	|____|____|____|_____________________________|
	|0   |0   |0   |0.75                         |
	|0   |0   |1   |1.5                          |
	|0   |1   |0   |3                            |
	|0   |1   |1   |7.5                          |
	|1   |0   |0   |15 (Default)                 |
	|1   |0   |1   |30                           |
	|1   |1   |0   |75                           |
	|1   |1   |1   |Reserved                     |
	|____|____|____|_____________________________|



   |MS1|MS0|Measurement Mode 
   |___|___|____________________________
   |0  |0 
   |   |   |Normal measurement configuration (Default). In normal measurement 
   |   |   |configuration the device follows normal measurement flow. The positive and 
   |   |   |negative pins of the resistive load are left floating and high impedance. 
   |___|___|__________________________________________________
   |0   1 
   |		Positive bias configuration for X, Y, and Z axes. In this configuration, a positive 
   |		current is forced across the resistive load for all three axes. 
   |______________________________________________________________________
   |1   0 
   |		Negative bias configuration for X, Y and Z axes. In this configuration, a negative 
   |		current is forced across the resistive load for all three axes.. 
   |___________________________________________________________
   |1   1 
   |		This configuration is reserved. 
   |____________________________________________________________
	*/
	
	// Select Configuration register A(0x00)
	// Normal measurement configuration, data rate o/p = 0.75 Hz(0x60)
	registerWrite(bus, 0x1E, 0x00 , 0x34);
	
	//Register 0x02
	/*
	
	MR7   MR6  MR5  MR4  MR3  MR2  MR1     MR0 
    HS(0) (0)  (0)  (0)  (0)  (0)  MD1_(0) MD0_(1) 

    HS ->> 1 => Set this pin to enable High Speed I2C, 3400kHz. 


    MD1 MD0 Operating Mode 
    
	0   0 
        Continuous-Measurement Mode. In continuous-measurement mode, 
        the device continuously performs measurements and places the 
        result in the data register. RDY goes high when new data is placed 
        in all three registers. After a power-on or a write to the mode or 
        configuration register, the first measurement set is available from all 
        three data output registers after a period of 2/fDO and subsequent 
        measurements are available at a frequency of fDO, where fDO is the 
        frequency of data output. 
    
	0   1 
        Single-Measurement Mode (Default). When single-measurement 
        mode is selected, device performs a single measurement, sets RDY 
        high and returned to idle mode. Mode register returns to idle mode 
        bit values. The measurement remains in the data output register and 
    	RDY remains high until the data output register is read or another 
        measurement is performed. 
    
    1   0 
	    Idle Mode. Device is placed in idle mode. 
	    
    1   1 
	    Idle Mode. Device is placed in idle mode. 


	*/
	// Select Mode register(0x02)
	// Continuous measurement mode(0x00)
	registerWrite(bus, 0x1E, 0x02, 0x00);
		
	//sleep for set register and reboot sensor
	//sleep 1 sec
	sleep(1);

}


// read data from sensor and return what we want
// if select_output == x return raw x 
// if select_output == y return raw y 
// if select_output == z return raw z 
// if select_output == h return head angle
float hmc5883Read(int bus, char select_output)
{
	// Read 6 bytes of data from register(0x03)
	// xMag msb, xMag lsb, zMag msb, zMag lsb, yMag msb, yMag lsb
	char *data;
	data = new char[6];
	
	registerRead(bus, 0x1E, 0x03, 6, data);
	
	int xMag, yMag, zMag;
	float heading = 0;
	
	// Convert the data
	xMag = (int16_t)(data[0]<<8 | data[1]);
	
	zMag = (int16_t)(data[2]<<8 | data[3]);
	
	yMag = (int16_t)(data[4]<<8 | data[5]);
	
	// Output raw data to screen
	//for debug uncommment it
	//printf("Magnetic field in X-Axis : %d \n", xMag);
	//printf("Magnetic field in Y-Axis : %d \n", yMag);
	//printf("Magnetic field in Z-Axis : %d \n", zMag);
		
	//calculate head of sensor
	heading = 180 * atan2(yMag,xMag)/3.141592;
		
	//set heading of device
	if(heading < 0)
	{
     		heading += 360;
    }
	 	
    //for select bettwin inputs
	switch(select_output) {
		//show x raw data
		case 'x' :
    		//for debug uncommment it
			//printf("Magnetic field in X-Axis : %d \n", xMag);
			return (float) xMag;
      		
		//show y raw data
		case 'y' :
    		//for debug uncommment it
			//printf("Magnetic field in Y-Axis : %d \n", yMag);
			return (float) yMag;
      		
		//show z raw data
		case 'z' :
    		//for debug uncommment it
			//printf("Magnetic field in Z-Axis : %d \n", zMag);
			return (float) zMag;
      	
	  	//show head angel
		case 'h' :
    		//for debug uncommment it
			//printf("head x-y: %f \n", heading);
			return (float)heading;
      		
    	//anything else
    	default :
      		cout<< "Enter x, y, z or h"<< endl; 	
		  
	}
	
}


//############## ADXL345 ##############

// Setup sensor and set registers
void setup_ADXL345(int bus)
{       
	// Enable measuring    
    registerWrite(bus, 0x53, 0x2D, 0x08);
     
    // Select Bandwidth rate register(0x20)
    // normal mode ,Output Data Rate = 100Hz(0x0a)
    registerWrite(bus, 0x53, 0x2C, 0x0A);
	
    // Select Power Control register (0x2d)
    // Auto Sleep disable(0x88)
    registerWrite(bus, 0x53, 0x0D, 0x08);
	 
    // Select Data  format register(0x31)
    // Self test disabled, 4-wire interface,full resolution,range +/- 16g
    registerWrite(bus, 0x53, 0x31, 0x0A);
	 	
}


double adxl345Read(int bus, char axis)
{
    //reads the raw data
    char reg[1] = {0x32};
    write(bus, reg, 1);
    
    char *data;
	data = new char[2];
	
	if (axis == 'x')
	{
		registerRead(bus, 0x53, 0x32, 2, data);
		return (int16_t)(data[1] << 8 | data[0])/256.0;
	}
	else if (axis =='y')
	{
		registerRead(bus, 0x53, 0x34, 2, data);
		return (int16_t)(data[1] << 8 | data[0])/256.0;
	}
	else if (axis =='z')
	{
		registerRead(bus, 0x53, 0x36, 2, data);
		return (int16_t)(data[1] << 8 | data[0])/256.0;
	}
}

//############## L3G4200D ##############

// Setup sensor and set registers
void setup_L3G4200D(int bus)
{
	// Enable x, y, z and turn off power down:
	registerWrite(bus, 0x69, 0x20, 0b00001111);

	// If you'd like to adjust/use the HPF, you can edit the line below to configure CTRL_REG2:
	registerWrite(bus, 0x69, 0x21, 0b00000000);
	
	// Configure CTRL_REG3 to generate data ready interrupt on INT2
	// No interrupts used on INT1, if you'd like to configure INT1
	// or INT2 otherwise, consult the datasheet:
	registerWrite(bus, 0x69, 0x22, 0b00001000);

	// CTRL_REG4 controls the full-scale range, among other things:
	registerWrite(bus, 0x69, 0x23, 0b00110000);

	// CTRL_REG5 controls high-pass filtering of outputs, use it
	// if you'd like:
	registerWrite(bus, 0x69, 0x24, 0b00000000);
}


int l3g4200dRead(int bus, char axis)
{
	char *MSB;
	MSB = new char[1];
	
	char *LSB;
	LSB = new char[1];
	
	if (axis == 'x')
	{
		registerRead(bus, 0x69, 0x29, 1, MSB);
		registerRead(bus, 0x69, 0x28, 1, LSB);
		return (int16_t)((MSB[0] << 8) | LSB[0]);
	}
	else if (axis == 'y')
	{
		registerRead(bus, 0x69, 0x2B, 1, MSB);
		registerRead(bus, 0x69, 0x2A, 1, LSB);
		return (int16_t)((MSB[0] << 8) | LSB[0]);
	}
	else if (axis == 'z')
	{
		registerRead(bus, 0x69, 0x2D, 1, MSB);
		registerRead(bus, 0x69, 0x2C, 1, LSB);
		return (int16_t)((MSB[0] << 8) | LSB[0]);
	}
}

