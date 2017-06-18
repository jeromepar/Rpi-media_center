#include <Arduino.h> 

class Potentiometer
{
public:
  Potentiometer(int pin_gray_in, int pin_black_in);
  int get_delta();                        // returns the delta value
  void handle_interrupts(); // increment or decrement the counter
  void update(); // call it as often as possible (each loop) to internally refresh delta
private:
   int delta;		//delta value
   int pin_gray;
   int pin_black;
   unsigned int date;
   unsigned int previous_gray_state;
};
