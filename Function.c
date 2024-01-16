/**************************************************************************
* FONCTION.C :
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
* version : 1.5
* auteur : Michele Schiavo
* date : 03.12.2015
* corrig� PWM servos ajout� fonction Mesure_Tension_Batterie
**
* version : 1.8
* auteur : Carbone / Anderes
* date : 26 novembre 2019
* Ajouts de commandes RS232
*
* version : 1.9
* auteur : Didier Moullet
* date : 26 d�cembre 2019
* Ajouts de commandes RS232
* Ajout du mode transparent . Les informations remontent 
* via le RS232 de la carte moteur
*
**********************************************************************
* Commentaires : Ce module contient toutes les fonctions diverses pour 
* piloter la carte F8C
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
 version : 2.01
* auteurs : DC/JMA
* date : 24 mars 2021
* Refonte du projet pour correspondre � la structure utilis�e par nos �l�ves
* Ajout structure I2C lib ecole


* version : 2.02
* auteurs : DC/JMA
* date : 5 d�cembre 2021
* Mise � jour de la librairie PCA9685

**********************************************************************/

#define EXTERN
#include <declar.h>	// D�claration des variables globales et fonctions pour le projet

#include <stdio.h>	// Cette fonction standard est universelle quel que soit
										// le microcontr�leur utilis� et le compilateur utilis�
#include <math.h>
#include <string.h>
#include <stdlib.h>			// Librairie standard C

#define CAPT_LIGNE_ADDRESS 0x40

code unsigned char txt_AS[2]="AS";			// Commanse AS,
code unsigned char  txt_SE1[3]="SE1";			   // Command SE, pilote le servo 1 (0-180�)
code unsigned char  txt_SE2[3]="SE2";			   // Command SE, pilote le servo 2 (0-180�)
code unsigned char	txt_VE[2]="VE";			// Commande VE, envoie la date et l'heure de compilation 
code unsigned char	txt_HELP[4]="HELP";         // Continue (reprise apr�s pause)
code unsigned char	txt_XO[2]="XO";			// Commande XO, envoie XOFF 
code unsigned char	txt_XN[2]="XN";			// Commande XN, envoie XON 
code unsigned char	txt_CL[2]="CL";			// Commande CL, retourne l'état du capteur de ligne 
code unsigned char	txt_WR[2]="WR";

unsigned char ligne; //Variable pour le suivi de ligne

//unsigned int tension_batterie;

/****************************************************
* Fonction delai_ms()																   *
* Type : void																				*
* Input : ms																				*
* Output : -																				*
* Attend par unit� de 1ms		(Timer 5 configur� en auto-reload	   	*
****************************************************/
void Delai_ms(long ms)
{ 
   long var_loc; 
   
   SFRPAGE   = CONFIG_PAGE;
   
//	TMR3CN&=0xFE;			// T3XCLK -> Fclock = Fclock / 12 (T clock = 1000ns)
//	TMR3RLL=0x18;			// 1ms/1000ns=1000
//	TMR3RLH=0xFC;			// FFFF-3E8 = F830
//   TMR3RLL=0x20;			// 1ms/83ns=12000
//	TMR3RLH=0xD1;			// 65236-53536 = 12000 --> 0xD120
   
	TMR5CN|= 0x04;		// Timer 5 d�marr�
	for(var_loc=0; var_loc<ms;var_loc++)
		{
			while(!(TMR5CN&0x80));	// Attend fin du comptage TF5H ? =1
			TMR5CN&=0x7F;						// Efface TF3H
		}
	TMR5CN&= (~0x04);						// Timer 3 stopp�
      
   SFRPAGE   = LEGACY_PAGE;    
}
/****************************************************
* Fonction : Lecture_COMM														*
* Type : void																				*
* Input : -																					*
* Output : -																				*
* Va lire les ports COMM 0 (PC) et 1 (moteur)				*
****************************************************/
void Lectures_COMM()
{
	Lecture_RS232_UART0();
	Lecture_RS232_UART1();
}

/****************************************************
* Fonction : Lecture_RS232() de l'UART0							*
* Type : void																				*
* Input : -																					*
* Output : -																				*
* D�code les commandes RS232 de l'UART 0						*
****************************************************/
void Lecture_RS232_UART0()
{
		indice_string=0;
		/****************************************************/
		/* Y a-t-il des commandes qui sont arriv�es ? 			*/
		/****************************************************/
		if (command_uart>0) 											// Une nouvelle commande est l�, d�code l� !
		{
			adr_pointer_buffer_uart=&receive_uart[read_buffer_uart];		// L'adresse o� se trouve la commande pendante
			/********************************************************************
			// Converti les caract�res re�u dans le buffer ciculaires en une chaine de caract�re
			// pour que le traitement de la commande soit fait correctement
			*/
			while (receive_uart[read_buffer_uart]!='\r')
			{
				/* Cr�e la chaine de carct�re qui sera test�e */
				commande_string[indice_string]=receive_uart[read_buffer_uart];
				receive_uart[read_buffer_uart]=0x00;			// Efface (=0) ce caract�re
				indice_string++;		// Caract�re suivant dans la chaine de caract�re
				read_buffer_uart++;	// Caract�re suivant dans le buffer
			}
			receive_uart[read_buffer_uart]=0x00;			// Efface (=0) ce caract�re
			read_buffer_uart++;	// Pour que read_buffer_uart=write_buffer_uart, car la commande est d�cod�e
			commande_string[indice_string]='\r';
			indice_string++;
			commande_string[indice_string]='\0';// Fin de chaine
			indice_string++;
			// G�n�re une erreur et n'envoie pas la ligne si on est plus long que la longueur maximum d�finie (LONGUEUR_COMMANDE_MAX)
			if (indice_string>LONGUEUR_COMMANDE_MAX) 
			{
				commande_string[0]='\r';
				commande_string[1]='\0';	// Si la chaine est plus grande que la valeur MAX autoris�e par une ligne, alors erreur !!! 
			}
			command_uart--;			// La commande a �t� d�cod�e et effac�e.
			if (!(strncmp(commande_string,txt_VE,2))) 	// The VE command is recognized
			{
				unsigned char code date[] = __DATE__;
				unsigned char code time[] = __TIME__;
           
            Send_string("\n\r",PC_BLUETOOTH);
            Send_string("- UART utilise : ",PC_BLUETOOTH);
            Send_number(PC_BLUETOOTH,PC_BLUETOOTH);
            Send_string(" (carte strategie)\n\r",PC_BLUETOOTH);
            Send_string("- Firmware Strategie version : ",PC_BLUETOOTH);
            Send_number(VERSION,PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            Send_string("- Date de compilation : ",PC_BLUETOOTH);
            Send_string(date,PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            Send_string("- Heure de compilation : ",PC_BLUETOOTH);
            Send_string(time,PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
				    Send_string("\n\r",PC_BLUETOOTH);
				    Send_string("- UART utilise : ",PC_BLUETOOTH);
            Send_number(UART1,PC_BLUETOOTH);
				    Send_string(" (carte moteur)\n\r",PC_BLUETOOTH);
						Send_string(commande_string, UART1);					// Take the value
			}
         
         else if (!(strncmp(commande_string,txt_HELP,4))) 	// The HELP command is recognized
			{
            
            Send_string("\n\r",PC_BLUETOOTH);
            Send_string("- COMMANDES RECONNUES : ",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            
				Send_string("Avance\t\t\t\t\t",PC_BLUETOOTH);
            Send_string("AVxx, xx est en [mm]",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            
				Send_string("Recule\t\t\t\t\t",PC_BLUETOOTH);
            Send_string("RExx, xx est en [mm]",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            
				Send_string("Tourne a Droite\t\t\t\t",PC_BLUETOOTH);
            Send_string("DRxxx, xxx est en [degres]",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
				
				Send_string("Tourne a Gauche\t\t\t\t",PC_BLUETOOTH);
            Send_string("GAxxx, xxx est en [degres]",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            
				Send_string("Vitesse roue droite\t\t\t",PC_BLUETOOTH);
            Send_string("VDxx, xx en [%]",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            
				Send_string("Vitesse roue gauche\t\t\t",PC_BLUETOOTH);
            Send_string("VGxx, xx en [%]",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);

				Send_string("Positionne Servo 1\t\t\t",PC_BLUETOOTH);
            Send_string("SE1xx, xxx est en [us]",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            
				Send_string("Positionne Servo 2\t\t\t",PC_BLUETOOTH);
				Send_string("SE2xx, xxx est en [us]",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            
				Send_string("Renvoie les valeurs fixes du robot\t",PC_BLUETOOTH);
				Send_string("VA (sans valeur)",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            
				Send_string("Modifie la vitesse du robot\t\t",PC_BLUETOOTH);
				Send_string("VIxx, xx est en [%] ",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            						
				Send_string("Modifie l'acceleration du robot\t\t",PC_BLUETOOTH);
            Send_string("ACxx, xx est l'acceleration ",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            
				Send_string("Stoppe le robot\t\t\t\t",PC_BLUETOOTH);
            Send_string("ST ",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            
				Send_string("Renvoie les version des firmware\t",PC_BLUETOOTH);
            Send_string("VE ",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            						
				Send_string("Mets en pause le robot\t\t\t",PC_BLUETOOTH);
            Send_string("PA ",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
           
				Send_string("Continue apres une pause\t\t",PC_BLUETOOTH);
            Send_string("CO ",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
						
				Send_string("Affiche valeurs asservissement\t\t",PC_BLUETOOTH);
            Send_string("AS ",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
						
				Send_string("Affiche valeurs encodeurs\t\t",PC_BLUETOOTH);
            Send_string("EC ",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
						
				Send_string("Affiche le courant des moteurs\t\t",PC_BLUETOOTH);
            Send_string("MC ",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
            
				Send_string("Affiche la tension de la batterie\t",PC_BLUETOOTH);
            Send_string("MT ",PC_BLUETOOTH);
            Send_string("\n\r",PC_BLUETOOTH);
						
			}
 			else if (!(strncmp(commande_string,txt_AS,2))) 	// The AS command is recognized
			{ 
				// La commande EC va renvoyer du texte que l'on va transmettre au PC...
				Send_string("EXPLICATIONS\n\r",PC_BLUETOOTH);
				
				Send_string("Acc\t\t",PC_BLUETOOTH);
				Send_string("Acceleration",PC_BLUETOOTH);
				Send_string("\n\r",PC_BLUETOOTH);

				Send_string("Vit\t\t",PC_BLUETOOTH);
				Send_string("Vitesse",PC_BLUETOOTH);
				Send_string("\n\r",PC_BLUETOOTH);

				Send_string("Vit tour \t",PC_BLUETOOTH);
				Send_string("Vitesse tourne",PC_BLUETOOTH);
				Send_string("\n\r",PC_BLUETOOTH);

				Send_string("CP\t\t",PC_BLUETOOTH);
				Send_string("Coeff proportionnel",PC_BLUETOOTH);
				Send_string("\n\r",PC_BLUETOOTH);
				
				Send_string("CI\t\t",PC_BLUETOOTH);
				Send_string("Coeff integral",PC_BLUETOOTH);
				Send_string("\n\r",PC_BLUETOOTH);
				
				Send_string("CD\t\t",PC_BLUETOOTH);
				Send_string("Coeff differentiel",PC_BLUETOOTH);
				Send_string("\n\r",PC_BLUETOOTH);

				Send_string("\tSi ces valeurs depassent 100, alors la regul n'est pas assuree\n\r",PC_BLUETOOTH);

				Send_string("CD100\t\t",PC_BLUETOOTH);
				Send_string("Nbre de corr a 100% droite",PC_BLUETOOTH);
				Send_string("\n\r",PC_BLUETOOTH);

				Send_string("CG100\t\t",PC_BLUETOOTH);
				Send_string("Nbre de corr a 100% gauche",PC_BLUETOOTH);
				Send_string("\n\r",PC_BLUETOOTH);

				Send_string("\tSi ces valeurs depassent 100, alors la regul n'est pas assuree\n\r",PC_BLUETOOTH);
				Send_string("MaxcorD\t\t",PC_BLUETOOTH);
				Send_string("Valeur calculee correction droite",PC_BLUETOOTH);
				Send_string("\n\r",PC_BLUETOOTH);
				
				Send_string("MaxcorG\t\t",PC_BLUETOOTH);
				Send_string("Valeur calculee correction gauche",PC_BLUETOOTH);
				Send_string("\n\r",PC_BLUETOOTH);
				
				Send_string("\tValeurs min. PWM pour bouger le robot\n\r",PC_BLUETOOTH);
				Send_string("Zne_AV\t\t",PC_BLUETOOTH);
				Send_string("PWM minimum pour avancer",PC_BLUETOOTH);
				
				Send_string("\n\r",PC_BLUETOOTH);
				Send_string("Zne_RE\t\t",PC_BLUETOOTH);
				Send_string("PWM minimum pour reculer",PC_BLUETOOTH);
				
				Send_string("\n\r",PC_BLUETOOTH);
				Send_string("Zne_DR\t\t",PC_BLUETOOTH);
				Send_string("PWM minimum pour tourner a droite",PC_BLUETOOTH);
				Send_string("\n\r",PC_BLUETOOTH);
				
				Send_string("Zne_GA\t\t",PC_BLUETOOTH);
				Send_string("PWM minimum pour tourner a gauche",PC_BLUETOOTH);
				Send_string("\n\r",PC_BLUETOOTH);
				Send_string(commande_string, UART1);					// Take the value
			}
			else if (!(strncmp(commande_string,txt_SE1,3))) 	// Commande SE1
			{
				unsigned int dummy;
				// EXEMPLE DE RECUPERATION DE VALEUR NUMERIQUE
				adr_pointer_buffer_uart=&commande_string;
				dummy = atoi (adr_pointer_buffer_uart+3);						// R�cup�re la valeur
				Cmd_Servo(1,(unsigned int)dummy);
				Send_string("\n\r",PC_BLUETOOTH); // Pour sauter la ligne 
			}
			else if (!(strncmp(commande_string,txt_SE2,3))) 	// Commande SE2
			{
				unsigned int dummy;
				// EXEMPLE DE RECUPERATION DE VALEUR NUMERIQUE
				adr_pointer_buffer_uart=&commande_string;
				dummy = atoi(adr_pointer_buffer_uart+3);						// R�cup�re la valeur
				Cmd_Servo(2 ,(unsigned int)dummy);
				Send_string("\n\r",PC_BLUETOOTH); // Pour sauter la ligne 
			}
			else if (!(strncmp(commande_string,txt_XO,2))) 	// The XOFF command is recognized
			{
				Putc_uart(XOFF,CARTE_MOTEUR);
			}
			else if (!(strncmp(commande_string,txt_XN,2))) 	// The XON command is recognized
			{ 
				Putc_uart(XON,CARTE_MOTEUR); 
			}
			else if (!(strncmp(commande_string,txt_CL,2))) 	// The CL command is recognized
			{ 
            ligne = Lire_Ligne(CAPT_LIGNE_ADDRESS); // fix: lag de la valeur de 1 lecture
            Delai_ms(10);
            
				ligne = Lire_Ligne(CAPT_LIGNE_ADDRESS);
            if(ligne < 16)
            {
               Send_string("0",PC_BLUETOOTH); // add trailing Zero
               Send_string(itoa(ligne, 16),PC_BLUETOOTH); 
            }
            else
            {
            Send_string(itoa(ligne, 16),PC_BLUETOOTH); // no need to add trailing Zero
         }
            
				Send_string("\n\r",PC_BLUETOOTH); // Pour sauter la ligne 
			}
			else if (!(strncmp(commande_string,txt_WR,2))) 	// The XON command is recognized
			{
				Delai_ms(1);
				while (Roule);
				Send_string("\n\rDONE",PC_BLUETOOTH); // Pour sauter la ligne 
			}
// On a d�cod� toutes les commandes qui pilotent la carte Strat�gie, si pas trouv�, alors envoie la contenu de string_commande � la carte moteur  
			else
			{
				Send_string(commande_string, CARTE_MOTEUR);					// Take the value
            
            // Robot already send NC +  ^J
             
            //Delai_ms(50); // wait to be sure the flag is set
            // while(Roule);
            // Send_string("OK\n\r",PC_BLUETOOTH); // Pour indiquer à l'esp32 stratégie qu'on a fini !
			}
	}
}

/****************************************************
* Fonction : Lecture_RS232() de l'UART1							*
* Type : void																				*
* Input : -																					*
* Output : -																				*
* Fait �cho dde ce qui se passe sur l'UART1					*
* (carte moteur)																		*
****************************************************/
void Lecture_RS232_UART1()
{
		indice_string1=0;
		/****************************************************/
		/* Y a-t-il des informations quii sont arriv�es ? 			*/
		/****************************************************/
		if (command_uart1>0) 							// Une nouvelle commande est l�, d�code l� !
		{
			/********************************************************************
			// Converti les caract�res re�u dans le buffer ciculaires en une chaine de caract�re
			// pour que le traitement de la commande soit fait correctement
			*/
			while (receive_uart1[read_buffer_uart1]!='\r')
			{
				/* Cr�e la chaine de carct�re qui sera test�e */
				commande_string1[indice_string1]=receive_uart1[read_buffer_uart1];
				receive_uart1[read_buffer_uart1]=0x00;			// Efface (=0) ce caract�re
				indice_string1++;		// Caract�re suivant dans la chaine de caract�re
				read_buffer_uart1++;	// Caract�re suivant dans le buffer
			}
			receive_uart1[read_buffer_uart1]=0x00;			// Efface (=0) ce caract�re
			read_buffer_uart1++;	// Pour que read_buffer_uart=write_buffer_uart, car la commande est d�cod�e
			commande_string1[indice_string1]='\r';
			indice_string1++;
			commande_string1[indice_string1]='\0';// Fin de chaine
			indice_string1++;
			// G�n�re une erreur et n'envoie pas la ligne si on est plus long que la longueur maximum d�finie (LONGUEUR_COMMANDE_MAX)
			if (indice_string1>LONGUEUR_COMMANDE_MAX) 
			{
				commande_string1[0]='\r';
				commande_string1[1]='\0';	// Si la chaine est plus grande que la valeur MAX autoris�e par une ligne, alors erreur !!! 
			}
			command_uart1--;			// La commande a �t� d�cod�e et effac�e.
			Send_string(commande_string1,UART0);
		}
}

/****************************************************
* Procedure Commande_servo							      *
* Type : void										         *
* Input : servo_num, temps pulse en us					*
* Output : -										         *
* 													            *
****************************************************/
void Cmd_Servo( unsigned char servo_num, unsigned int pulse)
{  
   if(servo_num == 1)
   {
      PCA9685_setPulse_us(1,PCA9685_ADDRESS,0, pulse);   //Servo H-B M
   }
   
   if(servo_num == 2)
   {
      PCA9685_setPulse_us(1,PCA9685_ADDRESS,1, pulse);   //Servo H-B M
   }
     
}

/****************************************************
* Procedure Lire_Ligne							          *
* Type :                               				 *
* Input : adresse du capteur						       *
* Output : valeur sur 8 bits de la ligne	          *
* 													             *
****************************************************/ 
unsigned char Lire_Ligne(unsigned char adresse)
{
idata unsigned char buf[2]; //Pour comm SMBUS 
   
   buf[0] = 0x1;
   I2CWrite(1, adresse, buf, 1);    
   I2CRead(1,adresse,buf,1);
   return buf[0]; 
}


/****************************************************
* Procedure Calibration_capteur				          *
* Type :                               				 *
* Input : adresse du capteur						       *
* Output :                             	          *
* 													             *
****************************************************/ 
void Calibration_capteur(unsigned char adresse)
{
idata unsigned char buf[2]; //Pour comm SMBUS 
   
   buf[0] = 0x14;
   I2CWrite(1, adresse, buf, 1);    
}

/******************************************************************
* Procedure Code_Rouge
* G�re le PCA9685 et le capteur de ligne pour envoi de la couleur *
* Type :                               				               *
* Input :  						                                       *
* Output :                             	                        *
* 													                           *
******************************************************************/ 
void  Code_Rouge()
{
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,4,99); //Rouge1
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,5,0);  //Vert1
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,6,0);  //Blue1
   
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,7,99); //Rouge2
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,8,0);  //Vert2
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,9,0);  //Blue2
   //Envoi le code sur le suiveur
}   

/******************************************************************
* Procedure Code_Vert
* G�re le PCA9685 et le capteur de ligne pour envoi de la couleur *
* Type :                               				               *
* Input :  						                                       *
* Output :                             	                        *
* 													                           *
******************************************************************/ 
void  Code_Vert()
{
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,4,0);  //Rouge1
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,5,99); //Vert1
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,6,0);  //Blue1
   
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,7,0);  //Rouge2
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,8,99); //Vert2
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,9,0);  //Blue2
   //Envoi le code sur le suiveur
} 


/******************************************************************
* Procedure Code_Bleu
* G�re le PCA9685 et le capteur de ligne pour envoi de la couleur *
* Type :                               				               *
* Input :  						                                       *
* Output :                             	                        *
* 													                           *
******************************************************************/ 
void  Code_bleu()
{
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,4,0);  //Rouge1
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,5,0);  //Vert1
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,6,99); //Blue1
   
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,7,0);  //Rouge2
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,8,0);  //Vert2
   PCA9685_setDutyCycle(1,PCA9685_ADDRESS,9,99); //Blue2
   //Envoi le code sur le suiveur
}


void StartCountDown(void)
{
	SFRPAGE   = CONFIG_PAGE;
   TMR4CN&=0xFE;			// T3XCLK -> Fclock = Fclock / 12 (T clock = 1000ns)
	TMR4RLL=0xA0;			// 60ms/1000ns=60000
	TMR4RLH=0x15;			// FFFF-EA60 = 15A0
	TMR4CN|= 0x04;		// Timer 4 d�marr�
   SFRPAGE   = LEGACY_PAGE;
}

char* itoa(int val, int base)
{
    static char buf[32] = {0};

    int i = 30;

    for(; val && i ; --i, val /= base)

        buf[i] = "0123456789abcdef"[val % base];

    return &buf[i+1];

}