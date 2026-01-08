//--------------------------------------------------------------
// File     : Front.h
// Datum    : 08/01/2026
// Version  : 1.0
// Autor    : JB
// mods by  : J. de Bruijne
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.x
// Module   : CMSIS_BOOT, M4_CMSIS_CORE
// Function : VGA_core DMA LIB 320x240, 8bit color
//--------------------------------------------------------------

#ifndef FRONT_H
#define FRONT_H

#include "logic.h"   // Voor Resultaat enum en eventuele logic functies
#include <stdio.h>
#include <string.h>

// ====================
// Front Layer Status
// ====================

/**
 * @enum FrontStatus
 * @brief Statuscodes voor de front-end laag
 */
typedef enum
{
    FRONT_OK,                     /**< Geen fouten */
    FRONT_ERROR_EMPTY_INPUT,       /**< Lege invoerstring */
    FRONT_ERROR_PARSE,             /**< Fout tijdens parseren */
    FRONT_ERROR_UNKNOWN_COMMAND    /**< Commando niet herkend */
} FrontStatus;

// ====================
// Command Type & Struct
// ====================

/**
 * @enum CommandType
 * @brief Type commando dat geparsed wordt
 */
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

/**
 * @struct Command
 * @brief Struct die alle mogelijke parameters van een commando bevat
 */
typedef struct
{
    CommandType type;           /**< Type commando */

    int x, y;                   /**< Hoofd-coördinaten (startpunt) */
    int x2, y2;                 /**< Tweede punt (lijn/figuur/rechthoek) */
    int x3, y3;                 /**< Derde punt (figuur) */
    int x4, y4;                 /**< Vierde punt (figuur) */
    int x5, y5;                 /**< Vijfde punt (figuur) */

    int breedte, hoogte;        /**< Afmetingen voor rechthoek */
    int dikte;                  /**< Lijndikte */
    int radius;                 /**< Cirkel radius */

    int start;                  /**< Startindex voor herhaal */
    int aantal;                 /**< Aantal voor herhaal of bitmap index */
    int gevuld;                 /**< Gevuld = 1 of 0 */

    int bitmap_nr;              /**< Index bitmap */

    char kleur[20];             /**< Kleurnaam als string */
    char tekst[110];            /**< Tekst voor TEKST commando */
    char fontnaam[30];          /**< Lettertype */
    int fontgrootte;            /**< Grootte lettertype */
    char fontstijl[20];         /**< Stijl lettertype */
} Command;

// ====================
// Front Layer Functions
// ====================

/**
 * @brief Parseert een invoerstring en vult een Command struct.
 * 
 * @param input Pointer naar invoerstring van terminal of script
 * @param cmd Pointer naar Command struct die gevuld wordt
 * @return FrontStatus Succes of fouttype
 */
FrontStatus parse_command(const char* input, Command* cmd);

/**
 * @brief Verwerkt een invoerstring volledig: parse + aanroepen logic layer.
 * Fouten worden direct naar terminal gestuurd.
 * 
 * @param input_line Invoerstring van terminal/script
 */
void front_handle_input(const char* input_line);

/**
 * @brief Zet een front- of logic-layer foutcode om naar een leesbare string.
 * 
 * @param code Foutcode (FrontStatus of Resultaat)
 * @return const char* Leesbare foutmelding
 */
const char* status_to_string(int code);

/**
 * @brief Initialiseert USART2 voor communicatie met PC/terminal.
 * Configuratie: 115200 baud, RXNE interrupt enabled.
 */
void USART2_Init(void);

/**
 * @brief Verwerkt de ringbuffer van USART2 en roept parser aan bij volledige lijnen.
 */
void USART2_BUFFER(void);

/**
 * @brief Stuurt een string via USART2 naar de terminal.
 *
 * @param str Pointer naar null-terminated string
 */
void USART2_SendString(const char *str);

#endif // FRONT_H
