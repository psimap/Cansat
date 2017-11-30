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
// Sadra CanSat Team Headers
#include "i2c.h"
#include "gy.h"
#include "si.h"

using namespace std;

//############## BMP085 ##############
//const unsigned char OSS = 0;  // Oversampling Setting
// Calibration values
int ac1 ,ac2, ac3, b1, b2, mb, mc, md, b5;
unsigned int ac4, ac5, ac6;

//############## Main ##############

int main()
{
	int bus;
	bus = i2cCreate();
	
	// Setup BMP085
	setup_BMP085(bus);
	
	// Setup HMC5883
	setup_HMC5883(bus);
	
	// Setup ADXL345
	setup_ADXL345(bus);
	
	// Setup L3G4200D
	setup_L3G4200D(bus);
	
	ofstream log;
	// append data to the log file
	log.open ("/home/final/log.txt", fstream::app);
	log<< "Sadra CanSat Team\n\n";
	log<< "Time(ms)    Temperature(C)    Humidity(%)    Available sattelites    Latitude(degree)    Longitude(degree)    Altitide    Speed(Km/h)   accX(m/s^2)    accYaccX(m/s^2)    accZaccX(m/s^2)    Pressure(Pa)    Temperature(C)    Head(degree)";
	// close log file
	log.close();
	
	while(1)
	{		
		// append data to the log file
		log.open ("/home/final/log.txt", fstream::app);
		
		log<< "\n"<< "___________________________________________________________________________________________________________________________________________________________________"<< "\n";
		
		log<< "    ";
		//cout<< time()<< " ms	";
		
		
		// Print Temperature from SL7021
		log<< si7021Read(bus, 't')<< "    ";
		cout<<"Temp: "<< si7021Read(bus, 't')<<endl;
		
		// Print Humidity from SL7021
		log<< si7021Read(bus, 'h')<< "    ";
		cout<<"Hum: "<< si7021Read(bus, 'h')<<endl;
		
		
		cout<< "Available sattelites: "<< strRead(bus, 1, 1)<< endl;
		
		cout<< "Laitude: "<< strRead(bus, 2, 9)<< endl;
		
		cout<< "Longitude: "<< strRead(bus, 3, 9)<< endl;
		
		cout<< "Altitide: "<< strRead(bus, 4, 7)<< endl;
		
		cout<< "Speed: "<< strRead(bus, 5, 6)<< endl;
		
		cout<< "UV: "<< strRead(bus, 6, 4)<< endl;
		
		
		// Print heading from HMC5883
		cout<< "head = "<< hmc5883Read(bus, 'h')<< endl;
		log<< hmc5883Read(bus, 'h')<< "    ";
		
		// BMP085 local variables
		float pressure, temperature, altitude;
		// Print temperature from BMP085
		temperature = bmp085Read(bus, 't');
		cout << "temp: " << temperature << endl;
		log<< temperature << "    ";
		
		// Print pressure from BMP085
		pressure = bmp085Read(bus, 'p');
  		cout << "pre: " << pressure << endl;
  		log<< pressure << "    ";
  		
  		// Print altitude from the international barometric formula
		altitude = 44330 * ( 1 - pow((pressure / 101325) , ( 1 / 5.255 )));
		cout << "alt: " << altitude << endl;
		log<< altitude<< "    ";
		
		// Print gyroscope data from L3G4200D
		cout<< l3g4200dRead(bus, 'x')<< endl;
		cout<< l3g4200dRead(bus, 'y')<< endl;
		cout<< l3g4200dRead(bus, 'z')<< endl;
		
		// Print accelometer data from ADXL345
		cout << "Ax: " << adxl345Read(bus, 'x') << endl;
		log<< adxl345Read(bus, 'x') << "    ";	
		cout << "Ay: " << adxl345Read(bus, 'y') << endl;
		log<< adxl345Read(bus, 'y') << "    ";
		cout << "Az: " << adxl345Read(bus, 'z') << endl;
		log<< adxl345Read(bus, 'z') << "    ";
		
		// Close log file
		log.close();
		
		usleep(100000); //Just here to slow down the serial to make it more readable
		
	}
}
