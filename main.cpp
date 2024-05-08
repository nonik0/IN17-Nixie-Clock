#include "Arduino.h"
#include "Wire.h"
#include "avr/io.h"
#include "avr/interrupt.h"
#include "avr/pgmspace.h"

#define BLANK 0xFF

#define SHOW_TIME()                       \
  char hour = is24Hour                    \
                  ? Hour                  \
                  : (Hour + 11) % 12 + 1; \
  Digits[0] = hour / 10;                  \
  Digits[1] = hour % 10;                  \
  Digits[2] = Minute / 10;                \
  Digits[3] = Minute % 10;                \
  Digits[4] = Second / 10;                \
  Digits[5] = Second % 10;

enum ClockState
{
  DISPLAYOFF,
  TIME,
  MANUAL,
  CATHODE_CLEAN,
};

const int NUM_DIGITS = 6;
const unsigned long CYCLE_PERIOD_MS = 20;
const unsigned char DIGIT_ANODES[NUM_DIGITS] = {0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
const int NUM_MANUAL_CYCLES = 20;
const int NUM_CLEANING_CYCLES = 100;

// current digit values
unsigned char Digits[NUM_DIGITS] = {6, 5, 4, 3, 2, 1};
unsigned char Index = 0;

// current time values
volatile unsigned char Hour = 0;
volatile unsigned char Minute = 0;
volatile unsigned char Second = 0;

// for manual display tracking
bool Blanks[NUM_DIGITS] = {false, false, false, false, false, false};
volatile unsigned char ManualSecondsLeft = 0;
bool FinishedCycleIn = false;
bool StartedCycleOut = false;

// for cycle animation tracking
unsigned long LastCycleMillis = 0;
unsigned int CyclesLeft;

String SerialIn;
ClockState State = TIME;
bool is24Hour = true;

// timer1 used for RTC
ISR(TIMER1_COMPA_vect)
{
  if (ManualSecondsLeft > 0)
    ManualSecondsLeft--;

  if (++Second == 60)
  {
    Second = 0;

    if (++Minute == 60)
    {
      Minute = 0;

      if (++Hour == 24)
      {
        Hour = 0;
      }
    }
  }

  if (State == TIME)
  {
    SHOW_TIME()
  }
}

// timer2 used for nixie tube multiplexing
ISR(TIMER2_COMPA_vect)
{
  PORTB = 0x00; // blank cathode
  PORTD = 0x00; // blank anodes

  if (State == DISPLAYOFF)
    return;

  PORTB = Digits[Index];                                         // set cathode
  PORTD = (Digits[Index] == BLANK) ? 0x00 : DIGIT_ANODES[Index]; // turn on anode for digit
  Index = (Index + 1) % NUM_DIGITS;
}

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
    SerialIn = Serial.readString();

    switch (SerialIn[0])
    {
    case '0':
      State = DISPLAYOFF;
      break;
    case '1':
      State = TIME;
      break;

    case 'h':
      is24Hour = false;
      break;
    case 'H':
      is24Hour = true;
      break;

    case 'T':
      Hour = (SerialIn[1] - '0') * 10 + SerialIn[2] - '0';
      Minute = (SerialIn[3] - '0') * 10 + SerialIn[4] - '0';
      Second = (SerialIn[5] - '0') * 10 + SerialIn[6] - '0';
      break;

    case 'C':
      if (State != TIME)
        break;

      State = CATHODE_CLEAN;
      LastCycleMillis = 0;
      CyclesLeft = NUM_CLEANING_CYCLES;
      break;
    case 'M':
      if (State != TIME)
        break;

      State = MANUAL;
      LastCycleMillis = 0;
      CyclesLeft = NUM_MANUAL_CYCLES;
      ManualSecondsLeft = 5;
      FinishedCycleIn = false;
      StartedCycleOut = false;

      for (int i = 0; i < NUM_DIGITS; i++)
      {
        Blanks[i] = SerialIn[i + 1] == '_';
        Digits[i] = Blanks[i] ? random(0, 10) : SerialIn[i + 1] - '0';
      }
      break;

    default:
      break;
    }
  }

  if (CyclesLeft > 0)
  {
    if (millis() - LastCycleMillis > CYCLE_PERIOD_MS)
    {
      for (int i = 0; i < NUM_DIGITS; i++)
        Digits[i] = (Digits[i] + 1) % 10;

      LastCycleMillis = millis();
      CyclesLeft--;
    }
  }
  else if (State == MANUAL)
  {
    if (ManualSecondsLeft > 0)
    {
      if (!FinishedCycleIn)
      {
        // cycle in complete, restore blanks
        for (int i = 0; i < NUM_DIGITS; i++)
          if (Blanks[i])
            Digits[i] = BLANK;

        FinishedCycleIn = true;
      }
    }
    else // manual display time over, cycle out
    {
      if (!StartedCycleOut)
      {
        StartedCycleOut = true;
        LastCycleMillis = 0;
        CyclesLeft = NUM_MANUAL_CYCLES;

        // replace blanks with digit when cycling
        for (int i = 0; i < NUM_DIGITS; i++)
          if (Blanks[i])
            Digits[i] = random(0, 10);
      }
      else
      {
        SHOW_TIME()
        State = TIME;
      }
    }
  }
  else if (State == CATHODE_CLEAN)
  {
    SHOW_TIME()
    State = TIME;
  }
}