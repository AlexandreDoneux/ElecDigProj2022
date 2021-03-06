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

void test_diodes(int val_max, int true_val, int ech_aff){
   if(true_val < val_max){
      
     printf("IN %d\n",true_val); // On envoie vers la connexion serial "ALARM_IN distance"
      output_high(PIN_E0); // allume la led verte
   }
   else{
      // si valeur mesuree est en desous de la valeur max
      printf("OUT %d\n",true_val); // On envoie vers la connexion serial "ALARM_OUT distance"
      output_toggle(PIN_E1); // fait clignoter la led rouge
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
   int16 val_mesuree; // sur 16 bits car peut depasser 255
   int16 val_max = 120; // idem
   //int val_min = 0; // a implementer dans le futur ?
   
   while(TRUE)
   {
     
      if(flag_suivant){ // tranforme les donnee en int et reconstruit le nombre 
         // code ASCII -> Les valeurs dans data sont des char. en soustrayant 48 on obtiens leur equivalent en int
         // int16 -> de base int se fait sur 8 bits. On peut y mettre comme valeur max 255.
      int num1 = data[1] - 48; 
         int num2 = data[2] - 48;
         int num3 = data[3] - 48;
         val_max =((num1*100)+(num2*10)+num3); // remise sous la bonne forme
         flag_suivant = 0;
      }
      
      // On envoie un signal vers le sensor pendant 300 microsecondes
      output_high(PIN_C0);
      delay_us(20); // On laisse le signal en haut pendant 20 microsecondes. Le HCSR04 peut recevoir un signal trigger minimum 10 microsecondes
      output_low(PIN_C0);
      
   // Tant qu'il ne recoit pas de reponse du sensor il bloque le code. Une fois qu'il recois une reponse il remets le timer a 0.
      while(!input(PIN_C1)){}
      set_timer0(0);

    // Tant qu'il recoit une reponse du sensor il bloque. Une fois que cette reponse est finie il recupere le temps du timer et ...   
      while(input(PIN_C1)){}
      duration = (long)get_timer0(); 
      val_mesuree = (int16)(duration / 145); // calcule de la distance
      
      if (val_mesuree >= 1000){ // Si on depasse les 1000 on mets 0 aux deux afficheurs et un point au premier. (distance max mesurable : environs 400 cm)
         output_b(0);
         output_high(PIN_B5); 

      }
      else{
         ech_aff = afficheurs(val_mesuree, ech_aff);
      }
      test_diodes(val_max, val_mesuree, ech_aff);
 
   }

}
