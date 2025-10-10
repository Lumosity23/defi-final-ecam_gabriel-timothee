/*
  ╔══════════════════════════════════════════════╗
  ║   Défi final d'info-élec – BAC1 ECAM         ║
  ╚══════════════════════════════════════════════╝
  Projet : Jeu de contrôle d’un servo moteur via joystick avec feedback LED RGB (ARDUINO UNO)
  Auteurs : Timothée C. & Gabriel B.
  Matériel (Modules choisis) : servo, LED RGB, joystick, et millis()
  Objectif : Faire correspondre l’angle du servo à un angle choisi aléatoirement.
  Particularité : Lecture de l’angle réel du servo via le potentiomètre. (ce dernier a ete mode pour pouvoir lire la tenison au born du potentiometre)
  Indication LED RGB :
    - Rouge -> orange -> jaune : éloigné -> rapproché
    - Vert : angle trouvé
    - Bleu : gagné 
    - Blanc : initialisation
  Fonctionnement :
    - Le servo doit être positionné à l’angle aléatoire et maintenu pendant un certain temps.
    - Une fois l’angle atteint, le servo se repositionne à 90° (position initiale), un nouvel angle est choisi et une nouvelle partie commence.
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
const uint8_t pin_servo = 9, pin_RED, pin_GREEN, pin_BLUE, pin_SWITCH = A2, pin_Y_axes = A0, pin_X_axes = A1, resolution_ADC = 10, pin_ANGLE_effectif = A3, angle_max_potentiometre = 270;

uint8_t erreur = 0;
uint8_t angle_random;
volatile bool initPartie = false;
uint16_t mesure_axe_X = 0;
unsigned long times_ms = 0;
float angle_effectif;
uint16_t val_max;
uint8_t etat_RGB;

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
  //checkInterruptPin(pin_SWITCH); //verification que la pin d'interuption est valide
  //attachInterrupt(digitalPinToInterrupt(pin_SWITCH), init_p, LOW); // on declenche la veille d'interuption sur le pin du bouton
  times_ms = millis();
  val_max = fond_echelle(resolution_ADC);
}

void loop() 
{
  for(int i = 0; i <= 180; i++)
  {
    int commande_angle = i;
  Servomoteur1.write(commande_angle);
  //Serial.println(mesure_angle_effectif(A3));
  Serial.println(commande_angle-mesure_angle_effectif(A3));
  delay(200); 
  }
  
  /*
  if (initPartie) 
  { // ceci reagit a l'interuption et appel la fonction init
    initPartie = false;   // on remet à false l'initialisation
    init_p(); // on exécute la fonction init()
  }
  erreur = angle_random - angle_effectif;

  if (abs(erreur) >= 10)//rouge
  {
    etat_RGB = orange;
  } 
  if (abs(erreur) <= 10)//orange
  {
    etat_RGB = jaune
  }
  if (millis() >= times_ms + 20)
  {
    angle_effectif = mesure_angle_effectif(pin_ANGLE_effectif);
    mesure_axe_X = analogRead(pin_X_axes);
  }*/
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

uint8_t mesure_angle_effectif(const uint8_t pin_servo_angle)//
{//angle = (tension / (fond d'échelle/angle_max)) 45 formule par Timothée le goat des maths plus fort que nguyen
  uint16_t val_num = analogRead(pin_servo_angle);
  float angle_effectif = (val_num * (3.3/val_max))/(3.3/270);
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

void checkInterruptPin(const uint8_t pin) // verifie que la pin d'interuption est valide si non bloque le programme
{
  if (digitalPinToInterrupt(pin) == -1) 
  {
    Serial.print("Erreur : ");
    Serial.print(pin);
    Serial.println(" n'est pas une pin d'interruption valide sur cette carte.");
    commande_LED_PWM(5); // Bleu pour indiquer une erreur
    delay(100);
    commande_LED_PWM(1); // Rouge pour indiquer une erreur
    delay(100);
    while (1); delay(800);// Blocage
  }
}
