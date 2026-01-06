#include "logic.h"
#include "stm32_ub_vga_screen.h"
#include "stm32f4xx_hal.h"

// Lijst van toegestane kleuren
const char *kleuren[] = { "zwart", "blauw", "lichtblauw", "groen", "lichtgroen", "cyaan", "lichtcyaan", "rood", "lichtrood", "magenta", "lichtmagenta", "bruin", "geel", "grijs", "wit"};
const char *fontnamen[] = {"arial", "consolas"};
const char *stijlen[] = {"normaal", "vet", "cursief"};

static Commando geschiedenis[20];
static int geschiedenis_index = 0;
static int totaal_opgeslagen = 0;

static int aantal_kleur = sizeof(kleuren) / sizeof(kleuren[0]);
static int aantal_fontnaam = sizeof(fontnamen) / sizeof(fontnamen[0]);
static int aantal_stijl = sizeof(stijlen) / sizeof(stijlen[0]);

static void log_commando(Commando c) {
    geschiedenis[geschiedenis_index] = c;
    geschiedenis_index = (geschiedenis_index + 1) % 20;
    if (totaal_opgeslagen < 20) totaal_opgeslagen++;
}

static int contains(const char *items[], const int aantal, const char *item) {
	for (int i = 0; i < aantal; i++) {
		if (strcmp(item, items[i]) == 0) {
			return 1;  // gevonden
		}
	}
	return 0;  // niet gevonden
}

int validColor(const char *kleur) {
	return contains(kleuren, aantal_kleur, kleur);
}

int validFont(const char *fontnaam) {
	return contains(fontnamen, aantal_fontnaam, fontnaam);
}

int validFontstijl(const char *stijl) {
	return contains(stijlen, aantal_stijl, stijl);
}

uint8_t kleurToCode(const char *kleur[]) {
	uint8_t code;

	if (strcmp(kleur, "zwart") == 0) code = VGA_COL_BLACK;
	else if (strcmp(kleur, "blauw") == 0) code = VGA_COL_BLUE;
	else if (strcmp(kleur, "lichtblauw") == 0) code = VGA_COL_LIGHT_BLUE;
	else if (strcmp(kleur, "groen") == 0) code = VGA_COL_GREEN;
 	else if (strcmp(kleur, "lichtgroen") == 0) code = VGA_COL_LIGHT_GREEN;
	else if (strcmp(kleur, "cyaan") == 0) code = VGA_COL_CYAN;
	else if (strcmp(kleur, "lichtcyaan") == 0) code = VGA_COL_LIGHT_CYAN;
	else if (strcmp(kleur, "rood") == 0) code = VGA_COL_RED;
	else if (strcmp(kleur, "lichtrood") == 0) code = VGA_COL_LIGHT_RED;
	else if (strcmp(kleur, "magenta") == 0) code = VGA_COL_MAGENTA;
	else if (strcmp(kleur, "lichtmagenta") == 0) code = VGA_COL_LIGHT_MAGENTA;
	else if (strcmp(kleur, "bruin") == 0) code = VGA_COL_BROWN;
	else if (strcmp(kleur, "geel") == 0) code = VGA_COL_YELLOW;
	else if (strcmp(kleur, "grijs") == 0) code = VGA_COL_GREY;
	else if (strcmp(kleur, "wit") == 0) code = VGA_COL_WHITE;
	return code;
}

/* ===================== COMMANDO’S ===================== */
// Teken een lijn (zoals eerder)
Resultaat lijn(int x, int y, int x2, int y2, const char *kleur, int dikte) {
    if (x < 0 || x >= SCHERM_BREEDTE ||
    	y < 0 || y >= SCHERM_HOOGTE ||
		x2 < 0 || x2 >= SCHERM_BREEDTE ||
		y2 < 0 || y2 >= SCHERM_HOOGTE)
    {
    	return ERROR_OUT_OF_BOUNDS;
    }

    if (dikte <= 0)
        return ERROR_INVALID_PARAM_THICKNESS;

    if (!validColor(kleur))
        return ERROR_INVALID_COLOR;

    // stuur naar VGA scherm
    int status = UB_VGA_DrawLine(x, y, x2, y2, kleurToCode(kleur), dikte);
    if (status != 0) {
    	return vgaStatusToResultaat(status);
    }

    // Opslaan
    Commando c;
        memset(&c, 0, sizeof(Commando));
        c.type = CMD_LIJN; c.p1 = x; c.p2 = y; c.p3 = x2; c.p4 = y2; c.p5 = dikte;
        strncpy(c.kleur, kleur, 19);
        log_commando(c);

    return OK;
}

// Teken een rechthoek
Resultaat rechthoek(int x_lup, int y_lup, int breedte, int hoogte, const char *kleur, int gevuld) {
    if (x_lup < 0 || x_lup >= SCHERM_BREEDTE || y_lup < 0 || y_lup >= SCHERM_HOOGTE || x_lup + breedte > SCHERM_BREEDTE || y_lup + hoogte > SCHERM_HOOGTE)
        return ERROR_OUT_OF_BOUNDS;

    if (breedte <= 0 || hoogte <= 0)
        return ERROR_INVALID_PARAM_SIZE;

    if (gevuld < 0 || gevuld > 1)
        return ERROR_INVALID_PARAM_FILLED;

    if (!validColor(kleur))
        return ERROR_INVALID_COLOR;

    // stuur naar VGA scherm
    int status = UB_VGA_DrawRectangle(x_lup, y_lup, breedte, hoogte, kleurToCode(kleur), gevuld);
    if (status != 0) {
        return vgaStatusToResultaat(status);
    }

    Commando c;
        memset(&c, 0, sizeof(Commando));
        c.type = CMD_RECHTHOEK; c.p1 = x_lup; c.p2 = y_lup; c.p3 = breedte; c.p4 = hoogte; c.p5 = gevuld;
        strncpy(c.kleur, kleur, 19);
        log_commando(c);

    return OK;
}

// Teken tekst
Resultaat tekst(int x, int y, const char *kleur, const char tekst[100], const char fontnaam[20], int fontgrootte, const char fontstijl[20]) {
    if (x < 0 || x >= SCHERM_BREEDTE || y < 0 || y >= SCHERM_HOOGTE)
        return ERROR_OUT_OF_BOUNDS;

    if (strlen(tekst) > 100)
        return ERROR_TEXT_TOO_LONG;

    if (!validColor(kleur))
        return ERROR_INVALID_COLOR;
    if (!validFont(fontnaam))
        return ERROR_INVALID_PARAM_FONTNAME;
    if (!validFontstijl(fontstijl))
        return ERROR_INVALID_PARAM_FONSTYLE;
    if (fontgrootte != 1 && fontgrootte != 2)
        return ERROR_INVALID_PARAM_FONTSIZE;

	//int status = UB_VGA_DrawText(10, 20, "zwart","the quick brown fox jumps over the lazy dog", "consolas", 1, "vet");
    int status = UB_VGA_DrawText(x, y, kleurToCode(kleur), tekst, fontnaam, fontgrootte, fontstijl);
    if (status != 0) {
        return vgaStatusToResultaat(status);
    }

    Commando c;
        memset(&c, 0, sizeof(Commando));
        c.type = CMD_TEKST; c.p1 = x; c.p2 = y; c.p3 = fontgrootte;
        strncpy(c.kleur, kleur, 19);
        strncpy(c.tekst_inhoud, tekst, 99);
        strncpy(c.fontnaam, fontnaam, 19);
        strncpy(c.fontstijl, fontstijl, 19);
        log_commando(c);
    return OK;
}

// Teken bitmap
Resultaat bitmap(int nr, int x_lup, int y_lup) {
    if (x_lup < 0 || x_lup >= SCHERM_BREEDTE || y_lup < 0 || y_lup >= SCHERM_HOOGTE)
        return ERROR_OUT_OF_BOUNDS;

    /* 0–3 = pijlen, 4–5 = smileys */
    if (nr < 0 || nr > 5)
        return ERROR_INVALID_PARAM;

    int status = UB_VGA_DrawBitmap(nr, x_lup, y_lup);
    if (status != 0) {
        return vgaStatusToResultaat(status);
    }

    Commando c;
        memset(&c, 0, sizeof(Commando));
        c.type = CMD_BITMAP; c.p1 = nr; c.p2 = x_lup; c.p3 = y_lup;
        log_commando(c);

    return OK;
}

// Wis het scherm
Resultaat clearscherm(const char *kleur) {
    if (!validColor(kleur))
        return ERROR_INVALID_COLOR;

    //stuur naar VGA scherm
    int status = UB_VGA_FillScreen(kleurToCode(kleur));
    if (status != 0) {
        return vgaStatusToResultaat(status);
    }

    Commando c;
        memset(&c, 0, sizeof(Commando));
        c.type = CMD_CLEAR;
        strncpy(c.kleur, kleur, 19);
        log_commando(c);
    return OK;
}

Resultaat wacht(int msecs) {
    if (msecs < 0) return ERROR_INVALID_PARAM;

    // Gebruik de CPU frequentie om te berekenen hoeveel loops we nodig hebben
    // Voor een F407 op 168MHz is dit ongeveer:
    //uint32_t count = msecs * (SystemCoreClock / 10000);

    wachten(msecs);
    //for (/*volatile*/ uint32_t i = 0; i < count; i++) {
   //     //__NOP(); // Doe niets
   // }

    Commando c;
        memset(&c, 0, sizeof(Commando));
        c.type = CMD_WAIT; c.p1 = msecs;
        log_commando(c);

    return OK;
}

int wachten(msecs){
	uint32_t count = msecs * (SystemCoreClock / 10000);
	for (volatile uint32_t i = 0; i < count; i++);
}

Resultaat cirkel(int x, int y, int radius, const char *kleur) {
    if (radius <= 0) {
        return ERROR_INVALID_PARAM; // Straal moet positief zijn
    }
    // Controleer of de hele cirkel binnen de schermgrenzen valt
    if (x - radius < 0 || x + radius >= SCHERM_BREEDTE || y - radius < 0 || y + radius >= SCHERM_HOOGTE) {
        return ERROR_OUT_OF_BOUNDS;
    }
    if (!validColor(kleur)) {
        return ERROR_INVALID_COLOR;
    }

    // stuur naar VGA scherm
    int status = UB_VGA_DrawCircle(x, y, radius, kleurToCode(kleur));
    if (status != 0) {
        return vgaStatusToResultaat(status);
    }

    Commando c;
        memset(&c, 0, sizeof(Commando));
        c.type = CMD_CIRKEL; c.p1 = x; c.p2 = y; c.p3 = radius;
        strncpy(c.kleur, kleur, 19);
        log_commando(c);

    return OK;
}

Resultaat figuur(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int x5, int y5, const char *kleur) {
    // Punten in arrays zetten voor makkelijke verwerking
    int x[] = {x1, x2, x3, x4, x5};
    int y[] = {y1, y2, y3, y4, y5};
    int aantal_punten = 5;
    Resultaat res;

    //Validatie: Kleur controleren
    if (!validColor(kleur)) {
        return ERROR_INVALID_COLOR;
    }

    //Validatie: Alle punten controleren op schermgrenzen
    for (int i = 0; i < aantal_punten; i++) {
        if (x[i] < 0 || x[i] >= SCHERM_BREEDTE || y[i] < 0 || y[i] >= SCHERM_HOOGTE) {
            return ERROR_OUT_OF_BOUNDS;
        }
    }

    if (!validColor(kleur)) return ERROR_INVALID_COLOR;

        //Teken de lijnen direct via de VGA driver (niet via lijn())
        uint8_t c_code = kleurToCode(kleur);
        UB_VGA_DrawLine(x1, y1, x2, y2, c_code, 1);
        UB_VGA_DrawLine(x2, y2, x3, y3, c_code, 1);
        UB_VGA_DrawLine(x3, y3, x4, y4, c_code, 1);
        UB_VGA_DrawLine(x4, y4, x5, y5, c_code, 1);
        UB_VGA_DrawLine(x5, y5, x1, y1, c_code, 1);

        //Sla alles op als één commando
        Commando c;
        memset(&c, 0, sizeof(Commando));
        c.type = CMD_FIGUUR;
        c.p1 = x1; c.p2 = y1; c.p3 = x2; c.p4 = y2; c.p5 = x3;
        c.p6 = y3; c.p7 = x4; c.p8 = y4; c.p9 = x5; c.p10 = y5;
        strncpy(c.kleur, kleur, 19);
        log_commando(c);

    return OK;
}
// onderste code met chat gescheven
Resultaat herhaal(int aantal, int hoevaak) {
	if (aantal <= 0 || aantal > totaal_opgeslagen || hoevaak <= 0) {
	        return ERROR_INVALID_PARAM; //
	}

    for (int h = 0; h < hoevaak; h++) {
        int idx = (geschiedenis_index - aantal + 20) % 20;
        for (int i = 0; i < aantal; i++) {
            Commando *c = &geschiedenis[idx];
            switch (c->type) {
                case CMD_LIJN:      UB_VGA_DrawLine(c->p1, c->p2, c->p3, c->p4, kleurToCode(c->kleur), c->p5); break;
                case CMD_RECHTHOEK: UB_VGA_DrawRectangle(c->p1, c->p2, c->p3, c->p4, kleurToCode(c->kleur), c->p5); break;
                case CMD_CIRKEL:    UB_VGA_DrawCircle(c->p1, c->p2, c->p3, kleurToCode(c->kleur)); break;
                case CMD_TEKST:     UB_VGA_DrawText(c->p1, c->p2, kleurToCode(c->kleur), c->tekst_inhoud, c->fontnaam, c->p3, c->fontstijl); break;
                //case CMD_BITMAP:    UB_VGA_DrawBitmap(c->p1, c->p2, c->p3); break;
                case CMD_CLEAR:     UB_VGA_FillScreen(kleurToCode(c->kleur)); break;
                case CMD_WAIT:      wachten(c->p1); break;
                case CMD_FIGUUR:
                    // Teken alle 5 de lijnen opnieuw vanuit de opgeslagen parameters p1 t/m p10
                    UB_VGA_DrawLine(c->p1, c->p2, c->p3, c->p4, kleurToCode(c->kleur), 1);
                    UB_VGA_DrawLine(c->p3, c->p4, c->p5, c->p6, kleurToCode(c->kleur), 1);
                    UB_VGA_DrawLine(c->p5, c->p6, c->p7, c->p8, kleurToCode(c->kleur), 1);
                    UB_VGA_DrawLine(c->p7, c->p8, c->p9, c->p10, kleurToCode(c->kleur), 1);
                    UB_VGA_DrawLine(c->p9, c->p10, c->p1, c->p2, kleurToCode(c->kleur), 1);
                    break;
                default:
                    break;
            }
            idx = (idx + 1) % 20;
        }
    }
    return OK;
}

Resultaat vgaStatusToResultaat(int status) {
    switch (status) {
        case 0:
            return OK;  // Alles OK
        case 1:
            return ERROR_VGA_INVALID_COORDINATE;
        case 2:
            return ERROR_VGA_INVALID_PARAMETER;
        default:
            return ERROR_VGA;
    }
}
