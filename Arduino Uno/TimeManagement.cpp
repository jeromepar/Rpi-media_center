/*
  TimeManagement.cpp - TimeManagement library
  Copyright (c) 2012 J. Parisot.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#if ARDUINO >= 100
#include <Arduino.h> 
#else
#include <WProgram.h> 
#endif


#include "TimeManagement.h"

//using namespace std;

TimeManagement::TimeManagement() {
time_ref_s=micros()/1000000;
time_second_ref_ms=0;
  
}

int TimeManagement::newSec(){
unsigned long compteur_ms=(micros()/1000-time_second_ref_ms);
//Serial.println(compteur_us);
if(compteur_ms>1000){
	time_second_ref_ms=(micros()/1000000)*1000;
	return 1;
}else {
	return 0;
}

}
void TimeManagement::setTime(long seconds){
//r�initialisation du ref des temps
time_ref_s=micros()/1000000-seconds;
time_second_ref_ms=0;
} //from the start of the week (Moonday 00h00)

char* TimeManagement::day(){
int day=(((micros()/1000000-time_ref_s)%SECS_PER_WEEK)/(SECS_PER_DAY));
switch(day)
{
  case 1:
  return("Monday");
  break;
  case 2:
  return("Tuesday");
  break;
  case 3:
  return("Wednesday");
  break;
  case 4:
  return("Thursday");
  break;
  case 5:
  return("Friday");
  break;
  case 6:
  return("Saturday");
  break;
  case 7:
  //FALLTHROUGH
  case 0:
  return("Sunday");
  break;
  default:
  return("Day number not found");
}
}
int TimeManagement::hour(){
return(((micros()/1000000-time_ref_s)%SECS_PER_DAY)/(SECS_PER_HOUR));
}
int TimeManagement::hour12(){
return(hour()%12);
}
int TimeManagement::minute(){
return(((micros()/1000000-time_ref_s)%SECS_PER_HOUR)/(SECS_PER_MIN));
}
int TimeManagement::second(){
return((micros()/1000000-time_ref_s)%(SECS_PER_MIN));
}

long TimeManagement::timeInSec(int day, int hour, int minute, int second){
  long sec=(long)second+(long)60*((long)minute+(long)60*((long)hour+(long)24*(long)day));
  return(sec);
}

long TimeManagement::getTime() {
  return((micros()/(long)1000000-time_ref_s));
}

 Alarm::Alarm(){
    state=0;
    nbrMaxRepetition=2;
    nbrRepetition=0;
    value=0;
    delayValue=5*60;
    currentValue=0;
  }
  void Alarm::disable(){
    state=0;
  }
  void Alarm::enable(){
    state=1;
  }
  int Alarm::getState(){
    return(state);
  }
  void Alarm::setValue(long sec){
    value=sec;    
    currentValue=value;
    Serial.print("Alarme initialisee a : ");
    Serial.print(currentValue);
    Serial.println(" secondes");
  }
  void Alarm::setValue(int hour, int minute){
   setValue(timeInSec(hour,minute));
  }
  void Alarm::setDelayValue(long sec){
    delayValue=sec;
  }
  void Alarm::stopAlarm(){
    currentValue=value;
    nbrRepetition=0;
    //Serial.println("Stop");
    
  }
  void Alarm::delayAlarm(){
    
    if (nbrRepetition>nbrMaxRepetition) {
      currentValue=currentValue+60; //sonne toutes les minutes après le nbr max de répétitions
    }else{
    currentValue+=delayValue;
    nbrRepetition=+1;
    }
    //Serial.println("Delay");
  }
  
long Alarm::timeInSec(int hour, int minute){
  long sec=(long)60*((long)minute+(long)60*(long)hour);
  return(sec);
}
int Alarm::ringing(long currentTime){
  //Serial.print("test de l'alarme pour: ");
  //Serial.print(currentTime%SECS_PER_DAY);
  //Serial.println(" secondes");
  if((state==0) || ((currentTime%SECS_PER_DAY)!=currentValue)) {
    return 0;
  }else {
    return 1;
  }
}

