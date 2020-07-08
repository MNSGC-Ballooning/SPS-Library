#include <SPS.h>                                        //Include the library!
      
#define SPS_SERIALA Serial2
#define SPS_SERIALB Serial3

#define logRate 4000                                          //Establish a log rate

SPS SpsA(&SPS_SERIALA);                                //Serial SPS - This is the only difference in operation between the two
SPS SpsB(&SPS_SERIALB);
//SPS SpsA(Wire,I2C_PINS_18_19);                          //I2C SPS

unsigned long prevTime = 0;                          

void setup() {
  Serial.begin(115200);                                       //Wait for the serial monitor to be connected before turning the system on- not critical to operation
  while (!Serial) ;
  delay (100);
  Serial.println("Serial active!");
  
  SPS_SERIALA.begin(115200);                                   //SPS serial initialization
  SPS_SERIALB.begin(115200);
  delay(1000);                                                //Delay to ensure connection, can be much shorter

  SpsA.initOPC();                                             //Initialize the SPS sensor
  SpsB.initOPC();
  Serial.println("SPS active!");

  delay(2000);                                                //The particle counters will not collect accurate data until 30 seconds after being turned on
  Serial.println("System initialized!");

//  Serial.println(SpsA.CSVHeader());                         //Prints a header in CSV format for the SPS particle counter
}

void loop() {
  if (millis() - prevTime >= logRate){                         //4000ms loop
   prevTime = millis();

//   Serial.println("SPS: " + SpsA.logUpdate());                 //Print the updates in CSV format

   Serial.println(SpsA.logReadout("OPC 1"));                 //Print the updates in a serial friendly format
   Serial.println(SpsB.logReadout("OPC 2"));                 //Print the updates in a serial friendly format
  }
}
