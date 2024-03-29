/**********************************************************************
* INT.C :
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
*
*
* Commentaires : Toutes les fonctions d'interruptions se trouvent dans ce fichier.
*
*
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
#include <declar.h>	// D�claration des variables globales et fonctions pour le projet#include <declar.h> 
#include <ctype.h>

/***************************************************************************************************
*			  	    			 I N T	F O N C T I O N S        			   					   *
***************************************************************************************************/


/*************************************************
*
* Fontion  :Int_uart0
* Type : Il se passe quelque chose dans SBUF0
* Input : SBUF0
* Output : la variable command_uart est incr�ment�e si un CR 
*          est d�tect�. 
* Declaration : -
*
* Commentaires : Cette fonction est appel�e via RI0 et TI0
*           ces flags sont remis � 0 dans cette fonction
*           Je ne traite pas les erreurs de transmissions 
*
****************************************************/
void Int_uart0(void) interrupt 4
{
	bit out=0;
	char dummy=0;
	bit new_command_arrived_UART0=0;
	// R�ception de caract�res 
	if (RI0)
	{
		// Lit le carac�re re�u dans SBUF0 et met en majuscule le caract�re re�u...
		receive_uart[write_buffer_uart]=toupper(SBUF0);
		// ********************************************
		// On a re�u un XOFF attend de recevoir un XON au prochain caract�re re�u
		// ********************************************
		if (receive_uart[write_buffer_uart]==XOFF) // Oups on re�oit un XOFF... alors stoppe l'envoi des caract�re jusqu'au XON
																							 // Et surtout n'envoie plus rien !!! jusqu'au XON 
		{
			// Attention on d�pend du PC ici qui doit absolument travailler avec du Handshake..sinon on bloque tout ici
			XOFF_received=1;
		}
		if (XOFF_received)
		{
			/**********************************************
			/*On a re�u un XON, alors on peut repartir... *
			/**********************************************/
			if (receive_uart[write_buffer_uart]==XON)
			{
				XOFF_received=0;
			}
		}
		else	// R�ception normale....
		{
			if (XOFF_sent==0) // J'ai envoy� un XOFF, alors ne m�morise plus ce qui arrive 
				// On perdra des caract�re, mais cela n'est pas grave...car un XOFF est parti !
			{
				if (receive_uart[write_buffer_uart]==0x0D)	// D�code CR = 0x0D = \r
				{
					command_uart++;									// Une nouvelle commande est arriv�e!
					new_command_arrived_UART0=1;		// Ce flag indique qu'une nouvelle commande est arriv�e.
				}
				write_buffer_uart++;  
			}
			// Handshake avec XOFF- XON 
			// Met dans des variables int les valeurs des pointeurs lecture et �criture
			read_position=0x00FF&read_buffer_uart;
			write_position=0x00FF&write_buffer_uart;
			etat_buffer=write_position-read_position;
			if (etat_buffer<0)
			{
				etat_buffer=etat_buffer+256;	// Si on est n�gatif, alors ajoute 256
			}
			// Attention 70% du buffer est plein....
			if ((etat_buffer>179)&&(XOFF_sent==0))
			{
				TI0=0;
				SBUF0 = XOFF;			// Le buffer erst presque plein > 70%, alors envoie un XOFF
				while(TI0);
				TI0=0;
				XOFF_sent=1;
			}
			// Envoie XON seulement si un XOFF a �t� envoy� auparavant...
			// ET si le buffer est <20%...
			if ((etat_buffer<50)&&(XOFF_sent)) 
			{
				TI0=0;
				SBUF0 = XON;	;		// Envoie un XON, car le buffer est presque vide...
				while(TI0);
				TI0=0;
				XOFF_sent=0;	// Reparti pour un XOFF si besion 
			}
		}
		RI0=0;								// Flag RI0=0
	}
	// Emission de caract�res
	if (TI0)
	{
		TI0=0;
		write_char=0;						// La transmission s'est bien d�roul�e,
														// On peut passer au caract�re suivant si besion
	}
}
/*************************************************
*
* Fontion : Int_uart1
* Type : Il se passe quelque chose dans SBUF1
* Input : SBUF1
* Output : la variable command_uart est incr�ment�e si un CR 
*          est d�tect�. 
* Declaration : -
*
* Commentaires : Cette fonction est appel�e via RI1 et TI1
*           ces flags sont remis � 0 dans cette fonction
*           Je ne traite pas les erreurs de transmissions 
*
****************************************************/
void Int_uart1(void) interrupt 16
{
	bit out=0;
	char dummy=0;
	bit new_command_arrived_UART1=0;
	// R�ception de cract�res
	if ((SCON1&RI1)==RI1)
	{
		receive_uart1[write_buffer_uart1]=SBUF1;
		// ********************************************
		// On a re�u un XOFF attend de recevoir un XON au prochain caract�re re�u
		// ********************************************
		if (receive_uart1[write_buffer_uart1]==XOFF) // Oups on re�oit un XOFF... alors stoppe l'envoi des caract�re jusqu'au XON
																								// Et surtout n'envoie plus rien !!! jusqu'au XON 
		{
			// Attention on d�pend du PC ici qui doit absolument travailler avec du Handshake..sinon on bloque tout ici
			XOFF_received1=1;
		}
		if (XOFF_received1)
		{	
			/**********************************************
			/*On a re�u un XON, alors on peut repartir... *
			/**********************************************/
			if (receive_uart1[write_buffer_uart1]==XON)
			{
				XOFF_received1=0;
			}
		}
		else	// R�ception normale....
		{
			if (XOFF_sent1==0) // J'ai envoy� un XOFF, alors ne m�morise plus ce qui arrive 
				// On perdra des caract�re, mais cela n'est pas grave...car un XOFF est parti !
			{
				if (receive_uart1[write_buffer_uart1]==0x0D)	// D�code CR = 0x0D = \r
				{
					command_uart1++;									// Une nouvelle commande est arriv�e!
					new_command_arrived_UART1=1;		// Ce flag indique qu'une nouvelle commande est arriv�e.
				}
				write_buffer_uart1++; 
			}
			// Handshake avec XOFF- XON 
			// Met dans des variables int les valeurs des pointeurs lecture et �criture
			read_position1=0x00FF&read_buffer_uart1;
			write_position1=0x00FF&write_buffer_uart1;
			etat_buffer1=write_position1-read_position1;
			if (etat_buffer1<0)
			{
				etat_buffer1=etat_buffer1+256;	// Si on est n�gatif, alors ajoute 256
			}
         
         
         //Attention ce test g�n�re un probl�me (un XOFF est envoy� et plus jamais de XON)
			// Attention 70% du buffer est plein....
//			if ((etat_buffer1>179)&&(XOFF_sent1==0))
//			{
//				SCON1&=(~TI1);										// Efface le flag d'interruption TI1
//				SBUF1 = XOFF;			// Le buffer erst presque plein > 70%, alors envoie un XOFF
//				SCON1&=(~TI1);										// Efface le flag d'interruption TI1
//				XOFF_sent1=1;
//			}
//         
         
			// Envoie XON seulement si un XOFF a �t� envoy� auparavant...
			// ET si le buffer est <20%...
			if ((etat_buffer1<50)&&(XOFF_sent1)) 
			{
				SCON1&=(~TI1);										// Efface le flag d'interruption TI1
				SBUF1 = XON;	;		// Envoie un XON, car le buffer est presque vide...
				SCON1&=(~TI1);										// Efface le flag d'interruption TI1				
				XOFF_sent1=0;	// Reparti pour un XOFF si besion 
			}
		}
		SCON1&=(~RI1);		// RI1=0;
	}
	// Emission de caract�res
	if ((SCON1&TI1)==TI1)									// Clear the transmit bit when the byte is transmitted
	{
		SCON1&=(~TI1);		// TI1=0, La transmission s'est bien d�roul�e,
		write_char1=0;			// On peut passer au caract�re suivant si besion
	}
}




void Int_timer4(void) interrupt 19
{
   SFRPAGE   = CONFIG_PAGE;
   
   TMR4CN&=0x7F;  // reset le flag TF4H
   
   countdown++;
   if (countdown >=1000)   // 60s sont �coul�es
   {
      TMR4CN&= (~0x04);						// Timer 4 stopp�
      SFRPAGE   = LEGACY_PAGE; 
   }
   SFRPAGE   = LEGACY_PAGE;
}