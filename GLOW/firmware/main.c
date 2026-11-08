/********************************** (C) COPYRIGHT *******************************
- File Name          : main.c
- Description        : CH32V003 RGB Fading + WS2812 on PD6 (Sequential & Pure RGB Modes)
- Danjovic 2026
 *******************************************************************************/

#include "debug.h"

#define PWM_STEPS       100   // Resolu??o do Soft-PWM para os LEDs comuns
#define WS2812_PIN      GPIO_Pin_6
#define WS2812_PORT     GPIOD

// Vari¨¢veis globais de brilho para os LEDs comuns (0 a 100)
volatile u8 duty_red = 0;
volatile u8 duty_green = 0;
volatile u8 duty_blue = 0;

// Cor atual enviada ao WS2812 (R, G, B)
u8 ws_r = 0, ws_g = 0, ws_b = 0;

void Hardware_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};

    // Habilita clocks para GPIOC (LEDs comuns) e GPIOD (Bot?o e WS2812)
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD, ENABLE);

    // Configura PC1 (Blue), PC2 (Red), PC4 (Green) como Sa¨ªda Push-Pull
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // Configura PD1 como Entrada com Pull-Up (Bot?o)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    // Configura PD6 como Sa¨ªda Push-Pull para o WS2812
    GPIO_InitStructure.GPIO_Pin = WS2812_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(WS2812_PORT, &GPIO_InitStructure);
    
    // Garante PD6 em n¨ªvel baixo inicialmente
    GPIO_ResetBits(WS2812_PORT, WS2812_PIN);
}

// Rotina otimizada de envio para o WS2812 (Protocolo 800kHz em PD6 a 48MHz)
// Nota: Desabilita interrup??es momentaneamente para garantir a precis?o dos pulsos de tempo
void WS2812_Send_Byte(u8 byte)
{
    for(int i = 7; i >= 0; i--)
    {
        if(byte & (1 << i))
        {
            // Envia BIT '1': High por ~700ns, Low por ~600ns
            GPIO_SetBits(WS2812_PORT, WS2812_PIN);
            __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t");
            __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t");
            GPIO_ResetBits(WS2812_PORT, WS2812_PIN);
            __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t");
        }
        else
        {
            // Envia BIT '0': High por ~350ns, Low por ~800ns
            GPIO_SetBits(WS2812_PORT, WS2812_PIN);
            __asm__("nop\n\t"); __asm__("nop\n\t");
            GPIO_ResetBits(WS2812_PORT, WS2812_PIN);
            __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t");
            __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t"); __asm__("nop\n\t");
        }
    }
}

void WS2812_Set_Color(u8 r, u8 g, u8 b)
{
    __disable_irq(); // Protege o timing cr¨ªtico contra interrup??es
    
    // Ordem padr?o WS2812: Green, Red, Blue (GRB)
    WS2812_Send_Byte(g);
    WS2812_Send_Byte(r);
    WS2812_Send_Byte(b);

    __enable_irq();
    
    // Pulso de Reset (> 50us em Low)
    Delay_Us(80);
}

// Fun??o de Soft-PWM para atualizar os estados dos LEDs comuns
void Soft_PWM_Cycle(u16 cycles)
{
    for(u16 c = 0; c < cycles; c++)
    {
        for(u8 i = 0; i < PWM_STEPS; i++)
        {
            if(i < duty_red) GPIO_SetBits(GPIOC, GPIO_Pin_2);
            else             GPIO_ResetBits(GPIOC, GPIO_Pin_2);

            if(i < duty_green) GPIO_SetBits(GPIOC, GPIO_Pin_4);
            else               GPIO_ResetBits(GPIOC, GPIO_Pin_4);

            if(i < duty_blue) GPIO_SetBits(GPIOC, GPIO_Pin_1);
            else              GPIO_ResetBits(GPIOC, GPIO_Pin_1);
            
            Delay_Us(10);
        }
    }
}

// Define o brilho convertendo de 0-255 para 0-100
void Set_RGB_Percent(u8 red, u8 green, u8 blue)
{
    duty_red   = (red * PWM_STEPS) / 255;
    duty_green = (green * PWM_STEPS) / 255;
    duty_blue  = (blue * PWM_STEPS) / 255;
}

int main(void)
{
    SystemCoreClockUpdate();
    Delay_Init();
    
    Hardware_Init();

    const u8 colors[7][3] = {
        {255, 0, 0},   // 0: Vermelho
        {0, 255, 0},   // 1: Verde
        {0, 0, 255},   // 2: Azul
        {255, 255, 0}, // 3: Amarelo
        {0, 255, 255}, // 4: Ciano
        {255, 0, 255}, // 5: Magenta
        {255, 255, 255}// 6: Branco
    };

    u8 current_red = 0, current_green = 0, current_blue = 0;
    u8 target_r = 0, target_g = 0, target_b = 0;
    u8 color_index = 0;

    u8 pure_rgb_state = 0; 
    u8 fading_direction = 0; 

    while(1)
    {
        u8 button_pressed = (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_1) == 0);

        if(button_pressed)
        {
            // MODO BOT?O APERTADO: Sequ¨ºncia estrita R -> G -> B
            if(pure_rgb_state != 0) current_red = 0;
            if(pure_rgb_state != 1) current_green = 0;
            if(pure_rgb_state != 2) current_blue = 0;

            u8 *active_channel = (pure_rgb_state == 0) ? &current_red : 
                                 (pure_rgb_state == 1) ? &current_green : &current_blue;

            if(fading_direction == 0)
            {
                if(*active_channel < 255) (*active_channel)++;
                else fading_direction = 1;
            }
            else
            {
                if(*active_channel > 0) (*active_channel)--;
                else
                {
                    fading_direction = 0;
                    pure_rgb_state = (pure_rgb_state + 1) % 3;
                }
            }
        }
        else
        {
            // MODO BOT?O SOLTO: Ciclo normal pelas 7 cores
            if(color_index >= 7) color_index = 0;

            if(current_red == target_r && current_green == target_g && current_blue == target_b)
            {
                color_index = (color_index + 1) % 7;
                target_r = colors[color_index][0];
                target_g = colors[color_index][1];
                target_b = colors[color_index][2];
            }

            if(current_red < target_r) current_red++;
            else if(current_red > target_r) current_red--;

            if(current_green < target_g) current_green++;
            else if(current_green > target_g) current_green--;

            if(current_blue < target_b) current_blue++;
            else if(current_blue > target_b) current_blue--;
        }

        // Atualiza os LEDs comuns via Soft-PWM
        Set_RGB_Percent(current_red, current_green, current_blue);

        // Atualiza o WS2812 no pino PD6 com os mesmos valores de cor atuais
        WS2812_Set_Color(current_red, current_green, current_blue);
        
        // Controla a velocidade do fading
        Soft_PWM_Cycle(6); 
    }
}
