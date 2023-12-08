#include <LiquidCrystal_I2C.h>
#include <BasicStepperDriver.h>
#include <MultiDriver.h>
#include <SyncDriver.h>
#include <Arduino.h>
#include "SyncDriver.h"
#include "MultiDriver.h"
#include <Stepper.h>
#include <EncoderButton.h>
#include <ContinuousStepper.h>

//################################################ ENCODER #############################################
#define ENCODER_PIN1 10
#define ENCODER_PIN2 9
#define SWITCH_PIN 11

EncoderButton eb1(ENCODER_PIN1, ENCODER_PIN2, SWITCH_PIN);

//################################################ LCD ################################################
LiquidCrystal_I2C lcd(0x27,16,2);

//################################################ STEPPER  ###########################################

#define MOTOR_STEPS 200
#define DIR_X 5 // X = guida
#define STEP_X 2
#define DIR_Y 6 // Y = rotore
#define STEP_Y 3


BasicStepperDriver stepper_guida(MOTOR_STEPS, DIR_X, STEP_X);
BasicStepperDriver stepper_rotore (MOTOR_STEPS, DIR_Y, STEP_Y);
//SyncDriver controller(stepper_guida, stepper_rotore);



ContinuousStepper<StepperDriver> stepper1;

//
const byte numChars = 32;
//function variable
char receivedChars[numChars]; // an array to store the received data bool newData = false;
//#### variabili ##
long int num_spire = 0;
float altezza_bobina =0;
float diametro_filo = 0;
int inserimento = 0;
int inserimento_precedente = -1;
int direzione = 1;
int num_encoder_rotation = 0;
int multiplier = 1;

bool pausa = false;


void setup() {
  
    pinMode(8, OUTPUT); digitalWrite(8, HIGH);
    stepper_guida.begin(250, 16); 
    stepper_rotore.begin(250, 16);

    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print("Automatic coil");
    lcd.setCursor(5,1); lcd.print("Winder");
    eb1.setEncoderHandler(onEb1Encoder); 
    eb1.setDebounceInterval(50);
    eb1.setLongClickDuration(1000);
    eb1.setClickHandler(multiple_click_hendler);
    eb1.setLongClickHandler(longClickHandler); 
}



void loop() {

 eb1.update();
 switch(inserimento)
 {
   case 0:
    if(inserimento_precedente != inserimento)
    {

      lcd.clear();
      lcd.setCursor(1,0);
      lcd.print("Automatic coil");
      lcd.setCursor(5,1);
      lcd.print("Winder");
    }
     inserimento_precedente = inserimento;
     break;

  case 1:
    if(inserimento_precedente != inserimento)
    {
      lcd.clear();
      lcd.setCursor(2,0);
      lcd.print("Numero Spire");
      lcd.setCursor(1,1);
      lcd.print("N.");
    }
    inserimento_precedente = inserimento;
    lcd.setCursor(6,1);
    lcd.print(num_encoder_rotation);
    num_spire = num_encoder_rotation;
    break;

  case 2:
    if(inserimento_precedente != inserimento)
    {
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("lunghezza");
      lcd.setCursor(14,1);
      lcd.print("mm");
    }
    inserimento_precedente = inserimento;
    lcd.setCursor(6,1);
    lcd.print(num_encoder_rotation);
    altezza_bobina = num_encoder_rotation;
    break;    


  case 3:
    if(inserimento_precedente != inserimento)
    {
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print(" diametro filo ");
      lcd.setCursor(14,1);
      lcd.print("mm");
    } 
    inserimento_precedente = inserimento;
    lcd.setCursor(6,1);
    lcd.print(0.01*num_encoder_rotation);
    diametro_filo = 0.01*num_encoder_rotation;
    break;

  case 5:
    if(inserimento_precedente != inserimento)
    {
      lcd.clear();
      lcd.setCursor(3,0);
      lcd.print("completato ");
    }
    inserimento_precedente  = inserimento;
    break;


 }

 long int num_rotazioni=0;
 float lunghezza_raggiunta = 0;
 int guida_angle = (360/8) * diametro_filo; 
 bool first = true;


 while(num_rotazioni < num_spire && inserimento == 4)
 {
   digitalWrite(8, LOW);

   if (num_rotazioni == 0)
    lcd.clear();

  eb1.update();
  while(pausa)
  {
    if(first== true)
    {
      lcd.clear();
      lcd.setCursor(5,0);
      lcd.print("PAUSA");
      digitalWrite(8, HIGH);

    }
    first= false;
    eb1.update();

  }
  first= true;
  digitalWrite(8, LOW);

  stepper_rotore.rotate(-360);
  stepper_guida.rotate(guida_angle * direzione);
  

  num_rotazioni ++;
  lunghezza_raggiunta += diametro_filo;


  lcd.setCursor(2,0);
  lcd.print("Numero Spire");
  lcd.setCursor(7,1);
  lcd.print(num_rotazioni);

  if(lunghezza_raggiunta >= altezza_bobina)
  {
    if(direzione == 1)
      direzione = -1;
    else
    {
      direzione = 1;
    }

    lunghezza_raggiunta = 0;

  }

  if(num_rotazioni >= num_spire)
  {
    inserimento = 5;
  }


 }

 digitalWrite(8, HIGH);
}

//###########################################################################################################
//######################################## function definition ##############################################
//###########################################################################################################

void onEb1Encoder(EncoderButton& eb)
{
  if(num_encoder_rotation >= 0)
    num_encoder_rotation += multiplier * eb.increment();
  else
  {
    eb.resetPosition();
    num_encoder_rotation = 0;
    lcd.setCursor(6,1);
    lcd.print("    ");
  }

}


void multiple_click_hendler(EncoderButton& eb)
{
   
  switch(eb.clickCount())
  {
    case 1:
      multiplier = 1;
      break;

    case 2:
      multiplier = 2;
      break;

    case 3:
      multiplier = 5;
      break;

    case 4:
      multiplier = 10;
      break;

    case 5:
      multiplier = 50;
      break;

    case 6:
      multiplier = 500;
      break;

    default:
      multiplier = 1;
      break;
  }
 
}




void longClickHandler(EncoderButton& eb)
{
  if (inserimento != 4)
  {
    inserimento ++;
    eb.resetPosition();
    num_encoder_rotation = 0;
    multiplier = 1;
  }
  else
  {
    if ( pausa == true)
      pausa = false;
    else
      pausa = true;
  }

  if(inserimento == 6)
    inserimento = 1; 
}



