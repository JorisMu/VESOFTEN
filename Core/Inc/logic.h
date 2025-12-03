#ifndef LOGIC_H
#define LOGIC_H

#include <stdio.h>

typedef enum {
    OK,
    ERROR_INVALID_PARAM,
    ERROR_OUT_OF_BOUNDS,
    ERROR_INVALID_COLOR
} Resultaat;

// Schermdimensies
#define SCHERM_BREEDTE 800
#define SCHERM_HOOGTE 600

// Functies
Resultaat lijn(int x, int y, int x2, int y2, char kleur[20], int dikte);
Resultaat rechthoek(int x_lup, int y_lup, int breedte, int hoogte, char kleur[20], int gevuld);
Resultaat tekst(int x, int y, char kleur[20], const char *tekst, const char *fontnaam, int fontgrootte, const char *fontstijl);
Resultaat bitmap(int nr, int x_lup, int y_lup);
Resultaat clearscherm(char kleur[20]);

#endif
