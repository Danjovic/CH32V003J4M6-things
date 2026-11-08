#include "tm1638.h"

// Buffer global de 9 posi??es
uint8_t tm1638_display_buffer[9] = {0};

// Tabela de fonte 7 segmentos (0-9, A, b, C, d, E, F, Espa?o)
const uint8_t TM1638_Font[] = {
    0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F, // 0 a 9
    0x77, 0x7C, 0x39, 0x5E, 0x79, 0x71, 0x00                     // A, b, C, d, E, F, Vazio
};

static void TM1638_SetDIO_Mode(GPIOMode_TypeDef mode)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = TM1638_DIO_PIN;
    GPIO_InitStructure.GPIO_Mode = mode;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(TM1638_DIO_PORT, &GPIO_InitStructure);
}

void TM1638_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    // Habilita o clock das Portas A e C
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

    // Configura STB, CLK e DIO da Porta C como Sa赤da Push-Pull (50MHz)
    GPIO_InitStructure.GPIO_Pin = TM1638_STB_PIN | TM1638_CLK_PIN | TM1638_DIO_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Configura o pino do Buzzer/Rel谷 na Porta A como Sa赤da
    GPIO_InitStructure.GPIO_Pin = BUZZER_PIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // N赤veis iniciais em repouso
    GPIO_SetBits(TM1638_STB_PORT, TM1638_STB_PIN);
    GPIO_SetBits(TM1638_CLK_PORT, TM1638_CLK_PIN);
    GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN);
}

void TM1638_WriteByte(uint8_t data)
{
    uint8_t i;
    for (i = 0; i < 8; i++) {
        GPIO_ResetBits(TM1638_CLK_PORT, TM1638_CLK_PIN);
        if (data & 0x01) {
            GPIO_SetBits(TM1638_DIO_PORT, TM1638_DIO_PIN);
        } else {
            GPIO_ResetBits(TM1638_DIO_PORT, TM1638_DIO_PIN);
        }
        data >>= 1;
        GPIO_SetBits(TM1638_CLK_PORT, TM1638_CLK_PIN);
    }
}

void TM1638_SendCommand(uint8_t cmd)
{
    GPIO_ResetBits(TM1638_STB_PORT, TM1638_STB_PIN);
    TM1638_WriteByte(cmd);
    GPIO_SetBits(TM1638_STB_PORT, TM1638_STB_PIN);
}

void TM1638_SetBrightness(uint8_t brightness, uint8_t on)
{
    if (brightness > 7) {
        brightness = 7;
    }
    
    uint8_t cmd = 0x80;
    if (on) {
        cmd |= 0x08 | brightness;
    } else {
        cmd &= ~0x08;
    }
    
    TM1638_SendCommand(cmd);
}

void TM1638_UpdateDisplay(void)
{
    uint8_t i;
    TM1638_SendCommand(0x44); // Endere?o fixo

    // Escreve os 8 d赤gitos nos endere?os pares (0xC0, 0xC2, ...)
    for (i = 0; i < 8; i++) {
        GPIO_ResetBits(TM1638_STB_PORT, TM1638_STB_PIN);
        TM1638_WriteByte(0xC0 | (i << 1));
        TM1638_WriteByte(tm1638_display_buffer[i]);
        GPIO_SetBits(TM1638_STB_PORT, TM1638_STB_PIN);
    }

    // Escreve os LEDs individuais nos endere?os 赤mpares (0xC1, 0xC3, ...)
    for (i = 0; i < 8; i++) {
        uint8_t led_state = (tm1638_display_buffer[8] & (1 << i)) ? 1 : 0;
        GPIO_ResetBits(TM1638_STB_PORT, TM1638_STB_PIN);
        TM1638_WriteByte(0xC1 | (i << 1));
        TM1638_WriteByte(led_state);
        GPIO_SetBits(TM1638_STB_PORT, TM1638_STB_PIN);
    }

    TM1638_SetBrightness(4, 1);
}

uint8_t TM1638_ReadButtons(void)
{
    uint8_t raw[8] = {0};
    uint8_t keys = 0;
    uint8_t i, j;

    GPIO_ResetBits(TM1638_STB_PORT, TM1638_STB_PIN);
    TM1638_WriteByte(0x42); // Comando de leitura do teclado

    TM1638_SetDIO_Mode(GPIO_Mode_IN_FLOATING);

    for (i = 0; i < 4; i++) {
        uint8_t byte_val = 0;
        for (j = 0; j < 8; j++) {
            GPIO_ResetBits(TM1638_CLK_PORT, TM1638_CLK_PIN);
            byte_val >>= 1;
            if (GPIO_ReadInputDataBit(TM1638_DIO_PORT, TM1638_DIO_PIN)) {
                byte_val |= 0x80;
            }
            GPIO_SetBits(TM1638_CLK_PORT, TM1638_CLK_PIN);
        }

        // Remapeamento dos bot?es (f赤sicos: 1, 3, 5, 7, 2, 4, 6, 8)
        if (i == 0) {
            if (byte_val & 0x01) raw[0] = 1; // Bot?o 1
            if (byte_val & 0x10) raw[4] = 1; // Bot?o 5 -> Bot?o 2 l車gico
        }
        if (i == 1) {
            if (byte_val & 0x01) raw[1] = 1; // Bot?o 2 -> Bot?o 3 l車gico
            if (byte_val & 0x10) raw[5] = 1; // Bot?o 6 -> Bot?o 4 l車gico
        }
        if (i == 2) {
            if (byte_val & 0x01) raw[2] = 1; // Bot?o 3 -> Bot?o 5 l車gico
            if (byte_val & 0x10) raw[6] = 1; // Bot?o 7 -> Bot?o 6 l車gico
        }
        if (i == 3) {
            if (byte_val & 0x01) raw[3] = 1; // Bot?o 4 -> Bot?o 7 l車gico
            if (byte_val & 0x10) raw[7] = 1; // Bot?o 8 -> Bot?o 8 l車gico
        }
    }

    GPIO_SetBits(TM1638_STB_PORT, TM1638_STB_PIN);
    TM1638_SetDIO_Mode(GPIO_Mode_Out_PP);

    // Mapeia para bits sequenciais (Bit 0 = Bot?o 1 ... Bit 7 = Bot?o 8)
    for (uint8_t k = 0; k < 8; k++) {
        if (raw[k]) {
            keys |= (1 << k);
        }
    }

    return keys;
}

void TM1638_BitTest(void)
{
    uint8_t i, bit;
    for (i = 0; i < 9; i++) {
        tm1638_display_buffer[i] = 0x00;
    }
    TM1638_UpdateDisplay();
    Delay_Ms(500);

    for (i = 0; i < 9; i++) {
        for (bit = 0; bit < 8; bit++) {
            tm1638_display_buffer[i] = (1 << bit);
            TM1638_UpdateDisplay();
            Delay_Ms(150);
            tm1638_display_buffer[i] = 0x00;
        }
    }
    TM1638_UpdateDisplay();
}

