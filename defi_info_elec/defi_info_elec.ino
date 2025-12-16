/*
  ╔══════════════════════════════════════════════╗
  ║   Défi final d'info-élec – BAC1 ECAM         ║
  ╚══════════════════════════════════════════════╝
  Projet : Jeu de contrôle d’un servo moteur via joystick avec feedback LED RGB (ARDUINO UNO)
  Auteurs : Timothée C. & Gabriel B.
  Hardware : 2 led RGB, diffuseur limineux, condensateur pour éviter les rebonds du bouton du joystick, joystick avec bouton, arduino uno, servomoteur, 3 résistances, 3 transistors, du papier alu, boitier, 
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
//tej interrupt, pointer, test bouton dans le while, corriger angle effectif
#include <math.h>
#include <Servo.h>

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Servo.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C

#define eteint 0
#define rouge 1
#define orange 2
#define jaune 3
#define vert 4
#define bleu 5
#define blanc 6

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); 

Servo Servomoteur1;
const uint8_t pin_servo = 10, pin_RED = 5, pin_GREEN = 3, pin_BLUE = 6, pin_X_axes = A1, resolution_ADC = 10, pin_ANGLE_effectif = A2,pin_SWITCH = 4;

float angle_max_potentiometre = 270.0;

unsigned long start_time;

uint8_t erreur = 0;
long angle_random = 0;
uint16_t mesure_axe_X = 0;
unsigned long times_ms = 0;
unsigned long time_to_win = 0;
float angle_effectif = 0;
uint16_t val_max = 0;
uint8_t etat_RGB = 0;
uint8_t angle = 90;
const uint8_t temps_demarage = 5000;//temps de demarage de 5 sec
bool flag_angle_trouve = 0;
bool flag_partie_finie = 0;
bool flag_init_partie = 0;
bool state_switch = 0;
uint16_t limite_sup = 0, limite_inf = 0;

byte etat_partieActuel = 1;
byte etat_partiePrecendent = 0;

void setup() 
{
  Serial.begin(9600);
  Serial.println("DEBUT DEMARRAGE");

  start_time = millis();

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
  {
  Serial.println(F("SSD1306 allocation failed"));
  }

  pinMode(pin_SWITCH, INPUT_PULLUP);
  pinMode(pin_X_axes, INPUT);

  pinMode(pin_BLUE, OUTPUT);
  pinMode(pin_GREEN, OUTPUT);
  pinMode(pin_RED, OUTPUT);

  Servomoteur1.attach(pin_servo);

  val_max = fond_echelle(resolution_ADC);
  times_ms = millis();
  time_to_win = millis();

  delay(500);
  Serial.println("DEMARRAGE EFFECTUE AVEC SUCCES");
  angle_random = random(0, 181);
  display.clearDisplay();
  
}

void loop() 
{
  
  unsigned long time_ms = millis() - start_time;
  unsigned long secondes = time_ms / 1000;
  unsigned long centieme = (time_ms % 1000) / 10;

  state_switch = !digitalRead(pin_SWITCH);
  if (state_switch)
  {
    new_partie();
  }

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 28);
  display.print("time : ");
  display.print(secondes);
  display.print(".");
  if (centieme < 10) display.print("0");
  display.println(centieme);
  refresh();

  erreur = abs((angle_random - angle));
  mesure_axe_X = analogRead(pin_X_axes);
  if (millis() >= times_ms+100)
  {
    limite_sup = (val_max/4)*3;
  limite_inf = val_max/4;
    times_ms = millis();
    if (mesure_axe_X <= limite_inf) angle--;
    if (mesure_axe_X >= limite_sup) angle++;

    angle_effectif = mesure_angle_effectif(pin_ANGLE_effectif);
    angle = borne(180,1,angle);
    Serial.println(mesure_axe_X);
    Serial.print("angle = ");
    Serial.print(angle);
    Serial.print("    erreur = ");
    Serial.print(erreur);
    Serial.print("    effectif = ");
    Serial.println(angle_effectif);
  }
  
  Servomoteur1.write(angle);
  
    if (erreur < 3 && flag_partie_finie == 0)
    {
      etat_RGB = vert;
      flag_angle_trouve = 1;
      if (etat_partieActuel != etat_partiePrecendent)
      {
        time_to_win = millis();
        etat_partiePrecendent = etat_partieActuel;
      }
      //Serial.println("angle trouvé");
    } else if (erreur < 20 && erreur >= 3)
    {
      etat_RGB = jaune;
      flag_angle_trouve = 0;
      flag_partie_finie = 0;
      time_to_win = 0;
    } else if (erreur < 40 && erreur >= 20)
    {
      etat_RGB = orange;
      flag_angle_trouve = 0;
      flag_partie_finie = 0;
      time_to_win = 0;
    }
    else if (erreur >= 40)
    {
      etat_RGB = rouge;
      flag_angle_trouve = 0;
      flag_partie_finie = 0;
      time_to_win = 0;
    }
    
    if (flag_angle_trouve && millis() >= time_to_win+2000)
    {
      //time_to_win = millis();
      etat_partieActuel = 1;
      etat_partiePrecendent = 0;
      flag_partie_finie = 1;
      if (millis()>=time_to_win+1000)
      {
        new_partie();
      } 

    }
    commande_LED_PWM(etat_RGB);
  
  

}

/* 
la fonction init est la pour demarer un partie, elle est appeler de manier indirect par l'interuption via le if dans le loop.
apres reflection je pense que si on automatise le redemarage de la partie alors on peus simplement fair eun un if bloquant qui permet de lancer la prtie au debut avec le bouton,
mais il nous faudras quand meme un interupt pour arreter le jeu avec un clic long
*/



uint8_t mesure_angle_effectif(const uint8_t pin_servo_angle)//
{//angle = (tension / (fond d'échelle/angle_max)) 45 formule par Timothée le goat des maths plus fort que nguyen
  uint16_t val_num = analogRead(pin_servo_angle);
  float angle_effectif = (val_num * (3.3/val_max))/(3.3/angle_max_potentiometre);
  return angle_effectif;
}

uint16_t fond_echelle(uint8_t resolution)
{
  double fond_sechelle = (pow(2,resolution))-1;
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

    case 2: digitalWrite(pin_RED, 1); analogWrite(pin_GREEN, 10); break; //Orange

    case 3: analogWrite(pin_RED, 180); analogWrite(pin_GREEN, 30); break; // Jaune

    case 4: digitalWrite(pin_GREEN, 1); break; // Vert

    case 5: digitalWrite(pin_BLUE, 1); break; // Bleu

    case 6: digitalWrite(pin_RED, 1); digitalWrite(pin_GREEN, 1); digitalWrite(pin_BLUE, 1); break; // Blanc
  }
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

void new_partie()
{
  angle = 90;
  etat_RGB = bleu;
  commande_LED_PWM(etat_RGB);
  delay(2000);
  Servomoteur1.write(angle);
  etat_RGB = blanc;
  commande_LED_PWM(etat_RGB);

  display.setTextSize(3);
  display.setTextColor(WHITE);

  display.setCursor(35, 10);
  display.println("NEW");
  display.setCursor(25, 35);
  display.print("GAME");
  refresh();
  delay(1000);

  display.setCursor(55, 20);
  display.print("5");
  refresh();
  delay(1000);
  display.setCursor(55, 20);
  display.print("4");
  refresh();
  delay(1000);
  display.setCursor(55, 20);
  display.print("3");
  refresh();
  delay(1000);
  display.setCursor(55, 20);
  display.print("2");
  refresh();
  delay(1000);
  display.setCursor(55, 20);
  display.print("1");
  refresh();
  delay(1000);
  display.setCursor(45, 20);
  display.print("GO!");
  refresh();
  delay(500);
    Serial.println("-----------DEBUT DE PARTIE-----------");
    angle_random = random(0, 181);
  flag_partie_finie = 0;
  start_time = millis();
}

void refresh(void)
{
  display.display();
  display.clearDisplay();
}
