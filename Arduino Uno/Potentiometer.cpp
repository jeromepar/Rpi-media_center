//#include <avr/interrupt.h>


#include "Potentiometer.h"

/****************** end of static functions ******************************/

Potentiometer::Potentiometer(int pin_gray_in, int pin_black_in)
{
  delta = 0;
  pin_gray = pin_gray_in;
  pin_black = pin_black_in;
  
  pinMode(pin_gray, INPUT);
  pinMode(pin_black, INPUT);
  
  // activate the pull-up resistors
  digitalWrite(pin_gray, HIGH);
  digitalWrite(pin_black, HIGH);
  
  previous_gray_state = 1; //default state
}

int Potentiometer::get_delta()
{
	int out = delta;
	delta = 0;
	return(out);
}

void Potentiometer::update(){
	unsigned int state_gray_pin = digitalRead(pin_gray);
	
	if ( (state_gray_pin == 0) && (state_gray_pin != previous_gray_state)) //falling edge
		if ((millis()-date)>10) {
		  date = millis();

		  unsigned int state_black_pin = digitalRead(pin_black);
		  if(state_black_pin == LOW) {
			delta++;
		  } else {
			delta--;
		  }
    }
	previous_gray_state = state_gray_pin;
}
