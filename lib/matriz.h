#ifndef MATRIZ_H
#define MATRIZ_H

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"

// Função que define a intensidade de cada cor de cada led
uint32_t matrix_rgb2(double b, double r, double g);

// Função para acionar a matrix de leds
void animacao_pio(double *animacao, uint32_t iRgb_led, PIO pio, uint sm, double r, double g, double b);

// Main loop to monitor keypad and run animations
void gera_animacao(char tecla, PIO pio, uint sm);

#endif // MATRIZ_H
