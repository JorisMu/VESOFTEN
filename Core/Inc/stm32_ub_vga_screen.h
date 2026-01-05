/**
 ******************************************************************************
 * @file    stm32_ub_vga_screen.h
 * @author  J. Mullink
 * @version V1.0
 * @date    05-Jan-2026
 * @brief   Header file for stm32_ub_vga_screen.c, a library for generating
 *          a 320x240 VGA signal with 8-bit color on an STM32F4xx MCU.
 *
 * @note    This library uses TIM1, TIM2, and DMA2 for signal generation.
 *          HSync is on PB11, VSync on PB12.
 *          Color data is output on PE8-PE15 (R3G3B2 format).
 ******************************************************************************
 */

#ifndef __STM32F4_UB_VGA_SCREEN_H
#define __STM32F4_UB_VGA_SCREEN_H

//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32f4xx.h"
#include <stdint.h>
#include <stdbool.h>

/** @defgroup VGA_Driver VGA Display Driver
  * @brief A library to drive a VGA monitor at 320x240 resolution.
  * @{
  */

/** @defgroup VGA_Colors Standard 8-bit (R3G3B2) Color Palette
  * @{
  */
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
/**
  * @}
  */

/** @defgroup VGA_Screen_Dimensions Screen Dimensions
  * @{
  */
#define VGA_DISPLAY_X   320 /**< @brief Horizontal resolution of the display in pixels. */
#define VGA_DISPLAY_Y   240 /**< @brief Vertical resolution of the display in pixels. */
/**
  * @}
  */

/** @defgroup VGA_Typedefs Library Typedefs
  * @{
  */

/**
 * @brief Structure to define a rectangular area.
 */
typedef struct {
    int32_t x;      /**< @brief The x-coordinate of the top-left corner. */
    int32_t y;      /**< @brief The y-coordinate of the top-left corner. */
    int32_t width;  /**< @brief The width of the rectangle. */
    int32_t height; /**< @brief The height of the rectangle. */
} VGA_Rect;

/**
 * @brief Enum representing the status of a drawing operation.
 */
typedef enum {
    VGA_SUCCESS = 0,                /**< @brief Operation completed successfully. */
    VGA_ERROR_INVALID_COORDINATE,   /**< @brief A specified coordinate was outside the screen boundaries. */
    VGA_ERROR_INVALID_PARAMETER     /**< @brief An invalid parameter was passed (e.g., radius=0, width=0). */
} VGA_Status;

/**
 * @brief Internal structure holding VGA signal generation state.
 * @note  For internal driver use only.
 */
typedef struct {
  uint16_t hsync_cnt;   /**< @brief Counter for HSync lines. */
  uint32_t start_adr;   /**< @brief Start address in RAM for the current DMA transfer. */
  uint32_t dma2_cr_reg; /**< @brief Cached value of the DMA2 Stream 5 CR register. */
  VGA_Rect clip_rect;   /**< @brief Current clipping rectangle. */
} VGA_t;

/**
  * @}
  */

/** @defgroup VGA_Text_Styles Text Style Flags
  * @{
  */
#define TEXT_STYLE_NORMAL   0x00 /**< @brief Normal text style. */
#define TEXT_STYLE_BOLD     0x01 /**< @brief Bold text style. */
#define TEXT_STYLE_ITALIC   0x02 /**< @brief Italic text style. */
/**
  * @}
  */


/** @defgroup VGA_Hardware_Defs Hardware-specific Definitions
  * @brief These values configure the timers and DMA for signal generation.
  * @{
  */
#define VGA_TIM1_PERIODE         10-1
#define VGA_TIM1_PRESCALE        0

#define VGA_TIM2_HSYNC_PERIODE   2002-1
#define VGA_TIM2_HSYNC_PRESCALE  0
#define VGA_TIM2_HSYNC_IMP       240
#define VGA_TIM2_HTRIGGER_START  480
#define VGA_TIM2_DMA_DELAY       200

#define VGA_VSYNC_PERIODE        525
#define VGA_VSYNC_IMP            2
#define VGA_VSYNC_BILD_START     36
#define VGA_VSYNC_BILD_STOP      514

#define VGA_RAM_SIZE		     (VGA_DISPLAY_X+1)*VGA_DISPLAY_Y

#define VGA_GPIOE_BASE_ADR       ((uint32_t)0x40021000)
#define VGA_GPIO_ODR_OFFSET      ((uint32_t)0x00000014)
#define VGA_GPIO_BYTE_OFFSET     ((uint32_t)0x00000001)
#define VGA_GPIOE_ODR_ADDRESS    (VGA_GPIOE_BASE_ADR | VGA_GPIO_ODR_OFFSET | VGA_GPIO_BYTE_OFFSET)
#define VGA_GPIO_HINIBBLE        ((uint16_t)0xFF00)
/**
  * @}
  */


/** @defgroup VGA_Public_Variables Public Variables
  * @{
  */
extern VGA_t VGA;   /**< @brief Global instance of the VGA state struct. */
extern uint8_t VGA_RAM1[VGA_RAM_SIZE]; /**< @brief The framebuffer memory. */
/**
  * @}
  */


/** @defgroup VGA_Public_Functions Public API Functions
  * @{
  */

//--------------------------------------------------------------
// System and Screen Initialization
//--------------------------------------------------------------
void UB_VGA_Screen_Init(void);

//--------------------------------------------------------------
// Clipping Area Functions
//--------------------------------------------------------------
void UB_VGA_SetClipRect(const VGA_Rect *rect);
void UB_VGA_GetClipRect(VGA_Rect *rect);
void UB_VGA_ResetClipRect(void);

//--------------------------------------------------------------
// Graphics Primitives
//--------------------------------------------------------------
VGA_Status UB_VGA_FillScreen(uint8_t color);
VGA_Status UB_VGA_SetPixel(uint16_t xp, uint16_t yp, uint8_t color);
VGA_Status UB_VGA_FastHLine(int32_t x0, int32_t y, int32_t x1, uint8_t color);
VGA_Status UB_VGA_FastVLine(int32_t x, int32_t y0, int32_t y1, uint8_t color);

//--------------------------------------------------------------
// Shape Drawing Functions
//--------------------------------------------------------------
VGA_Status UB_VGA_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, uint8_t thickness);
VGA_Status UB_VGA_DrawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color);
VGA_Status UB_VGA_FillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color);
VGA_Status UB_VGA_DrawCircle(uint16_t center_x, uint16_t center_y, uint16_t radius, uint8_t color);
VGA_Status UB_VGA_FillCircle(uint16_t center_x, uint16_t center_y, uint16_t radius, uint8_t color);

//--------------------------------------------------------------
// Text and Bitmap Functions
//--------------------------------------------------------------
VGA_Status UB_VGA_DrawText(uint16_t x, uint16_t y, uint8_t color, const char* text, const char* font, uint8_t size, uint8_t style, VGA_Rect* bounding_box);
VGA_Status UB_VGA_DrawBitmap(uint8_t id, uint16_t x_lup, uint16_t y_lup);

/**
  * @}
  */

/**
  * @}
  */

#endif // __STM32F4_UB_VGA_SCREEN_H
