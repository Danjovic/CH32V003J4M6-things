#ifndef __TM1638_H
#define __TM1638_H


#include "debug.h" 
//#include "ch32v003_conf.h"

// Defini??o dos pinos do TM1638 na Porta C
#define TM1638_STB_PORT  GPIOC
#define TM1638_STB_PIN   GPIO_Pin_1

#define TM1638_CLK_PORT  GPIOC
#define TM1638_CLK_PIN   GPIO_Pin_2

#define TM1638_DIO_PORT  GPIOC
#define TM1638_DIO_PIN   GPIO_Pin_4

// Defini??o do pino do Buzzer/Rel¨¦ na Porta A
#define BUZZER_PORT      GPIOA
#define BUZZER_PIN       GPIO_Pin_2

// Buffer global do display (8 d¨ªgitos + 1 byte de LEDs)
extern uint8_t tm1638_display_buffer[9];

// Tabela de convers?o de 7 segmentos
extern const uint8_t TM1638_Font[];

// Prot¨®tipos das Fun??es
void TM1638_Init(void);
void TM1638_WriteByte(uint8_t data);
void TM1638_SendCommand(uint8_t cmd);
void TM1638_SetBrightness(uint8_t brightness, uint8_t on);
void TM1638_UpdateDisplay(void);
uint8_t TM1638_ReadButtons(void);
void TM1638_BitTest(void);

#endif /* __TM1638_H */