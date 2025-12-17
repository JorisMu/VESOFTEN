#include "logic.h"
#include "bitmap.h"
//dingen die nog gedaan moeten worden:
// - Errors (Dikte, frontletters, frontstyle, enz...)
// - herhaal functionaliteit
// - functies van de high layer direct uitvoeren wanneer functie niet in error schiet


// Lijst van toegestane kleuren
const char *kleuren[] = {
	"zwart", "blauw", "lichtblauw", "groen", "lichtgroen",
    "cyaan", "lichtcyaan", "rood", "lichtrood", "magenta",
    "lichtmagenta", "bruin", "geel", "grijs", "wit"
};

const char *fontnamen[] = {
	"arial", "consolas"
};

const char *stijlen[] = {
	"normaal", "vet", "cursief"
};

static int aantal = sizeof(kleuren) / sizeof(kleuren[0]);
static int aantal = sizeof(fontnamen) / sizeof(fontnamen[0]);
static int aantal = sizeof(stijlen) / sizeof(stijlen[0]);

static int contains(const char *items[], const int aantal, const char *item) {
	for (int i = 0; i < aantal; i++) {
		if (strcmp(item, items[i]) == 0) {
			return 1;  // gevonden
		}
	}
	return 0;  // niet gevonden
}

int validColor(const char *kleur) {
	return contains(kleuren, aantal, kleur);
}

int validFont(const char *fontnaam) {
	return contains(fontnamen, aantal, fontnaam);
}

int validFontstijl(const char *stijl) {
	return contains(stijlen, aantal, stijl);
}

/* ===================== MAIN COMMANDO’S ===================== */


/* ===================== COMMANDO’S ===================== */

// Teken een lijn (zoals eerder)
Resultaat lijn(int x, int y, int x2, int y2, char kleur[20], int dikte) {
    if (x < 0 || x >= SCHERM_BREEDTE || y < 0 || y >= SCHERM_HOOGTE ||
        x2 < 0 || x2 >= SCHERM_BREEDTE || y2 < 0 || y2 >= SCHERM_HOOGTE)
        return ERROR_OUT_OF_BOUNDS;

    if (dikte <= 0)
        return ERROR_INVALID_PARAM_THICKNESS;

    if (!validColor(kleur))
        return ERROR_INVALID_COLOR;

    printf("Lijn tekenen van (%d,%d) naar (%d,%d) met kleur %d en dikte %d\n",
           x, y, x2, y2, kleur, dikte);

    return OK;
}

// Teken een rechthoek
Resultaat rechthoek(int x_lup, int y_lup, int breedte, int hoogte, char kleur[20], int gevuld) {
    if (x_lup < 0 || x_lup >= SCHERM_BREEDTE || y_lup < 0 || y_lup >= SCHERM_HOOGTE ||
        x_lup + breedte > SCHERM_BREEDTE || y_lup + hoogte > SCHERM_HOOGTE)
        return ERROR_OUT_OF_BOUNDS;

    if (breedte <= 0 || hoogte <= 0)
        return ERROR_INVALID_PARAM_SIZE;

    if (gevuld != 0)
        return ERROR_INVALID_PARAM;

    if (!validColor(kleur))
        return ERROR_INVALID_COLOR;

    printf("Rechthoek tekenen op (%d,%d) met breedte %d, hoogte %d, kleur %d, %s\n",
           x_lup, y_lup, breedte, hoogte, kleur, gevuld ? "gevuld" : "niet gevuld");
    return OK;
}

// Teken tekst
Resultaat tekst(int x, int y, char kleur[20], const char *tekst, const char *fontnaam, int fontgrootte, const char *fontstijl) {
    if (x < 0 || x >= SCHERM_BREEDTE || y < 0 || y >= SCHERM_HOOGTE)
        return ERROR_OUT_OF_BOUNDS;

    if (!tekst || !fontnaam || !fontstijl)
        return ERROR_INVALID_PARAM;

    if (!validColor(kleur) || validFont(fontnaam) || validFontstijl(fontstijl) || (fontgrootte != 1 && fontgrootte != 2))
        return ERROR_INVALID_PARAM;

    printf("Tekst '%s' tekenen op (%d,%d) met kleur %d, font '%s', grootte %d, stijl '%s'\n",
           tekst, x, y, kleur, fontnaam, fontgrootte, fontstijl);
    return OK;
}

// Teken bitmap
Resultaat bitmap(int nr, int x_lup, int y_lup) {
    if (x_lup < 0 || x_lup >= SCHERM_BREEDTE || y_lup < 0 || y_lup >= SCHERM_HOOGTE)
        return ERROR_OUT_OF_BOUNDS;

    /* 0–3 = pijlen, 4–5 = smileys */
    if (nr < 0 || nr > 5)
        return ERROR_INVALID_PARAM;

    printf("Bitmap nummer %d tekenen op (%d,%d)\n", nr, x_lup, y_lup);
    return OK;
}

// Wis het scherm
Resultaat clearscherm(char kleur[20]) {
    if (!validColor(kleur))
        return ERROR_INVALID_COLOR;

    printf("Scherm wissen met kleur %d\n", kleur);
    return OK;
}

Resultaat wacht(int msecs) {
    if (msecs < 0) {
        return ERROR_INVALID_PARAM; // Wachten kan niet met negatieve tijd
    }

    printf("Wachten voor %d milliseconden...\n", msecs);
    return OK;
}

Resultaat herhaal(int aantal, int hoevaak) {
    if (aantal <= 0 || hoevaak <= 0) {
        // Een herhaling vereist minstens één commando en minstens één keer
        return ERROR_INVALID_PARAM;
    }
    return OK;
}

Resultaat cirkel(int x, int y, int radius, char kleur[20]) {
    if (radius <= 0) {
        return ERROR_INVALID_PARAM; // Straal moet positief zijn
    }
    // Controleer of de hele cirkel binnen de schermgrenzen valt
    if (x - radius < 0 || x + radius >= SCHERM_BREEDTE ||
        y - radius < 0 || y + radius >= SCHERM_HOOGTE) {
        return ERROR_OUT_OF_BOUNDS;
    }
    if (!validColor(kleur)) {
        return ERROR_INVALID_COLOR;
    }

    printf("Cirkel tekenen op middelpunt (%d,%d) met straal %d en kleur %s\n",
           x, y, radius, kleur);
    return OK;
}

Resultaat figuur(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int x5, int y5, char kleur[20]) {
    // Array van punten voor gemakkelijke iteratie
    int x[] = {x1, x2, x3, x4, x5};
    int y[] = {y1, y2, y3, y4, y5};
    int aantal_punten = 5; // Vaste 5 punten in deze implementatie

    if (!validColor(kleur)) {
        return ERROR_INVALID_COLOR;
    }

    // Controleer elk punt op grensoverschrijding
    for (int i = 0; i < aantal_punten; i++) {
        if (x[i] < 0 || x[i] >= SCHERM_BREEDTE ||
            y[i] < 0 || y[i] >= SCHERM_HOOGTE) {
            return ERROR_OUT_OF_BOUNDS;
        }
    }

    printf("Figuur tekenen met %d punten: (%d,%d), (%d,%d), (%d,%d), (%d,%d), (%d,%d) en kleur %s\n",
           aantal_punten, x1, y1, x2, y2, x3, y3, x4, y4, x5, y5, kleur);
    return OK;
}

