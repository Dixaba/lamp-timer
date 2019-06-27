#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define BUTTON PB4
#define LED PB3
#define OUTPUT PB1
#define RANDOM 1

// 10 second counter
volatile uint16_t timerCounter;
volatile bool buttonPressed;
volatile bool step;

uint16_t timeStamps[2] = {0, 0};
uint16_t switches[2] = {0, 0};

uint8_t done;

uint16_t time;
uint16_t random_number = 0;

bool isConfigured;

uint16_t lfsr16_next(uint16_t n)
{
  return (n >> 0x01U) ^ (-(n & 0x01U) & 0xB400U);
}

void random_init()
{
  random_number = 12345;
}

int16_t random()
{
  random_number = lfsr16_next(random_number);
  return (random_number % 1024 - 512) / 2;
}

ISR(TIM0_OVF_vect)
{
  // Debounce
  if (!isConfigured)
    {
      static uint8_t currState = PINB & (1 << BUTTON);
      static uint8_t lastState = currState;
      static uint16_t count = 0;
      uint8_t state = PINB & (1 << BUTTON);

      if (state != lastState)
        {
          count = 100;
        }

      if (count > 0)
        {
          count--;
        }
      else
        {
          if (state != currState)
            {
              currState = state;

              if (currState > 0)
                {
                  buttonPressed = true;
                }
            }
        }

      lastState = state;
    }

  timerCounter++;

  if (timerCounter == 40150) // 10 seconds
    {
      timerCounter = 0;
      step = true;
    }
}

int main()
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
  // clock frequency / 1
  TCCR0B = (1 << CS01);
  TIMSK0 |= (1 << TOIE0);
  // Set LED and system outputs as outputs
  DDRB |= (1 << LED) | (1 << OUTPUT);
  // Write 0
  PORTB |= (1 << LED);
  ADCSRA &= ~(1 << ADEN);
  // Next command will change prescaler
  CLKPR = (1 << CLKPCE);
  // Set prescaler 256
  CLKPR = (0 << CLKPCE) | (0 << CLKPS3);
  random_init();
  timerCounter = time = done = 0;
  isConfigured = buttonPressed = false;
  sei();

  while (1)
    {
      if (step)
        {
          step = false;
          time++;

          if (time == 8640)
            {
              time = 0;
              switches[0] = timeStamps[0] + random();
              switches[1] = timeStamps[1] + random();
            }

          if (isConfigured)
            {
              PORTB ^= (1 << LED);

              if (time == timeStamps[0])
                {
                  PORTB |= (1 << OUTPUT);
                }
              else
                if (time == timeStamps[1])
                  {
                    PORTB &= ~(1 << OUTPUT);
                  }
            }
        }

      if (buttonPressed && !isConfigured)
        {
          buttonPressed = false;
          timeStamps[done] = time;
          switches[done] = time + random();
          done++;
          PORTB ^= ((1 << OUTPUT));
          isConfigured = (done == 2);
        }

      sleep_cpu(); //и в самом конце цикла - уходим в сон.
    }
}
