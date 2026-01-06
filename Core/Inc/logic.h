#ifndef LOGIC_H
#define LOGIC_H

#include <stdio.h>
#include <string.h>

typedef enum {
    OK = 100,
    ERROR_INVALID_COLOR,
    ERROR_INVALID_PARAM_THICKNESS,
	ERROR_INVALID_PARAM_SIZE,
	ERROR_INVALID_PARAM_FILLED,
	ERROR_INVALID_PARAM_FONTNAME,
	ERROR_INVALID_PARAM_FONTSIZE,
	ERROR_INVALID_PARAM_FONSTYLE,
	ERROR_INVALID_PARAM,
    ERROR_OUT_OF_BOUNDS,
	ERROR_TEXT_TOO_LONG,
	ERROR_TOO_MANY_REPEATS,

	VGA_OK = 200,
	ERROR_VGA,
	ERROR_VGA_INVALID_COORDINATE,
	ERROR_VGA_INVALID_PARAMETER
} Resultaat;


// Schermdimensies
#define SCHERM_BREEDTE 320
#define SCHERM_HOOGTE 240

// Functies
Resultaat lijn(int x, int y, int x2, int y2, char kleur[20], int dikte);
Resultaat rechthoek(int x_lup, int y_lup, int breedte, int hoogte, char kleur[20], int gevuld);
Resultaat tekst(int x, int y, char kleur[20], const char tekst[100], const char fontnaam[20], int fontgrootte, const char fontstijl[20]);
Resultaat bitmap(int nr, int x_lup, int y_lup);
Resultaat clearscherm(char kleur[20]);

//extra functies
Resultaat wacht(int msecs);
Resultaat herhaal(int aantal, int hoevaak);
Resultaat cirkel(int x, int y, int radius, char kleur[20]);
Resultaat figuur(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int x5, int y5, char kleur[20]);

Resultaat vgaStatusToResultaat(int status);

#endif
