#ifndef __1WIRE_CONFIG__
#define	__1WIRE_CONFIG__
#include <stdio.h>
/*
 * === One Wire sensors data === 
 */
#define OW_DEBUG   1   //Enable printf debugs from onewire
#define MAXSENSORS 1 //tempbuf der bruges i sensorScan kan maks holde 31sensore, buffer der bruges ved afseldelse max 35sensore
#define SENSORSIZE 11
//Sensor array
enum SensorData { FAMILY, ID1, ID2, ID3, ID4, ID5, ID6, CRC, VALUE1, VALUE2, SIGN };
uint8_t sensorValues[MAXSENSORS*SENSORSIZE];
uint8_t sensorScan[MAXSENSORS*8]; //Kan evt. pege på tempbuf som i R2
char tempbuf[1024];  //atleast 250byte

#endif
