/*
  ╔══════════════════════════════════════════════╗
  ║   Défi final d'info-élec – BAC1 ECAM         ║
  ╚══════════════════════════════════════════════╝
  Projet : Jeu de contrôle d’un servo moteur via joystick avec feedback LED RGB (ARDUINO UNO)
  Auteurs : Timothée C. & Gabriel B.
  Matériel (Modules choisis) : servo, LED RGB, joystick, et moniteur série
  Objectif : Faire correspondre l’angle du servo à un angle choisi aléatoirement.
  Particularité : Lecture de l’angle réel du servo via le potentiomètre car c'est un peu nul de déterminer si l'angle à été atteint uniquement via la commande (ce dernier a ete mode pour pouvoir lire la tenison aux bornes du potentiometre)
  Indication LED RGB :
    - Rouge -> orange -> jaune : éloigné -> rapproché
    - Vert : angle trouvé
    - Bleu : gagné 
    - Blanc : initialisation
  Fonctionnement :
    - Le servo doit être positionné à l’angle aléatoire et maintenu pendant un certain temps.
    - Une fois l’angle atteint, le servo se repositionne à 90° (position initiale), un nouvel angle est choisi et une nouvelle partie commence.
  Contrainte : 
    -Ne pas utiliser de delay car c'est une foncion bloquante, c'est à dire qu'elle bloque le µC et que c'est une fonction qui cache d'autre fonction, et si plus tard par exemple je désactive les interruptions elle ne fonctionne plus, elle est donc cause de panne, et il est difficile à diagnostiquer
*/

#include <math.h>
#include <Servo.h>

#define eteint 0
#define rouge 1
#define orange 2
#define jaune 3
#define vert 4
#define bleu 5
#define blanc 6

Servo Servomoteur1;
const uint8_t pin_servo = 9, pin_RED = 9, pin_GREEN = 11, pin_BLUE = 6, pin_X_axes = A1, resolution_ADC = 10, pin_ANGLE_effectif = A3, angle_max_potentiometre = 270;

const uint8_t pin_SWITCH = 2;//ATTENTION cette pin doit être compatible avec un interruption matériel
static volatile bool flag_init_partie = 0;
bool* pflag_init_partie = &flag_init_partie;

uint8_t erreur = 0;
uint8_t angle_random = 0;
uint16_t mesure_axe_X = 0;
unsigned long times_ms = 0;
float angle_effectif = 0;
uint16_t val_max = 0;
uint8_t etat_RGB = 0;
uint8_t angle = 0;
const uint8_t temps_demarage = 5000;//temps de demarage de 5 sec

void setup() 
{
  Serial.begin(9600);
  Serial.println("DEBUT DEMARRAGE");
  pinMode(pin_SWITCH, INPUT);
  pinMode(pin_X_axes, INPUT);

  pinMode(pin_BLUE, OUTPUT);
  pinMode(pin_GREEN, OUTPUT);
  pinMode(pin_RED, OUTPUT);

  Servomoteur1.attach(pin_servo);
  //checkInterruptPin(pin_SWITCH); //verification que la pin d'interuption est valide
  //attachInterrupt(digitalPinToInterrupt(pin_SWITCH), init_p, LOW); // on declenche la veille d'interuption sur le pin du bouton
  times_ms = millis();
  val_max = fond_echelle(resolution_ADC);
  attachInterrupt(digitalPinToInterrupt(pin_SWITCH), init_partie, FALLING);
  Servomoteur1.write(90);
  delay(5000);
  /*uint32_t delay_de_depart = 4000000;
  while(delay_de_depart>0)
  {
    delay_de_depart--;
  }*/
  Serial.println("DEMARRAGE EFFECTUE AVEC SUCCES");
}

void init_partie(void)
{
  *pflag_init_partie = 1;//utilisation d'un pointer car c'est une interruption qui affecte une variable et je veux lire cette variable dans tout mon code donc je la déclare globale mais je veux être sur qu'en tout temps la variable soit "accurate"
}

int borne(int borne_sup, int borne_inf, int valeur_bornee)
{
  if (valeur_bornee >= borne_sup)
  {
    return borne_sup;
  } else if (valeur_bornee <= borne_inf)
  {
    return borne_inf;
  } else return valeur_bornee;
}
void loop() //
{
    mesure_axe_X = analogRead(pin_X_axes);
  if (millis() >= times_ms+10)
  {
    times_ms = millis();
    if (mesure_axe_X >= 1000) angle++;
    if (mesure_axe_X <= 300) angle --;
    angle_effectif = mesure_angle_effectif(pin_ANGLE_effectif);
    borne(181,0,angle);
    Serial.println(angle);
  }
  Servomoteur1.write(angle);
  
  erreur = angle_random - angle_effectif;
  if (abs(erreur) >= 10)//rouge
  {
    etat_RGB = orange;
  } 
  if (abs(erreur) <= 10)//orange
  {
    etat_RGB = jaune;
  }
  commande_LED_PWM(etat_RGB);

  /*if (millis() >= temps_demarage && flag_init_partie == 0) //permet un demarage de la partir malgre qu'on ne sache pas cliquer sur le bouton, si pin non adequate ou autre
  {
    flag_init_partie = 1;
  }*/
}

/* 
la fonction init est la pour demarer un partie, elle est appeler de manier indirect par l'interuption via le if dans le loop.
apres reflection je pense que si on automatise le redemarage de la partie alors on peus simplement fair eun un if bloquant qui permet de lancer la prtie au debut avec le bouton,
mais il nous faudras quand meme un interupt pour arreter le jeu avec un clic long
*/

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

uint8_t mesure_angle_effectif(const uint8_t pin_servo_angle)//
{//angle = (tension / (fond d'échelle/angle_max)) 45 formule par Timothée le goat des maths plus fort que nguyen
  uint16_t val_num = analogRead(pin_servo_angle);
  float angle_effectif = (val_num * (3.3/val_max))/(3.3/angle_max_potentiometre);
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
  digitalWrite(pin_RED, 0);   // Éteint le rouge (pour anode commune, 0 = éteint)
  digitalWrite(pin_GREEN, 0); // Éteint le vert
  digitalWrite(pin_BLUE, 0);  // Éteint le bleu
}

// Fonction pour définir la couleur de la LED en fonction d'un état avec PWM
void commande_LED_PWM(uint8_t etatat)
{
  turn_off_LED_PWM(); // Éteint d'abord toutes les couleurs

  switch(etatat)
  {
    case 0: turn_off_LED_PWM(); break; // LED éteinte

    case 1: digitalWrite(pin_RED, 1); break; // Rouge

    case 2: digitalWrite(pin_RED, 1); analogWrite(pin_GREEN, 127); break; // Orange

    case 3: digitalWrite(pin_RED, 1); analogWrite(pin_GREEN, 0); break; // Jaune

    case 4: digitalWrite(pin_GREEN, 1); break; // Vert

    case 5: digitalWrite(pin_BLUE, 1); break; // Bleu

    case 6: digitalWrite(pin_RED, 1); digitalWrite(pin_GREEN, 1); digitalWrite(pin_BLUE, 1); break; // Blanc
  }
}
