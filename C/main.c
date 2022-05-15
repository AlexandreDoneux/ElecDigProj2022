#include <main.h>
#include <stdio.h>
#use rs232(baud=9600, xmit=PIN_C6, rcv=PIN_C7, ERRORS)

#INT_RTCC
void  RTCC_isr(void) 
{

}

char *debut_data = "#";
char *data[4];
int flag_suivant = 0;  
int num_donner = 0;

#INT_RDA
void  RDA_isr(void) // reception et decoupe des donnees
{
   data[num_donner] = getc();
	if (strcmp(debut_data, data[0])){
		if (flag_suivant == 0){  
			num_donner ++;
			if (num_donner >= 4){    
				num_donner = 0;
				flag_suivant = 1;
			 }
		}
	}
}

void test_diodes(int val_max, int val_min, int true_val, int ech_aff){
   if((true_val > val_max) || (true_val < val_min)){
      // si valeur mesurée en dehors des limites
      printf("ALARM_OUT %d",true_val); // On envoie vers la connexion serial "ALARM_OUT distance"
      output_toggle(PIN_E1); // fait clignoter la led rouge
   }
   else{
      printf("ALARM_IN %d",true_val); // On envoie vers la connexion serial "ALARM_IN distance"
      output_high(PIN_E0); // allume la led verte
   }
}

int afficheurs(int true_val, int ech_aff){


   
   int val_point = 0b00000000; // valeur de base -# on affiche pas de point
   int val_affich = 0b00010000; // valeur de base, sera d'office changee'
   
   //echange a chaque affichage -X alternance des afficheurs
   if (ech_aff == 0){//aficheur 1 -> plus grand
      ech_aff = 1;
      val_affich = 0b00010000; //active le premier afficheur
      
      if (true_val >= 100){
         val_point = 0b00100000; // binaire pour allumer le point 
         true_val = true_val/100; //division entiere pour seulement avoir le premier chiffre
         
      }
      else{
         true_val = true_val/10;
      }
      
   }
   else{//afficheur 2 -# plus petit
      ech_aff = 0;
      val_affich = 0b01000000; //active le deuxieme afficheur
      
      if (true_val >= 100){
         true_val = (true_val/10)%10; //on prends le deuxieme chiffre des trois
      }
      else{
         true_val = true_val%10;
      }
   }
   
   delay_ms(150); //enlever IRL ?
   output_b(true_val | val_affich | val_point);
   // OU logique des differents nombres binaires.
   return ech_aff;
}

void main()
{
   setup_timer_0(RTCC_INTERNAL);      //409 us overflow


   enable_interrupts(INT_RTCC);
   enable_interrupts(INT_RDA);
   enable_interrupts(GLOBAL);
   setup_low_volt_detect(FALSE);
   int ech_aff = 0;
   long duration;
   int16 val_mesuree;
   int val_max = 120;
   int val_min = 0;
   
   while(TRUE)
   {
	  
      if(flag_suivant){ // tranforme les donnee en int et reconstruit le nombre 
	      // code ASCII -> Les valeurs dans data sont des char. en soustrayant 48 on obtiens leur équivalent en int
	      // int16 -> de base int se fait sur 8 bits. On peut y mettre comme valeur max 255.
	 int16 num1 = data[1] - 48; 
         int16 num2 = data[2] - 48;
         int16 num3 = data[3] - 48;
         val_max = (int16)((num1*100)+(num2*10)+num3); // remise sous la bonne forme
         flag_suivant = 0;
      }
      
      // On envoie un signal vers le sensor pendant 300 microsecondes
      output_high(PIN_C0);
      delay_us(20); // On laisse le signal en haut pendant 20 microsecondes. Le HCSR04 peut recevoir un signal trigger minimum 10 microsecondes
      output_low(PIN_C0);
      
	// Tant qu'il ne reçoit pas de réponse du sensor il bloque le code. Une fois qu'il reçois une réponse il remets le timer à 0.
      while(!input(PIN_C1)){}
      set_timer0(0);

 	// Tant qu'il reçoit une réponse du sensor il bloque. Une fois que cette réponse est finie il récupère le temps du timer et ...   
      while(input(PIN_C1)){}
      duration = (long)get_timer0(); 
      val_mesuree = (int16)(duration / 145); // calcule de la distance
      
      if (val_mesuree >= 1000){ // Si on dépasse les 1000 on mets 0 aux deux afficheurs et un point au premier. (distance max mesurable : environs 400 cm)
         output_b(0);
         output_high(PIN_B5); 

      }
      else{
         ech_aff = afficheurs(val_mesuree, ech_aff);
      }
      test_diodes(val_max, val_min, val_mesuree, ech_aff);
 
   }

}
