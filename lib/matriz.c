#include <stdio.h>
#include "hardware/pio.h"


#define NUM_LEDS        25 // Define constante com o número de leds da desenho
#define NUM_DESENHOS     7 // Define constante com o número de desenhos

// Inicializa vetor tridimensional "desenhos": para cada desenho, para cada LED, definindo {R, G, B} (valores entre 0 e 1)
double desenhos[NUM_DESENHOS][NUM_LEDS][3] = {
    // VIA "A" - VERDE
    {
        {0.00, 0.03, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.03, 0.00},
        {0.00, 0.03, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.03, 0.00},
        {0.00, 0.00, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00},
    },
    // VIA "A" - AMARELO
    {
        {0.03, 0.03, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.03, 0.03, 0.00}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.03, 0.03, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.03, 0.03, 0.00},
        {0.03, 0.03, 0.00}, {0.03, 0.03, 0.00}, {0.03, 0.03, 0.00}, {0.03, 0.03, 0.00}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.03, 0.03, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.03, 0.03, 0.00},
        {0.00, 0.00, 0.00}, {0.03, 0.03, 0.00}, {0.03, 0.03, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00},
    },
    // VIA "A" - VERMELHO
    {
        {0.03, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.03, 0.00, 0.00},
        {0.03, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.03, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00},
    },
    // VIA "B" - VERDE
    {
        {0.00, 0.00, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.03, 0.00},
        {0.00, 0.03, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.03, 0.00},
        {0.00, 0.00, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.03, 0.00}, {0.00, 0.00, 0.00},
    },
    // VIA "B" - AMARELO
    {
        {0.00, 0.00, 0.00}, {0.03, 0.03, 0.00}, {0.03, 0.03, 0.00}, {0.03, 0.03, 0.00}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.03, 0.03, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.03, 0.03, 0.00},
        {0.03, 0.03, 0.00}, {0.03, 0.03, 0.00}, {0.03, 0.03, 0.00}, {0.03, 0.03, 0.00}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.03, 0.03, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.03, 0.03, 0.00},
        {0.00, 0.00, 0.00}, {0.03, 0.03, 0.00}, {0.03, 0.03, 0.00}, {0.03, 0.03, 0.00}, {0.00, 0.00, 0.00},
    },
    // VIA "B" - VERMELHO
    {
        {0.00, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.03, 0.00, 0.00},
        {0.03, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.03, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.03, 0.00, 0.00}, {0.00, 0.00, 0.00},
    },
    // PEDESTRES - AZUL
    {
        {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.03}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.00, 0.00, 0.03}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.03}, {0.00, 0.00, 0.03}, {0.00, 0.00, 0.03}, {0.00, 0.00, 0.03}, {0.00, 0.00, 0.00},
        {0.00, 0.00, 0.00}, {0.00, 0.00, 0.03}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.00}, {0.00, 0.00, 0.03},
        {0.00, 0.00, 0.03}, {0.00, 0.00, 0.03}, {0.00, 0.00, 0.03}, {0.00, 0.00, 0.03}, {0.00, 0.00, 0.00},
    },
};

// Função que define a intensidade de cada cor de cada led
uint32_t desenho_rgb(double r, double g, double b) {
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

// Função para acionar a desenho de leds
void desenho_pio(double *desenho, uint32_t iRgb_led, PIO pio, uint sm, double r, double g, double b) {
    for (int16_t i = 0; i < NUM_LEDS; i++) {
        iRgb_led = desenho_rgb(r = 0.00, g = 0.00, desenho[24-i]);
        pio_sm_put_blocking(pio, sm, iRgb_led);
    }
}

/* Atualiza a desenho WS2812B utilizando o vetor tridimensional "desenhos"
 *      'desenho' é o índice do desenho que se deseja exibir (0 a NUM_DESENHOS-1)
 *      Cada LED terá sua cor definida por {R, G, B} em desenhos[desenho][i]
 */
void gera_desenho(uint desenho, PIO pio, uint sm) {
    uint32_t pixel;
    for (int i = 0; i < NUM_LEDS; i++) {
        double r = desenhos[desenho][i][0];
        double g = desenhos[desenho][i][1];
        double b = desenhos[desenho][i][2];
        pixel = desenho_rgb(r, g, b);
        pio_sm_put_blocking(pio, sm, pixel);
    }
}