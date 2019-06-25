#include <avr/io.h> // Для компиляции не из IDE - определения регистров
#include <limits.h>
#include <avr/sleep.h> // Да, будем спать
#include <avr/interrupt.h> // будет использоваться прерывание

//трачу два байта оперативки из 64 на глобальный таймер
volatile uint16_t timerCounter;

volatile uint16_t timeStamps[4] = {0, 0, 0, 0};


uint16_t time;

// часики - переполнение таймера инкрементирует globalTimer каждые 1/37 секунды
ISR(TIM0_OVF_vect)
{
  timerCounter++;

  if (timerCounter == 183) // 10 seconds
    {
      timerCounter = 0;
    }
}

// Button
ISR(INT0_vect)
{
}

int main()
{
  CLKPR = (1 << CLKPCE);
  CLKPR = (0 << CLKPCE) | (0 << CLKPS3) | (1 << CLKPS2) | (1 << CLKPS1) |
          (0 << CLKPS0);
  timerCounter = 0;
  set_sleep_mode(SLEEP_MODE_IDLE);
  sleep_enable();
  // clock frequency / 1024
  TCCR0B = _BV(CS02) | _BV(CS00);
  TIMSK0 |= _BV(TOIE0);
  DDRB |= (1 << PB0);      // pinMode(PB0, OUTPUT);
  PORTB &= ~(1 << PB0);    // digitalWrite(PB0, LOW);}
  time = 0;
  sei();

  while (1)
    {
      if (timerCounter == 0)
        {
          PORTB ^= (1 << PB0);
        }

      sleep_cpu(); //и в самом конце цикла - уходим в сон.
    }
}
