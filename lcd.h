#ifndef _LCD_H
#define _LCD_H


/* MODULE DEPENDENCIES */

/* CONSTANTS */

#define PORT_SPI        0
#define PIN_SPI_RESET   8
#define PIN_SPI_A0      6
#define PIN_SPI_CS      18

/* 8.400.000 ~ 1Hz signal */
#define US10_TICKS      84


/* TYPES */

/* PROTOTYPES */

void init_lcd(void);
void reset_lcd(void);
void copy_to_lcd(void);

void retardo(int ticks);
void do_write(unsigned char info, uint32_t a0);
void wr_data(unsigned char data);
void wr_cmd(unsigned char cmd);

uint8_t escribe_letra_L1(uint8_t letter, uint8_t posL1);
uint8_t escribe_letra_L2(uint8_t letter, uint8_t posL2);

void escribe_frase_L1(char* frase, int fraseLength);
void escribe_frase_L2(char* frase, int fraseLength);

uint8_t space_fill(char letter);


#endif // _LCD?H
