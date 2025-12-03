#include "logic.h"

// Teken een lijn (zoals eerder)
Resultaat lijn(int x, int y, int x2, int y2, char kleur[20], int dikte) {
    if (x < 0 || x >= SCHERM_BREEDTE || y < 0 || y >= SCHERM_HOOGTE ||
        x2 < 0 || x2 >= SCHERM_BREEDTE || y2 < 0 || y2 >= SCHERM_HOOGTE)
        return ERROR_OUT_OF_BOUNDS;
    if (dikte <= 0)
        return ERROR_INVALID_PARAM;
    if (validColor(kleur))
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
        return ERROR_INVALID_PARAM;
    if (validColor(kleur))
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
    if (validColor(kleur))
        return ERROR_INVALID_PARAM;

    printf("Tekst '%s' tekenen op (%d,%d) met kleur %d, font '%s', grootte %d, stijl '%s'\n",
           tekst, x, y, kleur, fontnaam, fontgrootte, fontstijl);
    return OK;
}

// Teken bitmap
Resultaat bitmap(int nr, int x_lup, int y_lup) {
    if (x_lup < 0 || x_lup >= SCHERM_BREEDTE || y_lup < 0 || y_lup >= SCHERM_HOOGTE)
        return ERROR_OUT_OF_BOUNDS;
    if (nr < 0)
        return ERROR_INVALID_PARAM;

    printf("Bitmap nummer %d tekenen op (%d,%d)\n", nr, x_lup, y_lup);
    return OK;
}

// Wis het scherm
Resultaat clearscherm(char kleur[20]) {
    if (validColor(kleur))
        return ERROR_INVALID_COLOR;

    printf("Scherm wissen met kleur %d\n", kleur);
    return OK;
}

int validColor(const char *kleur) {
    // Lijst van toegestane kleuren
    const char *kleuren[] = {
        "zwart", "blauw", "lichtblauw", "groen", "lichtgroen",
        "cyaan", "lichtcyaan", "rood", "lichtrood", "magenta",
        "lichtmagenta", "bruin", "geel", "grijs", "wit"
    };
    int aantalKleuren = sizeof(kleuren) / sizeof(kleuren[0]);

    for (int i = 0; i < aantalKleuren; i++) {
        if (strcmp(kleur, kleuren[i]) == 0) {
            return 0;  // kleur gevonden
        }
    }
    return 1;  // kleur niet gevonden
}
