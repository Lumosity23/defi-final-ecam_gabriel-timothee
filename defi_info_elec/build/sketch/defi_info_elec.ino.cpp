#include <Arduino.h>
#line 1 "/home/timo/code/ECAM/defi-final-ecam_gabriel-timothee/defi_info_elec/defi_info_elec.ino"
/*
defi final d'info elec bac 1 ecam
le projet comprends les 4 éléments de la liste suivant : servo moteur, une led RGB, un joystick et parallélisme(millis)
il tourne sur arduino uno
le but est de commander un servo moteur à l'aide du joystick pour que l'angle du servo moteur corresponde à un angle choisit aléatoirement. Une led RGB nous indiques si on se rapproche de l'angle ou pas grâce au code couleur suivant :
-rouge éloigné
-vert rapproché
-bleu gagné
une fois le servo moteur dans le bon angle pendant un certain temps, le servo se place à 90° (position inital), un nouvel angle est choisit et une nouvelle partie commence. Nous avons modifier le servo moteur en soudant un fil sur le potentiomètre pour connaître l'angle effectif du servomoteur pour comparer non pas la commande du servo mais l'angle réel.
*/

#include <math.h>
#include <Servo.h>
Servo Servomoteur1;
const uint8_t pin_servo = 9, pin_RED, pin_GREEN, pin_BLUE, pin_SWITCH = A2, pin_Y_axes = A0, pin_X_axes = A1, resolution_ADC = 10, pin_ANGLE_effectif = A3, angle_max_potentiometre = 270;

uint8_t erreur = 0;
uint8_t angle_random;
volatile bool initPartie = false;
uint16_t mesure_axe_X = 0;
unsigned long times_ms = 0;
float angle_effectif;
uint16_t val_max = 0;

/*
pin servo : PB1
pin RED : 
pin BLUE : 
pin GREEN :
pin SWITCH : PC1
pin axe X :

*/

#line 35 "/home/timo/code/ECAM/defi-final-ecam_gabriel-timothee/defi_info_elec/defi_info_elec.ino"
void setup();
#line 54 "/home/timo/code/ECAM/defi-final-ecam_gabriel-timothee/defi_info_elec/defi_info_elec.ino"
void loop();
#line 102 "/home/timo/code/ECAM/defi-final-ecam_gabriel-timothee/defi_info_elec/defi_info_elec.ino"
void init_p(void);
#line 111 "/home/timo/code/ECAM/defi-final-ecam_gabriel-timothee/defi_info_elec/defi_info_elec.ino"
void servoMoteur(int angleServo);
#line 123 "/home/timo/code/ECAM/defi-final-ecam_gabriel-timothee/defi_info_elec/defi_info_elec.ino"
uint8_t mesure_angle_effectif(const uint8_t pin_servo_angle);
#line 129 "/home/timo/code/ECAM/defi-final-ecam_gabriel-timothee/defi_info_elec/defi_info_elec.ino"
uint8_t lecture_joytick(const uint8_t pin, uint8_t resolution, uint8_t mesure);
#line 145 "/home/timo/code/ECAM/defi-final-ecam_gabriel-timothee/defi_info_elec/defi_info_elec.ino"
uint16_t fond_echelle(uint8_t resolution);
#line 151 "/home/timo/code/ECAM/defi-final-ecam_gabriel-timothee/defi_info_elec/defi_info_elec.ino"
void turn_off_LED_PWM();
#line 159 "/home/timo/code/ECAM/defi-final-ecam_gabriel-timothee/defi_info_elec/defi_info_elec.ino"
void commande_LED_PWM(uint8_t etatat);
#line 35 "/home/timo/code/ECAM/defi-final-ecam_gabriel-timothee/defi_info_elec/defi_info_elec.ino"
void setup() 
{
  Serial.begin(115200);
  pinMode(pin_SWITCH, INPUT);
  pinMode(pin_X_axes, INPUT);
  pinMode(pin_Y_axes, INPUT);

  pinMode(pin_BLUE, OUTPUT);
  pinMode(pin_GREEN, OUTPUT);
  pinMode(pin_RED, OUTPUT);

  Servomoteur1.attach(pin_servo);
  Servomoteur1.write(0);
  // on devrai ajouter un verification d'availability de la pin d'interuption
  attachInterrupt(digitalPinToInterrupt(pin_SWITCH), init_p, LOW); // on declenche l'interuption sur le pin du bouton
  times_ms = millis();
  uint16_t val_max = fond_echelle(resolution_ADC);
}

void loop() 
{
  if (initPartie) 
  { // ceci reagit a l'interuption et appel la fonction init
    initPartie = false;   // on remet à false l'initialisation
    init_p(); // on exécute la fonction init()
  }
  erreur = angle_random - angle_effectif;

  if (abs(erreur) >= 10)
  {
    analogWrite(pin_RED, erreur);
    digitalWrite(pin_GREEN, 0);
    digitalWrite(pin_BLUE, 0);
  } 
  if (abs(erreur) <= 10)
  {
    analogWrite(pin_GREEN, erreur);
    digitalWrite(pin_RED, 0);
    digitalWrite(pin_BLUE, 0);
  }
  if (millis() >= times_ms + 20)
  {
    angle_effectif = mesure_angle_effectif(pin_ANGLE_effectif);
    mesure_axe_X = analogRead(pin_X_axes);
    
 /* Serial.println(mesure_axe_X);
  if (mesure_axe_X > 1024*0.75) Serial.println("++");
  if (mesure_axe_X < 1024*0.25) Serial.println("--");*/

  switch (lecture_joytick(pin_X_axes, resolution_ADC, mesure_axe_X)) 
  {
  case 1:Serial.println("++");   break;
  case 2:Serial.println("--");   break;
  default: Serial.println("rien");  break;
  }

  }
  //delay(20);
}

/* 
la fonction init est la pour demarer un partie, elle est appeler de manier indirect par l'interuption via le if dans le loop.
apres reflection je pense que si on automatise le redemarage de la partie alors on peus simplement fair eun un if bloquant qui permet de lancer la prtie au debut avec le bouton,
mais il nous faudras quand meme un interupt pour arreter le jeu avec un clic long
*/


void init_p(void)
{
  angle_random = random(181); //generation d'un nombre entre 0 et 180 (0 et max-1)
  Servomoteur1.write(0);
  digitalWrite(pin_GREEN, 1);
  initPartie = 1;
}


void servoMoteur(int angleServo)//et c'est chiant a faire un interup timer ?
{
  uint32_t times_ms;
  uint32_t temps;
  times_ms = millis();
  temps = ((angleServo/180)+1);  // convertir l'anger recu en un temps en "ms" pour faire touner le servo
  if (millis >= times_ms + temps){  // repeter tout les 20ms
    digitalWrite(pin_servo, 1);
    times_ms = millis();
  }
}

uint8_t mesure_angle_effectif(const uint8_t pin_servo_angle)
{//angle = tension / (fond d'échelle/angle_max) formule par Timothée le goat des maths plus fort que nguyen
  uint16_t val_num = analogRead(pin_servo_angle);
  float angle_effectif = (val_num * (3.3/val_max))/(val_max/angle_max_potentiometre);
  return angle_effectif;
}
uint8_t lecture_joytick(const uint8_t pin, uint8_t resolution, uint8_t mesure)//cette fonction est faite pour lire le joystick et renvoyer une valeur quand le joystick est trop penché (d'un coté comme de l'autre) 
{
  uint8_t limite_sup, limite_inf;
  limite_sup = 0.75 * val_max;
  limite_inf = 0.25 * val_max;
  if (mesure >= limite_sup)
  {
    return 1;
  } else if (mesure <= limite_inf)
  {
    return 2;
  }
  return 0;
}


uint16_t fond_echelle(uint8_t resolution)
{
  double fond_sechelle = pow(2,resolution);
  return fond_sechelle;
}
// Fonction pour éteindre toutes les couleurs de la LED
void turn_off_LED_PWM() 
{
  analogWrite(pin_RED, 0);   // Éteint le rouge (pour anode commune, 0 = éteint)
  analogWrite(pin_GREEN, 0); // Éteint le vert
  analogWrite(pin_BLUE, 0);  // Éteint le bleu
}

// Fonction pour définir la couleur de la LED en fonction d'un état avec PWM
void commande_LED_PWM(uint8_t etatat)
{
  turn_off_LED_PWM(); // Éteint d'abord toutes les couleurs

  switch(etatat)
  {
    case 0: turn_off_LED_PWM(); break; // LED éteinte

    case 1: analogWrite(pin_RED, 255); break; // Rouge

    case 2: analogWrite(pin_RED, 255); analogWrite(pin_GREEN, 127); break; // Orange

    case 3: analogWrite(pin_RED, 255); analogWrite(pin_GREEN, 0); break; // Jaune

    case 4: analogWrite(pin_GREEN, 255); break; // Vert

    case 5: analogWrite(pin_BLUE, 255); break; // Bleu

    case 6: analogWrite(pin_RED, 255); analogWrite(pin_GREEN, 255); analogWrite(pin_BLUE, 255); break; // Blanc
  }
}
