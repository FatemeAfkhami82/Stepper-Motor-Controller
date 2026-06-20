#include "stm32f10x.h"
#include <stdint.h>
#include "STM32F103-LCD-lib.h"
#include "STM32F103-Delay-lib.h"


#define LCD_RS 4
#define LCD_EN 5

#define KEYPAD_ROWS_MASK (0x0F << 10)	//set B13-B10 as rows
#define KEYPAD_ROW1_MASK (1<<13)
#define KEYPAD_ROW2_MASK (1<<12)
#define KEYPAD_ROW3_MASK (1<<11)
#define KEYPAD_ROW4_MASK (1<<10)

#define KEYPAD_COLS_MASK (0x0F << 6)	//set B9-B6 as cols
#define KEYPAD_PULLUPS   (0x0F << 6)

#define KEYPAD_ROW_PORT GPIOB
#define KEYPAD_COL_PORT GPIOB

const uint8_t keys[16] = {
        '1','2','3','A',
			  '4','5','6','B',
			  '7','8','9','C',
			  '*','0','#','D'
};

void keypad_init(void) {
    RCC->APB2ENR |= 0xFC;
    GPIOA->CRL = 0x44333333;
    GPIOB->CRL = 0x88444444;
    GPIOB->CRH = 0x44333388;
    GPIOC->CRH = 0x44444444;
    GPIOB->ODR |= KEYPAD_PULLUPS;
}

void keypad_set_all_rows_low(void) {
    KEYPAD_ROW_PORT->ODR &= ~(KEYPAD_ROWS_MASK);
}

uint8_t keypad_kbhit(void) {
    keypad_set_all_rows_low();
    delay_us(1);
    return ((KEYPAD_COL_PORT->IDR & KEYPAD_COLS_MASK) != KEYPAD_COLS_MASK);
}

uint8_t keypad_getkey(void) {
    uint32_t rows[4] = { KEYPAD_ROW1_MASK, KEYPAD_ROW2_MASK, KEYPAD_ROW3_MASK, KEYPAD_ROW4_MASK};

    for (int r = 0; r < 4; r++) {
        KEYPAD_ROW_PORT->ODR &= ~(rows[r]);
        delay_us(1);
        uint8_t cols = (KEYPAD_COL_PORT->IDR >> 6) & 0x0F;
        switch (cols) {
            case 0x0E: return keys[r * 4 + 3];
            case 0x0D: return keys[r * 4 + 2];
            case 0x0B: return keys[r * 4 + 1];
            case 0x07: return keys[r * 4 + 0];
        }
    }
    return 0;
}

int main(void) {
    keypad_init();
		LCD_init();
	
    while (1) {
			if (keypad_kbhit()) {
				LCD_cmd(LCD_1ST_LINE);
				LCD_putc(keypad_getkey());
			}
			delay_us(1000);
    }
}
