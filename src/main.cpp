#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define BUTTON PB4
#define LED PB3
#define OUTPUT PB1

// Flags
volatile uint16_t timerCounter;
volatile bool buttonPressed;
volatile bool step;

// Time when pressed and when to switch output
uint16_t timeStamps[2] = {0, 0};
uint16_t switches[2] = {0, 0};

// Init counter
uint8_t done;

// Time counter
uint16_t time;

uint16_t random_number = 0;

// It means exactly what are you thinking about
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

// Timer overflow interrupt
ISR(TIM0_OVF_vect)
{
  // Debounce code
  // If already configured - just ignore button
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

  // 10 seconds timer
  timerCounter++;

  if (timerCounter == 40150) // Counted by experiment
    {
      timerCounter = 0;
      step = true;
    }
}

int main()
{
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
  // Timer works at clock frequency / 1
  TCCR0B = (1 << CS01);
  TIMSK0 |= (1 << TOIE0);
  // Set LED and system outputs as outputs
  DDRB |= (1 << LED) | (1 << OUTPUT);
  // Write values
  PORTB |= (1 << LED);
  ADCSRA &= ~(1 << ADEN);
  // Next command will change prescaler
  CLKPR = (1 << CLKPCE);
  // Set prescaler to 256
  CLKPR = (0 << CLKPCE) | (0 << CLKPS3);
  // Init RNG
  random_init();
  // Init variables
  timerCounter = time = done = 0;
  isConfigured = buttonPressed = false;
  // Allow interrupts
  sei();

  while (1)
    {
      // 10 seconds passed
      if (step)
        {
          // Reset flag
          step = false;
          time++;

          if (time == 8640) // Exactly one day (24*60*60/10)
            {
              time = 0;
              // Recalculate switch times with RNG
              switches[0] = timeStamps[0] + random();
              switches[1] = timeStamps[1] + random();
            }

          if (isConfigured)
            {
              // Need to blink LED
              PORTB ^= (1 << LED);

              // Should we switch output?
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

      // Setup
      if (buttonPressed && !isConfigured)
        {
          buttonPressed = false;
          timeStamps[done] = time;
          switches[done] = time + random();
          done++;
          PORTB ^= ((1 << OUTPUT));
          isConfigured = (done == 2);
        }

      // Save battery
      sleep_cpu();
    }
}
