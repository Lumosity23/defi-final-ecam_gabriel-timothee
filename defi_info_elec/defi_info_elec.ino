#include <math.h>
//#include <Servo.h>
//Servo Servomoteur1;
const uint8_t pin_servo = 9, pin_RED, pin_GREEN, pin_SWITCH = A2, pin_Y_axes = A0, pin_X_axes = A1, resolution_ADC = 10;
uint8_t angle_randome = 70;
long randAngle;
volatile bool initPartie = false;
uint16_t mesure_axe_X = 0;


void setup() 
{
  Serial.begin(115200);
  pinMode(pin_SWITCH, INPUT);
  pinMode(pin_X_axes, INPUT);
  pinMode(pin_Y_axes, INPUT);

  pinMode(pin_GREEN, OUTPUT);
  pinMode(pin_RED, OUTPUT);

  //Servomoteur1.attach(pin_servo);
  //Servomoteur1.write(0);
  attachInterrupt(digitalPinToInterrupt(pin_SWITCH, BoutonInit, LOW));
}

uint8_t lecture_joytick(const uint8_t pin, uint8_t resolution, uint8_t mesure)
{
  uint8_t limite_sup, limite_inf;
  double val_max = pow(2, resolution);
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


void loop() 
{
  mesure_axe_X = analogRead(pin_X_axes);
  Serial.println(mesure_axe_X);
  if (mesure_axe_X > 1024*0.75) Serial.println("++");
  if (mesure_axe_X < 1024*0.25) Serial.println("--");
  /*switch (lecture_joytick(pin_X_axes, resolution_ADC, mesure_axe_X)) 
  {
  case 1:Serial.println("++");   break;
  case 2:Serial.println("--");   break;
  default: Serial.println("rien");  break;
  }*/

  if (initPartie) {
    initPartie = false;   // on remet à false l'initialisation
    init(); // on exécute la fonction init()
  }
  delay(20);
}

void init()
{
  randAngle = random(181); //reneration d'un nombre entre 0 et 180 (0 et max-1)
  servoMoteur(90);
  digitalWrite(pin_GREEN, 1);
}


void servoMoteur(int angleServo)
{
  u_int32_t times_ms;
  u_int32_t temps;
  times_ms = millis();
  temps = ((angleServo/180)+1);  // convertir l'anger recu en un temps en "ms" pour faire touner le servo
  if (millis >= times_ms + temps){  // repeter tout les 20ms
    digitalWrite(pin_servo, 1);
    times_ms = millis();
  }
}
