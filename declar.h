/**********************************************************************
* declar.h :
*
* CFPT Ecole d'�lectronique 
* salle : 1.32/1.33
* auteur : Didier Moullet
* revision : 1.0
* date: 24.9.2014
* revision : 1.1
* auteur : Michele Schiavo
* date : 18.10.2014
* ajout de la gestion des servos sur le port 1 du F38C
*
*
* version : 1.8
* auteur : Carbone / Anderes
* date : 26 novembre 2019
* Ajouts de commandes RS232
*
* version : 2.0
* auteur : Didier Moullet
* date : 26 f�vrier 2021
* Refonte des comm RS232, inclusion du Handsahke XON-XOFF
* Enlever tous les d�codages des commandes ne concernant que la carte moteur
* Il faut maintenant que les �l�ves code les commandes pour faire bouger le robot 
* 
* Exemple pour avance(1000) : Send_string("AV1000\r",CARTE_MOTEUR);
* Cela implique que les �l�ves doivent connaitre toutes les commandes de la cartes moteur.
*
* uart_number = 0 -> Communication avec le PC 
* uart_number = 1 -> Communication avec la carte moteur du robot


* version : 2.01
* auteurs : DC/JMA
* date : 24 mars 2021
* Refonte du projet pour correspondre � la structure utilis�e par nos �l�ves
* Ajout structure I2C lib ecole


* version : 2.02
* auteurs : DC/JMA
* date : 5 d�cembre 2021
* Mise � jour de la librairie PCA9685




**********************************************************************/

#include <REG51F380.H>				// sfr declarations

#include ".\CFPT_uC_lib\SMBUS.h"
#include ".\CFPT_uC_lib\PCA9685.h"


#define VERSION      2.03
#define CONFIG_PAGE 0x0F
#define LEGACY_PAGE 0x00

// --------------------------------------------------------------------
// D�finition des constantes
// --------------------------------------------------------------------

#define ENTREES_0 P0
#define ENTREES_1 P1
#define ENTREES_2 P2


#define FALSE				0
#define SYSCLK       48000000          // SYSCLK frequency in Hz 12000000 or 48000000
#define TRUE				1
#define XON					0x11
#define XOFF				0x13

#define UART0				0					// Uart 0, PC ou Bluetooth
#define PC_BLUETOOTH	0
#define UART1				1					// Uart 1, carte moteur
#define CARTE_MOTEUR 1				// UART 1 = carte moteur

#define RI1					0x01			// SCON1 n'est pas bit adressable
#define TI1					0x02


#define LONGUEUR_COMMANDE_MAX		80		// Nombre maximum de caract�re pour une commande !!!



extern bit getLineFollowerFlag();

extern void setLineFollowerFlag(bit newValue);

/***********/
/* p0 port */
/***********/
// P0.0  -  TX1 (UART1), Open-Drain, Digital
// P0.1  -  RX1 (UART1), Open-Drain, Digital
//sbit SW_AV =P0^2;		//Switch � l'avant du Robot
//sbit SW_AR =P0^3;		//Switch � l'arri�re du Robot
// P0.4  -  TX0 (UART0), Open-Drain, Digital
// P0.5  -  RX0 (UART0), Open-Drain, Digital
//sbit CAP_TR =P0^6;		//Capteur de transparence
//sbit Roule= P0^7;

/***********/
/* P1 port */
/***********/
//sbit Free=P1^0;  !! occup� par servo1
//sbit Free=P1^1;  !! occup� par servo2
sbit Led_verte=P1^2;
sbit Led_jaune=P1^3;
sbit Led_rouge=P1^4;
//sbit Free=P1^5;


/***********/
/* P2 port */

/***********/
sbit Tirette=P2^0;      // Tirette pour d�part
sbit Bp1=P2^1;
sbit Bp2=P2^2;
sbit Bp3=P2^3;
sbit Roule=P2^4;


/***********/
/* P3 port */
/***********/

//sbit Free=P3^0;

#ifdef PUBLIC		/********************** main program ******************************/

// --------------------------------------------------------------------
// D�claration des variables globales et des fonctions
// --------------------------------------------------------------------

// --------------------------------------------------------------------
// D�finitions des fonctions utilis�es 
// --------------------------------------------------------------------

void			Delai_ms(long ms);					// attend x ms...
void			Init_Device(void);					// Initialise le F38C
void			Lectures_COMM(void);						// Va lire les ports de communication (UART0 et UART1)
void			Lecture_RS232_UART0(void);				// Decode les commandes RS232 UART0
void			Lecture_RS232_UART1(void);						// Remonte les informartions de la carte moteur
void			Putc_uart (unsigned char dat, bit uart_number);		// Envoie un charactere dans l'UART
void			Send_float (float number,bit uart_number);	// Envoie un flottant avec 3 chiffres apr�s la virgule
void			Send_number(long number,bit uart_number);		// Envoie un chiffre nentier via RS232
void			Send_string (unsigned char *pointer,bit uart_number);		// Envoie une chaine de caract�re dans l'UART
void        Cmd_Servo( unsigned char servo_num, unsigned int pulse); // commande servo HS422	
unsigned char Lire_Ligne(unsigned char adresse); //R�cup�re la valeur lue du capteur
void        Calibration_capteur(unsigned char adresse); //Lance une calibration du capteur
void        Code_Rouge();  //G�re le PCA9685 et le capteur de ligne pour envoi de la couleur
void        Code_Vert();   //G�re le PCA9685 et le capteur de ligne pour envoi de la couleur
void        Code_Bleu();   //G�re le PCA9685 et le capteur de ligne pour envoi de la couleur

extern char* itoa(int val, int base);

// --------------------------------------------------------------------
// Variables globales 
// --------------------------------------------------------------------

unsigned char	dummy1;					// Variable � usage g�n�ral



// UART0 commande via le bluetooth ou le cable USB
unsigned char	command_uart;						// Nbre de commandes pendante dans le buffer
unsigned char	read_buffer_uart;				// Pointeur de lecture dans le buffer des caract�res
unsigned char	write_buffer_uart;			// Pointeur de l'�criture des caract�res dasn le buffer
// Handshake UART0 RS232 variables
bit XOFF_received;
bit XOFF_sent;
bit	write_char;			// Un caract�re est dans le buffer d'envoi (0 ou 1)
signed int	etat_buffer;				// Si cette valeur d�passe 80% de 256, alors on envoie un XOFF,
																// puis on attend que cette valeur passe en dessous de 20% pour envoyer un XON
signed int read_position;
signed int write_position;

unsigned char indice_string;
char commande_string[LONGUEUR_COMMANDE_MAX];		// C'est dans ce tableau que la commande re�ue est test�e


/* arrays */
unsigned char xdata 	receive_uart[256];	// Buffer circulaire (256 bytes)
/* pointer */
unsigned char	*adr_pointer_buffer_uart;		// Pointeur du buffer

// UART1
unsigned char	command_uart1;						// Nbre de commandes pendante dans le buffer
unsigned char	read_buffer_uart1;				// Pointeur de lecture dans le buffer des caract�res
unsigned char	write_buffer_uart1;			// Pointeur de l'�criture des caract�res dasn le buffer
// Handshake UART1 RS232 variables
bit XOFF_received1;
bit XOFF_sent1;
bit	write_char1;			// Un caract�re est dans le buffer d'envoi (0 ou 1)
signed int	etat_buffer1;				// Si cette valeur d�passe 80% de 256, alors on envoie un XOFF,
																// puis on attend que cette valeur passe en dessous de 20% pour envoyer un XON
signed int read_position1;
signed int write_position1;

unsigned char indice_string1;	// C'est dans ce tableau que la commande re�ue est test�e
char commande_string1[LONGUEUR_COMMANDE_MAX];
/* arrays */
unsigned char xdata 	receive_uart1[256];	// Buffer circulaire (256 bytes)

/* servo */

unsigned char index;
unsigned char msk;

/* countdown */
unsigned int countdown;

#else	
/**********************************************************************************/
/* D�claration des variables pour les modules externes au MAIN (autres fichiers) 	*/
/* pour faire simple, il suffit de mettre le mot cl� EXTERN devant								*/
/* la d�claration de chaque variable ci dessus																		*/
/**********************************************************************************/

// --------------------------------------------------------------------
// D�finitions des fonctions utilis�es 
// --------------------------------------------------------------------

extern void			Delai_ms(long ms);					// attend x ms...
extern void			Init_Device(void);					// Initialise le F38C
extern void			Lectures_COMM(void);						// Va lire les ports de communication (UART0 et UART1)
extern void			Lecture_RS232_UART0(void);				// Decode les commandes RS232 UART0
extern void			Lecture_RS232_UART1(void);						// Remonte les informartions de la carte moteur
extern void			Putc_uart (unsigned char dat,bit uart_number);		// Envoie un charactere dans l'UART
extern void			Send_float (float number,bit uart_number);	// Envoie un flottant avec 3 chiffres apr�s la virgule
extern void			Send_number(long number,bit uart_number);		// Envoie un chiffre nentier via RS232
extern void			Send_string (unsigned char *pointer,bit uart_number);		// Envoie une chaine de caract�re dans l'UART
extern void       Cmd_Servo( unsigned char servo_num, unsigned int pulse); // commande servo HS422	
extern unsigned char Lire_Ligne(unsigned char adresse); //R�cup�re la valeur lue du capteur
extern void       Calibration_capteur(unsigned char adresse); //Lance une calibration du capteur
extern void        Code_Rouge(); //G�re le PCA9685 et le capteur de ligne pour envoi de la couleur
extern void        Code_Vert();  //G�re le PCA9685 et le capteur de ligne pour envoi de la couleur
extern void        Code_Bleu();  //G�re le PCA9685 et le capteur de ligne pour envoi de la couleur


extern char* itoa(int val, int base);
// --------------------------------------------------------------------
// Variables globales 
// --------------------------------------------------------------------

extern unsigned char	dummy1;					// Variable � usage g�n�ral
// UART0
extern unsigned char	command_uart;						// Nbre de commandes pendante dans le buffer
extern unsigned char	read_buffer_uart;				// Pointeur de lecture dans le buffer des caract�res
extern unsigned char	write_buffer_uart;			// Pointeur de l'�criture des caract�res dasn le buffer

// Handshake UART0 RS232 variables
extern bit XOFF_received;
extern bit XOFF_sent;
extern bit	write_char;			// Un caract�re est dans le buffer d'envoi (0 ou 1)
extern signed int	etat_buffer;				// Si cette valeur d�passe 80% de 256, alors on envoie un XOFF,
																// puis on attend que cette valeur passe en dessous de 20% pour envoyer un XON
extern signed int read_position;
extern signed int write_position;

extern unsigned char indice_string;
extern char commande_string[LONGUEUR_COMMANDE_MAX];		// C'est dans ce tableau que la commande re�ue est test�e

/* arrays */
extern unsigned char xdata 	receive_uart[256];	// Buffer circulaire (256 bytes)
/* pointer */
extern unsigned char	*adr_pointer_buffer_uart;		// Pointeur du buffer

// UART1
extern unsigned char	command_uart1;						// Nbre de commandes pendante dans le buffer
extern unsigned char	read_buffer_uart1;				// Pointeur de lecture dans le buffer des caract�res
extern unsigned char	write_buffer_uart1;			// Pointeur de l'�criture des caract�res dasn le buffer
// Handshake UART1 RS232 variables
extern bit XOFF_received1;
extern bit XOFF_sent1;
extern bit	write_char1;			// Un caract�re est dans le buffer d'envoi (0 ou 1)
extern signed int	etat_buffer1;				// Si cette valeur d�passe 80% de 256, alors on envoie un XOFF,
																// puis on attend que cette valeur passe en dessous de 20% pour envoyer un XON
extern signed int read_position1;
extern signed int write_position1;

extern unsigned char indice_string1;	// C'est dans ce tableau que la commande re�ue est test�e
extern char commande_string1[LONGUEUR_COMMANDE_MAX];

/* arrays */
extern unsigned char xdata 	receive_uart1[256];	// Buffer circulaire (256 bytes)


/* servo */

extern unsigned char index;
extern unsigned char msk;

/* countdown */
extern unsigned int countdown;

#endif
