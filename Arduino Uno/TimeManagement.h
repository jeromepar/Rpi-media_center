/*
TimeManagement.h - TimeManagement library
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




/* Useful Constants */
#define SECS_PER_MIN  60
#define SECS_PER_HOUR 3600
#define SECS_PER_DAY  86400
#define SECS_PER_WEEK  604800
#define DAYS_PER_WEEK 7

class TimeManagement
{

private:
 long time_ref_s;
 unsigned long time_second_ref_ms;


public:
TimeManagement();
int newSec();
void setTime(long seconds); //from the start of the week (Moonday 00h00)
long timeInSec(int day, int hour, int minute, int second);
char* day();
int hour();
int hour12();
int minute();
int second();
long getTime();
};

class Alarm
{
  private:
  int activated;
  int state;
  int nbrRepetition;
  int nbrMaxRepetition;
  long value;
  long currentValue;
  long delayValue;
  long timeInSec(int hour, int minute);
  
  public:
  Alarm();
  void disable();
  void enable();
  int getState();
  void setValue(long sec);
  void setValue(int hour, int minute);
  void setDelayValue(long sec);
  void stopAlarm();
  void delayAlarm();
  int ringing(long currentTime);
};

