/**********************************************************************
* RS232.C :
*
* CFPT Ecole d'�lectronique 
* salle : 1.32/1.33
* auteur : Didier Moullet
* revision : 1.0
* date: 24.9.2014
*
* version : 1.5
* auteur : Michele Schiavo
* date : 03.12.2015
* corrig� PWM servos ajout� fonction Mesure_Tension_Batterie
*
*
* 
* Commentaires : Fonctions de bases pour envoyer des caract�res sur les diff�rents UART
* du F38C. Il y a deux UART 
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

#include <declar.h>		// D�claration des variables globales et fonctions pour le projet
#include <stdio.h>		// Cette fonction standard est universelle quel que soit
											// le microcontr�leur utilis� et le compilateur utilis�
#include <stdlib.h>		// Librairie standard
#include <string.h>
#include <math.h>


/********************************************
* Fonction : Putc_uart											*
* Type : void																*
* Input : data to send 											*
* Input : num�ro de l'UART (0 ou 1)					*
* Output : -					 											*
* Commentaire :		Ecrit le charactere dans	*
* l'UART choisi.														*
*********************************************/
void Putc_uart (unsigned char dat,bit uart_number)    	/* write byte to transmitter */
{
	if (uart_number==UART0)
	{
		// Si un XOFF a �t� re�u alors attend de recevoir un XON ! 
		while (XOFF_received);		// On a re�u un XOFF, attend le XON (XOFF_received=0) pour continuer
		SBUF0 = dat;						/* Envoie le caract�re */
		write_char=1;							/* Pr�pare l'envoi en mettant le flag � 1*/
		while (write_char);				/* Tant que le flag n'est pas � 0, continue */
	}
	else
	{
		// Si un XOFF a �t� re�u alors attend de recevoir un XON ! 
		while (XOFF_received1);		// On a re�u un XOFF, attend le XON (XOFF_received=0) pour continuer
		SBUF1 = dat;						/* Envoie le caract�re */
		write_char1=1;							/* Pr�pare l'envoi en mettant le flag � 1*/
		while (write_char1);				/* Tant que le flag n'est pas � 0, continue */
	}

}
/****************************************************
* Fonction : Send_float												*
* Type : void																				*
* Input : le nomnbre doit �tre un float							*
* Output : - 					 															*
* Comment : Envoie le nombre sur l'UART s�lectionn�	*
*****************************************************/
void Send_float (float number,bit uart_number)      
{
	char	i=0;
	if (number<0.)
	{
		number=-number;					// Si n�gatif, alors affiche le signe
		Putc_uart ('-',uart_number);
	}
	Send_number((long)(number),uart_number);			// Affiche la valeur enti�re
	number=number-((long)(number));											// Enl�ve la valeur enti�re
	Send_string(".",uart_number);									// Affiche le point
	for (i=0;i<2;i++)							// Affiche les 3 chiffres apr�s la virgule.
	{
		number=number*10.0;																// Tranfere en valeur enti�re le chiffre apr�s la virgule
		Send_number((long)(number),uart_number);		// Affiche la valeur enti�re 
		number=number-((long)(number));										// Enl�ve la valeur enti�re
	}
}
/****************************************************
* Fonction : Send_number											*
* Type : void																				*
* Input : le nomnbre doit �tre un long							*
* Output : - 					  															*
* Comment : Envoie le nombre sur l'UART s�lectionn�	*
*****************************************************/

void Send_number (long number,bit uart_number)      
{
	unsigned long rest=0;
	char number_ASCII[20];  // Pr�voit un buffer de 20 bytes
	char *point_to_string;	// Pointeur sur le string.

	// Il faut que le nombre soit diff�rent de 0, sinon affiche 0
	if (number!=0)
	{
		point_to_string=&number_ASCII[18];	// L'adresse du buffer est l�
		number_ASCII[19]='\0';							// Caract�re de fin de chaine
		if (number<0)
		{
			number=-number;										// Si n�gatif, alors affiche le signe
			Putc_uart ('-',uart_number);
		}
		// Conversion du nombre en chaine ASCII.
		while (number>0)
		{
			rest=number%10;										// Utilise le modulo 10
			if (rest==0)
			{*point_to_string='0';		// Met le "0" dans la chaine de charact�re
			}
			if (rest==1)
			{*point_to_string='1';		// Met le "1" dans la chaine de charact�re
			}
			if (rest==2)
			{*point_to_string='2';		// Met le "2" dans la chaine de charact�re
			}
			if (rest==3)
			{*point_to_string='3';		// Met le "3" dans la chaine de charact�re
			}
			if (rest==4)
			{*point_to_string='4';		// Met le "4" dans la chaine de charact�re
			}
			if (rest==5)
			{*point_to_string='5';		// Met le "5" dans la chaine de charact�re
			}
			if (rest==6)
			{*point_to_string='6';		// Met le "6" dans la chaine de charact�re
			}
			if (rest==7)
			{*point_to_string='7';		// Met le "7" dans la chaine de charact�re
			}
			if (rest==8)
			{*point_to_string='8';		// Met le "8" dans la chaine de charact�re
			}
			if (rest==9)
			{*point_to_string='9';		// Met le "9" dans la chaine de charact�re
			}
			number=number/10;					// Enl�ve 1 digit et continue tant que le nombre est 
																// diff�rent de 0
			point_to_string--;				// Caract�re suivant !
		}
		// Affiche la chaine de caract�re
			Send_string(point_to_string+1,uart_number);
	}
	else
	{
		point_to_string=&number_ASCII[18];	// L'adresse du buffer est l�
		number_ASCII[18]='0';								// C'est un "0"
		number_ASCII[19]='\0';							// Caract�re de fin de chaine
		Send_string(point_to_string,uart_number); // Affiche la chaine de caract�re
	}
}

/****************************************************
* Fonction : Send_string											*
* Type : void																				*
* Input : l'adresse de la chaine � envoyer					*
* Output : - 					 															*
* Comment : Envoie la chaine sur l'UART s�lectionn�	
* La chaine de caract�re DOIT se terminer avec le		*
* sans quoi le pointeur continuera � envoyer des 		*
* caract�res.... 																		*
*****************************************************/
void Send_string (unsigned char *pointer,bit uart_number)
{
	unsigned char end_string[]='\0';		// Caract�re de fin de chaine
	// Tant que le caract�re de fin de chaine n'est pas l� 
	while (strcmp (pointer,end_string))
	{
		Putc_uart(*pointer,uart_number); 	// Envoie le catract�re dans l'UART s�lectionn�
		pointer++;												// Caract�re suivant  
	}
}



