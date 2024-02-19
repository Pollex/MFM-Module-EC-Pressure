#include "../../include/mcu/util.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <avr/xmega.h>

#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#define cyclesToUs(cycles) ((cycles * 1000000UL) / F_CPU)

#define TCA0_OVF 256
#define TCA0_DIV 16
#define TCA0_DIV_gc TCA_SINGLE_CLKSEL_DIV16_gc

#define US_PER_TICK cyclesToUs(TCA0_DIV)          // 16 us at 4MHz
#define US_PER_OVF cyclesToUs(TCA0_OVF *TCA0_DIV) // 1024 us at 4MHz
#define MS_PER_OVF (US_PER_OVF / 1000)
#define MS_FRACT_OVF ((US_PER_OVF % 1000) >> 3)
#define MS_FRACT_MAX (1000 >> 3)

volatile unsigned long timer_millis = 0;
volatile unsigned long timer_overflow = 0;
volatile unsigned char timer_millis_fract = 0;

void delay_init(void)
{
  // Initialize timer
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_NORMAL_gc;
    TCA0.SINGLE.PER = TCA0_OVF;
    TCA0.SINGLE.CTRLA = TCA0_DIV_gc | TCA_SINGLE_ENABLE_bm;
  }
  sei();
}

uint32_t millis(void)
{
  uint32_t v;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    v = timer_millis;
  }
  return v;
}

uint32_t micros(void)
{
  uint32_t c, o;
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
  {
    c = TCA0.SINGLE.CNT;
    o = timer_overflow;
  }

  return ((o * TCA0_OVF) + c) * (US_PER_TICK);
}

void delay_ms(uint32_t ms)
{
  uint32_t start = millis();

  while ((millis() - start) < ms)
    ;
}

void delay_us(uint32_t us)
{
  uint32_t start = micros();

  while ((micros() - start) < us)
    ;
}

// Timer overflow
ISR(TCA0_OVF_vect)
{
  volatile unsigned long m = timer_millis;
  volatile unsigned char f = timer_millis_fract;

  m += MS_PER_OVF;
  f += MS_FRACT_OVF;
  if (f >= MS_FRACT_MAX)
  {
    f -= MS_FRACT_MAX;
    m++;
  }
  timer_overflow++;
  timer_millis = m;
  timer_millis_fract = f;
  TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}
