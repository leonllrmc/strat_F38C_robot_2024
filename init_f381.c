/////////////////////////////////////
//  Generated Initialization File  //
/////////////////////////////////////

/*
* version : 1.8
* auteur : Carbone / Anderes
* date : 11 decembre 2019
* Ajouts de commandes RS232
*
* version : 1.9
* auteur : Didier Moullet
* date : 26 décembre 2019
* Ajouts de commandes RS232
* Ajout du mode transparent . Les informations remontent 
* via le RS232 de la carte moteur
* 
* version : 2.0
* auteur : Didier Moullet
* date : 26 février 2021
* Refonte des comm RS232, inclusion du Handsahke XON-XOFF
* Enlever tous les décodages des commandes ne concernant que la carte moteur
* Il faut maintenant que les élèves code les commandes pour faire bouger le robot 
* 
* Exemple pour avance(1000) : Send_string("AV1000\r",CARTE_MOTEUR);
* Cela implique que les élèves doivent connaitre toutes les commandes de la cartes moteur.
*
* uart_number = 0 -> Communication avec le PC 
* uart_number = 1 -> Communication avec la carte moteur du robot

* version : 2.01
* auteurs : DC/JMA
* date : 24 mars 2021
* Refonte du projet pour correspondre à la structure utilisée par nos élèves
* Ajout structure I2C lib ecole


* version : 2.02
* auteurs : DC/JMA
* date : 5 décembre 2021
* Mise à jour de la librairie PCA9685

**********************************************************************/

//#include "compiler_defs.h"
//#include "C8051F380_defs.h"
#include "REG51F380.H"
#include "declar.h"

// Peripheral specific initialization functions,
// Called from the Init_Device() function
void Timer_Init()
{
   TCON      = 0x40;
   TMOD      = 0x20;
   CKCON     |= 0x09;  // DC/JMA plus propre que CKCON  = 0x09 
   
   //115200 b/s   en 48MHz
    TH1       = 0x30;
// timers defined in other page   
   
SFRPAGE   = CONFIG_PAGE;
//timer 4 used for delay_1ms

   TMR4RLL   = 0x00;
   TMR4RLH   = 0x00;


////timer 5 is used Delai_ms
   CKCON1   |= 0x04;
   TMR5RLL   = 0x80;
   TMR5RLH   = 0x44;
   TMR5L     = 0x80;
   TMR5H     = 0x44;

SFRPAGE   = LEGACY_PAGE;
}

void UART_Init()
{
   SCON0     = 0x10;
   //115200 avec 48MHz
   SBRLL1    = 0x30;
   SBRLH1    = 0xFF;
   SCON1     = 0x10;
   SBCON1    = 0x43;  
}

void Port_IO_Init()
{
    // P0.0  -  TX1 (UART1), Push-Pull,  Digital
    // P0.1  -  RX1 (UART1), Open-Drain, Digital
    // P0.2  -  Skipped,     Open-Drain, Digital
    // P0.3  -  Skipped,     Open-Drain, Digital
    // P0.4  -  TX0 (UART0), Push-Pull,  Digital
    // P0.5  -  RX0 (UART0), Open-Drain, Digital
    // P0.6  -  Skipped,     Open-Drain, Digital
    // P0.7  -  Skipped,     Open-Drain, Digital

    // P1.0  -  Skipped,     Open-Drain, Digital
    // P1.1  -  Skipped,     Open-Drain, Digital
    // P1.2  -  Skipped,     Push-Pull,  Digital
    // P1.3  -  Skipped,     Push-Pull,  Digital
    // P1.4  -  Skipped,     Push-Pull,  Digital
    // P1.5  -  Skipped,     Open-Drain, Digital
    // P1.6  -  Skipped,     Open-Drain, Digital
    // P1.7  -  Skipped,     Open-Drain, Digital

    // P2.0  -  Skipped,     Open-Drain, Digital
    // P2.1  -  Skipped,     Open-Drain, Digital
    // P2.2  -  Skipped,     Open-Drain, Digital
    // P2.3  -  Skipped,     Open-Drain, Digital
    // P2.4  -  Skipped,     Open-Drain, Digital
    // P2.5  -  Skipped,     Open-Drain, Digital
    // P2.6  -  SDA1 (SMBus1), Open-Drain, Digital
    // P2.7  -  SCL1 (SMBus1), Open-Drain, Digital

    // P3.0  -  Unassigned,  Open-Drain, Digital

    P0MDOUT   = 0x11;
    P1MDOUT   = 0x1C;
    P0SKIP    = 0xCC;
    P1SKIP    = 0xFF;
    P2SKIP    = 0x3F;
    XBR0      = 0x01;
    XBR1      = 0x40;
    XBR2      = 0x03;

}

void Oscillator_Init()
{
      
#if SYSCLK == 48000000

                     // +--------- clock interne LF
                     // | (1 : oscillateur LF : enable)
                     // | (0 : oscillateur LF: desable)
                     // |+-------- en lecture seule 1 : signal que oscillateur 
                     // ||         interne fonctionne à sa valeur de prog.
                     // ||++++---- réglage fin de la fréquence de l'osc. LF
                     // ||||||++-- choix du diviseur :
                     // ||||||||       (00 : Osc LF /8 -> f = 10 KHz)
                     // ||||||||       (01 : Osc LF /4 -> f = 20 KHz)
                     // ||||||||       (10 : Osc LF /2 -> f = 40 KHz)
                     // ||||||||       (11 : Osc LF /1 -> f = 80 KHz)
// OSCLCN = 0x00;    // 00000000 

                     // +--------- non utilisé
                     // |+++------ Sélection du clock USB 
                     // ||||           (010 : Oscil ext. : limiter la conso.)
                     // ||||+----- clock out select
                     // |||||          (0 : sortie sysclk non synchronisée)
                     // |||||          (1 : sortie sysclk synchronisée)
                     // |||||+++-- choix du clock système
                     // ||||||||       (000 : Oscil interne 48/4  --> 1.5, 3, 6
                     // ||||||||              ou 12 MHz selon le choix du 
                     // ||||||||              diviseur dans OSCICN
                     // ||||||||       (001 : Oscil externe = x  MHz)
                     // ||||||||       (010 : Oscil interne HF 48/2 = 24 MHz)
                     // ||||||||       (011 : Oscil interne HF 48/1 = 48 MHz)    
                     // ||||||||       (100 : Oscil interne LF = 80 KHz max)   
                     // ||||||||       (101 à 111 : réservés)   
   CLKSEL = 0x03;    // 00000011  

                     // +--------- clock interne HF
                     // |              (1 : oscillateur HF : enable)
                     // |              (0 : oscillateur HF: desable)
                     // |+-------- en lecture seule 1 : signal que oscillateur 
                     // ||              interne fonctionne à sa valeur de prog.
                     // ||+------- 1 : suspend l'oscillateur interne
                     // |||+++---- non utilisés
                     // ||||||++-- choix du diviseur :
                     // ||||||||       (00 : 12/8 -> f =  1.5 MHz)
                     // ||||||||       (01 : 12/4 -> f =  3   MHz)
                     // ||||||||       (10 : 12/2 -> f =  6   MHz)
                     // ||||||||       (11 : 12/1 -> f = 12   MHz)
   OSCICN = 0x83;    // 10000011 

#else


                     // +--------- clock interne LF
                     // | (1 : oscillateur LF : enable)
                     // | (0 : oscillateur LF: desable)
                     // |+-------- en lecture seule 1 : signal que oscillateur 
                     // ||         interne fonctionne à sa valeur de prog.
                     // ||++++---- réglage fin de la fréquence de l'osc. LF
                     // ||||||++-- choix du diviseur :
                     // ||||||||       (00 : Osc LF /8 -> f = 10 KHz)
                     // ||||||||       (01 : Osc LF /4 -> f = 20 KHz)
                     // ||||||||       (10 : Osc LF /2 -> f = 40 KHz)
                     // ||||||||       (11 : Osc LF /1 -> f = 80 KHz)
// OSCLCN = 0x00;    // 00000000 

                     // +--------- non utilisé
                     // |+++------ Sélection du clock USB 
                     // ||||           (010 : Oscil ext. : limiter la conso.)
                     // ||||+----- clock out select
                     // |||||          (0 : sortie sysclk non synchronisée)
                     // |||||          (1 : sortie sysclk synchronisée)
                     // |||||+++-- choix du clock système
                     // ||||||||       (000 : Oscil interne 48/4  --> 1.5, 3, 6
                     // ||||||||              ou 12 MHz selon le choix du 
                     // ||||||||              diviseur dans OSCICN
                     // ||||||||       (001 : Oscil externe = x  MHz)
                     // ||||||||       (010 : Oscil interne HF 48/2 = 24 MHz)
                     // ||||||||       (011 : Oscil interne HF 48/1 = 48 MHz)    
                     // ||||||||       (100 : Oscil interne LF = 80 KHz max)   
                     // ||||||||       (101 à 111 : réservés)   
   CLKSEL = 0x00;    // 00000000  

                     // +--------- clock interne HF
                     // |              (1 : oscillateur HF : enable)
                     // |              (0 : oscillateur HF: desable)
                     // |+-------- en lecture seule 1 : signal que oscillateur 
                     // ||              interne fonctionne à sa valeur de prog.
                     // ||+------- 1 : suspend l'oscillateur interne
                     // |||+++---- non utilisés
                     // ||||||++-- choix du diviseur :
                     // ||||||||       (00 : 12/8 -> f =  1.5 MHz)
                     // ||||||||       (01 : 12/4 -> f =  3   MHz)
                     // ||||||||       (10 : 12/2 -> f =  6   MHz)
                     // ||||||||       (11 : 12/1 -> f = 12   MHz)
   OSCICN = 0x83;    // 10000011 

#endif

//   FLSCL = 0x90;     // A utiliser si le clock system est à 48 MHz
   
  
}

void Interrupts_Init()
{
    EIE2      = 0x12;
    IT01CF    = 0x67;
    IE        = 0x10;
}

// Initialization function for device,
// Call Init_Device() from your main program
void Init_Device(void)
{
    Timer_Init();
    UART_Init();
    Port_IO_Init();
    Oscillator_Init();
    Interrupts_Init();
    SMBus1InitAndEnISR (1,SYSCLK,100000); // Configure and enable SMBus1 Enable Interrupt Service
    SMBus0InitAndEnISR (0,SYSCLK,0);      // Call this function to avoid WARNING but SMB0 not initialized
    PCA9685_En(); //Permet d'éviter les warnings des fonctions pour le PCA9685
}
