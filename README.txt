Optical Particle Counter Library (SPS)

University of Minnesota - Candler - MURI Project
Written in March 2020

These are the files for the trimmed OPC library (SPS.h). This will run the SPS30 optical particle counter used for MURI.
Serial begin must be called separately for systems that run through a hardware or software serial port.
The library has been optimized for Teensy 3.5/3.6. I2C will only work on a Teensy 3.x system, using the i2c_t3 system.



----------Sensor Overview----------



The Sensirion SPS30 is the particle counter that is planned for use on the BOLT campaign flights in Sweden.
The SPS30 weighs 26g. The system runs on a UART system, and requires 5V of power. On average, the
system will draw 60mA. At worst, the system will pull 80mA of power.*

*During testing, the current for the SPS30 power draw spiked to 120mA. However, during this test, the current draw was normally 30mA.

After plugging in the supplied SPS plug, from the black wire to the red wire, the wires correspond to:
Wire closest to the exterior of the SPS - Black wire - GND
Green wire - Select pin, leave floating.
Purple wire - TX wire, connect to the microcontroller RX.
White wire - RX wire, connect to the microcontroller TX.
Wire closest to the interior of the SPS - Red wire - PWR.

The full data sheet is located in this folder, including the documentation for an I2C setup. For a single SPS and microcontroller operating on an I2C bus,
After plugging in the supplied SPS plug, from the black wire to the red wire, the wires correspond to:
Wire closest to the exterior of the SPS - Black wire - GND.
Green wire - Select pin - connect to ground.
Purple wire - SCL, connect to the microcontroller SCL. Also connect a 10k ohm pullup to PWR.
White wire - SDA, connect to the microcontroller SDA. Also connect a 10k ohm pullup to PWR.
Wire closest to the interior of the SPS - Red wire - PWR.

In order to operate the system with a pump, a small rod must be pushed into the fan (the exhaust, large slits of the SPS) in 
order to jam it. The SPS pump attachment should then be placed over the face of the SPS. The pump requires 6V of power at most,
with a power draw between 0.17A and 0.23A. The pump for the SPS30 will be run on #### volts to match the flow rate assumed by
the manufacturer.

With the full pump system, the SPS30 weighs ###g.

The SPS30 runs the read data function with any record data function, and
can record new data every 1 seconds. The SPS30 serial is 115200 baud. The
SPS30 is configured for UART communication. I2C communication is available, but not recommended.
The SPS30 has 10 data points. In the log update command, 12 data points are recorded into a string.

The SPS30 logs the number of hits, the time since the last successful log, Mass Concentrations 1um, 2.5um, 4.0um, 10um, Number Concentrations 0.3um - 0.5um, 0.3um - 1um, 0.3um - 2.5um, 0.3um - 4.0um, 0.3um - 10um, Average Particle size.



----------Important Notes Regarding Operation and Data Analysis----------



The I2C system is implemented into the code and documentation, but it must still undergo testing and debugging to ensure it works.
It is strongly recommended that if the I2C system is used, then the SCL and SDA wires ought to be less than 20cm in length.

Each bin will contain the particles of the bin below it in addition to the particles inclusively within the bin.

In order to operate the OPCs, an object must be created (with class SPS)
AND .initOPC() must be called. After this, the particle counters will be
active and ready for use. The commands are identical for I2C function or serial function, other than the 
initial constructor call.

With the SPS, .logUpdate() and .logReadout() will call .readData() automatically.

To directly access the data from the SPS30, the data is saved in a struct that can be accessed by .SPSdata.
The struct contains an array of four float data points called "mas" that can access the mass concentration data.
For instance, to access the 0.3um to 4.0um mass concentration, you could use a call to .SPSdata.mas[2]. Similarly,
the data for the number concentrations are stored in an array of five float data points called "nums". The average
particle size is stored in a float data point that can be accessed using "aver".

Any other data from the sensors, such as particle counter statuses or other data arrangements, are not stored.

If .logUpdate() or .logReadout() are not called, and the user directly calls .readData(), then only the mass concentrations,
number concentrations, and average particle sizes in the struct will be updated.

Any questions, comments, or concerns should be directed towards Nathan Pharis <phari009@umn.edu> or
the current library maintainer.

This code is written with the purpose of conducting high altitude ballooning experiments on micron level particles. As
a result, some functions of the sensors have been discarded as unnecessary, or are not clearly laid out. Feel free to
modify the code to suit your specific needs.

Data from the first 30 seconds of powering on the sensors will not be reliable.



----------Commands for OPCSensor Library----------



All OPC:
 - construct with a reference to the serial port name (&serialName), and separately begin the serial connection.
		- By default, I2C mode is disabled. To enable the I2C communication option, enter the SPS.h file, and uncomment the define macro for I2C_MODE
		- For I2C communication, construct with an I2C port name and pins (Wire#,I2C_PINS_##_##). You will not need to begin the wire connection.
		- Note that the I2C_PINS_##_## is a enumerated class within i2c_t3 that allows for use of alternate wire pins. Simply input the numbers of the pins
		  used, starting with the lower pin. For example, for Wire0 (or just Wire) on a Teensy 3.5/3.6 on the default pins, use I2C_PINS_18_19
 - .getTot() - returns total number of hits (int)
 - .getLogQuality() - returns the quality of the log (bool)
 - .powerOn() - used to start full system (void) (called by initOPC)
 - .powerOff() - used to end measurements (void)
 - .initOPC() - will initialize the OPC (void)
 - .CSVHeader() - will provide a header for the logUpdate data string (String)
 - .logUpdate() - will return a data string in CSV format (String)
 - .logReadout() - will return a data string in CSV format AND print the number particle concentrations to a monitor. (String)
 - .readData() - will read the data and return a bool indicating success (bool)
 - .setReset(int) - will manually set the automatic reboot time (void). The default is 20 minutes of constantly failed logging. This will cause a time delay 
					of approximately 20 seconds in the code operation.

SPS:
- .clean() - used to clean the system using the internal fan. (void) (called by initOPC)
- .getFanStatus() - will return a bool indicating whether or not the fan is active
