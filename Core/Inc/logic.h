#ifndef LOGIC_H
#define LOGIC_H

#include <stdio.h>
#include <string.h>

#include "stm32_ub_vga_screen.h"
#include "stm32f4xx_hal.h"

// Schermdimensies
#define SCHERM_BREEDTE 320
#define SCHERM_HOOGTE 240

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

typedef enum {
    CMD_LIJN,
    CMD_RECHTHOEK,
    CMD_TEKST,
    CMD_BITMAP,
    CMD_CLEARSCHERM,
    CMD_WACHT,
	CMD_WAIT,
    CMD_HERHAAL,
    CMD_CLEAR,
    CMD_CIRKEL,
    CMD_FIGUUR,
    CMD_UNKNOWN
} CommandType;

typedef struct {
    CommandType type;
    int p1, p2, p3, p4, p5, p6, p7, p8, p9, p10; // Generieke parameters (x, y, breedte, dikte, etc.)
    char kleur[20];
    char tekst_inhoud[100];
    char fontnaam[20];
    char fontstijl[20];
} Commando;

// Functies
// Functies met const char* om string-overruns te voorkomen
Resultaat lijn(int x, int y, int x2, int y2, const char *kleur, int dikte);
Resultaat rechthoek(int x_lup, int y_lup, int breedte, int hoogte, const char *kleur, int gevuld);
Resultaat tekst(int x, int y, const char *kleur, const char tekst[100], const char fontnaam[20], int fontgrootte, const char fontstijl[20]);
Resultaat bitmap(int nr, int x_lup, int y_lup);
Resultaat clearscherm(const char *kleur);
Resultaat wacht(int msecs);
Resultaat herhaal(int aantal, int hoevaak);
Resultaat cirkel(int x, int y, int radius, const char *kleur);
Resultaat figuur(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int x5, int y5, const char *kleur);

Resultaat vgaStatusToResultaat(int status);

#endif
