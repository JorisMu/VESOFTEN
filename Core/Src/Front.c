
 /**
 * @file    Front.c
 * @brief   VGA_core DMA LIB 320x240, 8bit color
 * @details Deze C-code draait op een STM32F4 microcontroller
 *          en implementeert een UART-command interface waarmee 
 *          je via tekstcommando’s (bijv. via een terminal) 
 *          grafische opdrachten kunt sturen naar een VGA/logic-laag.
 *
 * @date    12.01.2026
 * @author  J. de Bruijne
 */

 //--------------------------------------------------------------
// File     : Front.c
// Datum    : 08/01/2026
// Version  : 1.0
// Autor    : JB
// mods by  : J. de Bruijne
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.x
// Module   : CMSIS_BOOT, M4_CMSIS_CORE
// Function : VGA_core DMA LIB 320x240, 8bit color
//--------------------------------------------------------------

#include "stm32f4xx.h"
#include "front.h"
#include "logic.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define UART_BUF_SIZE 128
#define UART_RX_BUFFER_SIZE 128

// UART buffers
static char uart_rx_buffer[UART_RX_BUFFER_SIZE];
static uint16_t uart_rx_index = 0;
static volatile uint8_t uart_line_ready = 0;

volatile char uart_buf[UART_BUF_SIZE]; // Ringbuffer
volatile uint16_t uart_head = 0;
volatile uint16_t uart_tail = 0;

char *line_buffer = NULL; ///< Dynamische buffer voor één complete lijn
uint16_t line_idx = 0;    ///< Index in dynamische buffer


// Functies

/**
 * @brief Stuurt een foutmelding via UART.
 * @param msg De foutmelding als string.
 * @return Geen.
 */
void front_send_error(const char *msg)
{
    USART2_SendString(msg);
    USART2_SendString("\r\n");
}

/**
 * @brief Converteert een foutcode naar een leesbare string.
 * @param code Foutcode van Front, Logic of VGA layer.
 * @return const char* Leesbare foutmelding.
 */
const char* status_to_string(int code)
{
    switch(code)
    {
        case FRONT_OK: return "FRONT OK";
        case FRONT_ERROR_EMPTY_INPUT: return "FRONT ERROR: lege input";
        case FRONT_ERROR_PARSE: return "FRONT ERROR: parser fout";
        case FRONT_ERROR_UNKNOWN_COMMAND: return "FRONT ERROR: onbekend commando";

        case OK: return "LOGIC OK";
        case ERROR_INVALID_COLOR: return "LOGIC ERROR: ongeldig kleur";
        case ERROR_INVALID_PARAM_THICKNESS: return "LOGIC ERROR: ongeldig dikte";
        case ERROR_INVALID_PARAM_SIZE: return "LOGIC ERROR: ongeldige afmetingen";
        case ERROR_INVALID_PARAM_FILLED: return "LOGIC ERROR: ongeldig gevuld veld";
        case ERROR_INVALID_PARAM_FONTNAME: return "LOGIC ERROR: ongeldige fontnaam";
        case ERROR_INVALID_PARAM_FONTSIZE: return "LOGIC ERROR: ongeldige fontgrootte";
        case ERROR_INVALID_PARAM_FONSTYLE: return "LOGIC ERROR: ongeldig fontstijl";
        case ERROR_INVALID_PARAM: return "LOGIC ERROR: ongeldig parameter";
        case ERROR_OUT_OF_BOUNDS: return "LOGIC ERROR: coördinaten buiten scherm";
        case ERROR_TEXT_TOO_LONG: return "LOGIC ERROR: tekst te lang";
        case ERROR_TOO_MANY_REPEATS: return "LOGIC ERROR: te veel herhalingen";

        case VGA_OK: return "VGA OK";
        case ERROR_VGA: return "IO ERROR: VGA fout";
        case ERROR_VGA_INVALID_COORDINATE: return "IO ERROR: VGA ongeldig coördinaat";
        case ERROR_VGA_INVALID_PARAMETER: return "IO ERROR: VGA ongeldig parameter";

        default: return "Onbekende foutcode";
    }
}

/**
 * @brief Parseert een commando string en vult een Command struct.
 * @param input De input string (bijv. "LIJN,0,0,100,100,rood,2").
 * @param cmd Pointer naar Command struct die gevuld wordt.
 * @return FrontStatus code (FRONT_OK of foutcode).
 */
FrontStatus parse_command(const char* input, Command* cmd)
{
    if (input == NULL || cmd == NULL || strlen(input) == 0)
        return FRONT_ERROR_EMPTY_INPUT;

    char buffer[200];
    strncpy(buffer, input, sizeof(buffer)-1);
    buffer[sizeof(buffer)-1] = '\0';

    char* Commando = strtok(buffer, ",");
    if (Commando == NULL)
        return FRONT_ERROR_PARSE;

    // LIJN command
    if (strcmp(Commando, "lijn") == 0)
    {
        cmd->type = CMD_LIJN;
        int n = sscanf(input, "lijn,%d,%d,%d,%d, %19[^,],%d",
                       &cmd->x, &cmd->y, &cmd->x2, &cmd->y2, cmd->kleur, &cmd->dikte);
        if(n != 6)
            return FRONT_ERROR_PARSE;
    }

    // RECHTHOEK command
    else if(strcmp(Commando, "rechthoek") == 0)
    {
        cmd->type = CMD_RECHTHOEK;
        int n = sscanf(input, "rechthoek,%d,%d,%d,%d, %19[^,],%d",
                       &cmd->x, &cmd->y, &cmd->breedte, &cmd->hoogte, cmd->kleur, &cmd->gevuld);
        if(n != 6)
            return FRONT_ERROR_PARSE;
    }

    // TEKST command
    else if (strcmp(Commando, "tekst") == 0)
    {
        cmd->type = CMD_TEKST;
        // Voeg spaties toe vóór stringvelden
        int n = sscanf(input, "tekst,%d,%d, %19[^,], %199[^,], %19[^,],%d, %19s",
                       &cmd->x, &cmd->y, cmd->kleur, cmd->tekst,
                       cmd->fontnaam, &cmd->fontgrootte, cmd->fontstijl);
        if (n != 7)
            return FRONT_ERROR_PARSE;
    }

    // BITMAP command
    else if(strcmp(Commando, "bitmap") == 0)
    {
        cmd->type = CMD_BITMAP;
        int n = sscanf(input, "bitmap,%d,%d,%d", &cmd->bitmap_nr, &cmd->x, &cmd->y);
        if(n != 3) return FRONT_ERROR_PARSE;
    }

    // CLEARSCHERM command
    else if(strcmp(Commando, "clearscherm") == 0)
    {
        cmd->type = CMD_CLEARSCHERM;
        int n = sscanf(input, "clearscherm, %19[^,\n]", cmd->kleur); // spatie voor kleur
        if(n != 1) return FRONT_ERROR_PARSE;
    }

    // WACHT command
    else if(strcmp(Commando, "wacht") == 0)
    {
        cmd->type = CMD_WACHT;
        int n = sscanf(input, "wacht,%d", &cmd->aantal);
        if(n != 1) return FRONT_ERROR_PARSE;
    }

    // HERHAAL command
    else if(strcmp(Commando, "herhaal") == 0)
    {
        cmd->type = CMD_HERHAAL;
        int n = sscanf(input, "herhaal,%d,%d", &cmd->start, &cmd->aantal);
        if(n != 2) return FRONT_ERROR_PARSE;
    }

    // CIRKEL command
    else if(strcmp(Commando, "cirkel") == 0)
    {
        cmd->type = CMD_CIRKEL;
        int n = sscanf(input, "cirkel,%d,%d,%d, %19[^,\n]", &cmd->x, &cmd->y, &cmd->radius, cmd->kleur);
        if(n != 4) return FRONT_ERROR_PARSE;
    }

    // FIGUUR command
    else if(strcmp(Commando, "figuur") == 0)
    {
        cmd->type = CMD_FIGUUR;
        int n = sscanf(input, "figuur,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d, %19[^,\n]",
                       &cmd->x, &cmd->y, &cmd->x2, &cmd->y2, &cmd->x3, &cmd->y3,
                       &cmd->x4, &cmd->y4, &cmd->x5, &cmd->y5, cmd->kleur);
        if(n != 11) return FRONT_ERROR_PARSE;
    }

    // ERROR unknown command
    else
    {
        cmd->type = CMD_UNKNOWN;
        return FRONT_ERROR_UNKNOWN_COMMAND;
    }

    return FRONT_OK;
}

/**
 * @brief Verwerkt één inputregel, valideert en voert het commando uit.
 * @param input_line Input string van UART.
 * @return Geen, fouten worden via UART gerapporteerd.
 */
void front_handle_input(const char* input_line)
{

    Command cmd;
    FrontStatus parse_status = parse_command(input_line, &cmd);

    if(parse_status != FRONT_OK)
    {
        front_send_error(status_to_string(parse_status));
        return;
    }

    Resultaat result = OK;

    switch(cmd.type)
    {
        case CMD_LIJN: result = lijn(cmd.x, cmd.y, cmd.x2, cmd.y2, cmd.kleur, cmd.dikte); break;
        case CMD_RECHTHOEK: result = rechthoek(cmd.x, cmd.y, cmd.breedte, cmd.hoogte, cmd.kleur, cmd.gevuld); break;
        case CMD_TEKST: result = tekst(cmd.x, cmd.y, cmd.kleur, cmd.tekst, cmd.fontnaam, cmd.fontgrootte, cmd.fontstijl); break;
        case CMD_CIRKEL: result = cirkel(cmd.x, cmd.y, cmd.radius, cmd.kleur); break;
        case CMD_FIGUUR: result = figuur(cmd.x, cmd.y, cmd.x2, cmd.y2, cmd.x3, cmd.y3, cmd.x4, cmd.y4, cmd.x5, cmd.y5, cmd.kleur); break;
        case CMD_CLEARSCHERM: result = clearscherm(cmd.kleur); break;
        case CMD_BITMAP: result = bitmap(cmd.bitmap_nr, cmd.x, cmd.y); break;
        case CMD_WACHT: result = wacht(cmd.aantal); break;
        case CMD_HERHAAL: result = herhaal(cmd.start, cmd.aantal); break;
        default: result = ERROR_INVALID_PARAM; break;
    }

    if(result != OK)
    	front_send_error(status_to_string(result));
    else
    	USART2_SendString("OK uitgevoerd!\r\n");
}

/* ======================= UART INIT & INTERRUPT ======================= */

void USART2_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // PA2 = TX, PA3 = RX
    GPIOA->MODER |= (2 << (2*2)) | (2 << (3*2));
    GPIOA->AFR[0] |= (7 << (2*4)) | (7 << (3*4));

    uint32_t pclk1 = SystemCoreClock / 4;
    USART2->BRR = pclk1 / 115200;

    // TE, RE, UE, RXNE interrupt
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;
    NVIC_EnableIRQ(USART2_IRQn);
}

/**
 * @brief USART2 interrupt handler.
 * Plaatst ontvangen karakters in de ringbuffer.
 */
void USART2_IRQHandler(void)
{
    if (USART2->SR & USART_SR_RXNE)
    {
        char c = USART2->DR;
        uint16_t next = (uart_head + 1) % UART_BUF_SIZE;
        if(next != uart_tail)  // buffer niet vol
        {
            uart_buf[uart_head] = c;
            uart_head = next;
        }
    }
}

/* ======================= UART TRANSMIT ======================= */

void USART2_SendChar(char c)
{
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = c;
}

void USART2_SendString(const char *str)
{
    while(*str) USART2_SendChar(*str++);
}

/* ======================= UART BUFFER PROCESSING ======================= */

/**
 * @brief Verwerkt de ringbuffer.
 * Bouwt dynamische buffer voor volledige lijnen en roept parser aan.
 */
void USART2_BUFFER(void)
{
    while(uart_tail != uart_head)
    {
        char c = uart_buf[uart_tail];
        uart_tail = (uart_tail + 1) % UART_BUF_SIZE;

        // Dynamische buffer aanmaken/grotere maken
        if(line_buffer == NULL)
        {
            line_buffer = malloc(1);
            line_idx = 0;
        }
        else
        {
            char* tmp = realloc(line_buffer, line_idx + 1);
            if(tmp != NULL) line_buffer = tmp;
        }

        line_buffer[line_idx++] = c;

        // Einde lijn detecteren (\r of \n)
        if(c == '\r' || c == '\n')
        {
            line_buffer[line_idx] = '\0'; // sluit string
            front_handle_input(line_buffer); // parse + call logic layer
            free(line_buffer);
            line_buffer = NULL;
            line_idx = 0;
            USART2_SendString("UART Ready!!!\r\n");
        }
    }
}
