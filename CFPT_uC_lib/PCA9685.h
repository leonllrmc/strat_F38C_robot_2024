/*===========================================================================*=
   CFPT - Projet : PCA9685.h
   Auteur        : CFPTELEC
   Date creation : 28.06.2019
  =============================================================================
   Descriptif: Fonctions pour communiquer avec un circuit PCA9685.
               Ce circuit permet de piloter jusqu'à 16 servomoteurs.
               Attention, à l'alimentation des servomoteurs.
   
   Datasheet: https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf
   
   Les fonctions pour le capteur (voir descriptif dans les entêtes des fonctions)
      char PCA9685_En();
      char PCA9685_InitServoMotor(unsigned char bus, unsigned char address, unsigned char type);
      char PCA9685_init(unsigned char bus,unsigned char address,unsigned int freq );
      char PCA9685_setDutyCycle(unsigned char bus,unsigned char address, char channel, int value);
      char PCA9685_setPulse_us(unsigned char bus, unsigned char address, char channel, unsigned int pulseTime);
	  char PCA9685_setAngleForServoMotor(unsigned char bus,unsigned char address, char channel, char value);
      char PCA9685_setFreq(unsigned char bus, unsigned char address, unsigned int freq);
      char PCA9685_stop(unsigned char bus, unsigned char address);
   
   Les fonctions d'abstraction utilisées pour communiquer sur le bus I2C sont:
       unsigned char I2CWrite   (unsigned char bus, unsigned char addr, unsigned char *buffer, unsigned char buffersize);
       unsigned char I2CRegWrite(unsigned char bus, unsigned char addr, unsigned char reg    , unsigned char dataVal);
       unsigned char I2CRead    (unsigned char bus, unsigned char addr, unsigned char *buffer, unsigned char buffersize);
       unsigned char I2CRegRead (unsigned char bus, unsigned char addr, unsigned char reg    , char *error); 

=*===========================================================================*/
#ifndef __PCA9685_H__
#define __PCA9685_H__
   
#pragma SAVE
#pragma REGPARMS
#pragma OT(0,SPEED);

#define PCA9685_ADDRESS 0x82

#define PCA9685_MODE1 0x00
#define PCA9685_MODE2 0x01
#define PCA9685_LED0_ON_L 0x06
#define PCA9685_ALLCALL 0x01
#define PCA9685_SLEEP 0x10
#define PCA9685_AI 0x20               // Register Auto-Increment enabled.
#define PCA9685_OUTDRV 0x04           // The 16 LEDn outputs are configured with a totem pole structure.
#define PCA9685_PRE_SCALE 0xFE
#define PCA9685_MAX_DUTY_CYCLE 4095
#define PCA9685_REGISTERS_PER_CHANNEL 4


char PCA9685_En();

char PCA9685_InitServoMotor(unsigned char bus, unsigned char address, unsigned char type);

char PCA9685_init(unsigned char bus,unsigned char address,unsigned int freq );

char PCA9685_setDutyCycle(unsigned char bus,unsigned char address, char channel, int value);

char PCA9685_setPulse_us(unsigned char bus, unsigned char address, char channel, unsigned int pulseTime);

char PCA9685_setAngleForServoMotor(unsigned char bus,unsigned char address, char channel, char value);


// Set frequency : freq in [Hz] in between 24 and 1526 [Hz]
// This register can be set only if SLLEP bit of MODE1 is set to 1
char PCA9685_setFreq(unsigned char bus, unsigned char address, unsigned int freq);

char PCA9685_stop(unsigned char bus, unsigned char address);

#pragma RESTORE
#endif                                 /* #define __PCA9685_H__         */