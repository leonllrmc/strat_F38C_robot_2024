/*=============================================================================
   CFPT - Projet : SMBUS.c
   Auteur        : FM
   Date création : 28.06.2019
  =============================================================================
  Descriptif: 
              Possibiliter d'activer ou désactiver SMBx lors de l'appel de la fonction
              CrossBar: SMBUS0 Activé (si SMBus0InitAndEnISR appelé)
              CrossBar: SMBUS1 Activé (si SMBus1InitAndEnISR appelé)
              Interruption: SMBUS0 Activé (si SMBus0InitAndEnISR appelé)
              Interruption: SMBUS1 Activé (si SMBus1InitAndEnISR appelé)
              Timer2 configuré et utilisé pour le clock de SMBUS0 et SMBUS1
=*===========================================================================*/



#include "SMBUS.h"
#include <reg51f380.h>      // registres 51f380


//SMB0  
DATATYPE unsigned char SMB0_NUM_BYTES_WR = 2;   // Number of bytes to write
                                                // Master -> Slave
DATATYPE unsigned char SMB0_NUM_BYTES_RD = 3;   // Number of bytes to read
                                                // Master <- Slave
// Global holder for SMBus data
// All receive data is written here
DATATYPE unsigned char SMB0_DATA_IN[NUM_BYTES_MAX_RD];
// Global holder for SMBus data.
// All transmit data is read from here
DATATYPE unsigned char SMB0_DATA_OUT[NUM_BYTES_MAX_WR];
DATATYPE unsigned char SMB0_TARGET;       // SMB0_TARGET SMBus slave address

bit SMB0_BUSY;                            // Software flag to indicate when the
                                          // SMBRead() or SMBWrite() functions
                                          // have claimed the SMBus

bit SMB0_RW;                              // Software flag to indicate the
                                          // direction of the current transfer


//SMB1  
DATATYPE unsigned char SMB1_NUM_BYTES_WR = 2;   // Number of bytes to write
                                                // Master -> Slave
DATATYPE unsigned char SMB1_NUM_BYTES_RD = 3;   // Number of bytes to read
                                                // Master <- Slave
// Global holder for SMBus data
// All receive data is written here
DATATYPE unsigned char SMB1_DATA_IN[NUM_BYTES_MAX_RD];
// Global holder for SMBus data.
// All transmit data is read from here
DATATYPE unsigned char SMB1_DATA_OUT[NUM_BYTES_MAX_WR];
DATATYPE unsigned char SMB1_TARGET;       // SMB0_TARGET SMBus slave address

bit SMB1_BUSY;                            // Software flag to indicate when the
                                          // SMBRead() or SMBWrite() functions
                                          // have claimed the SMBus

bit SMB1_RW;                              // Software flag to indicate the
                                          // direction of the current transfer



DATATYPE unsigned long NUM_ERRORS=0;      // Counter for the number of errors.

void SMBusISR   (void);

void SMBWrite   (unsigned char bus);
void SMBRead    (unsigned char bus);

void Timer2Init (unsigned long sysClk, unsigned long smbFrequency ); // Use by SMBUS0 and SMBUS1






/*-----------------------------------------------------------------------------
   I2CWrite ()
   -----------------------------------------------------------------------------
   Descriptif: Envoie de plusieurs (buffersize) bytes (buffer)  sur le bus 
               I2C (bus) sur le circuit a l'adresse (addr)
   Entrée    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
unsigned char I2CWrite(unsigned char bus,unsigned char addr, unsigned char *buffer, unsigned char buffersize) {
   unsigned char arrayIndex = 0;
   
   if(bus != SMBUS_DUMMY){
      if(bus==0)//SMBUS0
      {
         while (SMB0_BUSY);
         for(arrayIndex=0;arrayIndex<buffersize;arrayIndex++){
            SMB0_DATA_OUT[arrayIndex] = buffer[arrayIndex];
         }
         
         NUM_ERRORS = 0;
         SMB0_TARGET = addr;        // SMB0_TARGET the Slave for next SMBus transfer   
         SMB0_NUM_BYTES_WR = buffersize;
         SMBWrite(bus);             // Initiate SMBus write
         while (SMB0_BUSY);
         
      }
      else//SMBUS1   
      {
        
         while (SMB1_BUSY);
         for(arrayIndex=0;arrayIndex<buffersize;arrayIndex++){
            SMB1_DATA_OUT[arrayIndex] = buffer[arrayIndex];
         }
         
         NUM_ERRORS = 0;
         SMB1_TARGET = addr;        // SMB1_TARGET the Slave for next SMBus transfer   
         SMB1_NUM_BYTES_WR = buffersize;
         SMBWrite(bus);             // Initiate SMBus write
         while (SMB1_BUSY);
      }
   }
      
   if(NUM_ERRORS)
      return(-1);
   return(0);
}

/*-----------------------------------------------------------------------------
   I2CRegWrite ()
   -----------------------------------------------------------------------------
   Descriptif: Envoie de 1 byte (dataVal) dans le registre (reg) sur le circuit 
               a l'adresse (addr) sur le bus I2C (bus)
   Entrée    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
unsigned char I2CRegWrite(unsigned char bus, unsigned char addr, unsigned char reg, unsigned char dataVal) {
//   bus=bus; //Eviter warning lors de la compilation
   if(bus != SMBUS_DUMMY){
      if(bus==0)//SMBUS0
      {
         while (SMB0_BUSY);
         SMB0_DATA_OUT[0] = reg;
         SMB0_DATA_OUT[1] = dataVal;
         
         NUM_ERRORS = 0;
         SMB0_TARGET = addr;        // SMB0_TARGET the Slave for next SMBus transfer   
         SMB0_NUM_BYTES_WR = 2;
         SMBWrite(bus);             // Initiate SMBus write
         while (SMB0_BUSY);
      }
      else//SMBUS1
      {
         while (SMB1_BUSY);
         SMB1_DATA_OUT[0] = reg;
         SMB1_DATA_OUT[1] = dataVal;
         
         NUM_ERRORS = 0;
         SMB1_TARGET = addr;        // SMB1_TARGET the Slave for next SMBus transfer   
         SMB1_NUM_BYTES_WR = 2;
         SMBWrite(bus);             // Initiate SMBus write
         while (SMB1_BUSY);
      }
   }
   if(NUM_ERRORS)
      return(-1);
   return(0);
}

/*-----------------------------------------------------------------------------
   I2CRead ()
   -----------------------------------------------------------------------------
   Descriptif: Envoie de 1 byte (dataVal) dans le registre (reg) sur le circuit 
               a l'adresse (addr) sur le bus I2C (bus)
   Entrée    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
unsigned char I2CRead(unsigned char bus,unsigned char addr, unsigned char *buffer, unsigned char buffersize) {
   unsigned char arrayIndex = 0;

   if(bus != SMBUS_DUMMY){
      if(bus==0)//SMBUS0
      {
         while (SMB0_BUSY);
         SMB0_TARGET = addr;     // SMB0_TARGET the Slave for next SMBus transfer
         SMB0_NUM_BYTES_RD = buffersize;
         SMBRead(bus);   
         while (SMB0_BUSY);
         
         if(NUM_ERRORS){
            for(arrayIndex=0;arrayIndex<buffersize;arrayIndex++){
               buffer[arrayIndex] = 0;
            }
            return(-1);
         }

         for(arrayIndex=0;arrayIndex<buffersize;arrayIndex++){
               buffer[arrayIndex] = SMB0_DATA_IN[arrayIndex];
            }
         return(0);
      }
      else//SMBUS1
      {
         while (SMB1_BUSY);
         SMB1_TARGET = addr;     // SMB1_TARGET the Slave for next SMBus transfer
         SMB1_NUM_BYTES_RD = buffersize;
         SMBRead(bus);   
         while (SMB1_BUSY);
         
         if(NUM_ERRORS){
            for(arrayIndex=0;arrayIndex<buffersize;arrayIndex++){
               buffer[arrayIndex] = 0;
            }
            return(-1);
         }

         for(arrayIndex=0;arrayIndex<buffersize;arrayIndex++){
               buffer[arrayIndex] = SMB1_DATA_IN[arrayIndex];
            }
         return(0);
      }
   }
   return(0);
}


/*-----------------------------------------------------------------------------
   I2CRegRead ()
   -----------------------------------------------------------------------------
   Descriptif: Lecture de 1 byte dans le registre (reg) sur le circuit 
               a l'adresse (addr) sur le bus I2C (bus)
   Entrée    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
unsigned char I2CRegRead(unsigned char bus, unsigned char addr, unsigned char reg, char *error) {
   idata unsigned char buf[2]={0,0};
   
   if(bus != SMBUS_DUMMY){
      if(bus==0)//SMBUS0
      {
         while (SMB0_BUSY);
         SMB0_DATA_OUT[0] = reg;
         NUM_ERRORS = 0;
         SMB0_TARGET = addr;        // SMB0_TARGET the Slave for next SMBus transfer   
         SMB0_NUM_BYTES_WR = 1;
         SMBWrite(bus);             // Initiate SMBus write
         while (SMB0_BUSY);
         I2CRead(bus, addr, buf, 1);
         
      }
      else//SMBUS1
      {
         while (SMB1_BUSY);
         SMB1_DATA_OUT[0] = reg;
         NUM_ERRORS = 0;
         SMB1_TARGET = addr;        // SMB1_TARGET the Slave for next SMBus transfer   
         SMB1_NUM_BYTES_WR = 1;
         SMBWrite(bus);             // Initiate SMBus write
         while (SMB1_BUSY);
         I2CRead(bus, addr, buf, 1);
      }
   }
   if(NUM_ERRORS)
      *error=-1;
   return(buf[0]);
}



//-----------------------------------------------------------------------------
// SMBus1Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// SMBus1 configured as follows:
// - SMBus1 enabled
// - Slave mode inhibited
// - Timer2H used as clock source. The maximum SCL frequency will be
//   approximately 1/3 the Timer2H overflow rate
// - Setup and hold time extensions enabled
// - Bus Free and SCL Low timeout detection enabled
//
void SMBus1InitAndEnISR (unsigned char enSMBus, unsigned long sysClk, unsigned long smbFrequency)
{
   unsigned char sfrpageNb = SFRPAGE;
   unsigned char strDummy[1];
   
   if(enSMBus){
      Timer2Init(sysClk,smbFrequency);          // Configure Timer2L for use with SMBus
                                                // clock source
      SFRPAGE   = SMBUS1_PAGE;
       
                     // +-------------- ENSMB: SMBus Enable
                     // |                  0 : SMBus disabled
                     // |                  1 : SMBus enabled
                     // |+------------- INH: SMBus Slave Inhibit
                     // ||                 0 : Slave Mode enabled
                     // ||                 1 : Slave Mode inhibited
                     // ||+------------ BUSY: SMBus Busy Indicator
                     // |||+----------- EXTHOLD: SMBus Setup and Hold Time 
                     // ||||               0 : SDA Extended Setup disabled
                     // ||||               1 : SDA Extended Setup enabled
                     // |||| +--------- SMBTOE: SCL Timeout Detection Enable
                     // |||| |             0 : disabled
                     // |||| |             1 : enabled
                     // |||| |+-------- SMBFTE: Free Timeout Detection Enable
                     // |||| ||            0 : disabled
                     // |||| ||            1 : enabled
                     // |||| ||++------ SMBCS1-SMBCS0: Clock Source Selection
                     // |||| ||||          00 = timer 0 
                     // |||| ||||          01 = timer 1 
                     // |||| ||||          10 = timer 2H 
                     // |||| ||||          11 = timer 2L 
      SMB1CF  = 0x50;// x1x1/11xx
      SMB1CF |= 0x02;// xxxx/xx10   
      SMB1CF |= 0x80;// 1xxx/xxxx   // Enabled
      
         
      XBR2 |= 0x02;        // Enable SMBus1 pins
      
      EIE2 |= 0x08;  // Enable the SMBus1 interrupt
   }
   I2CWrite   (SMBUS_DUMMY, 0, strDummy, 0);
   I2CRegWrite(SMBUS_DUMMY, 0, 0 , 0);
   I2CRead    (SMBUS_DUMMY, 0, strDummy, 0);
   I2CRegRead (SMBUS_DUMMY, 0, 0 , strDummy); 

   SFRPAGE   = sfrpageNb;
}


//-----------------------------------------------------------------------------
// SMBus0Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// SMBus configured as follows:
// - SMBus enabled
// - Slave mode inhibited
// - Timer2H used as clock source. The maximum SCL frequency will be
//   approximately 1/3 the Timer2H overflow rate
// - Setup and hold time extensions enabled
// - Bus Free and SCL Low timeout detection enabled
//
void SMBus0InitAndEnISR (unsigned char enSMBus, unsigned long sysClk, unsigned long smbFrequency){

   unsigned char sfrpageNb = SFRPAGE;
   unsigned char strDummy[1];
   
   if(enSMBus){
      Timer2Init(sysClk,smbFrequency);           // Configure Timer2L for use with SMBus
      SFRPAGE   = LEGACY_PAGE;
                        // clock source
      
                        // +-------------- ENSMB: SMBus Enable
                        // |                  0 : SMBus disabled
                        // |                  1 : SMBus enabled
                        // |+------------- INH: SMBus Slave Inhibit
                        // ||                 0 : Slave Mode enabled
                        // ||                 1 : Slave Mode inhibited
                        // ||+------------ BUSY: SMBus Busy Indicator
                        // |||+----------- EXTHOLD: SMBus Setup and Hold Time 
                        // ||||               0 : SDA Extended Setup disabled
                        // ||||               1 : SDA Extended Setup enabled
                        // |||| +--------- SMBTOE: SCL Timeout Detection Enable
                        // |||| |             0 : disabled
                        // |||| |             1 : enabled
                        // |||| |+-------- SMBFTE: Free Timeout Detection Enable
                        // |||| ||            0 : disabled
                        // |||| ||            1 : enabled
                        // |||| ||++------ SMBCS1-SMBCS0: Clock Source Selection
                        // |||| ||||          00 = timer 0 
                        // |||| ||||          01 = timer 1 
                        // |||| ||||          10 = timer 2H 
                        // |||| ||||          11 = timer 2L 
      SMB0CF = 0x50;    // 0101 00xx   // Disable slave mode;
                                       // Enable setup & hold time
                                       // extensions;
                                       // Disable SMBus Free timeout detect;
                                       // Disable SCL low timeout detect;
                                   
      SMB0CF |= 0x02;   // xxxx xx10   // Use Timer2H overflows as SMBus clock
                                       // source;
                                   
      SMB0CF |= 0x80;   // 1xxx xxxx   // Enable SMBus;
      
      XBR0 |= 0x04;  // Enable SMBus0 pins
      
      EIE1 |= 0x01;  // Enable the SMBus0 interrupt
      
   }
   I2CWrite   (SMBUS_DUMMY, 0, strDummy, 0);
   I2CRegWrite(SMBUS_DUMMY, 0, 0 , 0);
   I2CRead    (SMBUS_DUMMY, 0, strDummy, 0);
   I2CRegRead (SMBUS_DUMMY, 0, 0 , strDummy); 
   SFRPAGE   = sfrpageNb;
}


//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SMBus0 Interrupt Service Routine (ISR)
//-----------------------------------------------------------------------------
//
// SMBus0 ISR state machine
// - Master only implementation - no slave or arbitration states defined
// - All incoming data is written to global variable array <SMB0_DATA_IN>
// - All outgoing data is read from global variable array <SMB0_DATA_OUT>
//
void SMBus0ISR (void) interrupt 7
{
   bit FAIL = 0;                       // Used by the ISR to flag failed
                                       // transfers

   static unsigned char sent_byte_counter;
   static unsigned char rec_byte_counter;
   
   unsigned char sfrpageNb = SFRPAGE;
   SFRPAGE   = LEGACY_PAGE;
   if (ARBLOST0 == 0)                  // Check for errors
   {
      // Normal operation
      switch (SMB0CN & 0xF0)           // Status vector
      {
         // Master Transmitter/Receiver: START condition transmitted.
         case SMB_MTSTA:
            SMB0DAT = SMB0_TARGET;     // Load address of the SMB0_TARGET slave
            SMB0DAT &= 0xFE;           // Clear the LSB of the address for the
                                       // R/W bit
            SMB0DAT |= SMB0_RW;        // Load R/W bit
            STA0 = 0;                  // Manually clear START bit
            rec_byte_counter = 1;      // Reset the counter
            sent_byte_counter = 1;     // Reset the counter
            break;

         // Master Transmitter: Data byte transmitted
         case SMB_MTDB:
            if (ACK0)                  // Slave ACK0?
            {
               if (SMB0_RW == WRITE)   // If this transfer is a WRITE,
               {
                  if (sent_byte_counter <= SMB0_NUM_BYTES_WR)
                  {
                     // send data byte
                     SMB0DAT = SMB0_DATA_OUT[sent_byte_counter-1];
                     sent_byte_counter++;
                  }
                  else
                  {
                     STO0 = 1;         // Set STO0 to terminate transfer
                     SMB0_BUSY = 0;    // And free SMBus interface
                  }
               }
               else {}                 // If this transfer is a READ,
                                       // proceed with transfer without
                                       // writing to SMB0DAT (switch
                                       // to receive mode)


            }
            else                       // If slave NACK,
            {
               STO0 = 1;               // Send STOP condition, followed
               //STA0 = 1;             // By a START
               SMB0_BUSY = 0;
               NUM_ERRORS++;           // Indicate error
            }
            break;

         // Master Receiver: byte received
         case SMB_MRDB:
            if (rec_byte_counter < SMB0_NUM_BYTES_RD)
            {
               SMB0_DATA_IN[rec_byte_counter-1] = SMB0DAT; // Store received
                                                          // byte
               ACK0 = 1;               // Send ACK0 to indicate byte received
               rec_byte_counter++;     // Increment the byte counter
            }
            else
            {
               SMB0_DATA_IN[rec_byte_counter-1] = SMB0DAT; // Store received
                                                          // byte
               SMB0_BUSY = 0;          // Free SMBus interface
               ACK0 = 0;               // Send NACK to indicate last byte
                                       // of this transfer

               STO0 = 1;               // Send STOP to terminate transfer
            }
            break;

         default:
            FAIL = 1;                  // Indicate failed transfer
                                       // and handle at end of ISR
            break;

      } // end switch
   }
   else
   {
      // ARBLOST = 1, error occurred... abort transmission
      FAIL = 1;
   } // end ARBLOST if

   if (FAIL)                           // If the transfer failed,
   {
      SMB0CF &= ~0x80;                 // Reset communication
      SMB0CF |= 0x80;
      STA0 = 0;
      STO0 = 0;
      ACK0 = 0;

      SMB0_BUSY = 0;                   // Free SMBus

      FAIL = 0;

      NUM_ERRORS++;                    // Indicate an error occurred
   }

   SI0 = 0;                            // Clear interrupt flag
   SFRPAGE   = sfrpageNb ;
}



//-----------------------------------------------------------------------------
// SMBus1 Interrupt Service Routine (ISR)
//-----------------------------------------------------------------------------
//
// SMBus1 ISR state machine
// - Master only implementation - no slave or arbitration states defined
// - All incoming data is written to global variable array <SMB1_DATA_IN>
// - All outgoing data is read from global variable array <SMB1_DATA_OUT>
//
void SMBus1ISR (void) interrupt 18
{
   bit FAIL = 0;                       // Used by the ISR to flag failed
                                       // transfers

   static unsigned char sent_byte_counter;
   static unsigned char rec_byte_counter;
   unsigned char sfrpageNb = SFRPAGE;
   SFRPAGE   = SMBUS1_PAGE;
   if (ARBLOST1 == 0)                  // Check for errors
   {
      // Normal operation
      switch (SMB1CN & 0xF0)           // Status vector
      {
         // Master Transmitter/Receiver: START condition transmitted.
         case SMB_MTSTA:
            SMB1DAT = SMB1_TARGET;     // Load address of the SMB0_TARGET slave
            SMB1DAT &= 0xFE;           // Clear the LSB of the address for the
                                       // R/W bit
            SMB1DAT |= SMB1_RW;        // Load R/W bit
            STA1 = 0;                  // Manually clear START bit
            rec_byte_counter = 1;      // Reset the counter
            sent_byte_counter = 1;     // Reset the counter
            break;

         // Master Transmitter: Data byte transmitted
         case SMB_MTDB:
            if (ACK1)                  // Slave ACK0?
            {
               if (SMB1_RW == WRITE)   // If this transfer is a WRITE,
               {
                  if (sent_byte_counter <= SMB1_NUM_BYTES_WR)
                  {
                     // send data byte
                     SMB1DAT = SMB1_DATA_OUT[sent_byte_counter-1];
                     sent_byte_counter++;
                  }
                  else
                  {
                     STO1 = 1;         // Set STO0 to terminate transfer
                     SMB1_BUSY = 0;    // And free SMBus interface
                  }
               }
               else {}                 // If this transfer is a READ,
                                       // proceed with transfer without
                                       // writing to SMB0DAT (switch
                                       // to receive mode)


            }
            else                       // If slave NACK,
            {
               STO1 = 1;               // Send STOP condition, followed
               //STA0 = 1;             // By a START
               SMB1_BUSY = 0;
               NUM_ERRORS++;           // Indicate error
            }
            break;

         // Master Receiver: byte received
         case SMB_MRDB:
            if (rec_byte_counter < SMB1_NUM_BYTES_RD)
            {
               SMB1_DATA_IN[rec_byte_counter-1] = SMB1DAT; // Store received
                                                          // byte
               ACK1 = 1;               // Send ACK0 to indicate byte received
               rec_byte_counter++;     // Increment the byte counter
            }
            else
            {
               SMB1_DATA_IN[rec_byte_counter-1] = SMB1DAT; // Store received
                                                          // byte
               SMB1_BUSY = 0;          // Free SMBus interface
               ACK1 = 0;               // Send NACK to indicate last byte
                                       // of this transfer

               STO1 = 1;               // Send STOP to terminate transfer
            }
            break;

         default:
            FAIL = 1;                  // Indicate failed transfer
                                       // and handle at end of ISR
            break;

      } // end switch
   }
   else
   {
      // ARBLOST = 1, error occurred... abort transmission
      FAIL = 1;
   } // end ARBLOST if

   if (FAIL)                           // If the transfer failed,
   {
      SMB1CF &= ~0x80;                 // Reset communication
      SMB1CF |= 0x80;
      STA1 = 0;
      STO1 = 0;
      ACK1 = 0;

      SMB1_BUSY = 0;                   // Free SMBus

      FAIL = 0;

      NUM_ERRORS++;                    // Indicate an error occurred
   }

   SI1 = 0;                            // Clear interrupt flag
   SFRPAGE   = sfrpageNb;
}



//-----------------------------------------------------------------------------
// Interrupt Service Routines
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Support Functions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// SMBWrite
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : SMBUS Number
//
// Writes a single byte to the slave with address specified by the <SMBx_TARGET>
// variable.
// Calling sequence:
// 1) Write SMBx_TARGET slave address to the <SMBx_TARGET> variable
// 2) Write outgoing data to the <SMBx_DATA_OUT> variable array
// 3) Call SMBWrite()
//
void SMBWrite (unsigned char bus)
{
   if(bus==0)//SMBUS0
   {
      SFRPAGE   = LEGACY_PAGE;
      while (SMB0_BUSY);                  // Wait for SMBus to be free.
      SMB0_BUSY = 1;                      // Claim SMBus (set to busy)
      SMB0_RW = 0;                        // Mark this transfer as a WRITE
      STA0 = 1;                           // Start transfer
   }
   else//SMBUS1
   {
      SFRPAGE   = SMBUS1_PAGE;
      while (SMB1_BUSY);                  // Wait for SMBus to be free.
      SMB1_BUSY = 1;                      // Claim SMBus (set to busy)
      SMB1_RW = 0;                        // Mark this transfer as a WRITE
      
      STA1 = 1;                           // Start transfer
   }

}

//-----------------------------------------------------------------------------
// SMBRead
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : SMBUS Number
//
// Reads a single byte from the slave with address specified by the <SMBx_TARGET>
// variable.
// Calling sequence:
// 1) Write SMBx_TARGET slave address to the <SMBx_TARGET> variable
// 2) Call SMBWrite()
// 3) Read input data from <SMBx_DATA_IN> variable array
//
void SMBRead (unsigned char bus)
{
   if(bus==0)//SMBUS0
   {
      SFRPAGE   = LEGACY_PAGE;
      while (SMB0_BUSY);                  // Wait for bus to be free.
      SMB0_BUSY = 1;                      // Claim SMBus (set to busy)
      SMB0_RW = 1;                        // Mark this transfer as a READ

      STA0 = 1;                           // Start transfer

      while (SMB0_BUSY);                  // Wait for transfer to complete
      //SFRPAGE   = LEGACY_PAGE;
   }
   else
   {
      SFRPAGE   = SMBUS1_PAGE;
      while (SMB1_BUSY);                  // Wait for bus to be free.
      SMB1_BUSY = 1;                      // Claim SMBus (set to busy)
      SMB1_RW = 1;                        // Mark this transfer as a READ

      STA1 = 1;                           // Start transfer

      while (SMB1_BUSY);                  // Wait for transfer to complete
      //SFRPAGE   = LEGACY_PAGE;
   }

}






//-----------------------------------------------------------------------------
// Timer2Init
//-----------------------------------------------------------------------------
//
// Return Value : None
// Parameters   : None
//
// Timer2 configured for use by the SMBus Clock feature as
// follows:
// - Timer2 in 2 x 8-bit auto-reload mode
// - SYSCLK/12 or SYSCLK as Timer2 clock source (depend of value of SYSCLK)
// - Timer2 reload registers loaded for SMB_FREQUENCY overflow period
// - Timer2 pre-loaded to overflow SMB_FREQUENCY
// - Timer2 enabled
void Timer2Init (unsigned long sysClk, unsigned long smbFrequency ){
   
   // See datasheet C8051F38x Rev1.4 page 275
   unsigned char scale = 1;
   TR2 = 0;
   TMR2CN = 0x00;                      // Clear Timer2 configure register
   
// Make sure the Timer can produce the appropriate frequency in 8-bit mode
// Supported SMBus Frequencies range from 10kHz to 100kHz.  The CKCON register
// settings may need to change for frequencies outside this range.
   if ((sysClk/smbFrequency/3) < 255){
      scale = 1;
      CKCON |= 0x20;                // Timer2H clock source = SYSCLK
      CKCON |= 0x10;                // Timer2L clock source = SYSCLK
   }
   else if ((sysClk/smbFrequency/12/3) < 255){
      scale = 12;
      CKCON  &= ~0x20;              // Timer2H clock source = SYSCLK / 12 or EXT_clk
      CKCON  &= ~0x10;              // Timer2L clock source = SYSCLK / 12
      T2XCLK = 0;                   // Timer2H clock source = SYSCLK / 12
   }
   else{

      // Si c'est pas possible... en attendant de trouver mieux
      scale = 12;
      CKCON  &= ~0x20;              // Timer2H clock source = SYSCLK / 12 or EXT_clk
      CKCON  &= ~0x10;              // Timer2L clock source = SYSCLK / 12
      T2XCLK = 0;                   // Timer2H clock source = SYSCLK / 12
   }

  
   T2SPLIT = 1;      // Timer 2 Split Mode Enable.
                     // When this bit is set, Timer 2 operates as 
                     // two 8-bit timers with auto-reload.

   // Timer2 configured to overflow at 1/3 the rate defined by SMB_FREQUENCY
   // See datasheet page 209
   TMR2RLH = -(sysClk/smbFrequency/scale/3);
   TMR2H   = -(sysClk/smbFrequency/scale/3);
   
   TMR2RLL = -(sysClk/smbFrequency/scale/3);
   TMR2L   = -(sysClk/smbFrequency/scale/3);
   
   TR2 = 1;    // Timer 2 Run Control
               // Timer 2 is enabled by setting this bit to 1.
               // In 8-bit mode, this bit enables/disables TMR2H only;
               // TMR2L is always enabled in split mode.
}



