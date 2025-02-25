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
uint32_t desenho_rgb(double b, double r, double g);

// Função para acionar a desenho de leds
void desenho_pio(double *desenho, uint32_t iRgb_led, PIO pio, uint sm, double r, double g, double b);

// Main loop to monitor keypad and run animations
void gera_desenho(uint desenho, PIO pio, uint sm);

#endif // MATRIZ_H
