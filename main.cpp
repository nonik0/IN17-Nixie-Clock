#include "Arduino.h"
#include "avr/pgmspace.h"
#include "Wire.h"
#include "avr/io.h"
#include "avr/interrupt.h"

#define BLANK 0xFF

enum State
{
  DISPLAYOFF,
  TIME,
  MANUAL,
  ANIMATION,
  CATHODE_CLEAN,
};

// global variables
unsigned char INDEX = 1;

unsigned char DIGIT6 = 6;
unsigned char DIGIT5 = 5;
unsigned char DIGIT4 = 4;
unsigned char DIGIT3 = 3;
unsigned char DIGIT2 = 2;
unsigned char DIGIT1 = 1;

volatile unsigned char HOUR = 0;
volatile unsigned char MINUTE = 0;
volatile unsigned char SECOND = 0;

volatile unsigned char SECONDS_LEFT = 0;

String serialIn;
State state = TIME;

// timer1 used for RTC
ISR(TIMER1_COMPA_vect)
{
  if (++SECOND == 60)
  {
    SECOND = 0;

    if (++MINUTE == 60)
    {
      MINUTE = 0;

      if (++HOUR == 24)
        HOUR = 0;
    }
  }

  if (SECONDS_LEFT > 0)
    SECONDS_LEFT--;

  if (state == TIME)
  {
    DIGIT6 = HOUR / 10;
    DIGIT5 = HOUR % 10;
    DIGIT4 = MINUTE / 10;
    DIGIT3 = MINUTE % 10;
    DIGIT2 = SECOND / 10;
    DIGIT1 = SECOND % 10;
  }
}

// timer2 used for nixie tube multiplexing
ISR(TIMER2_COMPA_vect)
{
  PORTB = 0x00; // blank cathode
  PORTD = 0x00; // blank anodes

  if (state == DISPLAYOFF)
    return;

  switch (INDEX++)
  {
  case 1:
    PORTB |= DIGIT6;                         // set cathode
    PORTD = (DIGIT6 == BLANK) ? 0x00 : 0x04; // turn on anode for digit
    break;
  case 2:
    PORTB |= DIGIT5;
    PORTD = (DIGIT5 == BLANK) ? 0x00 : 0x08;
    break;
  case 3:
    PORTB |= DIGIT4;
    PORTD = (DIGIT4 == BLANK) ? 0x00 : 0x10;
    break;
  case 4:
    PORTB |= DIGIT3;
    PORTD = (DIGIT3 == BLANK) ? 0x00 : 0x20;
    break;
  case 5:
    PORTB |= DIGIT2;
    PORTD = (DIGIT2 == BLANK) ? 0x00 : 0x40;
    break;
  case 6:
    PORTB |= DIGIT1;
    PORTD = (DIGIT1 == BLANK) ? 0x00 : 0x80;
    INDEX = 1;
    break;
  }
}

// void animation()
// {
//   MANUAL_DISPLAY_TIME = 0xFF;

//   for (int i = 6; i >= 1; i--)
//   {
//     switch (i)
//     {
//     case 1:
//       DIGIT1 = BLANK;
//       break;
//     case 2:
//       DIGIT2 = BLANK;
//       break;
//     case 3:
//       DIGIT3 = BLANK;
//       break;
//     case 4:
//       DIGIT4 = BLANK;
//       break;
//     case 5:
//       DIGIT5 = BLANK;
//       break;
//     case 6:
//       DIGIT6 = BLANK;
//       break;
//     }

//     delay(50);
//   }

//   delay(50);

//   for (int i = 6; i >= 1; i--)
//   {
//     switch (i)
//     {
//     case 1:
//       DIGIT1 = SECOND % 10;
//       break;
//     case 2:
//       DIGIT2 = SECOND / 10;
//       break;
//     case 3:
//       DIGIT3 = MINUTE % 10;
//       break;
//     case 4:
//       DIGIT4 = MINUTE / 10;
//       break;
//     case 5:
//       DIGIT5 = HOUR % 10;
//       break;
//     case 6:
//       DIGIT6 = HOUR / 10;
//       break;
//     }

//     delay(50);
//   }

//   delay(50);

//   MANUAL_DISPLAY_TIME = 0;
// }

// void cathode_clean()
// {
//   MANUAL_DISPLAY_TIME = 0xFF;

//   for (int i = 0; i < 200; i++)
//   {
//     DIGIT6 = random(0, 10);
//     DIGIT5 = random(0, 10);
//     DIGIT4 = random(0, 10);
//     DIGIT3 = random(0, 10);
//     DIGIT2 = random(0, 10);
//     DIGIT1 = random(0, 10);
//     delay(20);
//   }

//   MANUAL_DISPLAY_TIME = 0;
// }

void setup()
{
  Serial.begin(9600);

  // configure pins
  for (int i = 0; i <= 12; i++)
    pinMode(i, OUTPUT);

  cli(); // disable global interrupts

  // timer1 1Hz interrupt
  TCCR1A = 0x00;
  TCCR1B = 0x00;
  TCNT1 = 0x00;
  OCR1A = 0x3D08;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);

  // timer2 1kHz interrupt
  TCCR2A = 0x00;
  TCCR2B = 0x00;
  TCNT2 = 0x00;
  OCR2A = 0xF9;
  TCCR2A |= (1 << WGM21);
  TCCR2B |= (1 << CS22);
  TIMSK2 |= (1 << OCIE2A);

  sei(); // enable global interrupts
}

void loop()
{
  if (Serial.available() > 0)
  {
    // read input from ESP8266/ESPHome
    serialIn = Serial.readString();

    switch (serialIn[0])
    {
    case '0':
      state = DISPLAYOFF;
      break;
    case '1':
      state = TIME;
      break;
    case 'T':
      HOUR = (serialIn[1] - '0') * 10 + serialIn[2] - '0';
      MINUTE = (serialIn[3] - '0') * 10 + serialIn[4] - '0';
      SECOND = (serialIn[5] - '0') * 10 + serialIn[6] - '0';
      break;
    case 'M':
      state = MANUAL;

      // TODO: transition animation
      DIGIT6 = serialIn[1] != '_' ? serialIn[1] - '0' : BLANK;
      DIGIT5 = serialIn[2] != '_' ? serialIn[2] - '0' : BLANK;
      DIGIT4 = serialIn[3] != '_' ? serialIn[3] - '0' : BLANK;
      DIGIT3 = serialIn[4] != '_' ? serialIn[4] - '0' : BLANK;
      DIGIT2 = serialIn[5] != '_' ? serialIn[5] - '0' : BLANK;
      DIGIT1 = serialIn[6] != '_' ? serialIn[6] - '0' : BLANK;

      SECONDS_LEFT = 5;
      break;
    default:
      break;
    }
  }

  if (state == MANUAL && SECONDS_LEFT == 0)
  {
    state = TIME;
  }
}