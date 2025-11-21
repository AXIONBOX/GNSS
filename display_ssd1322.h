#pragma once

#include <stdint.h>

// Initialisation complète du bus SPI OLED + SSD1322.
void display_init();

// Réinitialisation « hard » de l'écran (SPI + SSD1322).
void display_full_reinit();

// Envoi du contenu d'un buffer 8 bits (256x64) vers le SSD1322.
void display_push(uint8_t* buf, int w, int h);

