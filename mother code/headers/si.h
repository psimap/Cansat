#pragma once
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <iostream>
#include <stdio.h>
#include <fstream>
#include "i2c.h"


//############## SI7021 ##############

// Reads temperature ( mode = 't' ) and humidity ( mode = 'h' )
float si7021Read(int bus, char mode)
{
	if ( mode == 't')
	{
		double temp_code;
		char *data;
		data = new char[2];
		registerRead(bus, 0x40, 0xE3, 2, data);
		temp_code = (int16_t)(data[0] << 8 | data[1]);
		
		return (float)(((175.72 * temp_code)/65536)-46.85);
	}
	if ( mode == 'h')
	{
		char *data;
		data = new char[2];
		registerRead(bus, 0x40, 0xE5, 2, data);
		double Y_out1, Y_out2;
		Y_out1 = (125*(data[0]/100)*25600)>>16;
    	Y_out2 = (125*((data[0]%100)*256+data[1]))>>16;
    	
		return (float)(Y_out1 + Y_out2 - 6);
	}
}

