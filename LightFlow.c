#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h" // Biblioteca para permitir o BOOTSEL pelo botão B

#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#include "lib/ssd1306.h"

#define GPIO_BOTAO_A    5
#define GPIO_BOTAO_B    6
#define GPIO_LED_R      13
#define GPIO_LED_G      11
#define GPIO_LED_B      12
#define GPIO_BTN_JOY    22
#define GPIO_VRX_JOY    26
#define GPIO_VRY_JOY    27

#define I2C_PORT        i2c1
#define GPIO_I2C_SDA    14     
#define GPIO_I2C_SLC    15
#define ADDRESS         0x3C

#define PWM_FREQUENCY   50          // Frequência definida em 50Hz para que o servo motor funcione
#define CLOCK_BASE      125000000   // 125 MHz
#define PWM_DIVISER     125.0       // Divisor inteiro de 125.0. Poderia ser de até 

/*
 * Cria variáveis globais
 * static: variável permanece na memória durante toda a execução do programa 
 * volatile: a variável pode ser alterada por eventos externos
 */ 
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)

ssd1306_t ssd; // Variável de inicialização do display

static volatile int  adc_valor_x; // Variável para armazenar o valor do eixo X do joystick
static volatile int  adc_valor_y; // Variável para armazenar o valor do eixo Y do joystick

// Variável  para calcular o PWM WRAP (número de ciclos do clock do PWM)
// O PWM WRAP é calculado pela fórmula da FREQUÊNCIA PWM = FREQUÊNCIA DE CLOCK DO RP2040 / (DIVISOR  * WRAP)
uint16_t wrapValue = CLOCK_BASE / (PWM_FREQUENCY * PWM_DIVISER); // 20.000

// Prototipação da função de interrupção
static void gpio_irq_handler(uint gpio, uint32_t events);

// Função para acender o LED por um tempo específico
void controlaLed(uint gpio, bool operacao) {
    gpio_put(gpio, operacao); // Liga/Desliga o LED indicado no parâmetro gpio
}

// Controlar o DISPLAY LCD
bool bLed_B_R = 0;
bool bDesenhaQuadro = 1;

// Callback da interrupção
void gpio_irq_handler(uint gpio, uint32_t events) {
    // Obtém o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Verifica se passou tempo suficiente desde o último evento (200 ms de debouncing)
    if (current_time - last_time > 200000) {
        last_time = current_time; // Atualiza o tempo do último evento

        // Alterna os LEDs RGB entre aceso/apagado
        if (gpio == GPIO_BOTAO_A) {
            bLed_B_R = !bLed_B_R;
            controlaLed(GPIO_LED_G, !gpio_get(GPIO_LED_G));
            pwm_set_gpio_level(GPIO_LED_B, (bLed_B_R) ? 20000 : 0);
            pwm_set_gpio_level(GPIO_LED_R, (bLed_B_R) ? 20000 : 0);
        } 
        // Alterna o LED VERDE entre aceso/apagado e modifica a borda do display com uma animação
        else if (gpio == GPIO_BTN_JOY) { 
            controlaLed(GPIO_LED_G, !gpio_get(GPIO_LED_G));
            ssd1306_rect(&ssd, 3, 3, 122, 60, true, false); // Desenha um retângulo na TELA LCD
            ssd1306_rect(&ssd, 5, 5, 118, 55, !bDesenhaQuadro, false); // Desenha um retângulo 
            ssd1306_rect(&ssd, 7, 7, 114, 52, !bDesenhaQuadro, false); // Desenha um retângulo  
            ssd1306_send_data(&ssd); 
            bDesenhaQuadro = !bDesenhaQuadro;
        }
        // Reseta a placa 
        else if (gpio == GPIO_BOTAO_B) {
            // Reseta a placa e entra em modo BOOTSEL
            reset_usb_boot(0, 0);
        }

    }
}

// Inicializa LEDs RGB
void init_leds() {
    gpio_init(GPIO_LED_R);
    gpio_set_dir(GPIO_LED_R, GPIO_OUT);

    gpio_init(GPIO_LED_G);
    gpio_set_dir(GPIO_LED_G, GPIO_OUT);

    gpio_init(GPIO_LED_B);
    gpio_set_dir(GPIO_LED_B, GPIO_OUT);
}

// Inicializa botões A e B
void init_botoes() {
    gpio_init(GPIO_BOTAO_A);
    gpio_set_dir(GPIO_BOTAO_A, GPIO_IN); // Configura o pino como entrada
    gpio_pull_up(GPIO_BOTAO_A); // Habilita o pull-up interno

    gpio_init(GPIO_BOTAO_B);
    gpio_set_dir(GPIO_BOTAO_B, GPIO_IN); // Configura o pino como entrada
    gpio_pull_up(GPIO_BOTAO_B); // Habilita o pull-up interno

    // Inicializa botão do JOYSTICK
    gpio_init(GPIO_BTN_JOY);
    gpio_set_dir(GPIO_BTN_JOY, GPIO_IN);
    gpio_pull_up(GPIO_BTN_JOY); 
}

// Inicializa Display SSD1306
void init_I2C() {
    // Inicializa a interface serial I2C usando a frequência de 400 Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    // Define a linha SDA na GPIO 14 (GPIO_I2C_SDA)
    gpio_set_function(GPIO_I2C_SDA, GPIO_FUNC_I2C);

    // Define a linha SLC na GPIO 15 (GPIO_I2C_SLC) 
    gpio_set_function(GPIO_I2C_SLC, GPIO_FUNC_I2C); 

    // Coloca o pino SDA em PULL UP
    gpio_pull_up(GPIO_I2C_SDA); 

    // Coloca o pino SLC em PULL UP
    gpio_pull_up(GPIO_I2C_SLC); 

    // Inicializa o display LCD
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ADDRESS, I2C_PORT); 

    // Passa os parâmetros de configuração do display
    ssd1306_config(&ssd); 

    // Envia os dados para o display
    ssd1306_send_data(&ssd); 

    // Inicia o display com todos os pixels apagados
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

// Inicializa ADC relacionando com o JOYSTICK
void init_adc() {
    adc_init();
    adc_gpio_init(GPIO_VRX_JOY);
    adc_gpio_init(GPIO_VRY_JOY);
}

void init_pwm() {
    // Habilita o pino GPIO como PWM (LEDs RGB AZUL e VERMELHO)
    gpio_set_function(GPIO_LED_B, GPIO_FUNC_PWM);
    gpio_set_function(GPIO_LED_R, GPIO_FUNC_PWM);
    
    // Captura o canal (slice) PWM 
    uint16_t slicePWMLedB = pwm_gpio_to_slice_num(GPIO_LED_B);
    uint16_t slicePWMLedR = pwm_gpio_to_slice_num(GPIO_LED_R);

    // Define o dividor de clock do PWM em 2.0
    pwm_set_clkdiv(slicePWMLedB, PWM_DIVISER);
    pwm_set_clkdiv(slicePWMLedR, PWM_DIVISER);

    // Define o valor WRAP PWM
    pwm_set_wrap(slicePWMLedB, wrapValue);
    pwm_set_wrap(slicePWMLedR, wrapValue);

    // Habilita o PWM no slice correspondente
    pwm_set_enabled(slicePWMLedB, 1);
    pwm_set_enabled(slicePWMLedR, 1);
}

// Essa função mapeia valores de grandezas diferentes como acontece com o PWM e o ADC ou ADC e PIXEL
int  mapValue(int  x, int  in_min, int  in_max, int  out_min, int  out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Variáveis para controlar a posição do quadrado no DISPLAY LCD (Posição inicial é o centro da tela)
int  lcdLeft = (128/2)-4;
int  lcdTop = (64/2)-4;

// Rotina principal
int main() {
    // Inicializa comunicação USB CDC para o monitor serial
    stdio_init_all();

    // Chama funções de inicialização (LEDs, BOTÕES, interface I2C, conversor ADC)
    init_leds();
    init_botoes();
    init_I2C();
    init_adc();
    init_pwm();

    // Configuração da interrupções - BOTÕES A, B e JOYSTICK
    gpio_set_irq_enabled_with_callback(GPIO_BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(GPIO_BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(GPIO_BTN_JOY, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Apaga o DISPLAY LCD e desenha um quadrado 8x8 no centro
    ssd1306_fill(&ssd, false);
    ssd1306_rect(&ssd, lcdTop, lcdLeft, 8, 8, true, true); 
    ssd1306_send_data(&ssd);
    
    // Laço principal do programa 
    while (true) {
        // Seleciona o ADC para o eixo X do JOYSTICK
        adc_select_input(1);
        adc_valor_x = adc_read();
        // Seleciona o ADC para o eixo Y do JOYSTICK
        adc_select_input(0);
        adc_valor_y = adc_read();

        // Muda a intensidade da luz dos LEDs AZUL e VERMELHO de acordo com a posição do JOYSTICK
        // A função mapValue converte o valor ADC para PWM
        int  pwm_valor_x = mapValue(adc_valor_x, 0, 4095, 0, wrapValue);
        int  pwm_valor_y = mapValue(adc_valor_y, 0, 4095, 0, wrapValue);

        // A função mapValue converte o valor ADC para as coordenadas do DISPLAY LCD (128x64 pixels)
        lcdLeft = mapValue(adc_valor_x, 0, 4095, 3, 117);
        lcdTop = mapValue(adc_valor_y, 0, 4095, 56, 3);

        // Apaga a tela do DISPLAY LCD e desenha um quadrado 8x8 que se movimenta proporcionalmente aos valores capturados pelo joystick
        ssd1306_fill(&ssd, 0); // Apaga a tela do DISPLAY LCD
        sleep_ms(40); // Pausa para não sobrecarregar o processamento
        ssd1306_rect(&ssd, lcdTop, lcdLeft, 8, 8, 1, 1); // Desenha um retângulo 8x8 
        ssd1306_send_data(&ssd);
        sleep_ms(40); // Pausa para não sobrecarregar o processamento

        // Mostra no terminal os valores dos eixos X e Y do JOYSTICK
        printf("JOYSTICK - PWM X = %d / PWM Y = %d\n", pwm_valor_x, pwm_valor_y);
        printf("JOYSTICK - ADC X = %d / ADC Y = %d\n", adc_valor_x, adc_valor_y);
        printf("QUADRADO - Top   = %d / Left  = %d\n", lcdTop, lcdLeft);

        /* 
         * Acende e apaga os LEDs AZUL e VERMELHO se o JOYSTICK for movimentado 
         * Obs: os intervalos foram definidos de acordo com a calibração do potenciômetro do JOYSTICK para que o LED não seja acionado quando estiver em posição centralizada
         *      Pode ser diferente de placa para placa
        */ 
        if (adc_valor_y >= 1650 && adc_valor_y <= 2150 && !bLed_B_R) {
            pwm_set_gpio_level(GPIO_LED_B, 0); 
        }
        else {
            pwm_set_gpio_level(GPIO_LED_B, pwm_valor_y); 
        }
        if (adc_valor_x >= 1800 && adc_valor_x <= 2300 && !bLed_B_R) {
            pwm_set_gpio_level(GPIO_LED_R, 0); 
        }
        else {
            pwm_set_gpio_level(GPIO_LED_R, pwm_valor_x);
        }
    }

    return 0; // Retorno padrão da função main para programa finalizado corretamente
}