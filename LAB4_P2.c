#include <stm32f10x.h>
#include <stdio.h>
#include <STM32F103-LCD-lib.h>

#define BUTTON_PIN 6

volatile int isRunning = 0;
volatile int count = 0;
char durationStr[10] = {0};

void init_GPIO(void);
void init_TIM2(void);
int button_pushed(void);
void start_stop_cronometer(void);
void update_display(void);
void converter(int count, char *durationStr);
void delay_ms(uint32_t ms); // ?? ????????

int main() {
    // 72MHz Clock
    RCC->CR |= 0x01010081;
    RCC->CFGR |= 0x001d8402;
    RCC->APB1ENR |= (1<<0); // TIM2 clock enable
    RCC->APB2ENR |= 0xFC | (1 << 9); // GPIO and ADC1 clock enable

    LCD_init();
    init_GPIO();
    init_TIM2();

    LCD_cmd(LCD_CLEAR);
    LCD_puts("00.00");

    int last_button_state = 1;

    while (1) {
        int current_button_state = button_pushed();
        if (last_button_state == 1 && current_button_state == 0) {
            // Button was released and now pressed: falling edge
            start_stop_cronometer();
            delay_ms(200); // Debounce
        }
        last_button_state = current_button_state;

        update_display();
    }
}

void init_GPIO() {
    GPIOB->CRL &= ~(0xF << (4*6)); // Clear bits
    GPIOB->CRL |= (0x8 << (4*6));  // Input with pull-up/pull-down
    GPIOB->ODR |= (1 << 6);        // Pull-up
}

void init_TIM2() {
    TIM2->PSC = 7200 - 1;      // Prescaler
    TIM2->ARR = 100 - 1;       // Auto-reload value
    TIM2->DIER |= (1<<0);      // Enable update interrupt
    TIM2->CR1 |= (1<<7);       // Auto-reload preload enable
    NVIC_EnableIRQ(TIM2_IRQn);
}

int button_pushed() {
    return !(GPIOB->IDR & (1<<BUTTON_PIN)); // Active low
}

void TIM2_IRQHandler(void) {
    if(TIM2->SR & (1<<0)) {
        TIM2->SR &= ~(1<<0);
        if(isRunning) {
            count++;
            if(count >= 10000) { // 99.99 seconds
                count = 9999; // Stop at max
                isRunning = 0;
                TIM2->CR1 &= ~(1<<0); // Stop timer
            }
        }
    }
}

void start_stop_cronometer() {
    isRunning = !isRunning;
    if(isRunning) {
        TIM2->CR1 |= (1<<0); // Start timer
    } else {
        TIM2->CR1 &= ~(1<<0); // Stop timer
    }
}

void update_display() {
    converter(count, durationStr);
    LCD_cmd(LCD_1ST_LINE);
    LCD_puts(durationStr);
}

void converter(int count, char *durationStr) {
    int seconds = count / 100;
    int centiseconds = count % 100;

    durationStr[0] = (seconds / 10) + '0';
    durationStr[1] = (seconds % 10) + '0';
    durationStr[2] = '.';
    durationStr[3] = (centiseconds / 10) + '0';
    durationStr[4] = (centiseconds % 10) + '0';
    durationStr[5] = '\0';
}
