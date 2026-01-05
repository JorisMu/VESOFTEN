/**
 ******************************************************************************
 * @file    bitmaps.h
 * @author  J. Mullink
 * @version V1.0
 * @date    05-Jan-2026
 * @brief   Header file for bitmap definitions used with the VGA driver.
 ******************************************************************************
 */

#ifndef __BITMAPS_H
#define __BITMAPS_H

#include <stdint.h>

/** @defgroup VGA_Bitmaps Bitmap Definitions
  * @brief Data and structures for bitmap rendering.
  * @ingroup VGA_Driver
  * @{
  */

/**
 * @brief A special color value used in bitmap data to indicate a transparent pixel.
 * @details Pixels with this color value will not be drawn, allowing the background to show through.
 */
#define BITMAP_TRANSPARENT_COLOR 0xFE

/**
 * @brief Structure to define a bitmap image.
 */
typedef struct {
    const uint16_t width;  /**< @brief The width of the bitmap in pixels. */
    const uint16_t height; /**< @brief The height of the bitmap in pixels. */
    const uint8_t (*data)[16]; /**< @brief Pointer to the bitmap's 2D data array. */
} Bitmap_t;

/**
 * @brief Enumeration of available bitmap IDs.
 * @details Use these IDs with the UB_VGA_DrawBitmap() function.
 */
typedef enum {
    BITMAP_ARROW_UP = 0,    /**< @brief ID for the 'up arrow' bitmap. */
    BITMAP_ARROW_RIGHT,     /**< @brief ID for the 'right arrow' bitmap. */
    BITMAP_ARROW_DOWN,      /**< @brief ID for the 'down arrow' bitmap. */
    BITMAP_ARROW_LEFT,      /**< @brief ID for the 'left arrow' bitmap. */
    BITMAP_SMILEY_ANGRY,    /**< @brief ID for the 'angry smiley' bitmap. */
    BITMAP_SMILEY_HAPPY,    /**< @brief ID for the 'happy smiley' bitmap. */
    NUM_BITMAPS             /**< @brief The total number of defined bitmaps. Used for array sizing. */
} BITMAP_ID;

/**
 * @brief Extern declaration for the global array of bitmaps.
 * @details The actual bitmap data is defined in `bitmaps.c`.
 */
extern const Bitmap_t vga_bitmaps[];

/**
  * @}
  */

#endif // __BITMAPS_H
