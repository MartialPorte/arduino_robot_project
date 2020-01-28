#include <SoftwareSerial.h>
#include <Servo.h> 

/*CONFIG PINS*/
//-- MOTEUR A --
#define ENA 5 //Connecté à Arduino pin 5(sortie pwm)
#define IN1 2 //Connecté à Arduino pin 2
#define IN2 3 //Connecté à Arduino pin 3

//-- MOTEUR B --
#define ENB 6 //Connecté à Arduino pin 6(Sortie pwm)
#define IN3 4 //Connecté à Arduino pin 4
#define IN4 7 //Connecté à Arduino pin 7

// Sharp IR GP2Y0A41SK0F (4-30cm, analog)
#define sensor A0 

/*Bluetoocth Connexion*/
SoftwareSerial BTSerial(0, 1); // RX | TX  = > BT-TX=0 BT-RX=1

/*CONSIGNES MAX PWM*/
int consigne_av_arr = 230;
int consigne_g_d = 200;

/*Variables*/
char cmd;
float a=0, b=0;
int mesure=0;
int warning=0;
float angle_hor=90;
float a_h_old=0;
float angle_ver=90;
float a_v_old = 0;
int val = 5;
bool secu = false;

Servo S_hor;
Servo S_ver;

/********************************************************************/
/*******************Fonction Commmande Moteur************************/
void PWM_motor(int A, int B){
  analogWrite(ENA, A);
  analogWrite(ENB, B);
}

void Dir_motor(char c, int w){
  switch(c){
    case 'z' :
      if(w == 0){
        /*MARCHE AVANT*/
        // Direction du Moteur A
        digitalWrite(IN1,LOW); 
        digitalWrite(IN2,HIGH);
        // Direction du Moteur B
        digitalWrite(IN3,LOW);
        digitalWrite(IN4,HIGH);
        if(a < consigne_av_arr && b < consigne_av_arr){ a+=val; b+=val; } 
      }
      else {
        /*STOP*/
        // Direction du Moteur A
        digitalWrite(IN1,LOW); 
        digitalWrite(IN2,LOW);
        // Direction du Moteur B
        digitalWrite(IN3,LOW);
        digitalWrite(IN4,LOW);
        if(a > 0 && b > 0){ a-=val; b-=val; }
      }
      break;
    
    case 's' :
      /*MARCHE ARRIERE*/
      // Direction du Moteur A
      digitalWrite(IN1,HIGH); 
      digitalWrite(IN2,LOW);
      // Direction du Moteur B
      digitalWrite(IN3,HIGH);
      digitalWrite(IN4,LOW);
      if(a < consigne_av_arr && b < consigne_av_arr){ a+=val; b+=val; }
      break;
    
    case 'q' :  
      /*GAUCHE*/
      // Direction du Moteur A
      digitalWrite(IN1,LOW); 
      digitalWrite(IN2,HIGH);
      // Direction du Moteur B
      digitalWrite(IN3,HIGH);
      digitalWrite(IN4,LOW);
      if(a < consigne_g_d && b < consigne_g_d){ a+=val; b+=val; }
      break;
    
    case 'd' :  
      /*DROITE*/
      // Direction du Moteur A
      digitalWrite(IN1,HIGH); 
      digitalWrite(IN2,LOW);
      // Direction du Moteur B
      digitalWrite(IN3,LOW);
      digitalWrite(IN4,HIGH);
      if(a < consigne_g_d && b < consigne_g_d){ a+=val; b+=val; }
      break;

    case 'x' :  
      /*STOP*/
      // Direction du Moteur A
      digitalWrite(IN1,LOW); 
      digitalWrite(IN2,LOW);
      // Direction du Moteur B
      digitalWrite(IN3,LOW);
      digitalWrite(IN4,LOW);
      if(a > 0 && b > 0){ a-=val; b-=val; }
      break;
      
    default : a=0 ; b=0;
   }
}

/********************************************************************/
/*******************Fonction Securité SHARP************************/
int secu_SHARP(int res, char c){

  if (c == 'w'){ secu =! secu ; cmd = 'x';}

  if (secu == true){
    if (res < 300 && res > 20){ return 0;} //Tout va bien
    else {return 1;} //Attention danger !
  }
  else{
    return 0;
  }
}

/********************************************************************/
/*******************Fonction Commmande Tourelle************************/
void Dir_tourelle(char c){

  switch(c){
  case 'i' :
      /*Haut*/
      angle_ver = 25;
      break;
    
    case 'k' :  
      /*Bas*/
      angle_ver = 110;
      break;
    
     case 'j':
      /*Gauche*/
      angle_hor = 170;
      break;

    case 'l' :
      /*Droite*/
      angle_hor = 10;
      break;

    case 'n' :
      angle_ver = 50 ; angle_hor = 90;
      break;     
  }  
}

void Go_tourelle( int ang_v, int ang_h){

  if(ang_h != a_h_old || ang_v != a_v_old){
    S_hor.write((int)ang_h);
    S_ver.write((int)ang_v);
  
    a_h_old = ang_h;
    a_v_old = ang_v;
 }
}

/********************************************************************/

void setup() {
   /*TIMER CONFIG*/
   /* https://www.teachmemicro.com/arduino-timer-interrupt-tutorial/ */
   TIMSK2 = (TIMSK2 & B11111110) | 0x01; //Enable timer overflow
   TCCR2B = (TCCR2B & B11111000) | 0x04; //Set prescaler divisor to 64 -> Timer periode = 1ms

   /*Bluetooth CONFIG*/
   BTSerial.begin(9600);

   /*PINS CONFIG*/
   pinMode(ENA,OUTPUT);
   pinMode(ENB,OUTPUT);
   pinMode(IN1,OUTPUT);
   pinMode(IN2,OUTPUT);
   pinMode(IN3,OUTPUT);
   pinMode(IN4,OUTPUT);
   pinMode(sensor, INPUT);

   S_hor.attach(10);
   S_ver.attach(9);

   /*Motor Initialisation*/
   digitalWrite(ENA,LOW);// Moteur A - Ne pas tourner (désactivation moteur)
   digitalWrite(ENB,LOW);// Moteur B - Ne pas tourner (désactivation moteur)
   // Direction du Moteur A
   digitalWrite(IN1,LOW); 
   digitalWrite(IN2,LOW);
   // Direction du Moteur B
   digitalWrite(IN3,LOW);
   digitalWrite(IN4,LOW);
}
 
 
void loop() {

  /*Communication Bluetooth*/
  while (BTSerial.available()){ 
    cmd = BTSerial.read();    
  }

    /*Gérération en continu des signaux de PWM moteurs*/
    PWM_motor(a,b);
  
    /*Mise en position de la tourelle*/
    Go_tourelle(angle_ver, angle_hor);
  
    /*Mesure distance de sécurité SHARP*/
    mesure = analogRead(sensor);
}
 
 
ISR(TIMER2_OVF_vect){
     warning = secu_SHARP(mesure, cmd);
     Dir_motor(cmd, warning);
     Dir_tourelle(cmd);
}
