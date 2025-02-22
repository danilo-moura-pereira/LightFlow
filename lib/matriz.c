#include <stdio.h>
#include "hardware/pio.h"

// Define constante com o número de leds da matriz
#define NUM_LEDS 25

// Inicializa vetor bidimencional com animações
double animacoes[10][NUM_LEDS] = {
    // ZERO
    {0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.07, 0.0, 0.0, 0.07, 
     0.07, 0.0, 0.0, 0.07, 0.0,
     0.0, 0.07, 0.0, 0.0, 0.07,
     0.07, 0.07, 0.07, 0.07, 0.0},
    
    // UM
    {0.0, 0.07, 0.07, 0.0, 0.0,
     0.0, 0.0, 0.07, 0.0, 0.07, 
     0.0, 0.0, 0.07, 0.0, 0.0,
     0.0, 0.0, 0.07, 0.0, 0.0,
     0.0, 0.07, 0.07, 0.07, 0.0},

    // DOIS
    {0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.07, 0.0, 0.0, 0.0, 
     0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.07,
     0.07, 0.07, 0.07, 0.07, 0.0},
    
    // TRÊS
    {0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.07, 0.0, 0.0, 0.0, 
     0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.07, 0.0, 0.0, 0.0,
     0.07, 0.07, 0.07, 0.07, 0.0},
    
    // QUATRO
    {0.07, 0.0, 0.0, 0.07, 0.0,
     0.0, 0.07, 0.0, 0.0, 0.07, 
     0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.07, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.07, 0.0},
    
    // CINCO
    {0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.07, 
     0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.07, 0.0, 0.0, 0.0,
     0.07, 0.07, 0.07, 0.07, 0.0},
    
    // SEIS
    {0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.0, 0.0, 0.0, 0.07, 
     0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.07, 0.0, 0.0, 0.07,
     0.07, 0.07, 0.07, 0.07, 0.0},
    
    // SETE
    {0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.07, 0.0, 0.0, 0.0, 
     0.0, 0.0, 0.07, 0.07, 0.07,
     0.0, 0.07, 0.0, 0.0, 0.0,
     0.0, 0.0, 0.0, 0.07, 0.0},
    
    // OITO
    {0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.07, 0.0, 0.0, 0.07, 
     0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.07, 0.0, 0.0, 0.07,
     0.07, 0.07, 0.07, 0.07, 0.0},
    
    // NOVE
    {0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.07, 0.0, 0.0, 0.07, 
     0.07, 0.07, 0.07, 0.07, 0.0,
     0.0, 0.07, 0.0, 0.0, 0.0,
     0.07, 0.07, 0.07, 0.07, 0.0}
};

// Função que define a intensidade de cada cor de cada led
uint32_t matrix_rgb2(double r, double g, double b) {
    unsigned char R, G, B;
    R = r * 255;
    G = g * 255;
    B = b * 255;
    return (G << 24) | (R << 16) | (B << 8);
}

// Função para acionar a matrix de leds
void animacao_pio(double *animacao, uint32_t iRgb_led, PIO pio, uint sm, double r, double g, double b) {
    for (int16_t i = 0; i < NUM_LEDS; i++) {
        iRgb_led = matrix_rgb2(r = 0.0, g = 0.0, animacao[24-i]);
        pio_sm_put_blocking(pio, sm, iRgb_led);
    }
}

// Gera animação conforme número 
void gera_animacao(uint numero, PIO pio, uint sm) {
    uint32_t iRgb_led;
    double r = 0.0, b = 0.0 , g = 0.0;
    animacao_pio(animacoes[numero], iRgb_led, pio, sm, r, g, b);
}