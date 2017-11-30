#pragma once
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>


double str2num(char *data, int len)
{
	double a = 0;
	int dec = 0;
	bool mark = 0;
	
	if (data[0] == 0)	return NULL; // detects if no data available
	
	for (int i=0; i<len; i++)
    {
    	if(data[i] == 32) // ignore the empty spaces
    	{
    		continue;
		}
    	if(data[i] == 45) // detects the negative mark
    	{
    		mark = 1;
    		continue;
		}
    	if(data[i] == 46) // locates the decimal point
    	{
    		dec = len - i - 1;
    		continue;
		}
	    a = a * 10;
    	a = a + (data[i] - 48);
	}
	
	if (mark == 1) a = a * -1;
	a = a / pow(10,dec);
	
	return a;
}
