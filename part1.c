#include <stm32f10x.h>
void delay_ms(uint16_t t);

int main() {
    const uint8_t steps[4] = {0x09, 0x0C, 0x06, 0x03};
    RCC->APB2ENR |= 0xFC;    /* Enable GPIOA clock */
		GPIOA->CRL = 0x44443333; /* PA0-PA3 as outputs */

    while (1) {

        GPIOA->ODR = steps[0];   /* go to next step */
			  delay_ms(50);
				GPIOA->ODR = steps[1];
			  delay_ms(50);
				GPIOA->ODR = steps[2];
			  delay_ms(50);
				GPIOA->ODR = steps[3];
        delay_ms(50); 
			}
}
void delay_ms(uint16_t t) {
    volatile unsigned long l = 0;
    for (uint16_t i = 0; i < t; i++)
        for (l = 0; l < 6000; l++) {}
}
