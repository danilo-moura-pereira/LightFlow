#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h" // Biblioteca para permitir o BOOTSEL pelo botão B

#include "hardware/uart.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"
#include "hardware/timer.h"

#include "lib/ssd1306.h"
#include "lib/matriz.h"
#include "ws2812.pio.h"

// Push-buttons "A" e "B"
#define GPIO_BOTAO_A    5
#define GPIO_BOTAO_B    6

// LED RGB
#define GPIO_LED_R      13
#define GPIO_LED_G      11
#define GPIO_LED_B      12

// Joystick (potenciômetro)
#define GPIO_BTN_JOY    22
#define GPIO_VRX_JOY    26
#define GPIO_VRY_JOY    27

// Display OLED
#define I2C_PORT        i2c1
#define GPIO_I2C_SDA    14     
#define GPIO_I2C_SLC    15
#define ADDRESS         0x3C

// Matriz de LEDs 5x5
#define GPIO_WS2812     7
#define NUM_LEDS        25

// Buzzer "A"
#define GPIO_BUZZER     21

// PWM (Buzzer e LED RGB)
#define PWM_FREQUENCY   50          // Frequência definida em 50Hz para que o servo motor funcione
#define CLOCK_BASE      125000000   // 125 MHz
#define PWM_DIVISER     125.0       // Divisor inteiro de 125.0. Poderia ser de até 

// Constantes para os Tempos (em milissegundos) das luzes do semáforo
#define LIGHT_G_TIME     10000    // Tempo base para sinal verde
#define LIGHT_Y_TIME     4000    // Tempo base para sinal amarelo
#define LIGHT_PEDESTRIAN 10000   // Tempo para fase de pedestre

/*
 * Cria variáveis globais
 * static: variável permanece na memória durante toda a execução do programa 
 * volatile: a variável pode ser alterada por eventos externos
 */ 
static volatile uint32_t last_time = 0; // Armazena o tempo do último evento (em microssegundos)
static volatile PIO pio;
static volatile uint sm = 0;

// Variável global para flag de pedestre (setada via interrupção)
static volatile bool bPedestre = false;

// Variável que controla se a próxima a via a ser liberada é a VIA A depois da liberação para pedestres
static volatile bool bGreenA = false;

// Máquina de estados do semáforo
typedef enum {
    STATE_GREEN_A,      // Estado luz VERDE, via A
    STATE_YELLOW_A,     // Estado luz AMARELA, via A
    STATE_GREEN_B,      // Estado luz VERDE, via B
    STATE_YELLOW_B,     // Estado luz VERDE, via B
    STATE_PEDESTRIAN    // Estado travessia de PEDESTRE
} state_t;

// Armazena o estado atual (começa em VERDE)
state_t estadoAtual = STATE_GREEN_A;

// Armazena o tempo de início de um novo estado
uint32_t tempoInicioEstado = 0;

// Inicializa vetor para apagar todos os leds
static double desenho_apaga[NUM_LEDS] = {
    {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00},
    {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00},
    {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00},
    {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00},
    {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}
};

ssd1306_t ssd; // Variável de inicialização do display

// Variável string para linhas do Display (máximo 12 caracteres por linha)
char linhaTextoDisplay[12];

static volatile int  adc_valor_x, adc_valor_y; // Variável para armazenar o valor do eixo do joystick
float fluxoViaA, fluxoViaB; // Variável para armazenar o valor do eixo Y do joystick em percentual

// Variável  para calcular o PWM WRAP (número de ciclos do clock do PWM)
// O PWM WRAP é calculado pela fórmula da FREQUÊNCIA PWM = FREQUÊNCIA DE CLOCK DO RP2040 / (DIVISOR  * WRAP)
uint16_t wrapValue = CLOCK_BASE / (PWM_FREQUENCY * PWM_DIVISER); // 20.000

// Prototipação da função de interrupção
static void gpio_irq_handler(uint gpio, uint32_t events);

// Função que configura PIO da Matriz de LEDs 5x5
uint pio_config(PIO pio) {
    bool activateClock;
    activateClock = set_sys_clock_khz(128000, false);

    // Inicializa PIO
    uint offset = pio_add_program(pio, &ws2812_program);
    uint sm = pio_claim_unused_sm(pio, true);
    ws2812_program_init(pio, sm, offset, GPIO_WS2812);

    return sm;
}

// Callback da interrupção
void gpio_irq_handler(uint gpio, uint32_t events) {
    // Obtém o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    // Verifica se passou tempo suficiente desde o último evento (200 ms de debouncing)
    if (current_time - last_time > 200000) {
        last_time = current_time; // Atualiza o tempo do último evento

        // Alterna os LEDs RGB entre aceso/apagado
        if (gpio == GPIO_BOTAO_A) {
            bPedestre = true;
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

// Inicializa o PWM para o LED RGB e BUZZER "A"
void init_pwm() {
    pwm_config config = pwm_get_default_config();

    // Habilita o pino GPIO como PWM 
    gpio_set_function(GPIO_BUZZER, GPIO_FUNC_PWM);
    
    // Captura o canal (slice) PWM 
    uint16_t sliceBuzzer  = pwm_gpio_to_slice_num(GPIO_BUZZER);

    // Inicializa o canal PWM para o buzzer
    pwm_init(sliceBuzzer, &config, true);
}

// Inicializa a Matriz de LEDs 5x5
void init_WS2812() {
    pio = pio0; 
    sm = pio_config(pio);

    // Apaga LEDs
    desenho_pio(desenho_apaga, 0, pio, sm, 0, 0, 0);
}

// Função para acender o LED por um tempo específico
void controlaLed(uint gpio, bool operacao) {
    gpio_put(gpio, operacao); // Liga/Desliga o LED indicado no parâmetro gpio
}

// Essa função lê o canal ADC do joystick e retorna o valor
uint16_t read_joystick_axis(uint axis) {
    adc_select_input(axis);
    return adc_read();
}

// Essa função aciona o buzzer (nível de duty cycle: 0 a 65535)
void buzzer_beep(uint16_t dutyValue) {
    pwm_set_gpio_level(GPIO_BUZZER, dutyValue);
}

// Função para atualizar o display OLED
void escreve_oled(const char *texto, uint linha) {
    // Envia o texto para o display
    uint lin = 1;
    if (linha == 2) {
        lin = 14;
    }
    else if (linha == 3) {
        lin = 27;
    }
    else if (linha == 4) {
        lin = 40;
    }
    else if (linha == 5) {
        lin = 53;
    }
    ssd1306_draw_string(&ssd, "               ", 4, lin); 
    ssd1306_draw_string(&ssd, texto, 4, lin); 
    // Atualiza o display
    ssd1306_send_data(&ssd); 

    printf("OLED: %s\n", texto);
}

// Essa função mapeia valores de grandezas diferentes como acontece com o PWM e o ADC ou ADC e PIXEL
int  mapValue(int  x, int  in_min, int  in_max, int  out_min, int  out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Rotina principal
int main() {
    // Inicializa comunicação USB CDC para o monitor serial
    stdio_init_all();

    // Chama funções de inicialização (LEDs, BOTÕES, interface I2C, conversor ADC, etc.)
    init_leds();
    init_botoes();
    init_I2C();
    init_adc();
    init_pwm();
    init_WS2812();

    // Configuração da interrupções - PUSH-BUTTONS A e B 
    gpio_set_irq_enabled_with_callback(GPIO_BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(GPIO_BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    // Estado inicial
    escreve_oled("VIA A  VERDE   ", 2);
    escreve_oled("VIA B  VERMELHO", 3);

    // Laço principal do programa 
    while (true) {
        // Lê valor do ADC para o eixo X do JOYSTICK (Via "A")
        adc_valor_x = read_joystick_axis(1);

        // Lê valor do ADC para o eixo Y do JOYSTICK (Via "B")
        adc_valor_y = read_joystick_axis(0);
        printf("JOYSTICK - ADC X = %d / ADC Y = %d\n", adc_valor_x, adc_valor_y);

        // Calcula o fluxo de cada VIA em PERCENTUAL
        fluxoViaA = ((float)adc_valor_x / 4095.0f) * 100.0f;
        fluxoViaB = ((float)adc_valor_y / 4095.0f) * 100.0f;
        printf("FLUXO - VIA A: %.1f%% / FLUXO - VIA B: %.1f%%\n", fluxoViaA, fluxoViaB);
        
        // Verifica se o push-button "A" foi pressionado e exibe aviso ao pedestre no display OLED
        if (bPedestre) {
            escreve_oled("PEDESTRE ESPERE", 4);
            sleep_ms(100);
            escreve_oled("               ", 4);
        }
        else {
            escreve_oled("               ", 4);
        }

        uint32_t now = to_ms_since_boot(get_absolute_time()); // Atualiza o timer
        uint32_t elapsedTime = now - tempoInicioEstado; // Calcula o tempo transcorrido do último estado

        // Ajusta os tempos de verde com base no fluxo (cada ponto percentual adiciona 30 ms)
        uint32_t tempoLuzVerdeA = LIGHT_G_TIME + (uint32_t)(fluxoViaA * 80);
        uint32_t tempoLuzVerdeB = LIGHT_G_TIME + (uint32_t)(fluxoViaB * 80);

        // Máquina de estados
        uint indiceMatriz = 0; // Variável para controlar o que será exibido na Matriz de LEDs de acordo com o estado
        switch (estadoAtual) {
            case STATE_GREEN_A:
                if (elapsedTime >= tempoLuzVerdeA) {
                    estadoAtual = STATE_YELLOW_A;
                    tempoInicioEstado = now;

                    // Atualiza Display OLED
                    escreve_oled("VIA A  AMARELO ", 2);
                    escreve_oled("VIA B  VERMELHO", 3);
                }
                indiceMatriz = 0;
                break;
            case STATE_YELLOW_A:
                if (elapsedTime >= LIGHT_Y_TIME) {
                    // Se houver solicitação de pedestre, muda para estado de travessia de pedestre
                    if (bPedestre) {
                        estadoAtual = STATE_PEDESTRIAN;
                        tempoInicioEstado = now;

                        bPedestre = false;
                        bGreenA = false;

                        // Atualiza Display OLED
                        escreve_oled("VIA A  VERMELHO", 2);
                        escreve_oled("VIA B  VERMELHO", 3);
                    }
                    else {
                        estadoAtual = STATE_GREEN_B;
                        tempoInicioEstado = now;

                        // Atualiza Display OLED
                        escreve_oled("VIA B  VERDE   ", 2);
                        escreve_oled("VIA A  VERMELHO", 3);
                    }
                }
                indiceMatriz = 1;
                break;
            case STATE_GREEN_B:
                if (elapsedTime >= tempoLuzVerdeB) {
                    estadoAtual = STATE_YELLOW_B;
                    tempoInicioEstado = now;

                    // Atualiza Display OLED
                    escreve_oled("VIA B  AMARELO ", 2);
                    escreve_oled("VIA A  VERMELHO", 3);
                }
                indiceMatriz = 3;
                break;
            case STATE_YELLOW_B:
                if (elapsedTime >= LIGHT_Y_TIME) {
                    // Se houver solicitação de pedestre, muda para estado de travessia de pedestre
                    if (bPedestre) {
                        estadoAtual = STATE_PEDESTRIAN;
                        tempoInicioEstado = to_ms_since_boot(get_absolute_time());

                        bPedestre = false;
                        bGreenA = true;
                    }
                    else {
                        estadoAtual = STATE_GREEN_A;
                        tempoInicioEstado = now;

                        // Atualiza Display OLED
                        escreve_oled("VIA A  VERDE   ", 2);
                        escreve_oled("VIA B  VERMELHO", 3);
                    }
                }
                indiceMatriz = 4;
                break;
            case STATE_PEDESTRIAN:
                if (elapsedTime >= LIGHT_PEDESTRIAN) {
                    // Libera a via 
                    estadoAtual = (bGreenA) ? STATE_GREEN_A : STATE_GREEN_B;
                    tempoInicioEstado = now;

                    // Atualiza Display OLED
                    if (estadoAtual == STATE_GREEN_A) {
                        escreve_oled("VIA A  VERDE   ", 2);
                        escreve_oled("VIA B  VERMELHO", 3);
                    }
                    else {
                        escreve_oled("VIA B  VERDE   ", 2);
                        escreve_oled("VIA A  VERMELHO", 3);
                    }
                }
                else {
                    // Atualiza Display OLED
                    escreve_oled("               ", 4);
                    escreve_oled("PODE ATRAVESSAR", 4);
                    // Emite sinal sonoro
                    buzzer_beep(2000); // duty cycle
                    sleep_ms(500);
                    buzzer_beep(0);
                    sleep_ms(400);
                }
                indiceMatriz = 6;
                break;
        }
        // Atualizar o desenho WS2812B com base no estado do semáforo
        if (indiceMatriz == 0) {
            gera_desenho(indiceMatriz, pio, sm);
            sleep_ms(500);
            gera_desenho(indiceMatriz + 5, pio, sm);
            sleep_ms(400);
        } 
        else if (indiceMatriz == 3) {
            gera_desenho(indiceMatriz, pio, sm);
            sleep_ms(500);
            gera_desenho(indiceMatriz - 1, pio, sm);
            sleep_ms(400);
        }
        else {
            gera_desenho(indiceMatriz, pio, sm);
        }

        
        /* Atualizar o LED RGB conforme o nível de congestionamento:
         * Se fluxo > 80% em qualquer via => congestionamento (vermelho)
         * Se fluxo > 40% => fluxo normal (amarelo)
         * Caso contrário, pista livre (verde)
        */
        if (fluxoViaA <= 40.0f || fluxoViaB <= 40.0f) {
            controlaLed(GPIO_LED_R, false);
            controlaLed(GPIO_LED_G, true);
            controlaLed(GPIO_LED_B, false);

            // Atualiza Display OLED
            escreve_oled((fluxoViaA <= 40.0f) ? "VIA A   LIVRE" : "VIA B   LIVRE", 5);    
        }
        else if ((fluxoViaA > 40.0f && fluxoViaA <= 80.0f) && (fluxoViaB > 40.0f && fluxoViaB <= 80.0f)) {
            controlaLed(GPIO_LED_R, true);
            controlaLed(GPIO_LED_G, true);
            controlaLed(GPIO_LED_B, false);

            // Atualiza Display OLED
            escreve_oled((fluxoViaA > 40.0f) ? "VIA A   NORMAL" : "VIA B   NORMAL", 5);    
        } 
        else if (fluxoViaA > 80.0f || fluxoViaB > 80.0f) {
            controlaLed(GPIO_LED_R, true);
            controlaLed(GPIO_LED_G, false);
            controlaLed(GPIO_LED_B, false);

            // Atualiza Display OLED
            escreve_oled((fluxoViaA > 80.0f) ? "VIA A   ALERTA" : "VIA B   ALERTA", 5);    
        } 
        
        // Atualizar o OLED com informações sobre o estado das luzes
        char texto_oled[16];
        uint32_t restando = 0;
        if (estadoAtual == STATE_GREEN_A) {
            restando = (tempoLuzVerdeA > elapsedTime) ? tempoLuzVerdeA - elapsedTime : 0;
        } 
        else if (estadoAtual == STATE_GREEN_B) {
            restando = (tempoLuzVerdeB > elapsedTime) ? tempoLuzVerdeB - elapsedTime : 0;
        } 
        else if (estadoAtual == STATE_YELLOW_A || estadoAtual == STATE_YELLOW_B) {
            restando = (LIGHT_Y_TIME > elapsedTime) ? LIGHT_Y_TIME - elapsedTime : 0;
        } 
        else if (estadoAtual == STATE_PEDESTRIAN) {
            restando = (LIGHT_PEDESTRIAN > elapsedTime) ? LIGHT_PEDESTRIAN - elapsedTime : 0;
        }
        snprintf(texto_oled, sizeof(texto_oled), "TEMPO: %d seg", (restando/1000) + 1);

        // Atualiza Display OLED
        escreve_oled(texto_oled, 1);
        
        sleep_ms(100);
    }
    return 0; // Retorno padrão da função main para programa finalizado corretamente
}