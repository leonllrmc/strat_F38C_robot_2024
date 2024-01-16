/*-----------------------------------------------------------------------------
* MAIN.C :
*
* CFPT Ecole d'�lectronique 
* salle : 1.32/1.33
* auteur : Didier Moullet
* revision : 1.0
* date: 24.9.2014
*
* version : voir ligne 31
*
* TARGET: C8051F38C
* FLASH memory 16KB
* RAM 2304 bytes
*
* Fonction principale du programme qui pilote le robot �cole. 
* On peut envoyer les commandes directement au robot pour le faire bouger.
* Reste la boucladmin	e principale dans laquelle on peut travailler avec les capteurs
* pr�sents sur le F38C

* Exemple d'envoi de commande sur l'UART0
* Send_string("Sortie UART 0 ",0);
* Exemple d'envoi de commande sur l'UART1
* Send_string("Sortie UART 1 ",1);
*
* La r�ception des caract�res se fait via Interruption. D�s que le 
* le caract�re 0x0D (\r ou CR) est d�tect� la variable "command_uart"
* ou "command_uart1" est incr�ment�e. 
* Les buffers de r�ceptions sont circulaires et peuvent contenir jusqu'a 256
* caract�res. Apr�s cela, les premiers caract�res seront effac�s. 
* 
* R�vision 1.1 version 1.3
*
* Commandes Avance, Recule, Droite, Gauche et Stop impl�ment�es
*
* R�vision 1.2 version 1.4
*
* Commandes Pause et Continue impl�ment�es
*
* version : 1.5
* auteur : Michele Schiavo
* date : 03.12.2015
* corrig� PWM servos ajout� fonction Mesure_Tension_Batterie
*
** version : 1.6
* auteur : ???
* date : ??
*
* version : 1.7
* auteur : Carbone / Anderes
* date : ??
* Adaptation des I/O pour le concours 2018-2019
*
* version : 1.8
* auteur : Carbone / Anderes
* date : 11 decembre 2019
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
#define PUBLIC

// INCLUDES
//-----------------------------------------------------------------------------

#include <declar.h>	// D�claration des variables globales et fonctions pour le projet
#include <math.h>
#include <string.h>

unsigned char code date[] = __DATE__;
unsigned char code time[] = __TIME__;

char commandBuffer[10] = {0};


////// ---- Communication smbus ---------------------------------------------------


#define CAPT_LIGNE_ADDRESS 0x40



////// FIN ---- Communication smbus -----------------------------------------------





//Fixe les valeurs des but�es des servomoteurs (servo 1 et/ou servo 2)
#define FERMEE  800  //Temps en us
#define OUVERTE 2200 //Temps en us


unsigned char lineLeft;
unsigned char lineRight;

unsigned char angleCorrection;

unsigned char coeffBit1;
unsigned char coeffBit2;
unsigned char coeffBit3;

const unsigned char baseSpeed = 20;

const unsigned char COEFF_0 = 0;
const unsigned char COEFF_1 = 2;
const unsigned char COEFF_2 = 6;
const unsigned char COEFF_3 = 12;



unsigned char reverseBits(unsigned char num) {
    unsigned char reversedNum = 0;
    unsigned char temp;
   
   int i = 0;

    // Reverse the first 4 bits
    for (; i < 4; i++) {
        temp = (num & (1 << i));
        if(temp)
            reversedNum |= (1 << ((4 - 1) - i));
    }

    // Copy the last 4 bits
    reversedNum |= (num & (0xF0));

    return reversedNum;
}

//-----------------------------------------------------------------------------
// MAIN ROUTINE
//-----------------------------------------------------------------------------
void main (void)
{
  bit Flag_aff_txt0 = 1;
  bit Flag_aff_txt1 = 1;
  unsigned char ligne ; //Variable pour le suivi de ligne
  unsigned char i;
   
  Init_Device();						// Configure le F38C pour l'application en cours



//   Delai_ms(1000); //�vite le warning      
   
	//*********************
	// UART0 initialisation 115200-8-1-N
	// ********************
	read_buffer_uart=0;		// Le pointeur de lecture est initialis�
	write_buffer_uart=0;	// Le pointeur de lecture est initialis�
	command_uart=0;					// Pas de commande en cours
	XOFF_sent=0;						// Buffer < 20% donc on peut remplir le buffer jusqu'a 80%...avant un XOFF
	XOFF_received=0;				// On a pas re�u de XOFF, donc on continue la communication
	write_char=0;			// Un caract�re est dans le buffer d'envoi (0 ou 1)
	RI0=0;									// RI0=0, Pas d'interrutpion pendante. 
	//*********************
	// UART1 initialisation 115200-8-1-N communication moteur -> strat�gie
	// ********************
	read_buffer_uart1=0;		// Le pointeur de lecture est initialis�
	write_buffer_uart1=0;		// Le pointeur de lecture est initialis�
	XOFF_sent1=0;						// Buffer < 20% donc on peut remplir le buffer jusqu'a 80%...avant un XOFF
	XOFF_received1=0;				// On a pas re�u de XOFF, donc on continue la communication
	command_uart1=0;					// Pas de commande en cours
	write_char1=0;			// Un caract�re est dans le buffer d'envoi (0 ou 1)
	SCON1&=(~RI1);						// RI1=0, Pas d'interrutpion pendante. 
	// Efface les buffers
		for (i=0;i<255;i++)
	{
		receive_uart1[i]=0x00;		// Efface (=0) ce caract�re
		receive_uart[i]=0x00;		// Efface (=0) ce caract�re
	}
	//*******************************
	// Autorise toutes les interrupts
	//*******************************
	EA=1;
	Putc_uart (XOFF,CARTE_MOTEUR);	
  // init valeur timer 60s
   countdown = 0;
   SFRPAGE   = CONFIG_PAGE; 
   TMR4CN&= (~0x04);		
   SFRPAGE   = LEGACY_PAGE;
   
	Send_string("\n\r",PC_BLUETOOTH);	
	Send_string("- Carte strategie enclenchee ",PC_BLUETOOTH);
	Send_string("\n\r",PC_BLUETOOTH);	
	Send_string("- Version : ",PC_BLUETOOTH);
	Send_float(VERSION,PC_BLUETOOTH);
	Send_string("\n\r",PC_BLUETOOTH);
	Send_string("- La tirette doite etre presente pour pouvoir envoyer des commandes\n\r",PC_BLUETOOTH);
	Send_string("- via un terminal serie (TERATERM)\n\r",PC_BLUETOOTH);	
	Send_string("- Protocole XON_XOFF\n\r",PC_BLUETOOTH);	
	


   PCA9685_InitServoMotor(1, PCA9685_ADDRESS,1); // Initialisation du PCA pour Servo Digital 330Hz 

   //Positionne les servos dans une certaine position
   Cmd_Servo(1, OUVERTE); 
   Cmd_Servo(2, OUVERTE);

// Extinction des leds
   Led_verte=0;
   Led_jaune=0;
   Led_rouge=0;
 
   Putc_uart (XON,CARTE_MOTEUR);	

    
   
   while (Tirette) // Tant que pas de Tirette pr�sente  
   {
      if(Flag_aff_txt0) //Affiche le texte suivant 1 seule fois
      {
         Send_string("\n\r",PC_BLUETOOTH);	
         Send_string("- Tirette non presente : test bouton et led ",PC_BLUETOOTH);
         Send_string("\n\r",PC_BLUETOOTH);
         Flag_aff_txt0 = 0; //Evite la r�p�tition du texte pr�c�dent
      }
     
      if(!Bp1)    // Allume la led verte si bouton SW2 est appuy�, sinon la led est �teinte
      {
         Led_verte=1; 
         Cmd_Servo(1, FERMEE); 
         Cmd_Servo(2, FERMEE);
      }
      else
      {        
         Cmd_Servo(1, OUVERTE); 
         Cmd_Servo(2, OUVERTE);
         Led_verte=0;
      }
      
      if(!Bp2)    // Allume la led jaune si bouton SW3 est appuy�, sinon la led est �teinte
      {
         Led_jaune=1; 

      }
      else
      {
         Led_jaune=0;
      }
      
      if(!Bp3)    // Allume la led rouge si bouton SW4 est appuy�, sinon la led est �teinte
      {
         Led_rouge=1;  
      }
      else
      {
         Led_rouge=0;
      }
      
   
   } // Fin du test des boutons poussoirs et des leds (Mise en place de la Tirette)
   
	while (!Tirette)
	{
      if(Flag_aff_txt1) //Affiche le texte suivant 1 seule fois
		{
         Send_string("\n\r",PC_BLUETOOTH);	
			Send_string("- Tirette presente ",PC_BLUETOOTH);
			Send_string("\n\r",PC_BLUETOOTH);
			Send_string("- Tapez help ",PC_BLUETOOTH);
			Send_string("\n\r",PC_BLUETOOTH);
			Flag_aff_txt1 = 0; //Evite la r�p�tition du texte pr�c�dent
		}			
      
			Lectures_COMM();		// Va lire les port de communications (carte moteur et PC-Bluetooth
      
	}// Attente retrait de la tirette 
   

/*   
    // Instructions en C � ins�rer dans la strat�gie pour piloter le robot:
		// NE PAS OUBLIER D'INSERER CETTE INSTRUCTION APRES CHAQUE COMMANDE QUI FAIT BOUGER LE ROBOT 
		// while (Roule);  
		// while (Roule) 
		{
				// .... lignes de code pouvant �tre effectu�e pendant que le robot roule
		}
		Send_string("AVxxxx\r",CARTE_MOTEUR);		// xxxx (avance) repr�sente en mm la distance que le robot doit effectuer
		Send_string("RExxxx\r",CARTE_MOTEUR);		// xxxx (recule) repr�sente en mm la distance que le robot doit effectuer
		Send_string("GAxxx\r",CARTE_MOTEUR);		// xxx (gauche) repr�sente l'angle en degr�s que le robot doit effectuer
		Send_string("DRxxx\r",CARTE_MOTEUR);		// xxx (droite) repr�sente l'angle en degr�s que le robot doit effectuer
		
		// Instructions en C � ins�rer dans la strat�gie pour piloter le robot: 
		// INSTRUCTIONS QUI NE FONT PAS BOUGER LE ROBOT 
		Send_string("VIxxx\r",CARTE_MOTEUR);		// xxx repr�sente est comprise entre 0 et 100%
		Send_string("VDxxx\r",CARTE_MOTEUR);		// xxx vitesse roue droite repr�sente est comprise entre 0 et 100%
		Send_string("VGxxx\r",CARTE_MOTEUR);		// xxx vitesse roue gauche repr�sente est comprise entre 0 et 100%
		Send_string("ST\r",CARTE_MOTEUR);				// Stop le robot 
		...

		// Commandes � dispositions 
		Delai_ms(xxx);
		Cmd_Servo (xx, yy); //   xx--> Num servo   yy--> temps de la pulse   (pour laisser le temps au servo de se positionner, ajouter la commande Delai_ms(xx))
   ...
   
   
   //I/O DISPO :
   
   //Boutons
   Bp1  
   Bp2
   Bp3

   //Leds
   Led_verte
   Led_jaune
   Led_rouge

   Tirette      // Tirette pour d�part
   
  

*/   
   
                     /////////////////////////////////////////////////   
                     /////        DEBUT DE LA STRATEGIE          /////
                     /////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////   
/////     EXECUTER UNE STRATEGIE VIA LIGNE DE COMMANDE LIGNE INTERFACE    /////
///////////////////////////////////////////////////////////////////////////////

// Mettre en commentaire les 4 lignes suivantes pour execut� la strat�gie
//	while(1)
//	{
//		Lectures_COMM();		// Va lire les port de communications (carte moteur et PC-Bluetooth
//	}

///////////////////////////////////////////////////////////////////////////////   
/////  FIN EXECUTER UNE STRATEGIE VIA LIGNE DE COMMANDE LIGNE INTERFACE   /////
///////////////////////////////////////////////////////////////////////////////

   
///////////////////////////////////////////////////////////////   
/////     EXECUTER UNE STRATEGIE COMPILEE (automatique    /////
///////////////////////////////////////////////////////////////   
 

   // Lire le code sur la station de base
   
   
   
   
   // G�n�rer le code couleur pour lib�rer l'hydrog�ne




   // R�cup�re le ballon d'yhdrog�ne



  //    Send_string("VI50\r",CARTE_MOTEUR); //vitesse 50%
            strcpy(commandBuffer, "PW");
            strcat(commandBuffer, itoa(baseSpeed, 10)); // roue gauche au maximum
            strcat(commandBuffer, "\r");
            
            Send_string(commandBuffer, CARTE_MOTEUR);

            memset(commandBuffer, 0, sizeof(commandBuffer)); // On vide la commande

   //   StartCountDown(); // Lancement du d�compte, dans 60s max le robot s'arr�tera


	 while(1)
	 {
      /*
       
         // Send_string("AV10\r",CARTE_MOTEUR);
         // while (Roule);
			
          //Lecture du capteur de ligne        
         ligne = Lire_Ligne(CAPT_LIGNE_ADDRESS);

         lineLeft = (ligne & 0xF0) >> 4;
         lineRight = reverseBits((ligne & 0x0F));

         if(lineLeft > lineRight) // select only the most used line
         {
            Led_verte = 0;
            Led_rouge = 1;

            coeffBit1 = (lineLeft & 0x02) >> 1;
            coeffBit2 = (lineLeft & 0x04) >> 2;
            coeffBit3 = (lineLeft & 0x08) >> 3;

            angleCorrection = (coeffBit1 * COEFF_1) + (coeffBit2 * COEFF_2) + (coeffBit3 + COEFF_3);


            strcpy(commandBuffer, "PG");
            strcat(commandBuffer, itoa(baseSpeed, 10)); // roue gauche au maximum
            strcat(commandBuffer, "\r");
            
            Send_string(commandBuffer, CARTE_MOTEUR);

            memset(commandBuffer, 0, sizeof(commandBuffer)); // On vide la commande

            strcpy(commandBuffer, "PD");
            strcat(commandBuffer, itoa(baseSpeed - angleCorrection, 10)); // roue droite ralentie
            strcat(commandBuffer, "\r");
            
            Send_string(commandBuffer, CARTE_MOTEUR);

            memset(commandBuffer, 0, sizeof(commandBuffer)); // On vide la commande

         }
         else if (lineLeft < lineRight)
         {
            Led_verte = 1;
            Led_rouge = 0;

            coeffBit1 = (lineRight & 0x02) >> 1;
            coeffBit2 = (lineRight & 0x04) >> 2;
            coeffBit3 = (lineRight & 0x08) >> 3;

            angleCorrection = (coeffBit1 * COEFF_1) + (coeffBit2 * COEFF_2) + (coeffBit3 + COEFF_3);

            strcpy(commandBuffer, "PD");
            strcat(commandBuffer, itoa(baseSpeed, 10)); // roue droite au maximum
            strcat(commandBuffer, "\r");
            
            Send_string(commandBuffer, CARTE_MOTEUR);

            memset(commandBuffer, 0, sizeof(commandBuffer)); // On vide la commande

            strcpy(commandBuffer, "PG");
            strcat(commandBuffer, itoa(baseSpeed - angleCorrection, 10)); // roue gauche ralentie
            strcat(commandBuffer, "\r");
            
            Send_string(commandBuffer, CARTE_MOTEUR);

            memset(commandBuffer, 0, sizeof(commandBuffer)); // On vide la commande
         }*/



       
//          if((ligne&0x7E) == 0x18) 			// X E E A A E E X
//          {
//             Send_string("PD200\r",CARTE_MOTEUR); // Meme vitesse Roue Gauche et Droite
//             Send_string("PG200\r",CARTE_MOTEUR); //          
//          }       

//          if((ligne&0x7E) == 0x10) 			// X E E A E E E X
//          {
//             Send_string("PD200\r",CARTE_MOTEUR); // Ralentir la roue gauche ou acc�l�rer la droite
//             Send_string("PG150\r",CARTE_MOTEUR); //
//          }

          
      
         Delai_ms(10); //Temps de boucle
     
    }//Fin du While(1) 

//////////////////////////////////////////////////////////////////   
/////    FIN EXECUTER UNE STRATEGIE COMPILEE (automatique    /////
//////////////////////////////////////////////////////////////////    

	
	 
}//Fin du MAIN