#include <stdio.h>
#include <logic.h>
#include <string.h>
#include <stdlib.h>

typedef enum 
{
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

typedef struct 
{
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

static FrontStatus parse_command(const char* input, Command* cmd)
{
    if(input == NULL || cmd == NULL || strlen(input) == 0)
        return FRONT_ERROR_EMPTY_INPUT;

    // Maak een buffer, want strtok/sscanf mag input niet overschrijven
    char buffer[200];
    strncpy(buffer, input, sizeof(buffer)-1);
    buffer[sizeof(buffer)-1] = '\0';

    // Eerste string tot ,: is het commando
    char* Commando = strtok(buffer, ",");
    if(Commando == NULL)
        return FRONT_ERROR_PARSE;

    // Invullen van alle benodigde waardes per functie
    // Functie lijn: lijn, x, y, x’, y’, kleur, dikte
    if(strcmp(Commando, "LIJN") == 0) 
    {
        cmd->type = CMD_LIJN;
        int n = sscanf(input, "LIJN,%d,%d,%d,%d,%19[^,],%d",
                       &cmd->x, &cmd->y, &cmd->x2, &cmd->y2, cmd->kleur, &cmd->dikte);
        if(n != 6) return FRONT_ERROR_PARSE;
    }
    
    // Functie rechthoek: rechthoek, x_lup, y_lup, breedte, hoogte, kleur, gevuld (1,0) [als 1: rand (1px) met kleur]
    else if(strcmp(Commando, "RECHTHOEK") == 0) 
    {
        cmd->type = CMD_RECHTHOEK;
        int n = sscanf(input, "RECHTHOEK,%d,%d,%d,%d,%19[^,],%d",
                       &cmd->x, &cmd->y, &cmd->breedte, &cmd->hoogte, cmd->kleur, &cmd->gevuld);
        if(n != 6) return FRONT_ERROR_PARSE;
    }

    // Functie tekst: tekst, x, y, kleur, tekst, fontnaam (arial, consolas), fontgrootte (1,2), fontstijl (normaal, vet, cursief)
    else if(strcmp(Commando, "TEKST") == 0) 
    {
        cmd->type = CMD_TEKS;
        int n = sscanf(input, "TEKST,%d,%d,%19[^,],%109[^,],%29[^,],%d,%19[^,]",
                       &cmd->x, &cmd->y, cmd->kleur, cmd->tekst, cmd->fontnaam, &cmd->fontgrootte, cmd->fontstijl);
        if(n != 7) return FRONT_ERROR_PARSE;
    }

    // Functie bitmap: bitmap, nr, x-lup, y-lup  [tenminste: pijl (in 4 richtingen), smiley (boos, blij)]
    else if(strcmp(Commando, "BITMAP") == 0) 
    {
        cmd->type = CMD_BITMAP;
        int n = sscanf(input, "BITMAP,%d,%d,%d", 
                       &cmd->bitmap_nr, &cmd->x, &cmd->y);
        if(n != 3) return FRONT_ERROR_PARSE;
    }

    // Functie clearscherm: clearscherm, kleur
    else if(strcmp(Commando, "CLEARSCHERM") == 0) 
    {
        cmd->type = CMD_CLEARSCHERM;
        int n = sscanf(input, "CLEARSCHERM,%19s", 
                       cmd->kleur);
        if(n != 1) return FRONT_ERROR_PARSE;
    }

    // Functie wacht: wacht, msecs 
    else if(strcmp(Commando, "WACHT") == 0) 
    {
        cmd->type = CMD_WACHT;
        int n = sscanf(input, "WACHT,%d", 
                       &cmd->aantal); // gebruik 'aantal' voor ms
        if(n != 1) return FRONT_ERROR_PARSE;
    }

    // Functie herhaal: herhaal, aantal (laatst uitgevoerde commando’s), hoevaak (herhalen)
    else if(strcmp(Commando, "HERHAAL") == 0) 
    {
        cmd->type = CMD_HERHAAL;
        int n = sscanf(input, "HERHAAL,%d,%d", 
                       &cmd->start, &cmd->aantal);
        if(n != 2) return FRONT_ERROR_PARSE;
    }

    // Functie cirkel: cirkel, x, y, radius, kleur
    else if(strcmp(Commando, "CIRKEL") == 0) 
    {
        cmd->type = CMD_CIRKEL;
        int n = sscanf(input, "CIRKEL,%d,%d,%d,%19s", 
                       &cmd->x, &cmd->y, &cmd->radius, cmd->kleur);
        if(n != 4) return FRONT_ERROR_PARSE;
    }

    // Functie figuur: figuur, x1,y1, x2,y2, x3,y3, x4,y4, x5,y5, kleur
    else if(strcmp(Commando, "FIGUUR") == 0) 
    {
    cmd->type = CMD_FIGUUR;
    int n = sscanf(input, "FIGUUR,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%19s",
                   &cmd->x, &cmd->y, &cmd->x2, &cmd->y2, &cmd->x3, &cmd->y3, &cmd->x4, &cmd->y4, &cmd->x5, &cmd->y5, cmd->kleur);
    if(n != 11) return FRONT_ERROR_PARSE;
    }

    else 
    {
        cmd->type = CMD_UNKNOWN;
        return FRONT_ERROR_UNKNOWN_COMMAND;
    }

    return FRONT_OK;
}

void front_handler(Command* cmd) {
    char resultaat;

    switch(cmd->type) 
    {
        case CMD_LIJN:
            resultaat = lijn(cmd->x, cmd->y, cmd->x2, cmd->y2, cmd->kleur, cmd->dikte);
            break;
        case CMD_RECHTHOEK:
            resultaat = rechthoek(cmd->x, cmd->y, cmd->breedte, cmd->hoogte, cmd->kleur, cmd->gevuld);
            break;
        case CMD_TEKS:
            resultaat = tekst(cmd->x, cmd->y, cmd->kleur, cmd->tekst, cmd->fontnaam, cmd->fontgrootte, cmd->fontstijl);
            break;
        case CMD_CIRKEL:
            resultaat = cirkel(cmd->x, cmd->y, cmd->radius, cmd->kleur);
            break;
        case CMD_FIGUUR:
            resultaat = figuur(cmd->x, cmd->y, cmd->x2, cmd->y2, cmd->x3, cmd->y3, cmd->x4, cmd->y4, cmd->x5, cmd->y5, cmd->kleur);
            break;
        case CMD_CLEARSCHERM:
            resultaat = clearscherm(cmd->kleur);
            break;
        case CMD_BITMAP:
            resultaat = bitmap(cmd->bitmap_nr, cmd->x, cmd->y);
            break;
        case CMD_WACHT:
            resultaat = wait(cmd->aantal);
            break;
        case CMD_HERHAAL:
            resultaat = herhaal(cmd->start, cmd->aantal);
            break;
        default:
            printf("Onbekend commando\n");
            return;
    }

    handle_logic_result(resultaat);
}


