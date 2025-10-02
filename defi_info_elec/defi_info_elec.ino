#include <math.h>
#include <Servo.h>
Servo Servomoteur1;
const uint8_t pin_servo = 9, pin_RED, pin_GREEN, pin_SWITCH = A2, pin_Y_axes = A0, pin_X_axes = A1, resolution_ADC = 10;
uint8_t angle_randome = 70;
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
  switch (lecture_joytick(pin_X_axes, resolution_ADC, mesure_axe_X)) 
  {
  case 1:Serial.println("++");   break;
  case 2:Serial.println("--");   break;
  default: Serial.println("rien");  break;
  }
  delay(20);
}
