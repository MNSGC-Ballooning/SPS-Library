//Optical Particle Counter Library - SPS Trim

//University of Minnesota - Candler MURI
//Written March 2020

/*This is the definitions file for the OPC library. The code has been trimmed
so that only the SPS sensor will be functional. For the expanded code for
every sensor, contact Nathan Pharis at phari009@umn.edu
The code is for use with the MURI project.
Serial begin must be called separately.
 
The SPS 30 runs the read data function with the record data function, and
can record new data every 1 seconds.
*/

#include "SPS.h"



//////////OPC//////////



OPC::OPC(){}															//Non-serial constructor

OPC::OPC(Stream* ser){													//Establishes data IO stream
	s = ser;
}

int OPC::getTot(){ return nTot; }										//get the total number of data points

bool OPC::getLogQuality(){ return goodLog; }							//get the log quality

void OPC::initOPC(){													//Initialize the serial and OPC variables
	goodLog = false;													//Describe a state of successful or unsuccessful data intakes
	goodLogAge = 0;														//Age of the last good set of data
	badLog = 0;															//Number of bad hits in a row
	nTot = 1;															//Number of good hits, culminative
	resetTime = 1200000;												//autotrigger forced reset timer
}

String OPC::CSVHeader(){ return ("~"); }								//Placeholders: will always be redefined in sensor child classes

String OPC::logUpdate(){				
	String localDataLog = "OPC not specified!";
	return localDataLog;
}

String OPC::logReadout(String name){return "";}

bool OPC::readData(){ return false; }

void OPC::getData(float dataPtr[], unsigned int arrayFill){}

void OPC::getData(float dataPtr[], unsigned int arrayFill, unsigned int arrayStart){}

void OPC::powerOn(){}

void OPC::powerOff(){}

void OPC::setReset(unsigned long resetTimer){ resetTime = resetTimer; } //Manually set the length of the forced reset

uint16_t OPC::bytes2int(byte LSB, byte MSB){							//Two byte conversion to integers
	uint16_t val = ((MSB << 8) | LSB);
	return val;
}



//////////SPS//////////




SPS::SPS(i2c_t3 wireBus, i2c_pins pins) : OPC()							//I2C constructor for SPS object
{
	SPSWire = &wireBus;
	SPSpins = pins;
	iicSystem = true;
}

SPS::SPS(Stream* ser) : OPC(ser) {}										//Initialize stream using base OPC constructor

void SPS::powerOn()                                			            //SPS Power on command. This sends and recieves the power on frame
{
	if (!iicSystem){													//If the system is running serial...
		s->write(0x7E);                                                 //Send startup frame
		s->write((byte)0x00);
		s->write((byte)0x00);                                           //This is the actual command
		s->write(0x02);
		s->write(0x01);
		s->write(0x03);
		s->write(0xF9);
		s->write(0x7E);

		delay (100);
		for (unsigned int q = 0; q<7; q++) s->read();                   //Read the response bytes
		
	} else {															//If the system is running I2C...
		byte data[2] = {0x03,0x00};										//Data to write to set proper mode
		SPSWire->beginTransmission(SPS_ADDRESS);
		SPSWire->write(0x0010);											//Set Pointer
		SPSWire->write(data[0]);										//Write power on Data
		SPSWire->write(data[1]);
		SPSWire->write(CalcCrc(data));									//Every two bytes requires a checksum
		SPSWire->endTransmission();
	}
}

void SPS::powerOff()                              		                //SPS Power off command. This sends and recieves the power off frame
{
	if(!iicSystem){														//If the system is running serial...	
		s->write(0x7E);                                                 //Send shutdown frame
		s->write((byte)0x00);
		s->write(0x01);                                                 //This is the actual command
		s->write((byte)0x00);
		s->write(0xFE);
		s->write(0x7E);

		delay(100);
		for (unsigned int q = 0; q<7; q++) s->read();                   //Read the response bytes
	} else {															//If the system is running I2C...
		SPSWire->beginTransmission(SPS_ADDRESS);
		SPSWire->write(0x0104);											//Set Pointer
		SPSWire->endTransmission();
	}
}

void SPS::clean()                                		                //SPS Power off command. This sends and recieves the power off frame
{
	if(!iicSystem){														//If the system is running serial...
		s->write(0x7E);                                                 //Send clean frame
		s->write((byte)0x00);
		s->write(0x56);                                                 //This is the actual command
		s->write((byte)0x00);
		s->write(0xA9);
		s->write(0x7E);

		delay(100); 
		for (unsigned int q = 0; q<7; q++) s->read();                   //Read the response bytes
	} else {															//If the system is running I2C...
		SPSWire->beginTransmission(SPS_ADDRESS);
		SPSWire->write(0x5607);											//Set Pointer
		SPSWire->endTransmission();
	}
}

void SPS::initOPC()                            			  		        //SPS initialization code. Requires input of SPS serial stream.
{
	OPC::initOPC();														//Calls original init
	
	if(iicSystem) SPSWire->begin(I2C_MASTER,0x69,SPSpins,I2C_PULLUP_EXT,I2C_RATE_100); //Begin the wire if I2C with required specifications
	
	powerOn();                                       	            	//Sends SPS active measurement command
	delay(100);
	clean();															//clean to start. This does nothing if attached to the pump
	
	
}

String SPS::CSVHeader(){												//Returns the .logUpdate() data header in CSV format
	String header = "hits,lastLog,MC-1um,MC-2.5um,MC-4.0um,MC-10um,NC-0.5um,NC-1um,NC-2.5um,NC-4.0um,NC-10um,Avg. PM";
	return header;
}

String SPS::logUpdate(){                          				        //This function will parse the data and form loggable strings.
	unsigned int lastLog;
    String dataLogLocal; 
    if (readData()){                                                    //Read the data and determine the read success.
       goodLog = true;                                                  //This will establish the good log inidicators.
       goodLogAge = millis();
       badLog = 0;
       lastLog = millis() - goodLogAge;
	   dataLogLocal = (String(nTot++) + "," + String(lastLog) + ",");

   for(unsigned short k = 0; k<4; k++){                                 //This loop will populate the data string with mass concentrations.                                                       //below it.
         dataLogLocal += String(SPSdata.mas[k],6) + ',';            		    
   }

   for(unsigned short k = 0; k<5; k++){                                 //This loop will populate the data string with number concentrations.
        dataLogLocal += String(SPSdata.nums[k],6) + ',';
   }
   
   dataLogLocal += String(SPSdata.aver,6);                              //This adds the average particle size to the end of the bin.
    
  } else {
	 badLog ++;
	 if (badLog >= 5) goodLog = false;	
	 lastLog = millis() - goodLogAge;									//Good log situation the same as in the Plantower code
	 dataLogLocal = String(nTot) + "," + String(lastLog);
	 dataLogLocal += ",-,-,-,-,-,-,-,-,-,-";							//If there is bad data, the string is populated with failure symbols.              
	 if ((millis()-goodLogAge)>=resetTime) {							//If the age of the last good log exceeds the automatic reset trigger,
		 powerOff();													//the system will cycle and clean the dust bin.
		 delay (2000);
		 powerOn();
		 delay (100);
		 clean();
		 delay(2000);
		 goodLogAge = millis();
	 }
	}
	return dataLogLocal;
  }
  
String SPS::logReadout(String name){     
	unsigned int lastLog;
    String dataLogLocal; 
    if (readData()){                                                    //Read the data and determine the read success.
       goodLog = true;                                                  //This will establish the good log inidicators.
       goodLogAge = millis();
       badLog = 0;
       lastLog = millis() - goodLogAge;
	   dataLogLocal = (String(nTot++) + "," + String(lastLog) + ",");

   for(unsigned short k = 0; k<4; k++){                                 //This loop will populate the data string with mass concentrations.                                                       //below it.
         dataLogLocal += String(SPSdata.mas[k],6) + ',';            		    
   }

   for(unsigned short k = 0; k<5; k++){                                 //This loop will populate the data string with number concentrations.
        dataLogLocal += String(SPSdata.nums[k],6) + ',';
   }
   
   dataLogLocal += String(SPSdata.aver,6);                              //This adds the average particle size to the end of the bin.
    
    Serial.println();													//Clean serial monitor print
	Serial.println("=======================");
	Serial.println(("SPS: " + name));
	Serial.println();
	Serial.print("Successful Data Hits: ");
	Serial.println(String(nTot));
	Serial.print("Last log time: ");
	Serial.println(String(lastLog));
	Serial.println();
	Serial.print(".3 to .5 microns per cubic cm: ");
	Serial.println(String(SPSdata.nums[0],6));
	Serial.print(".3 to 1 microns per cubic cm: ");
	Serial.println(String(SPSdata.nums[1],6));
	Serial.print(".3 to 2.5 microns per cubic cm: ");
	Serial.println(String(SPSdata.nums[2],6));
	Serial.print(".3 to 4 microns per cubic cm: ");
	Serial.println(String(SPSdata.nums[3],6));
	Serial.print(".3 to 10 microns per cubic cm: ");
	Serial.println(String(SPSdata.nums[4],6));
	Serial.println("======================="); 
    
  } else {
	 badLog ++;
	 if (badLog >= 5) goodLog = false;	
	 lastLog = millis() - goodLogAge;									//Good log situation the same as in the Plantower code
	 dataLogLocal = "," + String(lastLog);
	 dataLogLocal += ",-,-,-,-,-,-,-,-,-,-";							//If there is bad data, the string is populated with failure symbols.              
	 
	 Serial.println();													//clean serial print
	 Serial.println("=======================");
	 Serial.println(("SPS: " + name));
	 Serial.println();
	 Serial.print("Successful Data Hits: ");
	 Serial.println(String(nTot));
	 Serial.print("Last log time: ");
	 Serial.println(String(lastLog));
	 Serial.println();
	 Serial.println("Bad log");
	 Serial.println("=======================");	 
	 
	 if ((millis()-goodLogAge)>=resetTime) {							//If the age of the last good log exceeds the automatic reset trigger,
		 powerOff();													//the system will cycle and clean the dust bin.
		 delay (2000);
		 powerOn();
		 delay (100);
		 clean();
		 delay(2000);
		 goodLogAge = millis();
	 }
	}
	return dataLogLocal;	
}

bool SPS::readData(){
	byte buffers[40] = {0};												//Reading buffer

	if(!iicSystem){														//If the SPS is configured in serial mode
		byte systemInfo[5] = {0};
		byte data = 0;
		byte checksum = 0;
		byte SPSChecksum = 0;


		s->write(0x7E);                                                 //The read data function will return true if the data request is successful.
		s->write((byte)0x00);
		s->write(0x03);                                                 //This is the actual command
		s->write((byte)0x00);
		s->write(0xFC);
		s->write(0x7E);

		if (! s->available()) return false;                             //If the given serial connection is not available, the data request will fail.

		if (s->peek() != 0x7E){                                         //If the sent start byte is not as expected, the data request will fail.
		 for (unsigned short j = 0; j<60; j++) data = s->read();        //The data buffer will be wiped to ensure the next data pull isn't corrupt.
		 return false;
		}

		if (s->available() < 47){                                       //If there are not enough data bytes available, the data request will fail. This
		return false;                                                   //will not clear the data buffer, because the system is still trying to fill it.
		}

		for(unsigned short j = 0; j<5; j++){                            //This will populate the system information array with the data returned by the                  
			systemInfo[j] = s->read();                                  //by the system about the request. This is not the actual data, but will provide
			if (j != 0) checksum += systemInfo[j];                      //information about the data. The information is also added to the checksum.
	

		if (systemInfo[3] != (byte)0x00){                               //If the system indicates a malfunction of any kind, the data request will fail.
		 for (unsigned short j = 0; j<60; j++) data = s->read();        //Any data that populates the main array will be thrown out to prevent future corruption.
		 return false;
		}

		byte stuffByte = 0;
		for(unsigned short j = 3; j < 40; j+=4){      					//This nested loop will read the bytes and convert to MSB
			for(unsigned short i = 0; i < 4; i++){
				unsigned short x = j - i;
				buffers[x] = s->read();
				
				if (buffers[x] == 0x7D) {                               //This hex indicates that byte stuffing has occurred. The
					stuffByte = s->read();                              //series of if statements will determine the original value
					if (stuffByte == 0x5E) buffers[x] = 0x7E;			//based on the following hex and replace the data.
					if (stuffByte == 0x5D) buffers[x] = 0x7D;
					if (stuffByte == 0x31) buffers[x] = 0x11;
					if (stuffByte == 0x33) buffers[x] = 0x13;
				}
				checksum += buffers[x];                                 //The data is added to the checksum.
			}
		}

		SPSChecksum = s->read();                                        //The provided checksum byte is read.
		data = s->read();                                               //The end byte of the data is read.

		if (data != 0x7E){                                              //If the end byte is bad, the data request will fail.
		   for (unsigned short j = 0; j<60; j++) data = s->read();      //At this point, there likely isn't data to throw out. However,
		   data = 0;                                                    //The removal is completed as a redundant measure to prevent corruption.
		   return false;
		}

		checksum = checksum & 0xFF;                                     //The local checksum is calculated here. The LSB is taken by the first line.
		checksum = ~checksum;                                           //The bit is inverted by the second line.

		if (checksum != SPSChecksum){                                   //If the checksums are not equal, the data request will fail.  
		  for (unsigned short j = 0; j<60; j++) data = s->read();       //Just to be certain, any remaining data is thrown out to prevent corruption.
		  return false;
		}
  
	} else {															//If the SPS is configured in I2C mode
		if(dataReady()){												//Check if data is available to pull
			SPSWire->beginTransmission(SPS_ADDRESS);
			SPSWire->write(0x0202);										//Set Pointer
			SPSWire->endTransmission(I2C_NOSTOP);						//request read
			SPSWire->sendRequest(SPS_ADDRESS,60,I2C_STOP);				//Fill the buffer
			SPSWire->finish();											//Wait for the buffer to fill
			
			if(SPSWire->available() != 60) return false;				//If the buffer does not fill, the data read failed.
			
			unsigned short i = 0;
			
			while(SPSWire->available()){								//Clear the buffer
				uint8_t data[3];
				
				for (uint8_t j = 0; j < 3; j++){						//read three bytes
					data[j] = SPSWire->readByte();
				}
				
				if (CalcCrc(data) != data[2]) return false;				//if the bytes fail the checksum, the data read failed.
				buffers[i++] = data[0];									//Otherwise, add the data to the buffer
				buffers[i++] = data[1];
			}		
		}else return false;												//If the data is not available to pull, the data read failed.
	}  
	
	memcpy((void *)&SPSdata, (void *)buffers, 40);						//Copy the data to the struct
	return true;                   
}

bool SPS::dataReady(){													//Check if the SPS is ready to send measurement data
	SPSWire->beginTransmission(SPS_ADDRESS);
	SPSWire->write(0x0202);												//Set Pointer
	SPSWire->endTransmission(I2C_NOSTOP);								//request read
	SPSWire->sendRequest(SPS_ADDRESS,3,I2C_STOP);
	SPSWire->finish();													//Wait to finish
	
	uint8_t data[3];
	uint8_t i = 0;
	
	while (SPSWire->available()){										//read data
		if (i < 3) data[i++] = SPSWire->readByte();
		else return false;
	}
	
	if((CalcCrc(data) == data[2])&&(data[0] == 0)&&(data[1] == 1)) return true;	//if the data is correctly transmitted and indicates data is ready, return true
	
	return false;
}

uint8_t SPS::CalcCrc(uint8_t data[2]) {									//Calculate the two byte checksum for I2C
	uint8_t crc = 0xFF;
	for(int i = 0; i < 2; i++) {
		crc ^= data[i];
		for(uint8_t bit = 8; bit > 0; --bit) {
			if(crc & 0x80) {
				crc = (crc << 1) ^ 0x31u;
			} else {
				crc = (crc << 1);
			}
		}
	}
	return crc;
}
