/**
 *      _____       _           _              _            
 *     |_   _|__ __| |_ __ _ __| |___ _ _   __| |___        
 *       | |/ -_|_-<  _/ _` / _` / _ \ '_| / _` / -_)       
 *       |_|\___/__/\__\__,_\__,_\___/_|   \__,_\___|       
 *         _ _         _                 ___  _    ___ ___  
 *      __| (_)____ __| |__ _ _  _ ___  / _ \| |  | __|   \ 
 *     / _` | (_-< '_ \ / _` | || (_-< | (_) | |__| _|| |) |
 *     \__,_|_/__/ .__/_\__,_|\_, /__/  \___/|____|___|___/ 
 *               |_|          |__/                          
 *
 *
 *   danjovic, November 2026
 *
 */

#include "debug.h"

/*
Pinout  CH32V003J4M
         __ __
TX Out 1|  v  |8            LED
   GND 2|     |7---[2k2]---->|----GND 
  N.C. 3|     |6--SCS   to OLED display
   +5V 4|_____|5--SDA 

*/


//                     _            _      
//      __ ___ _ _  __| |_ __ _ _ _| |_ ___
//     / _/ _ \ ' \(_-<  _/ _` | ' \  _(_-<
//     \__\___/_||_/__/\__\__,_|_||_\__/__/
//                                         

// Tabela de fonte 5x7 compacta (caracteres ASCII de ' ' a 'z')
static const unsigned char font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // 32 ' '
    {0x00, 0x00, 0x5F, 0x00, 0x00}, // 33 '!'
    {0x00, 0x07, 0x00, 0x07, 0x00}, // 34 '"'
    {0x14, 0x7F, 0x14, 0x7F, 0x14}, // 35 '#'
    {0x24, 0x2A, 0x7F, 0x2A, 0x12}, // 36 '$'
    {0x23, 0x13, 0x08, 0x64, 0x62}, // 37 '%'
    {0x36, 0x49, 0x55, 0x22, 0x50}, // 38 '&'
    {0x00, 0x05, 0x03, 0x00, 0x00}, // 39 '''
    {0x00, 0x1C, 0x22, 0x41, 0x00}, // 40 '('
    {0x00, 0x41, 0x22, 0x1C, 0x00}, // 41 ')'
    {0x14, 0x08, 0x3E, 0x08, 0x14}, // 42 '*'
    {0x08, 0x08, 0x3E, 0x08, 0x08}, // 43 '+'
    {0x00, 0x50, 0x30, 0x00, 0x00}, // 44 ','
    {0x08, 0x08, 0x08, 0x08, 0x08}, // 45 '-'
    {0x00, 0x60, 0x60, 0x00, 0x00}, // 46 '.'
    {0x20, 0x10, 0x08, 0x04, 0x02}, // 47 '/'
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 48 '0'
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 49 '1'
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 50 '2'
    {0x21, 0x41, 0x45, 0x4B, 0x31}, // 51 '3'
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 52 '4'
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 53 '5'
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 54 '6'
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 55 '7'
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 56 '8'
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 57 '9'
    {0x00, 0x36, 0x36, 0x00, 0x00}, // 58 ':'
    {0x00, 0x56, 0x36, 0x00, 0x00}, // 59 ';'
    {0x08, 0x14, 0x22, 0x41, 0x00}, // 60 '<'
    {0x14, 0x14, 0x14, 0x14, 0x14}, // 61 '='
    {0x00, 0x41, 0x22, 0x14, 0x08}, // 62 '>'
    {0x02, 0x01, 0x51, 0x09, 0x06}, // 63 '?'
    {0x32, 0x49, 0x79, 0x41, 0x3E}, // 64 '@'
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // 65 'A'
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // 66 'B'
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // 67 'C'
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // 68 'D'
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // 69 'E'
    {0x7F, 0x09, 0x09, 0x01, 0x01}, // 70 'F'
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // 71 'G'
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // 72 'H'
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // 73 'I'
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // 74 'J'
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // 75 'K'
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // 76 'L'
    {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // 77 'M'
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // 78 'N'
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // 79 'O'
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // 80 'P'
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // 81 'Q'
    {0x7F, 0x09, 0x09, 0x19, 0x66}, // 82 'R'
    {0x26, 0x49, 0x49, 0x49, 0x32}, // 83 'S'
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // 84 'T'
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // 85 'U'
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // 86 'V'
    {0x3F, 0x40, 0x38, 0x40, 0x3F}, // 87 'W'
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 88 'X'
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 89 'Y'
    {0x61, 0x51, 0x49, 0x45, 0x43}  // 90 'Z'
};


//       ___  _    ___ ___     __              _   _             
//      / _ \| |  | __|   \   / _|_  _ _ _  __| |_(_)___ _ _  ___
//     | (_) | |__| _|| |) | |  _| || | ' \/ _|  _| / _ \ ' \(_-<
//      \___/|____|___|___/  |_|  \_,_|_||_\__|\__|_\___/_||_/__/
// 

// Escreve byte 
void OLED_WriteByte(u8 addr, u8 data, u8 command)
{
    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET);
    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    
    I2C_Send7bitAddress(I2C1, addr << 1, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    
    // Co-0 = 0, D/C# = 0 (comando) ou 1 (dados)
    u8 control_byte = command ? 0x00 : 0x40;
    
    I2C_SendData(I2C1, control_byte);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    
    I2C_SendData(I2C1, data);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    
    I2C_GenerateSTOP(I2C1, ENABLE);
}

// Envia comando
void OLED_SendCommand(u8 addr, u8 cmd)
{
    OLED_WriteByte(addr, cmd, 1);
}

// Inicializa??o parametrizada por driver (0=SSD1306, 1=SH1106) e altura (32 ou 64)
void OLED_Init(u8 addr, u8 is_sh1106, u8 height)
{
    OLED_SendCommand(addr, 0xAE); // Display OFF
    OLED_SendCommand(addr, 0xD5); // Set Display Clock Divide Ratio
    OLED_SendCommand(addr, 0x80);
    OLED_SendCommand(addr, 0xA8); // Set Multiplex Ratio
    OLED_SendCommand(addr, (height == 64) ? 0x3F : 0x1F); // 63 para 64 linhas, 31 para 32 linhas
    OLED_SendCommand(addr, 0xD3); // Set Display Offset
    OLED_SendCommand(addr, 0x00);
    OLED_SendCommand(addr, 0x40); // Set Display Start Line
    OLED_SendCommand(addr, 0x8D); // Charge Pump
    OLED_SendCommand(addr, 0x14);
    OLED_SendCommand(addr, 0x20); // Memory Addressing Mode
    
    if (is_sh1106) {
        OLED_SendCommand(addr, 0x02); // Page Addressing Mode para SH1106
    } else {
        OLED_SendCommand(addr, 0x00); // Horizontal Addressing Mode para SSD1306
    }

    OLED_SendCommand(addr, 0xA1); // Set Segment Re-map
    OLED_SendCommand(addr, 0xC8); // Set COM Output Scan Direction
    OLED_SendCommand(addr, 0xDA); // Set COM Pins Hardware Configuration
    OLED_SendCommand(addr, (height == 64) ? 0x12 : 0x02); // 0x12 para 64 altura, 0x02 para 32 altura
    OLED_SendCommand(addr, 0x81); // Set Contrast
    OLED_SendCommand(addr, 0xCF);
    OLED_SendCommand(addr, 0xD9); // Set Pre-charge
    OLED_SendCommand(addr, 0xF1);
    OLED_SendCommand(addr, 0xDB); // Set VCOMH
    OLED_SendCommand(addr, 0x40);
    OLED_SendCommand(addr, 0xA4); // Entire Display On
    OLED_SendCommand(addr, 0xA6); // Normal Display
    OLED_SendCommand(addr, 0xAF); // Display ON
}




// Enche tela com um padr?o
void OLED_Fill(u8 addr, u8 height, u8 pattern)
{
    u8 total_pages = height / 8;
    for(u8 page = 0; page < total_pages; page++)
    {
        OLED_SendCommand(addr, 0xB0 + page);
        OLED_SendCommand(addr, 0x00);
        OLED_SendCommand(addr, 0x10);

        while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET);
        I2C_GenerateSTART(I2C1, ENABLE);
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
        I2C_Send7bitAddress(I2C1, addr << 1, I2C_Direction_Transmitter);
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
        I2C_SendData(I2C1, 0x40);
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

        for(u16 col = 0; col < 128; col++)
        {
            I2C_SendData(I2C1, pattern);
            while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
        }
        I2C_GenerateSTOP(I2C1, ENABLE);
    }
}

// Limpa todas as p?ginas da tela
void OLED_Clear(u8 addr, u8 height) {
    OLED_Fill(addr, height, 0);
}


// Escreve uma string na p?gina especificada com o offset correto do driver
void OLED_WriteString(u8 addr, u8 page, u8 is_sh1106, char *str)
{
    OLED_SendCommand(addr, 0xB0 + page);
    
    u8 col_offset = is_sh1106 ? 2 : 0;
    OLED_SendCommand(addr, 0x00 + (col_offset & 0x0F));
    OLED_SendCommand(addr, 0x10 + (col_offset >> 4));

    while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET);
    I2C_GenerateSTART(I2C1, ENABLE);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2C1, addr << 1, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    I2C_SendData(I2C1, 0x40);
    while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    while(*str)
    {
        char c = *str++;
        u8 char_index = (c >= ' ' && c <= 'Z') ? (c - ' ') : 0; // Mapeia para a tabela fonte
        
        for(u8 i = 0; i < 5; i++)
        {
            I2C_SendData(I2C1, font5x7[char_index][i]);
            while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
        }
        // Espa?o de 1 pixel entre os caracteres
        I2C_SendData(I2C1, 0x00);
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    }
    I2C_GenerateSTOP(I2C1, ENABLE);
}


// Desenha o padr?o visual adaptado para a altura e o driver (com offset se SH1106)
void OLED_DrawPattern(u8 addr, u8 is_sh1106, u8 height)
{
    u8 total_pages = height / 8; // 4 p?ginas para 32 pixels, 8 p?ginas para 64 pixels

    for(u8 page = 0; page < total_pages; page++)
    {
        OLED_SendCommand(addr, 0xB0 + page); // Define p?gina
        
        u8 col_offset = is_sh1106 ? 2 : 0;
        OLED_SendCommand(addr, 0x00 + (col_offset & 0x0F));        // Coluna baixa
        OLED_SendCommand(addr, 0x10 + (col_offset >> 4));         // Coluna alta

        while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) != RESET);
        I2C_GenerateSTART(I2C1, ENABLE);
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
        
        I2C_Send7bitAddress(I2C1, addr << 1, I2C_Direction_Transmitter);
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
        
        I2C_SendData(I2C1, 0x40); // Modo de dados
        while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

        for(u16 col = 0; col < 128; col++)
        {
            u8 pattern = (col % 2 == 0) ? 0xAA : 0x55;
            
            I2C_SendData(I2C1, pattern);
            while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
        }


        I2C_GenerateSTOP(I2C1, ENABLE);
    }
}



//      ___ ___ ___    __              _   _             
//     |_ _|_  ) __|  / _|_  _ _ _  __| |_(_)___ _ _  ___
//      | | / / (__  |  _| || | ' \/ _|  _| / _ \ ' \(_-<
//     |___/___\___| |_|  \_,_|_||_\__|\__|_\___/_||_/__/
//                                                       
void IIC_Init(u32 bound, u16 address)
{
    GPIO_InitTypeDef GPIO_InitStructure={0};
    I2C_InitTypeDef I2C_InitTSturcture={0};

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOC | RCC_APB2Periph_AFIO, ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_I2C1, ENABLE );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init( GPIOC, &GPIO_InitStructure );

    I2C_InitTSturcture.I2C_ClockSpeed = bound;
    I2C_InitTSturcture.I2C_Mode = I2C_Mode_I2C;
    I2C_InitTSturcture.I2C_DutyCycle = I2C_DutyCycle_16_9;
    I2C_InitTSturcture.I2C_OwnAddress1 = address;
    I2C_InitTSturcture.I2C_Ack = I2C_Ack_Enable;
    I2C_InitTSturcture.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init( I2C1, &I2C_InitTSturcture );

    I2C_Cmd( I2C1, ENABLE );
}

u8 I2C_CheckDevice(u8 address_7bit)
{
    u32 timeout = 4000;

    // Aguarda o barramento liberar
    while( I2C_GetFlagStatus( I2C1, I2C_FLAG_BUSY ) != RESET ) {
        if (--timeout == 0) return 0;
    }

    // Gera o START
    I2C_GenerateSTART( I2C1, ENABLE );
    while( !I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_MODE_SELECT ) );

    // Envia o endere?o com bit de escrita (0)
    I2C_Send7bitAddress( I2C1, address_7bit << 1, I2C_Direction_Transmitter );

    // Aguarda o evento de endere?o enviado (ADDR) ou falha (AF)
    timeout = 8000;
    while(1) {
        // Se recebeu ACK (Endere?o correspondido / ADDR flag setada)
        if( I2C_CheckEvent( I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED ) ) {
            I2C_GenerateSTOP( I2C1, ENABLE );
            return 1; // Dispositivo encontrado!
        }
        
        // Se deu Acknowledge Failure (NACK - dispositivo n?o existe)
        if( I2C1->STAR1 & (1 << 10) ) {
            I2C1->STAR1 &= ~(1 << 10); // Limpa flag AF
            I2C_GenerateSTOP( I2C1, ENABLE );
            return 0;
        }

        if(--timeout == 0) {
            I2C_GenerateSTOP( I2C1, ENABLE );
            return 0;
        }
    }
}

//                  _      _       _           
//     __ __ ____ _| |_ __| |_  __| |___  __ _ 
//     \ V  V / _` |  _/ _| ' \/ _` / _ \/ _` |
//      \_/\_/\__,_|\__\__|_||_\__,_\___/\__, |
//                                       |___/ 
void IWDG_Init(void)
{
    // Habilita o acesso de escrita aos registradores do IWDG
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    
    // Configura o Prescaler 
    IWDG_SetPrescaler(IWDG_Prescaler_256);
    
    // Configura o valor de reload 
    IWDG_SetReload(500); 
    
    // Recarrega e ativa o watchdog
    IWDG_ReloadCounter();
    IWDG_Enable();
}

void Delay_Ms_Safe(u16 ms)
{
    while(ms >= 200)
    {
        Delay_Ms(200);
         IWDG_ReloadCounter(); // Alimenta o watchdog a cada 200ms
        ms -= 200;            // Decrementa o tempo restante
    }
    if(ms > 0)
    {
        Delay_Ms(ms);
        IWDG_ReloadCounter();
    }
}


//      _    ___ ___  
//     | |  | __|   \ 
//     | |__| _|| |) |
//     |____|___|___/ 
//                    
// Configura??o do LED no pino PC4
void LED_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // Sa¨ªda Push-Pull
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    
    // Inicia com o LED apagado
    GPIO_ResetBits(GPIOC, GPIO_Pin_4);
}



//                 _      
//      _ __  __ _(_)_ _  
//     | '  \/ _` | | ' \ 
//     |_|_|_\__,_|_|_||_|
//                                                    
int main(void)
{
    SystemCoreClockUpdate();
    Delay_Init();

    USART_Printf_Init(115200);

    printf("SystemClk:%d\r\n", SystemCoreClock);
    printf("CH32V003 Universal I2C Scanner Iniciado\r\n");

    IIC_Init(100000, 0x00);
    LED_Init(); // Inicializa o LED no pino PC4
    IWDG_Init();

    while(1)
    {
        // Reinicializa o subsistema I2C a cada in¨ªcio de varredura
        IIC_Init(100000, 0x00);
        Delay_Ms(10); // Pequeno establiza para o barramento
        
        printf("\n--- Iniciando varredura I2C (0x01 a 0x7F) ---\r\nEncontrados: ");
        u8 devices_found = 0;
        u8 found_address = 0;

        // Varre todos os endere?os v¨¢lidos de 7 bits
        for(u8 addr = 1; addr < 0x7F; addr++)
        {
            if(I2C_CheckDevice(addr))
            {
                devices_found++;
                found_address = addr;
                printf("0x%02x,", addr);
            }
        }
        I2C_GenerateSTOP( I2C1, ENABLE );
        I2C_GenerateSTOP( I2C1, ENABLE );
        I2C_GenerateSTOP( I2C1, ENABLE );

        

        if(devices_found == 0)
        {
            printf("Nenhum!\r\n");
            GPIO_ResetBits(GPIOC, GPIO_Pin_4); // Apaga o LED em PC4
        }
        else
        {
            printf("\r\nVarredura concluida. Total encontrado: %d\r\n", devices_found);
            GPIO_SetBits(GPIOC, GPIO_Pin_4);   // Acende o LED em PC4

            if (found_address == 0x3C || found_address == 0x3D)  {
                char buf[32];

                // 1. SSD1306 @ 128x32
                sprintf(buf, "SSD1306 $%02X (128X32)", found_address);
                printf("Modo atual testado: %s\r\n", buf);
                OLED_Init(found_address, 0, 32);
                OLED_Clear(found_address, 32);
                OLED_WriteString(found_address, 1, 0, buf); // Desenha na p?gina 1                
                Delay_Ms_Safe(3000);
                OLED_Fill(found_address, 32,0xff);  // Acende tudo (0xFF)
                Delay_Ms_Safe(1500);
                OLED_Fill(found_address, 32,0x55);  // Acende tudo (0xFF)
                Delay_Ms_Safe(1500);
                OLED_Fill(found_address, 32,0xaa);  // Acende tudo (0xFF)
                Delay_Ms_Safe(1500);


                // 2. SSD1306 @ 128x64
                sprintf(buf, "SSD1306 $%02X (128X64)", found_address);
                printf("Modo atual testado: %s\r\n", buf);
                OLED_Init(found_address, 0, 64);
                OLED_Clear(found_address, 64);
                OLED_WriteString(found_address, 3, 0, buf); // Desenha na p?gina 3 (centro vertical)
                Delay_Ms_Safe(3000);
                OLED_Fill(found_address, 64,0xff);  // Acende tudo (0xFF)
                Delay_Ms_Safe(1500);
                OLED_Fill(found_address, 64,0x55);  // Acende tudo (0xFF)
                Delay_Ms_Safe(1500);
                OLED_Fill(found_address, 64,0xaa);  // Acende tudo (0xFF)
                Delay_Ms_Safe(1500);


                // 3. SH1106 @ 128x32
                sprintf(buf, "SH1106 $%02X (128X32)", found_address);
                printf("Modo atual testado: %s\r\n", buf);
                OLED_Init(found_address, 1, 32);
                OLED_Clear(found_address, 32);
                OLED_WriteString(found_address, 1, 1, buf);
                Delay_Ms_Safe(3000);
                OLED_Fill(found_address, 32,0xff);  // Acende tudo (0xFF)
                Delay_Ms_Safe(1500);
                OLED_Fill(found_address, 32,0x55);  // Acende tudo (0xFF)
                Delay_Ms_Safe(1500);
                OLED_Fill(found_address, 32,0xaa);  // Acende tudo (0xFF)
                Delay_Ms_Safe(1500);


                // 4. SH1106 @ 128x64
                sprintf(buf, "SH1106 $%02X (128X64)", found_address);
                printf("Modo atual testado: %s\r\n", buf);
                OLED_Init(found_address, 1, 64);
                OLED_Clear(found_address, 64);
                OLED_WriteString(found_address, 3, 1, buf);
                Delay_Ms_Safe(3000);
                OLED_Fill(found_address, 64,0xff);  // Acende tudo (0xFF)
                Delay_Ms_Safe(1500);
                OLED_Fill(found_address, 64,0x55);  // Acende tudo (0xFF)
                Delay_Ms_Safe(1500);
                OLED_Fill(found_address, 64,0xaa);  // Acende tudo (0xFF)
                Delay_Ms_Safe(1500);

            }
            
        }

        Delay_Ms_Safe(3000); // Aguarda 3 segundos para a pr¨®xima varredura
    }
}