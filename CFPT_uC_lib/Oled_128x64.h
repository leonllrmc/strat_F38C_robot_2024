/*===========================================================================*=
   CFPT - Projet : Oled_128x64.c
   Auteur        : CFPTELEC
   Date creation : 07.02.2020
  =============================================================================
   Descriptif: Fonctions pour communiquer avec un afficheur OLED 128X64.
   
   Datasheet: https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
   
   Les fonctions pour le capteur (voir descriptif dans les entetes des fonctions)
      void OLEDSendMessageINT(char jsonName[], int dataToSend, unsigned char lin, unsigned char col);
      void OLEDSendMessage(char strToSend[],unsigned char lin, unsigned char col);
      bit InitLCD(unsigned char no_port);
      void AfficherCaractere (char caractere);
      void SelectPosCaractLiCo(unsigned char ligne, unsigned char colonne);
      void AfficherChaineAZT(char str[]);
      void EffacerEcran ();
   
   Les fonctions d'abstraction utilisées pour communiquer sur le bus I2C sont:
      unsigned char I2CWrite   (unsigned char bus, unsigned char addr, unsigned char *buffer, unsigned char buffersize);
      unsigned char I2CRegWrite(unsigned char bus, unsigned char addr, unsigned char reg    , unsigned char dataVal);
      unsigned char I2CRead    (unsigned char bus, unsigned char addr, unsigned char *buffer, unsigned char buffersize);
      unsigned char I2CRegRead (unsigned char bus, unsigned char addr, unsigned char reg    , char *error); 

=*===========================================================================*/
#ifndef __OLED_128X64_H__
#define __OLED_128X64_H__
   
#pragma SAVE
#pragma REGPARMS
#pragma OT(0,SPEED);

// Device OLED128X64 Register
#define OLED128X64_ADDRESS                0x78 //0x3c
#define OLED128X64_COMMAND_MODE           0x80
#define OLED128X64_DATA_MODE              0x40
#define OLED128X64_DISPLAY_OFF_CMD        0xAE
#define OLED128X64_DISPLAY_ON_CMD         0xAF
#define OLED128X64_NORMAL_DISPLAY_CMD     0xA6
#define OLED128X64_INVERSE_DISPLAY_CMD    0xA7
#define OLED128X64_ACTIVATE_SCROLL_CMD    0x2F
#define OLED128X64_DECTIVATE_SCROLL_CMD   0x2E
#define OLED128X64_SET_BRIGHTNESS_CMD     0x81
#define OLED128X64_DISPLAY_ALL_ON_CMD     0xA5


extern code unsigned char Oled_BasicFont[96][8];

/*---------------------------------------------------------------------------*-
   OLEDSendMessageINT ()
  -----------------------------------------------------------------------------
   Descriptif: Affiche sur l'écran OLED 
   Entrée    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
extern void OLEDSendMessageINT(char jsonName[], int dataToSend, unsigned char lin, unsigned char col);

/*---------------------------------------------------------------------------*-
   OLEDSendMessage ()
  -----------------------------------------------------------------------------
   Descriptif: Affiche sur l'écran OLED 
   Entrée    : --
   Sortie    : --
-*---------------------------------------------------------------------------*/
extern void OLEDSendMessage(char strToSend[],unsigned char lin, unsigned char col);


/*---------------------------------------------------------------------------*-
   InitLCD
  -----------------------------------------------------------------------------
   Descriptif: Initialise le Display LCD 4 bits et le port de connexion.
               Caractéristique par défaut: 2 lignes carct. 5x7 dots
   Entrée    : no_port, Numéro du port utilisé par le LCD 4 bits (0, 1 ou 2)
   Sortie    : Error, 0 0k, 1 error
-*---------------------------------------------------------------------------*/
extern bit InitLCD(unsigned char no_port);

///*---------------------------------------------------------------------------*-
//   AfficherCaract ()
//  -----------------------------------------------------------------------------
//   Descriptif: Affiche un caractère à la position courante du curseur
//   Entrée    : Code Ascii du caractère à afficher
//   Sortie    : --
//-*---------------------------------------------------------------------------*/
extern void AfficherCaractere (char caractere);
//
///*---------------------------------------------------------------------------*-
//   SelectPosCaractLiCo ()
//  -----------------------------------------------------------------------------
//   Descriptif: Sélectionne la position du caractère passé en argument
//   Entrée    : ligne, numéro de la ligne d'affichage 0 ou 1
//               colonne, numéro de la colonne d'affichage 0 à 15
//   Sortie    : --
//-*---------------------------------------------------------------------------*/
extern void SelectPosCaractLiCo(unsigned char ligne, unsigned char colonne);
//
///*---------------------------------------------------------------------------*-
//   AfficherChaineAZT ()
//  -----------------------------------------------------------------------------
//   Descriptif: Affiche une chaîne de caractère AZT à la position courante du 
//               curseur
//   Entrée    : str, de caractères AZT
//   Sortie    : --
//-*---------------------------------------------------------------------------*/
extern void AfficherChaineAZT(char str[]);
//
///*---------------------------------------------------------------------------*-
//   AfficherChaineAZTCentreLi ()
//  -----------------------------------------------------------------------------
//   Descriptif: Affiche une chaîne de caractère AZT Centrer sur la ligne
//   Entrée    : str, chaine de caractères AZT
//               ligne, numéro de la ligne 0 ou 1
//   Sortie    : --
//-*---------------------------------------------------------------------------*/
//extern void AfficherChaineAZTCentreLi(char str[], unsigned char ligne);
//
///*---------------------------------------------------------------------------*-
//   EffacerEcran ()
//  -----------------------------------------------------------------------------
//   Descriptif: Efface l'écran LCD
//   Entrée    : --
//   Sortie    : --
//-*---------------------------------------------------------------------------*/
extern void EffacerEcran ();


#pragma RESTORE
#endif                                 /* #define __OLED_128X64_H__         */