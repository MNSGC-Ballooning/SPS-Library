//Optical Particle Counter Library - SPS Trim

//University of Minnesota - Candler MURI
//Written March 2020

/*This is the header file for the OPC library. The code has been trimmed
so that only the SPS sensor will be functional. For the expanded code for
every sensor, contact Nathan Pharis at phari009@umn.edu
The code is for use with the MURI project.
Serial begin must be called separately.
 
The SPS 30 runs the read data function with the record data function, and
can record new data every 1 seconds.
*/

#ifndef SPS_h
#define SPS_h

#include <arduino.h>
#include <Stream.h>

//#define I2C_MODE														//Uncomment this line to engage the SPS I2C option. This will clash with
																		//libraries that use Wire or twoWire instead of i2c_t3
#ifdef I2C_MODE
#include <i2c_t3.h>
#endif

#define SPS_ADDRESS 0x69

class OPC																//Parent OPC class
{
	protected:
	bool goodLog;														//Describe a state of successful or unsuccessful data intakes
	int badLog;															//Number of bad hits in a row
	int nTot;															//Number of good hits, culminative
	Stream *s;															//Declares data IO stream
	unsigned long goodLogAge;											//Age of the last good set of data
	unsigned long resetTime;											//Age the last good log must reach to trigger a reset
	uint16_t bytes2int(byte LSB, byte MSB);								//Convert given bytes to integers
	
	public:
	OPC();
	OPC(Stream* ser);													//Parent Constructor
	int getTot();														//Parent quality checks
	bool getLogQuality();												//get the quality of the log
	void initOPC();														//Initialization
	String CSVHeader();													//Placeholders
	String logUpdate();
	String logReadout(String name);													
	bool readData();
	void powerOn();
	void powerOff();
	void setReset(unsigned long resetTimer);							//Manually set the bad log reset timer
};

class SPS: public OPC
{
	private:
	bool altCleaned = false;											//The boolean for altitude based fan clean operation
	bool iicSystem = false;												//Indication of i2c or serial system 
	bool fanActive = true;

#ifdef I2C_MODE
	i2c_t3 *SPSWire;													//Local wire bus
	i2c_pins SPSpins;													//Local wire pins
	uint8_t	CalcCrc(uint8_t data[2]);									//SPS wire checksum calculation
	bool dataReady();													//data indicator
#endif	
	
	public:
	struct SPS30data {													//struct for SPS30 data
		float mas[4];
		float nums[5];
		float aver;		
	}SPSdata;

#ifdef I2C_MODE
	SPS(i2c_t3 wireBus, i2c_pins pins);									//I2C Constructor
#endif

	SPS(Stream* ser);													//Serial Constructor
	void powerOn();														//System commands for SPS
	void powerOff();
	void clean();
	void initOPC();														//Overrides of OPC data functions and initialization
	String CSVHeader();													//Returns a CSV header for log update
	String logUpdate();													//Returns the CSV string of SPS data
	String logReadout(String name);										//Log update, but with a nice serial print
	bool readData();													//data reader- generally controlled internally
	bool getFanStatus();												//Check to see if the fan is on
};

#endif
