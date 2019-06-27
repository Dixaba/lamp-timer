#include <avr/io.h>
// #include <avr/sleep.h>
#include <avr/interrupt.h>

#define BUTTON PB4
#define LED PB3
#define OUTPUT PB1
#define RANDOM 1

// 10 second counter
volatile uint16_t timerCounter;

volatile uint16_t timeStamps[2] = {0, 0};

uint16_t time;

bool isConfigured;
bool isActive;

volatile bool buttonPressed;

volatile bool step;

ISR(TIM0_OVF_vect)
{
  // Debounce
  // static uint8_t currState = 0;
  // static uint8_t lastState = 0;
  // uint8_t state = PINB & (1 << BUTTON);
  // static uint8_t count = 0;
  // if (state != lastState)
  //   {
  //     count = 5;
  //   }
  // if (count > 0)
  //   {
  //     count--;
  //   }
  // else
  //   {
  //     if (state != currState)
  //       {
  //         currState = state;
  //         if (currState == 1)
  //           {
  //             buttonPressed = true;
  //           }
  //       }
  //   }
  // lastState = state;
  timerCounter++;

  if (timerCounter == 1) // ~10 seconds
    {
      timerCounter = 0;
      step = true;
    }
}

uint16_t rand2()
{
  // Start convertion
  ADCSRA |= (1 << ADSC);

  // Wait for convertion to complete
  while (ADCSRA & (1 << ADSC));

  return ADC;
}

int main()
{
  cli();
  // set_sleep_mode(SLEEP_MODE_IDLE);
  // sleep_enable();
  // clock frequency / 1024
  TCCR0B = (1 << CS02) | (1 << CS00);
  TIMSK0 |= (1 << TOIE0);
  // Set LED and system outputs as outputs
  DDRB |= (1 << LED) | (1 << OUTPUT);
  // Write 0
  PORTB &= ~((1 << LED) | (1 << OUTPUT));
  // AREF = AVcc, port selected by defines
  // ADMUX = (1 << REFS0) | (RANDOM);
  // // ADC Enable and prescaler of 128
  // ADCSRA = (1 << ADEN) |
  //          (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  // Next command will change prescaler
  CLKPR = (1 << CLKPCE);
  // Set prescaler 256
  CLKPR = (0 << CLKPCE) |
          (0 << CLKPS3) | (1 << CLKPS2) | (1 << CLKPS1) | (0 << CLKPS0);
  timerCounter = time = 0;
  isConfigured = isActive = buttonPressed = false;
  sei();

  while (1)
    {
      if (step)
        {
          step = false;
          PORTB ^= (1 << LED);
        }

      // if (buttonPressed)
      //   {
      //     buttonPressed = false;
      //     PORTB ^= (1 << LED);
      //   }
      // sleep_cpu(); //и в самом конце цикла - уходим в сон.
    }
}
