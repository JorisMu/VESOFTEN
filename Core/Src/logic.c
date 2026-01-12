 /**
 * @file    logic.c
 * @brief   Bitmap pixel data definitions.
 * @details Logic layer voor VGA-tekencommando’s. Deze laag
 *          valideert invoer, vertaalt parameters naar VGA-codes en
 *          vormt de tussenlaag tussen applicatie en VGA-driver.
 *
 * @date    12.01.2026
 * @author  M. Untersalmberger
 */

#include "logic.h"

// Lijst van toegestane kleuren, lettertypes en stijlen voor validatie
const char *kleuren[] = { "zwart", "blauw", "lichtblauw", "groen", "lichtgroen", "cyaan", "lichtcyaan", "rood", "lichtrood", "magenta", "lichtmagenta", "bruin", "geel", "grijs", "wit"};
const char *fontnamen[] = {"arial", "consolas"};
const char *stijlen[] = {"normaal", "vet", "cursief"};

// Statische buffer om de laatste 20 commando's op te slaan voor de 'herhaal' functionaliteit
static Commando geschiedenis[20];
static int geschiedenis_index = 0;
static int totaal_opgeslagen = 0;

static int aantal_kleur = sizeof(kleuren) / sizeof(kleuren[0]);
static int aantal_fontnaam = sizeof(fontnamen) / sizeof(fontnamen[0]);
static int aantal_stijl = sizeof(stijlen) / sizeof(stijlen[0]);

/**
 * @brief Slaat een uitgevoerd commando op in het circulaire geheugen (geschiedenis).
 * @param c: Het Commando struct dat opgeslagen moet worden.
 * @return Geen.
 * @note Geheugengebruik: Schrijft naar een statische array van 20 structs.
 */
static void log_commando(Commando c)
{
    geschiedenis[geschiedenis_index] = c;
    // Circulaire index: als we bij 20 zijn, gaan we terug naar 0
    geschiedenis_index = (geschiedenis_index + 1) % 20;
    if (totaal_opgeslagen < 20) totaal_opgeslagen++;
}

/**
 * @brief Controleert of een string aanwezig is in een lijst van toegestane strings.
 * @param items: Array van strings.
 * @param aantal: Aantal items in de array.
 * @param item: De te zoeken string.
 * @return 1 indien gevonden, 0 indien niet gevonden.
 * @time O(n) waarbij n het aantal items is.
 */
static int contains(const char *items[], const int aantal, const char *item)
{
	for (int i = 0; i < aantal; i++)
	{
		if (strcmp(item, items[i]) == 0)
			return 1;  // gevonden
	}
	return 0;  // niet gevonden
}

// Hulpfuncties voor validatie van specifieke parameters
int validColor(const char *kleur) {return contains(kleuren, aantal_kleur, kleur);}
int validFont(const char *fontnaam) {return contains(fontnamen, aantal_fontnaam, fontnaam);}
int validFontstijl(const char *stijl) {return contains(stijlen, aantal_stijl, stijl);}

/**
 * @brief Vertaalt een tekstuele kleurnaam naar een hardware-specifieke VGA kleurcode.
 * @param kleur: String (bijv. "rood").
 * @return uint8_t VGA kleurcode.
 */
uint8_t kleurToCode(const char *kleur)
{
	uint8_t code;

	if (strcmp(kleur, "zwart") == 0)
		code = VGA_COL_BLACK;
	else if (strcmp(kleur, "blauw") == 0)
		code = VGA_COL_BLUE;
	else if (strcmp(kleur, "lichtblauw") == 0)
		code = VGA_COL_LIGHT_BLUE;
	else if (strcmp(kleur, "groen") == 0)
		code = VGA_COL_GREEN;
 	else if (strcmp(kleur, "lichtgroen") == 0)
 		code = VGA_COL_LIGHT_GREEN;
	else if (strcmp(kleur, "cyaan") == 0)
		code = VGA_COL_CYAN;
	else if (strcmp(kleur, "lichtcyaan") == 0)
		code = VGA_COL_LIGHT_CYAN;
	else if (strcmp(kleur, "rood") == 0)
		code = VGA_COL_RED;
	else if (strcmp(kleur, "lichtrood") == 0)
		code = VGA_COL_LIGHT_RED;
	else if (strcmp(kleur, "magenta") == 0)
		code = VGA_COL_MAGENTA;
	else if (strcmp(kleur, "lichtmagenta") == 0)
		code = VGA_COL_LIGHT_MAGENTA;
	else if (strcmp(kleur, "bruin") == 0)
		code = VGA_COL_BROWN;
	else if (strcmp(kleur, "geel") == 0)
		code = VGA_COL_YELLOW;
	else if (strcmp(kleur, "grijs") == 0)
		code = VGA_COL_GREY;
	else if (strcmp(kleur, "wit") == 0)
		code = VGA_COL_WHITE;
	return code;
}

/* ===================== COMMANDO’S ===================== */

/**
 * @brief Tekent een lijn op het scherm na validatie van de coördinaten.
 * @param x, y: Startpunt.
 * @param x2, y2: Eindpunt.
 * @param kleur: Kleurnaam als string.
 * @param dikte: Lijndikte in pixels.
 * @return Resultaat: OK, ERROR_OUT_OF_BOUNDS, ERROR_INVALID_COLOR, etc.
 */
Resultaat lijn(int x, int y, int x2, int y2, const char *kleur, int dikte)
{
	// Validatie: vallen de punten binnen het bereik?
    if (x < 0 || x >= SCHERM_BREEDTE || y < 0 || y >= SCHERM_HOOGTE || x2 < 0 || x2 >= SCHERM_BREEDTE ||y2 < 0 || y2 >= SCHERM_HOOGTE)
    	return ERROR_OUT_OF_BOUNDS;

    if (dikte <= 0)
        return ERROR_INVALID_PARAM_THICKNESS;

    if (!validColor(kleur))
        return ERROR_INVALID_COLOR;

    // Directe aanroep naar de hardware driver
    int status = UB_VGA_DrawLine(x, y, x2, y2, kleurToCode(kleur), dikte);
    if (status != 0)
    	return vgaStatusToResultaat(status);

    // Commando struct vullen en loggen voor de herhaal-functie
    Commando c;
    memset(&c, 0, sizeof(Commando));
    c.type = CMD_LIJN; c.p1 = x; c.p2 = y; c.p3 = x2; c.p4 = y2; c.p5 = dikte;
    strncpy(c.kleur, kleur, 19);
    log_commando(c);

    return OK;
}

/**
 * @brief Tekent een (gevulde) rechthoek.
 * @param x_lup, y_lup: Linkerbovenhoek coördinaten.
 * @param breedte, hoogte: Afmetingen.
 * @param kleur: Kleurnaam.
 * @param gevuld: 1 voor gevuld, 0 voor alleen rand.
 * @return Resultaat statuscode.
 */
Resultaat rechthoek(int x_lup, int y_lup, int breedte, int hoogte, const char *kleur, int gevuld)
{
	// Check of de volledige rechthoek binnen het scherm past
    if (x_lup < 0 || x_lup >= SCHERM_BREEDTE || y_lup < 0 || y_lup >= SCHERM_HOOGTE || x_lup + breedte > SCHERM_BREEDTE || y_lup + hoogte > SCHERM_HOOGTE)
        return ERROR_OUT_OF_BOUNDS;
    if (breedte <= 0 || hoogte <= 0)
        return ERROR_INVALID_PARAM_SIZE;
    if (gevuld < 0 || gevuld > 1)
        return ERROR_INVALID_PARAM_FILLED;
    if (!validColor(kleur))
        return ERROR_INVALID_COLOR;

    int status = UB_VGA_DrawRectangle(x_lup, y_lup, breedte, hoogte, kleurToCode(kleur), gevuld);
    if (status != 0)
        return vgaStatusToResultaat(status);

    Commando c;
    memset(&c, 0, sizeof(Commando));
    c.type = CMD_RECHTHOEK; c.p1 = x_lup; c.p2 = y_lup; c.p3 = breedte; c.p4 = hoogte; c.p5 = gevuld;
    strncpy(c.kleur, kleur, 19);
    log_commando(c);

    return OK;
}

/**
 * @brief Plaatst tekst op het scherm met specifiek font en grootte.
 * @param x, y: Startpositie.
 * @param kleur: Kleurnaam.
 * @param tekst: De te tonen string (max 100 tekens).
 * @param fontnaam: "arial" of "consolas".
 * @param fontgrootte: 1 (normaal) of 2 (groot).
 * @param fontstijl: "normaal", "vet", of "cursief".
 * @return Resultaat statuscode.
 */
Resultaat tekst(int x, int y, const char *kleur, const char tekst[100], const char fontnaam[20], int fontgrootte, const char fontstijl[20])
{
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

    int status = UB_VGA_DrawText(x, y, kleurToCode(kleur), tekst, fontnaam, fontgrootte, fontstijl);
    if (status != 0)
        return vgaStatusToResultaat(status);

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

/**
 * @brief Tekent een voorgedefinieerde bitmap (pijlen of smileys).
 * @param nr: Index van de bitmap (0-5), 0–3 = pijlen, 4–5 = smileys.
 * @param x_lup, y_lup: Positie op het scherm.
 * @return Resultaat statuscode.
 */
Resultaat bitmap(int nr, int x_lup, int y_lup)
{
    if (x_lup < 0 || x_lup >= SCHERM_BREEDTE || y_lup < 0 || y_lup >= SCHERM_HOOGTE)
        return ERROR_OUT_OF_BOUNDS;

    if (nr < 0 || nr > 5)
        return ERROR_INVALID_PARAM;

    int status = UB_VGA_DrawBitmap(nr, x_lup, y_lup);
    if (status != 0)
        return vgaStatusToResultaat(status);

    Commando c;
    memset(&c, 0, sizeof(Commando));
    c.type = CMD_BITMAP; c.p1 = nr; c.p2 = x_lup; c.p3 = y_lup;
    log_commando(c);

    return OK;
}

/**
 * @brief Vult het volledige scherm met één kleur.
 * @param kleur: Kleurnaam.
 * @return Resultaat statuscode.
 */
Resultaat clearscherm(const char *kleur)
{
    if (!validColor(kleur))
        return ERROR_INVALID_COLOR;

    int status = UB_VGA_FillScreen(kleurToCode(kleur));
    if (status != 0)
        return vgaStatusToResultaat(status);

    Commando c;
    memset(&c, 0, sizeof(Commando));
    c.type = CMD_CLEAR;
    strncpy(c.kleur, kleur, 19);
    log_commando(c);

    return OK;
}

/**
 * @brief Een 'busy-wait' vertraging gebaseerd op de CPU kloksnelheid.
 * @param msecs: Aantal milliseconden om te wachten.
 * @return Altijd 0.
 * @bottleneck Blokkeert de volledige CPU uitvoering; niet geschikt voor multitasking.
 */
int wachten(int msecs)
{
	uint32_t count = msecs * (SystemCoreClock / 10000);
	for (volatile uint32_t i = 0; i < count; i++);

	return 0;
}

/**
 * @brief Wrapper voor de wacht-functie die ook gelogd wordt in de geschiedenis.
 */
Resultaat wacht(int msecs)
{
    if (msecs < 0)
    	return ERROR_INVALID_PARAM;

    wachten(msecs);

    Commando c;
    memset(&c, 0, sizeof(Commando));
    c.type = CMD_WAIT; c.p1 = msecs;
    log_commando(c);

    return OK;
}

/**
 * @brief Tekent een cirkel op basis van middelpunt en straal.
 * @param x, y: Middelpunt.
 * @param radius: Straal in pixels.
 * @param kleur: Kleurnaam.
 * @return Resultaat statuscode.
 */
Resultaat cirkel(int x, int y, int radius, const char *kleur)
{
    if (radius <= 0)
        return ERROR_INVALID_PARAM;
    // Bounds check: past de cirkel binnen de randen van 320x240?
    if (x - radius < 0 || x + radius >= SCHERM_BREEDTE || y - radius < 0 || y + radius >= SCHERM_HOOGTE)
        return ERROR_OUT_OF_BOUNDS;
    if (!validColor(kleur))
        return ERROR_INVALID_COLOR;

    int status = UB_VGA_DrawCircle(x, y, radius, kleurToCode(kleur));
    if (status != 0)
        return vgaStatusToResultaat(status);

    Commando c;
    memset(&c, 0, sizeof(Commando));
    c.type = CMD_CIRKEL; c.p1 = x; c.p2 = y; c.p3 = radius;
    strncpy(c.kleur, kleur, 19);
    log_commando(c);

    return OK;
}

/**
 * @brief Tekent een gesloten figuur (5-hoek) door 5 punten te verbinden.
 * @param x1..y5: Coördinaten van de 5 hoekpunten.
 * @param kleur: Kleurnaam.
 * @return Resultaat statuscode.
 */
Resultaat figuur(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int x5, int y5, const char *kleur)
{
    int x[] = {x1, x2, x3, x4, x5};
    int y[] = {y1, y2, y3, y4, y5};
    int aantal_punten = 5;

    if (!validColor(kleur))
        return ERROR_INVALID_COLOR;

    // Controleer of alle 5 punten binnen het scherm vallen
    for (int i = 0; i < aantal_punten; i++)
    {
        if (x[i] < 0 || x[i] >= SCHERM_BREEDTE || y[i] < 0 || y[i] >= SCHERM_HOOGTE)
            return ERROR_OUT_OF_BOUNDS;
    }

    uint8_t c_code = kleurToCode(kleur);
    // Teken opeenvolgende lijnen tussen de punten
    UB_VGA_DrawLine(x1, y1, x2, y2, c_code, 1);
    UB_VGA_DrawLine(x2, y2, x3, y3, c_code, 1);
    UB_VGA_DrawLine(x3, y3, x4, y4, c_code, 1);
    UB_VGA_DrawLine(x4, y4, x5, y5, c_code, 1);
    UB_VGA_DrawLine(x5, y5, x1, y1, c_code, 1); // Sluit de figuur terug naar punt 1

    Commando c;
    memset(&c, 0, sizeof(Commando));
    c.type = CMD_FIGUUR;
    c.p1 = x1; c.p2 = y1; c.p3 = x2; c.p4 = y2; c.p5 = x3;
    c.p6 = y3; c.p7 = x4; c.p8 = y4; c.p9 = x5; c.p10 = y5;
    strncpy(c.kleur, kleur, 19);
    log_commando(c);

    return OK;
}

/**
 * @brief Herhaalt een specifiek aantal van de laatst uitgevoerde commando's.
 * @param aantal: Hoeveel voorgaande commando's herhaald moeten worden (max 20).
 * @param hoevaak: Hoe vaak deze reeks herhaald moet worden.
 * @return Resultaat statuscode.
 */
Resultaat herhaal(int aantal, int hoevaak)
{
	// Validatie: kunnen we wel zoveel commando's teruggaan in het geheugen?
	if (aantal <= 0 || aantal > totaal_opgeslagen || hoevaak <= 0)
	        return ERROR_INVALID_PARAM; //

    for (int h = 0; h < hoevaak; h++)
    {
    	// Bereken de startindex in de circulaire buffer
        int idx = (geschiedenis_index - aantal + 20) % 20;
        for (int i = 0; i < aantal; i++)
        {
            Commando *c = &geschiedenis[idx];
            // Her-uitvoeren van commando's op basis van hun type
            switch (c->type)
            {
                case CMD_LIJN:
                	UB_VGA_DrawLine(c->p1, c->p2, c->p3, c->p4, kleurToCode(c->kleur), c->p5);
                	break;
                case CMD_RECHTHOEK:
                	UB_VGA_DrawRectangle(c->p1, c->p2, c->p3, c->p4, kleurToCode(c->kleur), c->p5);
                	break;
                case CMD_CIRKEL:
                	UB_VGA_DrawCircle(c->p1, c->p2, c->p3, kleurToCode(c->kleur));
                	break;
                case CMD_TEKST:
                	UB_VGA_DrawText(c->p1, c->p2, kleurToCode(c->kleur), c->tekst_inhoud, c->fontnaam, c->p3, c->fontstijl);
                	break;
                case CMD_BITMAP:
                	UB_VGA_DrawBitmap(c->p1, c->p2, c->p3);
                	break;
                case CMD_CLEAR:
                	UB_VGA_FillScreen(kleurToCode(c->kleur));
                	break;
                case CMD_WAIT:
                	wachten(c->p1);
                	break;
                case CMD_FIGUUR:
                    UB_VGA_DrawLine(c->p1, c->p2, c->p3, c->p4, kleurToCode(c->kleur), 1);
                    UB_VGA_DrawLine(c->p3, c->p4, c->p5, c->p6, kleurToCode(c->kleur), 1);
                    UB_VGA_DrawLine(c->p5, c->p6, c->p7, c->p8, kleurToCode(c->kleur), 1);
                    UB_VGA_DrawLine(c->p7, c->p8, c->p9, c->p10, kleurToCode(c->kleur), 1);
                    UB_VGA_DrawLine(c->p9, c->p10, c->p1, c->p2, kleurToCode(c->kleur), 1);
                    break;
                default:
                    break;
            }
            idx = (idx + 1) % 20; // Volgende commando in de geschiedenis
        }
    }
    return OK;
}

/**
 * @brief Vertaalt returncodes van de VGA driver naar de Resultaat enum van de logic layer.
 * @param status: Integer status van de driver.
 * @return Resultaat enum (OK, ERROR_VGA_...).
 */
Resultaat vgaStatusToResultaat(int status)
{
    switch (status)
    {
        case 0:
            return OK;
        case 1:
            return ERROR_VGA_INVALID_COORDINATE;
        case 2:
            return ERROR_VGA_INVALID_PARAMETER;
        default:
            return ERROR_VGA;
    }
}
