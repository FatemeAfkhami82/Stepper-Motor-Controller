#include <stm32f10x.h>
#include <stdio.h>
#include <stdlib.h>
#define LCD_RS 5
#define LCD_EN 6
void lcd_init(void);
void lcd_putValue(unsigned char value);
void lcd_sendCommand(unsigned char cmd);
void lcd_sendData(unsigned char data);
void lcd_gotoxy(unsigned char x, unsigned char y);
void lcd_print(char * str);
void delay_ms(uint16_t t);
void delay_us(uint16_t t);
uint8_t keypad_kbhit(void);
uint8_t keypad_getkey(void);

int main() {
RCC->APB2ENR |= 0xFC; // Enable clocks for GPIO ports
GPIOA->CRL = 0x43333334; // PA1-PA6 as outputs for lcd 

// init keypad pins
GPIOB->CRL = 0x88444444; // PB6 and PB7 as inputs 
GPIOB->CRH = 0x33334488; // PB15-PB12 as outputs, PB8 and PB9 as inputs 
GPIOB->ODR |= (0xF<<6); // pull-up PB9-PB6 

lcd_init();
lcd_gotoxy(1,1); // move cursor to 1,1 
lcd_print("Enter Number: ");// show the message 
	
uint8_t keypressed;
uint8_t i = 0;

while (1) {
	keypressed = keypad_getkey(); 
	if (keypressed == 'y') { // if '=' is pressed
		i = 0;
	}
	else if (keypressed == 'e') { // if 'ON/C' is pressed
		lcd_sendCommand(0x01); // Clear LCD
    lcd_gotoxy(1, 1);
    lcd_print("Enter Number: ");
	} 
 else if ((keypressed != 0) && (keypressed != 'e') && (keypressed != 'y')) {
	 lcd_gotoxy(1 + i, 2);
   lcd_sendData((unsigned char)keypressed);
	 i = i + 1;
 }
 delay_ms(2);
}
}

void lcd_init()
{
	GPIOA->ODR &= ~(1<<LCD_EN); // LCD_EN = 0 
  delay_us(3000); // wait 3ms 
	lcd_sendCommand(0x33); // send 0x33 for init. 
	lcd_sendCommand(0x32); // send 0x32 for init. 
	lcd_sendCommand(0x28); // init. LCD 2 line,5x7 matrix 
	lcd_sendCommand(0x0e); // display on, cursor on 
	lcd_sendCommand(0x01); // clear LCD 
	delay_us(2000); // wait 2ms 
	lcd_sendCommand(0x06); // shift cursor right 
}
void lcd_sendCommand(unsigned char cmd)
{
	GPIOA->ODR &= ~ (1<<LCD_RS); // RS = 0 for command 
	lcd_putValue(cmd);
}
void lcd_sendData(unsigned char data)
{
	GPIOA->ODR |= (1<<LCD_RS); // RS = 1 for data 
	lcd_putValue(data);
}
void lcd_putValue(unsigned char value)
{
	GPIOA->BRR = 0x1E; // clear PA1-PA4 
	GPIOA->BSRR = (value>>3)&0x1E; // put high nibble on PA1-PA4 
	GPIOA->ODR |= (1<<LCD_EN); // EN = 1 for H-to-L pulse 
	delay_us(1); // make EN pulse wider 
	GPIOA->ODR &= ~ (1<<LCD_EN); // EN = 0 for H-to-L pulse 
	delay_us(100); // wait 
	GPIOA->BRR = 0x1E; // clear PA1-PA4 
	GPIOA->BSRR = (value<<1)&0x1E; // put low nibble on PA1-PA4 
	GPIOA->ODR |= (1<<LCD_EN); // EN = 1 for H-to-L pulse 
	delay_us(1); // make EN pulse wider 
	GPIOA->ODR &= ~(1<<LCD_EN); // EN = 0 for H-to-L pulse 

	delay_us(100); // wait 
}
void lcd_gotoxy(unsigned char x, unsigned char y)
{
	const unsigned char firstCharAdr[]={0x80,0xC0,0x94,0xD4};
	lcd_sendCommand(firstCharAdr[y-1] + x - 1);
	delay_us(100);
}
void lcd_print(char * str)
{
	unsigned char i = 0;
	while(str[i] != 0) // while it is not end of string 
{
	lcd_sendData(str[i]); // show str[i] on the LCD 
	i++;
}
}
// keypad rows: PB15-PB12 cols: PB9-PB6
// The function checks if a key is pressed or not; if pressed, it returns 1 and if not, it returns0.
uint8_t keypad_kbhit() 
{
	GPIOB->ODR &= ~(0x0F<<12); /* make all rows ones */
	delay_us(1);
	if(((GPIOB->IDR>>6)&0xF) == 0xF)
		return 0;
	else
		return 1;
}
// The function returns the pressed key. It returns 0 if no key is pressed.
uint8_t keypad_getkey()
{
	uint8_t keypadLookup[16]={'7','8','9','a','4','5','6','b','1','2','3','c','e','0','y','x'};
	// {'1','2','3','A','4','5','6','B','7','8','9','C','*','0','#','D'};
	const uint32_t rowSelect[4]={
		0x1000E000, // Row3-Row0 = 1110 
		0x2000D000, // Row3-Row0 = 1101 
		0x4000B000, // Row3-Row0 = 1011 
		0x80007000};// Row3-Row0 = 1011 
	if(keypad_kbhit() == 0) // if no key is pressed 
		return 0;
	for(int r = 0; r <= 3; r++) // rows 0 to 3 
	{
		GPIOB->BSRR = rowSelect[r]; // ground row r and make the others high 
		delay_us(1); // wait for the columns to be updated 
		uint8_t cols = (GPIOB->IDR>>6)&0xF;
		switch(cols)
			{
				case 0x0E: return keypadLookup[r*4+0]; // col0 is low 
				case 0x0D: return keypadLookup[r*4+1]; // col1 is low 
				case 0x0B: return keypadLookup[r*4+2]; // col2 is low 
				case 0x07: return keypadLookup[r*4+3]; // col3 is low 
				}
	}
	return 0;
}

void delay_ms(uint16_t t)
{
	volatile unsigned long l = 0;
	for(uint16_t i = 0; i < t; i++)
	for(l = 0; l < 6000; l++) {}
}

void delay_us(uint16_t t)
{
	volatile unsigned long l = 0;
	for(uint16_t i = 0; i < t; i++)
	for(l = 0; l < 6; l++) {}
}