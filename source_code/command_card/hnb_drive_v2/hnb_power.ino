//****************************************************************************
// GESTION DU MOTEUR ET DE LA BATTERIE
//****************************************************************************
//

//-----------------------------------------------------------------------------
// Chronometre pour imposer une consigne nulle si pas de pédalage pendant plus de 10 s
//-----------------------------------------------------------------------------

void  gestionChronometreArretPedalage(){
  if ((bChronoArretPedalage) && (millis()-u32ChronoArrretDebut>10000)){
     
        u8Etat=ETAT_ARRET;
        ConsigneDesiree=0;
        stopSpeed();
        bChronoArretPedalage=false;
   //     afficheValeurLed(255);
      }
}




//-----------------------------------------------------------------------------
// Gestion de l'état du velo 
//-----------------------------------------------------------------------------

/* ici on gère l'état du vélo en fonction du pédalage, du courant et de la rotation 
 *  de la roue
 */

void gestionEtat(){
int s16courantLimit=0;

switch (speedLevel){
  case 0: s16courantLimit=COURANT_MOTEUR_LIMITE_0;ConsigneDesiree=0;break;
  case 1: s16courantLimit=COURANT_MOTEUR_LIMITE_1;ConsigneDesiree=VITESSE_1;break;
  case 2: s16courantLimit=COURANT_MOTEUR_LIMITE_2;ConsigneDesiree=VITESSE_2;break;
  case 3: s16courantLimit=COURANT_MOTEUR_LIMITE_3;ConsigneDesiree=VITESSE_3;break;
}

if (u16Courant>COURANT_MOTEUR_MAX) u8Etat=ETAT_ARRET;

if (bPedalage) {
  
switch(u8Etat){
  //---------------------- Velo à l'arret 
  case ETAT_ARRET:
   
   if (ConsigneDesiree>0) {
    u8Etat=ETAT_DEMARRAGE;
    bChronoArretPedalage         =  false;
   }  
  break;
  //---------------------- 
  case ETAT_DEMARRAGE:
   if ( (u16Courant>=COURANT_DEMARRAGE_LIMITE) && (u16VitesseVelo<VITESSE_DEMARRAGE_LIMITE))
       u8Etat=ETAT_ARRET; // cas du retropedalage 
   else if (u16VitesseVelo>=VITESSE_DEMARRAGE_LIMITE) 
       u8Etat=ETAT_AUGMENTE_PWM ;
  break;
  //---------------------- 
   case ETAT_AUGMENTE_PWM:
   if ( (u16Courant<s16courantLimit) && (u16VitesseVelo>=ConsigneDesiree-VITESSE_REGULATION_SEUIL)){
      u8Etat=ETAT_REGULATION ;
      u16PWMTemp=u16PWM;
   }
   else if (u16Courant>=s16courantLimit) 
      u8Etat=ETAT_DIMINUE_PWM ;    
  break;

  //---------------------- 
  case ETAT_DIMINUE_PWM:
  if ( (u16VitesseVelo<ConsigneDesiree-VITESSE_REGULATION_SEUIL)&&(u16Courant<s16courantLimit) )
      u8Etat=ETAT_AUGMENTE_PWM;
   else if (u16VitesseVelo>=ConsigneDesiree-VITESSE_REGULATION_SEUIL){
      u8Etat=ETAT_REGULATION ;
      u16PWMTemp=u16PWM;
   }
  break;
  //---------------------- 
    case ETAT_REGULATION:
   if (  (u16Courant<=s16courantLimit) && (u16VitesseVelo<ConsigneDesiree-VITESSE_REGULATION_SEUIL))
    u8Etat=ETAT_AUGMENTE_PWM;
   else if ( (u16Courant>s16courantLimit)  ){   
      u8Etat=ETAT_DIMINUE_PWM ;       
   }
   break;

  //---------------------- 
  

  }
}  
else 
  u8Etat=ETAT_ARRET;
}


void gestionPWM() {
byte gain=2;

switch(u8Etat){
  case ETAT_ARRET:
   u16PWM=0;
  break;
  
  case ETAT_DEMARRAGE:
   u16PWM=PWM_DEMARRAGE;
  break;

  case ETAT_AUGMENTE_PWM:
   u16PWM=u16PWM+1;
  break;

  case ETAT_DIMINUE_PWM:
    u16PWM=u16PWM-1;
    if (u16PWM<0) u16PWM=0;
  break;

  //-----------------------------------------------------------------------------
  // REGULATION en VITESSE, on realise un correcteur proportionnel 
  // PWM=K(Consigne-vitesse rotation roue)
  // Experimentallement un gain supérieur à 2 conduit à un système oscillant (à vide)
  //-----------------------------------------------------------------------------

  case ETAT_REGULATION:
    if (abs(gain*( ConsigneDesiree- u16VitesseVelo ))<=5) 
         u16PWM=u16PWMTemp+gain*( ConsigneDesiree-u16VitesseVelo );
    else if (gain*( ConsigneDesiree- u16VitesseVelo )>5) u16PWM=u16PWMTemp+5;
    else if (gain*( ConsigneDesiree- u16VitesseVelo )<-5) u16PWM=u16PWMTemp-5;
  break;

  dac12bits.setVoltage(u16PWM,true);
  }
}
