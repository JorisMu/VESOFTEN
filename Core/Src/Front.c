#include "stm32f4xx.h"
#include <front.h>
#include <logic.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>


#define UART_BUF_SIZE 128
volatile char uart_buf[UART_BUF_SIZE]; // Ringbuffer
volatile uint16_t uart_head = 0;
volatile uint16_t uart_tail = 0;
char *buffer = NULL;
uint16_t idx = 0;

// ------------------------
// Front Error handling
// ------------------------
void front_send_error(const char *msg)
{
    USART2_SendString(msg);
    USART2_SendString("\r\n");
}

// ------------------------
// Convert code to string
// ------------------------
const char* status_to_string(int code) {
    switch(code) {
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

// ------------------------
// Parsing functie
// ------------------------
FrontStatus parse_command(const char* input, Command* cmd)
{
    if(input == NULL || cmd == NULL || strlen(input) == 0)
        return FRONT_ERROR_EMPTY_INPUT;

    char buffer[200];
    strncpy(buffer, input, sizeof(buffer)-1);
    buffer[sizeof(buffer)-1] = '\0';

    char* Commando = strtok(buffer, ",");
    if(Commando == NULL)
        return FRONT_ERROR_PARSE;

    // -------------------
    // Lijn
    // -------------------
    if(strcmp(Commando, "LIJN") == 0) 
    {
        cmd->type = CMD_LIJN;
        int n = sscanf(input, "LIJN,%d,%d,%d,%d,%19[^,],%d",
                       &cmd->x, &cmd->y, &cmd->x2, &cmd->y2, cmd->kleur, &cmd->dikte);
        if(n != 6) return FRONT_ERROR_PARSE;
    }
    // -------------------
    // Rechthoek
    // -------------------
    else if(strcmp(Commando, "RECHTHOEK") == 0) 
    {
        cmd->type = CMD_RECHTHOEK;
        int n = sscanf(input, "RECHTHOEK,%d,%d,%d,%d,%19[^,],%d",
                       &cmd->x, &cmd->y, &cmd->breedte, &cmd->hoogte, cmd->kleur, &cmd->gevuld);
        if(n != 6) return FRONT_ERROR_PARSE;
    }
    // -------------------
    // Tekst
    // -------------------
    else if(strcmp(Commando, "TEKST") == 0) 
    {
        cmd->type = CMD_TEKST;
        // Let op: tekst mag komma bevatten -> custom parser nodig
        char* temp = strdup(input); // copy input
        char* tok = strtok(temp, ","); // skip commando
        int i = 0;
        while(tok != NULL) {
            tok = strtok(NULL, ",");
            if(tok == NULL) break;
            if(i==0) cmd->x = atoi(tok);
            else if(i==1) cmd->y = atoi(tok);
            else if(i==2) strncpy(cmd->kleur, tok, sizeof(cmd->kleur)-1);
            else if(i==3) strncpy(cmd->tekst, tok, sizeof(cmd->tekst)-1);
            else if(i==4) strncpy(cmd->fontnaam, tok, sizeof(cmd->fontnaam)-1);
            else if(i==5) cmd->fontgrootte = atoi(tok);
            else if(i==6) strncpy(cmd->fontstijl, tok, sizeof(cmd->fontstijl)-1);
            i++;
        }
        free(temp);
        if(i != 7) return FRONT_ERROR_PARSE;
    }
    // -------------------
    // Bitmap
    // -------------------
    else if(strcmp(Commando, "BITMAP") == 0) 
    {
        cmd->type = CMD_BITMAP;
        int n = sscanf(input, "BITMAP,%d,%d,%d", 
                       &cmd->bitmap_nr, &cmd->x, &cmd->y);
        if(n != 3) return FRONT_ERROR_PARSE;
    }
    // -------------------
    // Clearscherm
    // -------------------
    else if(strcmp(Commando, "CLEARSCHERM") == 0) 
    {
        cmd->type = CMD_CLEARSCHERM;
        int n = sscanf(input, "CLEARSCHERM,%19s", cmd->kleur);
        if(n != 1) return FRONT_ERROR_PARSE;
    }
    // -------------------
    // Wacht
    // -------------------
    else if(strcmp(Commando, "WACHT") == 0) 
    {
        cmd->type = CMD_WACHT;
        int n = sscanf(input, "WACHT,%d", &cmd->aantal);
        if(n != 1) return FRONT_ERROR_PARSE;
    }
    // -------------------
    // Herhaal
    // -------------------
    else if(strcmp(Commando, "HERHAAL") == 0) 
    {
        cmd->type = CMD_HERHAAL;
        int n = sscanf(input, "HERHAAL,%d,%d", &cmd->start, &cmd->aantal);
        if(n != 2) return FRONT_ERROR_PARSE;
    }
    // -------------------
    // Cirkel
    // -------------------
    else if(strcmp(Commando, "CIRKEL") == 0) 
    {
        cmd->type = CMD_CIRKEL;
        int n = sscanf(input, "CIRKEL,%d,%d,%d,%19s", 
                       &cmd->x, &cmd->y, &cmd->radius, cmd->kleur);
        if(n != 4) return FRONT_ERROR_PARSE;
    }
    // -------------------
    // Figuur
    // -------------------
    else if(strcmp(Commando, "FIGUUR") == 0) 
    {
        cmd->type = CMD_FIGUUR;
        int n = sscanf(input, "FIGUUR,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%19s",
                       &cmd->x, &cmd->y, &cmd->x2, &cmd->y2, &cmd->x3, &cmd->y3,
                       &cmd->x4, &cmd->y4, &cmd->x5, &cmd->y5, cmd->kleur);
        if(n != 11) return FRONT_ERROR_PARSE;
    }
    else
    {
        cmd->type = CMD_UNKNOWN;
        return FRONT_ERROR_UNKNOWN_COMMAND;
    }

    return FRONT_OK;
}

// ------------------------
// Verwerk input + stuur errors via UART
// ------------------------
void front_handle_input(const char* input_line) {

    Command cmd;
    FrontStatus parse_status = parse_command(input_line, &cmd);

    if(parse_status != FRONT_OK) {
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
}

// ------------------------
// UART functies
// ------------------------
void USART2_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    GPIOA->MODER |= (2 << (2*2)) | (2 << (3*2));
    GPIOA->AFR[0] |= (7 << (2*4)) | (7 << (3*4));

    uint32_t pclk1 = SystemCoreClock / 4;
    USART2->BRR = pclk1 / 115200;
    USART2->CR1 = USART_CR1_TE | USART_CR1_RE | USART_CR1_UE | USART_CR1_RXNEIE;
    NVIC_EnableIRQ(USART2_IRQn);
}

void USART2_IRQHandler(void)
{
    if (USART2->SR & USART_SR_RXNE)
    {
        char c = USART2->DR;
        uint16_t next = (uart_head + 1) % UART_BUF_SIZE;
        if(next != uart_tail)
        {
            uart_buf[uart_head] = c;
            uart_head = next;
        }
    }
}

void USART2_SendChar(char c)
{
    while (!(USART2->SR & USART_SR_TXE));
    USART2->DR = c;
}

void USART2_SendString(const char *str)
{
    while (*str) USART2_SendChar(*str++);
}

// ------------------------
// Ringbuffer uitlezen en parser aanroepen
// ------------------------
void USART2_BUFFER(void)
{
    while(uart_tail != uart_head)
    {
        char c = uart_buf[uart_tail];
        uart_tail = (uart_tail + 1) % UART_BUF_SIZE;

        if(buffer == NULL)
        {
            buffer = malloc(1);
            idx = 0;
        }
        else
        {
            char* tmp = realloc(buffer, idx + 1);
            if(tmp == NULL) continue;
            buffer = tmp;
        }

        buffer[idx++] = c;

        if(c == '\n')
        {
            buffer[idx] = '\0';
            front_handle_input(buffer);

            free(buffer);
            buffer = NULL;
            idx = 0;

            USART2_SendString("UART Ready!!!\r\n");
        }
    }
}
