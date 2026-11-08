#include "debug.h"
#include "tm1638.h"

// Vari¨¢veis globais de configura??o
uint8_t time_digits[4] = {0, 0, 0, 0}; // MM:SS
uint8_t pass_digits[4] = {0, 0, 0, 0}; // Senha de 4 d¨ªgitos



// --- ESTRUTURA E CONFIGURA??ES DA FLASH ---
typedef struct {
    uint16_t saved_time;    // Tempo em segundos
    uint8_t pass[4];        // Senha de 4 d¨ªgitos
    uint16_t magic_check;   // N¨²mero m¨¢gico para validar dados gravados
} BombConfig_t;

#define CONFIG_FLASH_ADDRESS  0x08003FC0  // ?ltima p¨¢gina da Flash (16KB)
#define MAGIC_KEY             0xA55A

// Fun??o para carregar a configura??o da Flash
void Load_Bomb_Config(uint16_t *total_seconds, uint8_t *pass_digs) {
    BombConfig_t *config = (BombConfig_t *)CONFIG_FLASH_ADDRESS;
    
    uint16_t total_mins = config->saved_time / 60;
    uint16_t rem_secs = config->saved_time % 60;
    
    time_digits[0] = total_mins / 10;
    time_digits[1] = total_mins % 10;
    time_digits[2] = rem_secs / 10;
    time_digits[3] = rem_secs % 10;

    pass_digs[0] = config->pass[0];
    pass_digs[1] = config->pass[1];
    pass_digs[2] = config->pass[2];
    pass_digs[3] = config->pass[3];
}

// Fun??o para salvar a configura??o na Flash
void Save_Bomb_Config(void) {
    BombConfig_t new_config;
    new_config.saved_time = (time_digits[0] * 10 + time_digits[1]) * 60 + 
                            (time_digits[2] * 10 + time_digits[3]);
    new_config.pass[0] = pass_digits[0];
    new_config.pass[1] = pass_digits[1];
    new_config.pass[2] = pass_digits[2];
    new_config.pass[3] = pass_digits[3];
    new_config.magic_check = MAGIC_KEY;

    FLASH_Unlock();
    FLASH_ErasePage(CONFIG_FLASH_ADDRESS);
    
    uint32_t *src = (uint32_t *)&new_config;
    uint32_t dest = CONFIG_FLASH_ADDRESS;
    
    for (uint8_t i = 0; i < sizeof(BombConfig_t); i += 4) {
        FLASH_ProgramWord(dest, *src);
        dest += 4;
        src++;
    }
    FLASH_Lock();
}




// Inicializa o pino PD6 como Entrada com Pull-Up (Condi??o de Programa??o)
void Programming_Pin_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    
    // Habilita o clock das Portas A, C, D e AFIO
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD | RCC_APB2Periph_AFIO, ENABLE);
    
    // Configura PA2 como ENTRADA com Pull-Up interno (Bot?o de Programa??o)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Garante que o pino PD6 inicia desligado (LOW)
    GPIO_WriteBit(GPIOD, GPIO_Pin_6, Bit_RESET);

    // Configura PD6 como SA?DA Push-Pull 50MHz (LED / Buzzer de Alarme)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStructure);
}

// --- ETAPA 1: Ajuste do Tempo ---
void Run_Time_Adjustment(void)
{
    // Exibe "TIME" nos 4 primeiros d¨ªgitos (0 a 3)
    // T = 0x78, I = 0x30, M = 0x54, E = 0x79
    tm1638_display_buffer[0] = 0x78; // T
    tm1638_display_buffer[1] = 0x30; // I
    tm1638_display_buffer[2] = 0x54; // M
    tm1638_display_buffer[3] = 0x79; // E

    while(1) {
        // Atualiza os d¨ªgitos 5 a 8 com o tempo atual (¨ªndices 4 a 7 do buffer)
        tm1638_display_buffer[4] = TM1638_Font[time_digits[0]];
        tm1638_display_buffer[5] = TM1638_Font[time_digits[1]] |0x80;
        tm1638_display_buffer[6] = TM1638_Font[time_digits[2]];
        tm1638_display_buffer[7] = TM1638_Font[time_digits[3]];
        
        // Acende o LED 0 indicando o modo de ajuste de tempo
        tm1638_display_buffer[8] = 0x01; 

        TM1638_UpdateDisplay();

        uint8_t btn = TM1638_ReadButtons();
        if (btn != 0) {
            // Bot?o 1 (Bit 0): ENTER / Avan?ar para a senha
            if (btn & (1 << 0)) {
                tm1638_display_buffer[8] = 0xFF;
                TM1638_UpdateDisplay();
                Delay_Ms(300);
                break;
            }

            // Bot?o 2 (Bit 1): Reseta o tempo para 00:00
            if (btn & (1 << 1)) {
                time_digits[0] = 0;
                time_digits[1] = 0;
                time_digits[2] = 0;
                time_digits[3] = 0;
                Delay_Ms(200);
            }

            // Bot?es 5 a 8 (Bits 4 a 7): Incrementam os d¨ªgitos do tempo individualmente
            if (btn & (1 << 4)) { time_digits[0] = (time_digits[0] + 1) % 10; Delay_Ms(200); }
            if (btn & (1 << 5)) { time_digits[1] = (time_digits[1] + 1) % 10; Delay_Ms(200); }
            if (btn & (1 << 6)) { time_digits[2] = (time_digits[2] + 1) % 6; Delay_Ms(200); }
            if (btn & (1 << 7)) { time_digits[3] = (time_digits[3] + 1) % 10; Delay_Ms(200); }
        }
        Delay_Ms(50);
    }
}

// --- ETAPA 2: Ajuste da Senha ---
int Run_Password_Adjustment(void)
{
    while(1) {
        // Exibe "PASS" nos 4 primeiros d¨ªgitos (0 a 3)
        // P = 0x73, A = 0x77, S = 0x6D
        tm1638_display_buffer[0] = 0x73; // P
        tm1638_display_buffer[1] = 0x77; // A
        tm1638_display_buffer[2] = 0x6D; // S
        tm1638_display_buffer[3] = 0x6D; // S

        // Exibe a senha atual nos 4 ¨²ltimos d¨ªgitos (4 a 7)
        tm1638_display_buffer[4] = TM1638_Font[pass_digits[0]];
        tm1638_display_buffer[5] = TM1638_Font[pass_digits[1]];
        tm1638_display_buffer[6] = TM1638_Font[pass_digits[2]];
        tm1638_display_buffer[7] = TM1638_Font[pass_digits[3]];
        
        // Acende o LED 1 indicando o modo de ajuste de senha
        tm1638_display_buffer[8] = 0x02; 

        TM1638_UpdateDisplay();

        uint8_t btn = TM1638_ReadButtons();
        if (btn != 0) {
            // Bot?o 1 (Bit 0): ENTER
            if (btn & (1 << 0)) {
                uint16_t total_time = (time_digits[0] * 1000) + (time_digits[1] * 100) + 
                                      (time_digits[2] * 10) + time_digits[3];
                
                uint16_t total_pass = (pass_digits[0] * 1000) + (pass_digits[1] * 100) + 
                                      (pass_digits[2] * 10) + pass_digits[3];

                // Regra: Se ambos forem zero (00:00 e 0000), volta para o ajuste de tempo
                if (total_time == 0 && total_pass == 0) {
                    tm1638_display_buffer[8] = 0xFF;
                    TM1638_UpdateDisplay();
                    Delay_Ms(400);
                    return 0; // Volta para o ajuste de tempo
                } else {
                    tm1638_display_buffer[8] = 0xFF;
                    TM1638_UpdateDisplay();
                    Delay_Ms(400);
                    return 1; // Configura??o v¨¢lida!
                }
            }

            // Bot?o 2 (Bit 1): Reseta a senha para 0000
            if (btn & (1 << 1)) {
                pass_digits[0] = 0;
                pass_digits[1] = 0;
                pass_digits[2] = 0;
                pass_digits[3] = 0;
                Delay_Ms(200);
            }

            // Bot?es 5 a 8 (Bits 4 a 7): Incrementam os d¨ªgitos da senha individualmente
            if (btn & (1 << 4)) { pass_digits[0] = (pass_digits[0] + 1) % 10; Delay_Ms(200); }
            if (btn & (1 << 5)) { pass_digits[1] = (pass_digits[1] + 1) % 10; Delay_Ms(200); }
            if (btn & (1 << 6)) { pass_digits[2] = (pass_digits[2] + 1) % 10; Delay_Ms(200); }
            if (btn & (1 << 7)) { pass_digits[3] = (pass_digits[3] + 1) % 10; Delay_Ms(200); }
        }
        Delay_Ms(50);
    }
}


int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    
    TM1638_Init();
    Programming_Pin_Init();
    Delay_Ms(50);
    
// --- L?GICA DE BOOT (FLASH OU BOT?O) ---
    BombConfig_t *config = (BombConfig_t *)CONFIG_FLASH_ADDRESS;
    uint8_t button_pressed = (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == Bit_RESET);
    uint8_t flash_is_empty = (config->magic_check != MAGIC_KEY);

    if (flash_is_empty || button_pressed) {
        while(1) {
            Run_Time_Adjustment();          // Tela 1: Ajusta o Tempo (TIME)
            int result = Run_Password_Adjustment(); // Tela 2: Ajusta a Senha (PASS)
            
            if (result == 1) {
                Save_Bomb_Config();         // Salva na Flash ao t¨¦rmino
                break; // Configura??o aceita! Sai para o jogo
            }
        }
    } else {
        // Carrega os dados salvos da Flash
        Load_Bomb_Config(NULL, pass_digits);
    }

/*
    // Verifica se o pino PD6 est¨¢ aterrado para entrar no Modo de Programa??o
//    uint8_t enter_programming = (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_6) == Bit_RESET);
    uint8_t enter_programming = (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == Bit_RESET);

    if (enter_programming) {
        while(1) {
            Run_Time_Adjustment();          // Tela 1: Ajusta o Tempo (TIME)
            int result = Run_Password_Adjustment(); // Tela 2: Ajusta a Senha (PASS)
            
            if (result == 1) {
                break; // Configura??o aceita! Sai para o jogo
            }
        }
    } else {
        // Caso n?o entre em programa??o por jumper, define um tempo padr?o de teste (ex: 05:00)
        time_digits[0] = 0; time_digits[1] = 5; time_digits[2] = 0; time_digits[3] = 0;
    }
*/
    // --- ESTADO: ARMED (Aguardando Start com Bot?o 4) ---
// --- ESTADO: ARMED (Aguardando Start com Bot?o 4) ---
    uint32_t blink_timer = 0;
    uint8_t led_state = 0;

    while(1) {
        // Exibe "rEAdY" nos d¨ªgitos 1 a 5 (ou rEAdY no visor)
        // r = 0x50, E = 0x79, A = 0x77, d = 0x5E, y = 0x6E (ou Y)
        tm1638_display_buffer[0] = 0x50; // r
        tm1638_display_buffer[1] = 0x79; // E
        tm1638_display_buffer[2] = 0x77; // A
        tm1638_display_buffer[3] = 0x5E; // d
        tm1638_display_buffer[4] = 0x6E; // Y
        tm1638_display_buffer[5] = 0x00; // Vazio
        tm1638_display_buffer[6] = 0x00; // Vazio
        tm1638_display_buffer[7] = 0x00; // Vazio

        // Faz o LED 7 piscar a cada ~400ms
        blink_timer++;
        if (blink_timer >= 8) { // 8 * 50ms = 400ms
            led_state ^= 0x80;  // Inverte o estado do LED 7 (bit 7)
            blink_timer = 0;
        }
        tm1638_display_buffer[8] = led_state;

        TM1638_UpdateDisplay();

        uint8_t btn = TM1638_ReadButtons();
        // Bot?o 4 corresponde ao Bit 3 (0-indexed)
        if (btn & (1 << 3)) {
            // Pisca todos os LEDs rapidamente para confirmar o Start
            tm1638_display_buffer[8] = 0xFF;
            TM1638_UpdateDisplay();
            Delay_Ms(500);
            break; // Sai do estado ARMED e vai para o jogo
        }
        
        Delay_Ms(50);
    }

    // --- ESTADO: JOGO PRONTO / AGUARDANDO CONTAGEM ---
    // Exibe o tempo nos d¨ªgitos 1 a 4 (¨ªndices 0 a 3) e "----" nos d¨ªgitos 5 a 8 (¨ªndices 4 a 7)
    // Segmento de tra?o '-' = 0x40

        tm1638_display_buffer[0] = TM1638_Font[time_digits[0]];
        tm1638_display_buffer[1] = TM1638_Font[time_digits[1]];
        tm1638_display_buffer[2] = TM1638_Font[time_digits[2]];
        tm1638_display_buffer[3] = TM1638_Font[time_digits[3]];

        tm1638_display_buffer[4] = 0x40; // -
        tm1638_display_buffer[5] = 0x40; // -
        tm1638_display_buffer[6] = 0x40; // -
        tm1638_display_buffer[7] = 0x40; // -
        
        tm1638_display_buffer[8] = 0x00; // LEDs limpos

        TM1638_UpdateDisplay();

        // Aqui em breve iniciaremos a contagem regressiva segundo a segundo
        Delay_Ms(100);


// --- ESTADO: JOGO EM ANDAMENTO (CONTAGEM + DESARME) ---
    
    // 1. Converte o array de d¨ªgitos inicial para o total de segundos
    uint32_t total_seconds = (time_digits[0] * 10 + time_digits[1]) * 60 + 
                             (time_digits[2] * 10 + time_digits[3]);

    uint8_t dp_state = 0x80;
    
    // Valores reais guardados para a senha (iniciam em 0)
    uint8_t guess_digits[4] = {0, 0, 0, 0}; 
    // Flag de exibi??o: 1 = mostra o n¨²mero, 0 = mostra o tra?o "-" por timeout
    uint8_t display_active[4] = {0, 0, 0, 0}; 
    // Temporizadores de inatividade do display (agora com timeout de 800ms)
    uint32_t digit_timers[4] = {0, 0, 0, 0}; 

    while(1) {
        // Se o tempo acabou (Estouro / Explos?o)
        if (total_seconds == 0) {

            // Liga o alarme em PA2 (Buzzer / LED)
            GPIO_WriteBit(GPIOD, GPIO_Pin_6, Bit_SET);

            while(1) {
                tm1638_display_buffer[0] = TM1638_Font[0];
                tm1638_display_buffer[1] = TM1638_Font[0] | 0x80;
                tm1638_display_buffer[2] = TM1638_Font[0];
                tm1638_display_buffer[3] = TM1638_Font[0];
                
                tm1638_display_buffer[4] = 0x40;
                tm1638_display_buffer[5] = 0x40;
                tm1638_display_buffer[6] = 0x40;
                tm1638_display_buffer[7] = 0x40;
                
                tm1638_display_buffer[8] = 0xFF; // Alarme
                TM1638_UpdateDisplay();
                Delay_Ms(250);
                tm1638_display_buffer[8] = 0x00;
                TM1638_UpdateDisplay();
                Delay_Ms(250);
            }
        }

        // Converte segundos para MM:SS
        uint16_t current_mins = total_seconds / 60;
        uint16_t current_secs = total_seconds % 60;

        time_digits[0] = current_mins / 10;
        time_digits[1] = current_mins % 10;
        time_digits[2] = current_secs / 10;
        time_digits[3] = current_secs % 10;

        // Atualiza rel¨®gio (MM:SS) nos 4 primeiros d¨ªgitos
        tm1638_display_buffer[0] = TM1638_Font[time_digits[0]];
        tm1638_display_buffer[2] = TM1638_Font[time_digits[2]];
        tm1638_display_buffer[3] = TM1638_Font[time_digits[3]];
        tm1638_display_buffer[8] = 0x00; 

        // Loop de 1 segundo (dividido em 8 passos de 125ms = 4 Hz no ponto decimal)
        for (uint8_t i = 0; i < 8; i++) {
            // Atualiza o ponto decimal piscando a 4Hz no d¨ªgito 2
            tm1638_display_buffer[1] = TM1638_Font[time_digits[1]] | dp_state;
            dp_state ^= 0x80;

            // --- TRATAMENTO DOS D?GITOS DA SENHA (5 a 8) ---
            for (uint8_t d = 0; d < 4; d++) {
                if (digit_timers[d] > 0) {
                    if (digit_timers[d] > 125) digit_timers[d] -= 125;
                    else {
                        digit_timers[d] = 0;
                        display_active[d] = 0; // O display expira e mostra o tra?o "-", mas o valor ¨¦ preservado
                    }
                }

                // Renderiza o n¨²mero (se ativo) ou o tra?o "-" (0x40) se o display expirou
                if (display_active[d] == 0) {
                    tm1638_display_buffer[4 + d] = 0x40; // Tra?o
                } else {
                    tm1638_display_buffer[4 + d] = TM1638_Font[guess_digits[d]];
                }
            }

            TM1638_UpdateDisplay();

            // --- LEITURA DOS BOT?ES DURANTE A CONTAGEM ---
            uint8_t btn = TM1638_ReadButtons();
            if (btn != 0) {
                // Bot?o 1 (Bit 0): ENTER / Confirmar Senha
                if (btn & (1 << 0)) {
                    // Para confirmar, todos os 4 d¨ªgitos precisam estar com o display ativo
                        
                    if (guess_digits[0] == pass_digits[0] && guess_digits[1] == pass_digits[1] &&
                        guess_digits[2] == pass_digits[2] && guess_digits[3] == pass_digits[3]) {
                        
                        // --- BOMBA DESARMADA COM SUCESSO! ---
                        while(1) {
                            tm1638_display_buffer[0] = 0x3D; // g
                            tm1638_display_buffer[1] = 0x3F; // o
                            tm1638_display_buffer[2] = 0x3F; // o
                            tm1638_display_buffer[3] = 0x5E; // d
                            tm1638_display_buffer[4] = 0x00;
                            tm1638_display_buffer[5] = 0x00;
                            tm1638_display_buffer[6] = 0x00;
                            tm1638_display_buffer[7] = 0x00;
                            tm1638_display_buffer[8] = 0xFF; 
                            
                            // Padr?o 1: LEDs 1, 3, 5, 7 acesos (0x55)
                            tm1638_display_buffer[8] = 0x55; 
                            TM1638_UpdateDisplay();
                            Delay_Ms(200);

                            // Padr?o 2: LEDs 2, 4, 6, 8 acesos (0xAA)
                            tm1638_display_buffer[8] = 0xAA; 
                            TM1638_UpdateDisplay();
                            Delay_Ms(200);
                        }
                    } else {
                        // --- SENHA ERRADA: PENALIDADE (Corta o tempo pela metade) ---
                        total_seconds /= 2;
                        
                        tm1638_display_buffer[8] = 0xAA;
                        TM1638_UpdateDisplay();
                        Delay_Ms(300);

                        // Reseta a exibi??o para tra?os
                        for(uint8_t d=0; d<4; d++) {
                            display_active[d] = 0;
                            digit_timers[d] = 0;
                        }
                    }
                }
                Delay_Ms(200);
            

                // Bot?o 2 (Bit 1): Zera a senha digitada (retorna tudo para 0000 e tra?os)
                if (btn & (1 << 1)) {
                    for(uint8_t d=0; d<4; d++) {
                        guess_digits[d] = 0;
                        display_active[d] = 0;
                        digit_timers[d] = 0;
                    }
                    Delay_Ms(200);
                }

                // Bot?es 5 a 8 (Bits 4 a 7): Incrementam os d¨ªgitos da senha (timeout de 800ms)
                if (btn & (1 << 4)) { // Bot?o 5 -> D¨ªgito 5
                    guess_digits[0] = (guess_digits[0] + 1) % 10;
                    display_active[0] = 1;  
                    digit_timers[0] = 800;  // Timeout ajustado para 800ms
                    Delay_Ms(150);
                }
                if (btn & (1 << 5)) { // Bot?o 6 -> D¨ªgito 6
                    guess_digits[1] = (guess_digits[1] + 1) % 10;
                    display_active[1] = 1;
                    digit_timers[1] = 800;
                    Delay_Ms(150);
                }
                if (btn & (1 << 6)) { // Bot?o 7 -> D¨ªgito 7
                    guess_digits[2] = (guess_digits[2] + 1) % 10;
                    display_active[2] = 1;
                    digit_timers[2] = 800;
                    Delay_Ms(150);
                }
                if (btn & (1 << 7)) { // Bot?o 8 -> D¨ªgito 8
                    guess_digits[3] = (guess_digits[3] + 1) % 10;
                    display_active[3] = 1;
                    digit_timers[3] = 800;
                    Delay_Ms(150);
                }
            } else {
                Delay_Ms(125); 
            }
        }

        if (total_seconds > 0) total_seconds--;
    }




} //main()