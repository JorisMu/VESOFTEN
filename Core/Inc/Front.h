#ifndef FRONT_H
#define FRONT_H

#include "logic.h"   // Voor Resultaat enum en eventuele logic functies
#include <stdio.h>
#include <string.h>

// ====================
// Front Layer Status
// ====================
typedef enum {
    FRONT_OK,
    FRONT_ERROR_EMPTY_INPUT,
    FRONT_ERROR_PARSE,
    FRONT_ERROR_UNKNOWN_COMMAND
} FrontStatus;

// ====================
// Command Type & Struct
// ====================
typedef enum {
    CMD_LIJN,
    CMD_RECHTHOEK,
    CMD_TEKST,
    CMD_BITMAP,
    CMD_CLEARSCHERM,
    CMD_WACHT,
    CMD_HERHAAL,
    CMD_CIRKEL,
    CMD_FIGUUR,
    CMD_UNKNOWN
} CommandType;

typedef struct {
    CommandType type;
    int x, y, x2, y2, breedte, hoogte, dikte, radius, start, aantal;
    int gevuld;
    int bitmap_nr;
    char kleur[20];
    char tekst[110];
    char fontnaam[30];
    int fontgrootte;
    char fontstijl[20];
} Command;

// ====================
// Front Layer Functions
// ====================

/**
 * Parseert een invoerstring en vult een Command struct.
 * 
 * @param input Invoerstring van de terminal of script
 * @param cmd Pointer naar Command struct die gevuld wordt
 * @return FrontStatus die succes of type fout aangeeft
 */
FrontStatus parse_command(const char* input, Command* cmd);

/**
 * Verwerkt een invoerstring volledig: parse + aanroepen logic layer.
 * Fouten worden direct naar terminal gestuurd.
 * 
 * @param input_line Invoerstring van terminal/script
 */
void front_handle_input(const char* input_line);

/**
 * Zet een front- of logic-layer foutcode om naar een leesbare string.
 * 
 * @param code Foutcode (FrontStatus of Resultaat)
 * @return const char* leesbare foutmelding
 */
const char* status_to_string(int code);

#endif // FRONT_H
