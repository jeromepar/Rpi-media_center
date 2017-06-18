//I2C adress for LCD screen
#define LCD_ADRESS 0x27

//Potentiometer pins
#define POT_GRAY_PIN 11
#define POT_BLACK_PIN 12

//IR Receiver
#define IR_PIN 4
#define AIN_IR_CONTROL

//LED PIN
#define POWER_LED_PIN 8

//MACRO
#define TO_CHAR_POINTER(a) (char *)&a

#ifdef NEC_IR_CONTROL
//code for the dealExtreme remote control (NEC)
#define _POWER 	0xFFA25D
#define _MODE 	0xFF629D
#define _MUTE 	0xFFE21D
#define _PLAY 	0xFF22DD
#define _PREVIOUS 0xFF02FD
#define _NEXT 	0xFFC23D
#define _EQ 	0xFFE01F
#define _MINUS 	0xFFA857
#define _PLUS 	0xFF906F
#define _ZERO 	0xFF6897
#define _RANDOM 0xFF9867
#define _U_SD 	0xFFB04F
#define _ONE 	0xFF30CF
#define _TWO 	0xFF18E7
#define _THREE 	0xFF7A85
#define _FOUR 	0xFF10EF
#define _FIVE 	0xFF38C7
#define _SIX 	0xFF5AA5
#define _SEVEN 	0xFF42BD
#define _EIGHT 	0xFF4AB5
#define _NINE 	0xFF52AD
#endif

#ifdef AIN_IR_CONTROL
//code for the AIN system audio RM-Z20051
#define _SLEEP          0x61
#define _POWER          0xA81
#define _DISPLAY        0xD21
#define _CLOCK_SELECT   0x61
#define _CLOCK_SET      0xA61
#define _TUNER_MEM      0x701
#define _PLAY_MODE      0x14B9C
#define _REPEAT         0xB4B9C
#define _TAPE           0xC41
#define _CD             0xA41
#define _TUNER          0xF01
#define _FUNCTION       0x4B09
#define _PREVIOUS       0xCB9C
#define _NEXT           0x8CB9C
#define _BACK           0xCCB9C
#define _FORWARD        0x2CB9C
#define _PLAY           0x4CB9C
#define _PAUSE          0x9CB9C
#define _STOP           0x1CB9C
#define _CLEAR          0xF0B9C
#define _PLUS           0x481
#define _ENTER          0x3EB9C
#define _EQ             0x6309
#define _DISC_SKIP      0x7CB9C
#define _ALBUM_MINUS    0x74B9C
#define _MINUS          0xC81
#define _ALBUM_PLUS     0xF4B9C
#endif
