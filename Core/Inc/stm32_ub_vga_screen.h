/**
 * @file stm32_ub_vga_screen.h
 * @brief       VGA-Screen-Library for STM32F4xx
 * @details     This library offers functions to control a 320x240 pixel VGA screen.
 *              It uses TIM1 for the pixel clock, TIM2 for HSync, and DMA2 for transferring
 *              pixel data to the GPIO pins.
 *
 * @date        05.01.2026
 * @author      J. Mullink
 *
 * @note        This library is based on an existing open-source project and has been
 *              adapted for this specific application.
 */

//--------------------------------------------------------------
// File     : stm32_ub_vga_screen.h
//--------------------------------------------------------------

//--------------------------------------------------------------
#ifndef __STM32F4_UB_VGA_SCREEN_H
#define __STM32F4_UB_VGA_SCREEN_H


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "misc.h"
#include "stm32f4xx_dma.h"
#include <stdbool.h>


//--------------------------------------------------------------
/**
 * @brief Color definitions for the 8-bit R3G3B2 color space.
 */
//--------------------------------------------------------------
#define  VGA_COL_BLACK          0x00
#define  VGA_COL_BLUE           0x03
#define  VGA_COL_LIGHT_BLUE     0x5F
#define  VGA_COL_GREEN          0x1C
#define  VGA_COL_LIGHT_GREEN    0x9E
#define  VGA_COL_CYAN           0x1F
#define  VGA_COL_LIGHT_CYAN     0xDF
#define  VGA_COL_RED            0xE0
#define  VGA_COL_LIGHT_RED      0xF2
#define  VGA_COL_MAGENTA        0xE3
#define  VGA_COL_LIGHT_MAGENTA  0xF7
#define  VGA_COL_BROWN          0x88
#define  VGA_COL_YELLOW         0xFC
#define  VGA_COL_GREY           0x92
#define  VGA_COL_WHITE          0xFF

//--------------------------------------------------------------
/**
 * @brief Screen dimensions.
 */
//--------------------------------------------------------------
#define VGA_DISPLAY_X   320
#define VGA_DISPLAY_Y   240

//--------------------------------------------------------------
/**
 * @brief A structure to define a rectangular area.
 */
//--------------------------------------------------------------
typedef struct {
    int32_t x, y, width, height;
} VGA_Rect;


//--------------------------------------------------------------
/**
 * @brief Flags for text styling.
 */
//--------------------------------------------------------------
#define TEXT_STYLE_NORMAL   0x00
#define TEXT_STYLE_BOLD     0x01
#define TEXT_STYLE_ITALIC   0x02


//--------------------------------------------------------------
/**
 * @brief Main structure for VGA controller settings.
 */
//--------------------------------------------------------------
typedef struct {
  uint16_t hsync_cnt;   /*!< HSync line counter */
  uint32_t start_adr;   /*!< Start address of the current line in VGA_RAM */
  uint32_t dma2_cr_reg; /*!< Pre-calculated value for the DMA2 CR register */
  VGA_Rect clip_rect;   /*!< Clipping rectangle for drawing operations */
}VGA_t;

extern VGA_t VGA;



//--------------------------------------------------------------
/**
 * @brief Framebuffer for the VGA display.
 */
//--------------------------------------------------------------
extern uint8_t VGA_RAM1[(VGA_DISPLAY_X+1)*VGA_DISPLAY_Y];



//--------------------------------------------------------------
// Timer-1 configuration for Pixel Clock
//--------------------------------------------------------------
#define VGA_TIM1_PERIODE      10-1
#define VGA_TIM1_PRESCALE      0



//--------------------------------------------------------------
// Timer-2 configuration for HSync and DMA Trigger
//--------------------------------------------------------------
#define  VGA_TIM2_HSYNC_PERIODE   2002-1
#define  VGA_TIM2_HSYNC_PRESCALE     0

#define  VGA_TIM2_HSYNC_IMP       240
#define  VGA_TIM2_HTRIGGER_START  480
#define  VGA_TIM2_DMA_DELAY       200


//--------------------------------------------------------------
// VSync signal timing constants
//--------------------------------------------------------------
#define  VGA_VSYNC_PERIODE        525
#define  VGA_VSYNC_IMP  2
#define  VGA_VSYNC_BILD_START      36
#define  VGA_VSYNC_BILD_STOP      514
#define RAM_SIZE		(VGA_DISPLAY_X+1)*VGA_DISPLAY_Y


//--------------------------------------------------------------
// GPIO Port E address definitions for DMA transfer
//--------------------------------------------------------------
#define VGA_GPIOE_BASE_ADR     ((uint32_t)0x40021000)
#define VGA_GPIO_ODR_OFFSET    ((uint32_t)0x00000014)
#define VGA_GPIO_BYTE_OFFSET   ((uint32_t)0x00000001)
#define VGA_GPIOE_ODR_ADDRESS   (VGA_GPIOE_BASE_ADR | VGA_GPIO_ODR_OFFSET | VGA_GPIO_BYTE_OFFSET)

//--------------------------------------------------------------
// GPIO pin definitions for clearing the data lines
//--------------------------------------------------------------
#define VGA_GPIO_HINIBBLE  ((uint16_t)0xFF00)


//--------------------------------------------------------------
/**
 * @brief Status enumeration for VGA operations.
 */
//--------------------------------------------------------------
typedef enum {
    VGA_SUCCESS = 0,                /*!< Operation successful */
    VGA_ERROR_INVALID_COORDINATE,   /*!< Coordinate out of bounds */
    VGA_ERROR_INVALID_PARAMETER     /*!< Other invalid parameter (e.g., radius=0) */
} VGA_Status;


//--------------------------------------------------------------
// Global Function call
//--------------------------------------------------------------

/**
 * @brief Initializes the VGA screen controller.
 * @details This function configures all necessary timers, DMA, and GPIO peripherals
 *          to generate the VGA signal. It must be called once at startup.
 */
void UB_VGA_Screen_Init(void);

// Clipping functions
/**
 * @brief Sets the clipping rectangle for all drawing operations.
 * @param rect Pointer to a VGA_Rect structure defining the clipping area.
 */
void UB_VGA_SetClipRect(const VGA_Rect *rect);

/**
 * @brief Retrieves the current clipping rectangle.
 * @param rect Pointer to a VGA_Rect structure to store the current clipping area.
 */
void UB_VGA_GetClipRect(VGA_Rect *rect);

/**
 * @brief Resets the clipping rectangle to the full screen size.
 */
void UB_VGA_ResetClipRect(void);


// Graphics primitives
/**
 * @brief Fills the entire screen with a specified color.
 * @param color 8-bit color value (R3G3B2).
 * @return VGA_Status indicating success or error.
 */
VGA_Status UB_VGA_FillScreen(uint8_t color);

/**
 * @brief Sets a single pixel to a specified color.
 * @param xp X-coordinate of the pixel.
 * @param yp Y-coordinate of the pixel.
 * @param color 8-bit color value (R3G3B2).
 * @return VGA_Status indicating success or error.
 */
VGA_Status UB_VGA_SetPixel(uint16_t xp, uint16_t yp, uint8_t color);

/**
 * @brief Draws a fast horizontal line.
 * @param x0 Starting X-coordinate.
 * @param y Y-coordinate.
 * @param x1 Ending X-coordinate.
 * @param color 8-bit color value (R3G3B2).
 * @return VGA_Status indicating success or error.
 */
VGA_Status UB_VGA_FastHLine(int32_t x0, int32_t y, int32_t x1, uint8_t color);

/**
 * @brief Draws a fast vertical line.
 * @param x X-coordinate.
 * @param y0 Starting Y-coordinate.
 * @param y1 Ending Y-coordinate.
 * @param color 8-bit color value (R3G3B2).
 * @return VGA_Status indicating success or error.
 */
VGA_Status UB_VGA_FastVLine(int32_t x, int32_t y0, int32_t y1, uint8_t color);

// Shape drawing functions
/**
 * @brief Draws a line with a specified thickness.
 * @param x1 Starting X-coordinate.
 * @param y1 Starting Y-coordinate.
 * @param x2 Ending X-coordinate.
 * @param y2 Ending Y-coordinate.
 * @param color 8-bit color value (R3G3B2).
 * @param thickness Line thickness in pixels.
 * @return VGA_Status indicating success or error.
 */
VGA_Status UB_VGA_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, uint8_t thickness);

/**
 * @brief Draws a rectangle outline.
 * @param x X-coordinate of the top-left corner.
 * @param y Y-coordinate of the top-left corner.
 * @param width Width of the rectangle.
 * @param height Height of the rectangle.
 * @param color 8-bit color value (R3G3B2).
 * @return VGA_Status indicating success or error.
 */
VGA_Status UB_VGA_DrawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color);

/**
 * @brief Draws a filled rectangle.
 * @param x X-coordinate of the top-left corner.
 * @param y Y-coordinate of the top-left corner.
 * @param width Width of the rectangle.
 * @param height Height of the rectangle.
 * @param color 8-bit color value (R3G3B2).
 * @return VGA_Status indicating success or error.
 */
VGA_Status UB_VGA_FillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color);

/**
 * @brief Draws a circle outline.
 * @param center_x X-coordinate of the circle's center.
 * @param center_y Y-coordinate of the circle's center.
 * @param radius Radius of the circle.
 * @param color 8-bit color value (R3G3B2).
 * @return VGA_Status indicating success or error.
 */
VGA_Status UB_VGA_DrawCircle(uint16_t center_x, uint16_t center_y, uint16_t radius, uint8_t color);

/**
 * @brief Draws a filled circle.
 * @param center_x X-coordinate of the circle's center.
 * @param center_y Y-coordinate of the circle's center.
 * @param radius Radius of the circle.
 * @param color 8-bit color value (R3G3B2).
 * @return VGA_Status indicating success or error.
 */
VGA_Status UB_VGA_FillCircle(uint16_t center_x, uint16_t center_y, uint16_t radius, uint8_t color);

// Text and bitmap functions
/**
 * @brief Draws a text string.
 * @param x X-coordinate of the top-left corner of the text.
 * @param y Y-coordinate of the top-left corner of the text.
 * @param color 8-bit color value (R3G3B2).
 * @param text Pointer to the null-terminated string to draw.
 * @param font Pointer to the font name (e.g., "Consolas", "Arial").
 * @param size Font size.
 * @param style Text style (e.g., TEXT_STYLE_BOLD).
 * @param bounding_box Optional pointer to a VGA_Rect to store the dimensions of the rendered text.
 * @return VGA_Status indicating success or error.
 */
VGA_Status UB_VGA_DrawText(uint16_t x, uint16_t y, uint8_t color, const char* text, const char* font, uint8_t size, uint8_t style, VGA_Rect* bounding_box);

/**
 * @brief Draws a pre-defined bitmap.
 * @param id ID of the bitmap to draw.
 * @param x_lup X-coordinate of the top-left corner.
 * @param y_lup Y-coordinate of the top-left corner.
 * @return VGA_Status indicating success or error.
 */
VGA_Status UB_VGA_DrawBitmap(uint8_t id, uint16_t x_lup, uint16_t y_lup);


//--------------------------------------------------------------
#endif // __STM32F4_UB_VGA_SCREEN_H
