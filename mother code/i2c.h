#pragma once
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <iostream>
#include <stdio.h>
#include "str2num.h"

using namespace std;

//############## i2c functions ##############

// Create the I2C bus
int i2cCreate ()
{
	int file;
	char *bus = "/dev/i2c-0";
	if ((file = open(bus, O_RDWR)) < 0) 
	{
		cout<< "Failed to open the bus."<< endl;
		exit(1);
	}
	
	return file;
}

// Writes data to a specified register
void registerWrite(int bus, char address, char Register_to_set, char Data_for_set_to_register)
{
	ioctl(bus, I2C_SLAVE, address);
	
	char config[2] = {Register_to_set, Data_for_set_to_register};
	write(bus, config, 2);
}

// Reads data from a specified register
void registerRead(int bus, unsigned char address, unsigned char reg, int num, char *data)
{
 	ioctl(bus, I2C_SLAVE, address);
    
	char config[1]={reg};
	write(bus, config, 1);
	
	read(bus, data, num);
}

// Reads data form the microcontroller as strings and convert them to doubles
double strRead(int bus, unsigned char reg, int len)
{
    ioctl(bus,I2C_SLAVE,0x08);
    
    char config[1] = {reg};
	write(bus, config, 1);
    
    char *data;
	data = new char[10];
  	read(bus, data, len);
  	
	return str2num(data, len);
}


