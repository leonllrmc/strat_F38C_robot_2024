/*===========================================================================*=
   CFPT - Projet : PCA9685.c
   Auteur        : CFPTELEC
   Date creation : 28.06.2019
  =============================================================================
   Descriptif: Fonctions pour communiquer avec un circuit PCA9685.
               Ce circuit permet de piloter jusqu'à 16 servomoteurs.
               Attention, à l'alimentation des servomoteurs.
   
   Datasheet: https://www.nxp.com/docs/en/data-sheet/PCA9685.pdf
   
   Les fonctions pour le capteur (voir descriptif dans les entêtes des fonctions)


   
   Les fonctions d'abstraction utilisées pour communiquer sur le bus I2C sont:
       unsigned char I2CWrite   (unsigned char bus, unsigned char addr, unsigned char *buffer, unsigned char buffersize);
       unsigned char I2CRegWrite(unsigned char bus, unsigned char addr, unsigned char reg    , unsigned char dataVal);
       unsigned char I2CRead    (unsigned char bus, unsigned char addr, unsigned char *buffer, unsigned char buffersize);
       unsigned char I2CRegRead (unsigned char bus, unsigned char addr, unsigned char reg    , char *error); 

=*===========================================================================*/
#include "PCA9685.h"
#include "SMBUS.h"



unsigned int gRefFreq = 0;


/*---------------------------------------------------------------------------*-
   PCA9685_En ()
  -----------------------------------------------------------------------------
   Descriptif: cette fonction permet d'appeler toutes les fonctions de ce fichier
               afin d'éviter des warnings (uncalled segment) à la compilation.
               Les fonctions n'utiliseront pas le port I2C grâce an numero de 
               bus I2C: 255 ("SMBUS_DUMMY"). 
   Entrée    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
char PCA9685_En(){
   unsigned char bus = 255; // SMBUS_DUMMY
   unsigned char address = 0;
   PCA9685_InitServoMotor(bus,address, 0);
   PCA9685_init(bus, address, 0);
   PCA9685_setDutyCycle(bus, address, 0, 0);
   PCA9685_setAngleForServoMotor(bus, address, 0,0);
   PCA9685_setFreq(bus, address, 0);
   PCA9685_stop(bus, address);
   return 0;
}
   
   
/*------------------------------------------------------------------------------
   PCA9685_InitServoMotor ()
   -----------------------------------------------------------------------------
   Descriptif: Configure le circuit
               i2c device a l'adresse (addr) sur le bus I2C (bus)
   Entrée    : bus     : numéro du SMBus                              (0,1) 
               address : adresse du circuit PCA9685            (0x90..0x9F)
     type    : 0 : Servomoteur 50Hhz
               1 : Servomoteur 330hz
               2 : LED ou autre 1Khz
               autre : 50Hz
   Sortie    : --
-*------------------------------------------------------------------------------*/
char PCA9685_InitServoMotor(unsigned char bus, unsigned char address, unsigned char type){
   idata unsigned int freq = 50;
   if(type == 1)
   { 
      freq = 330;
   }
   if(type == 2)
   { 
      freq = 1000;
   }
   
   gRefFreq = freq; // Temporaire en attendant de trouver mieux (structure, ...)
   
   PCA9685_init(bus, address, freq);
   return 0;
}



char PCA9685_init(unsigned char bus, unsigned char address,unsigned int freq) {
      PCA9685_setFreq(bus, address, freq);
      return I2CRegWrite(bus, address, PCA9685_MODE1, PCA9685_ALLCALL | PCA9685_AI)
      + I2CRegWrite(bus, address, PCA9685_MODE2, PCA9685_OUTDRV);
}



/*------------------------------------------------------------------------------
   PCA9685_setDutyCycle ()
   -----------------------------------------------------------------------------
   Descriptif: Set largeur de pulse en pourcent
               i2c device a l'adresse (addr) sur le bus I2C (bus)
   Entrée    : bus       : numéro du SMBus                              (0,1) 
               address   : adresse du circuit PCA9685            (0x90..0x9F)
               pulseTime : Largeur de pulse en us
              
   Sortie    : --
-*------------------------------------------------------------------------------*/
char PCA9685_setDutyCycle(unsigned char bus, unsigned char address, char channel, int value) {
   idata unsigned char buf[5];
   
   value = value < 0? 0:
         value > 100? PCA9685_MAX_DUTY_CYCLE:
         (PCA9685_MAX_DUTY_CYCLE * (long)value) / 100;
   
   buf[0] = PCA9685_LED0_ON_L + (PCA9685_REGISTERS_PER_CHANNEL * channel);
   buf[1] = buf[2] = 0x00;
   buf[3] = value & 0xFF; buf[4] = (value >> 8) & 0xF;
   return I2CWrite(bus, address, buf, 5);
}




/*------------------------------------------------------------------------------
   PCA9685_setPulse_us ()
   -----------------------------------------------------------------------------
   Descriptif: Set largeur de pulse en us our chaque sortie
               i2c device a l'adresse (addr) sur le bus I2C (bus)
   Entrée    : bus       : numéro du SMBus                              (0,1) 
               address   : adresse du circuit PCA9685            (0x90..0x9F)
               pulseTime : Largeur de pulse en us
              
   Sortie    : --
-*------------------------------------------------------------------------------*/
char PCA9685_setPulse_us(unsigned char bus, unsigned char address, char channel, unsigned int pulseTime) {
   idata unsigned char buf[5];
   idata unsigned int freq = gRefFreq; // Temporaire en attendant de trouver mieux (structure, ...);
   idata unsigned int value = 0;

   value = (unsigned long) pulseTime * freq * 64 / 15625; // formule: registre LEDx_OFF = pulseTime * freq * 4096 / 1000000 
   
   if(value > 4095){
      value = 4095;
   }
   
   buf[0] = PCA9685_LED0_ON_L + (PCA9685_REGISTERS_PER_CHANNEL * channel);
   buf[1] = buf[2] = 0x00;
   buf[3] = value & 0xFF;
   buf[4] = (value >> 8) & 0xF;
   return I2CWrite(bus, address, buf, 5);
}



/*------------------------------------------------------------------------------
   PCA9685_setAngleForServoMotor ()
   -----------------------------------------------------------------------------
   Descriptif: Set largeur de pulse de chaque sortie correspondant a la valeur
			   exprimee en degré
               i2c device a l'adresse (addr) sur le bus I2C (bus)
   Entrée    : bus     : numéro du SMBus                              (0,1) 
               address : adresse du circuit PCA9685            (0x90..0x9F)
   Sortie    : --
-*------------------------------------------------------------------------------*/
char PCA9685_setAngleForServoMotor(unsigned char bus, unsigned char address, char channel, char value) {
   // idata unsigned char buf[5];
   //idata unsigned int valRegForAngle;
   idata unsigned int pulseTime; 
   value = value < -90? -90:
   value > 90? 90:value;
   
//   valRegForAngle = ((PCA9685_MAX_DUTY_CYCLE-1) * (150 + (long)value*50/90)) / 2000;
//   buf[0] = PCA9685_LED0_ON_L + (PCA9685_REGISTERS_PER_CHANNEL * channel);
//   buf[1] = buf[2] = 0x00;
//   buf[3] = valRegForAngle & 0xFF; buf[4] = (valRegForAngle >> 8) & 0xF;
//   return I2CWrite(bus, address, buf, 5);
   
   pulseTime = 1500 + 175 * (int)value / 23;  // Calcul : 700us pour 90° => on a utilisé 700us pour 92° => 700/92 = 175/23 pour travailler avec des int
   
   return PCA9685_setPulse_us(bus, address,channel, pulseTime);
}

char PCA9685_setFreq(unsigned char bus, unsigned char address, unsigned int freq) {
   
   I2CRegWrite(bus, address, PCA9685_MODE1, PCA9685_ALLCALL | PCA9685_AI | PCA9685_SLEEP);
   
   freq = freq < 24? 0xFF:
         freq > 1526? 0x03:
         25800000 / 4096 / freq-1; 
   I2CRegWrite(bus, address, PCA9685_PRE_SCALE, (unsigned char)freq);
   I2CRegWrite(bus, address, PCA9685_MODE1, PCA9685_ALLCALL | PCA9685_AI);
   return 0;
}



char PCA9685_stop(unsigned char bus, unsigned char address) {
   return I2CRegWrite(bus, address, PCA9685_MODE1, PCA9685_SLEEP);
}


