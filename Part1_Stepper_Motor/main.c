#include <stm32f10x.h>

#define STEP1 0x00C0
#define STEP2 0x0090
#define STEP3 0x0030
#define STEP4 0x0060

void delay(uint32_t ms) {
  SysTick->LOAD = (9000 * ms) - 1;
  SysTick->CTRL = 0x01;
  while (!(SysTick->CTRL & (1 << 16)));
  SysTick->CTRL = 0x00;
}

void step_write(uint16_t val) {
  GPIOA->ODR = val;
  delay(1000);
}

void step_cycle(void) {
  step_write(STEP1);
  step_write(STEP2);
  step_write(STEP3);
  step_write(STEP4);
}

void step_hold(void) {
  step_write(STEP1);
}

void gpio_init(void) {
  RCC->APB2ENR |= 0xFC;
  GPIOA->CRL = 0x33334444;
}

int main() {
  gpio_init();
  for (int i = 0; i < 6; i++) step_cycle();
  while (1) step_hold();
}
