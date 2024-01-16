#ifndef __I2C_H__
#define __I2C_H__
   
#pragma SAVE
#pragma REGPARMS
#pragma OT(0,SPEED);

// I2C 
#define  SMBUS1_PAGE 0x0F
#define  LEGACY_PAGE 0x00

#define  SMBUS_DUMMY 255

#define  SMBUS_ON  1
#define  SMBUS_OFF 0

#define  WRITE          0x00        // SMBus WRITE command
#define  READ           0x01        // SMBus READ command

// Status vector - top 4 bits only
#define  SMB_MTSTA      0xE0           // (MT) start transmitted
#define  SMB_MTDB       0xC0           // (MT) data byte transmitted
#define  SMB_MRDB       0x80           // (MR) data byte received
// End status vector definition

#define NUM_BYTES_MAX_WR 6
#define NUM_BYTES_MAX_RD 6

#define DATATYPE idata

typedef unsigned char i2c;

// Déclaré dans le fichier SMBUS.c 
void SMBus0InitAndEnISR  (unsigned char enSMBus, unsigned long sysClk, unsigned long smbFrequency );
void SMBus1InitAndEnISR  (unsigned char enSMBus, unsigned long sysClk, unsigned long smbFrequency );

unsigned char I2CWrite   (unsigned char bus, unsigned char addr, unsigned char *buffer, unsigned char buffersize);
unsigned char I2CRegWrite(unsigned char bus, unsigned char addr, unsigned char reg    , unsigned char dataVal);
unsigned char I2CRead    (unsigned char bus, unsigned char addr, unsigned char *buffer, unsigned char buffersize);
unsigned char I2CRegRead (unsigned char bus, unsigned char addr, unsigned char reg    , char *error); 






/*   i2c_read: read from i2c bus.
 *
 *   Arguments:
 *      bus: i2c bus object.
 *      addr: device address.
 *      buffer: buffer where the read data will be written.
 *      buffersize: size to be read.
 *
 *   Returns:
 *      Success: 0.
 *      Failure: A value other than 0.
 *            I2C_JOIN_ERR if ioctl call to join i2c bus has failed.
 *            I2C_READ_ERR if the read call to i2c file descriptor has failed.
 *         errno wil be set by the respective function.
 *
 *   Notes:   Check the device documentation about the read process. Usually, you
 *   should issue a blank write (no value) on the desired register to read its value.
 *
 *   Example: Read n values from register 0x10 on a device with address 0x50
 *
 *   #include "SMBUS.h"
 *
 *   #define DEVICE_ADDR   0x50
 *   #define DEVICE_REG   0x10
 *
 *   int main(int argc, char *argv[])
 *   {
 *      i2c bus;
 *      unsigned char buffer[n];
 *
 *      buffer[0] = DEVICE_REG;
 *
 *      bus = i2c_open("/dev/i2c-1");
 *
 *      i2c_write(bus, DEVICE_ADDR, buffer, 1);
 *      i2c_read(bus, DEVICE_ADDR, buffer, n);
 *   }
 *   */


#pragma RESTORE
#endif                                 /* #define __I2C_H__            */