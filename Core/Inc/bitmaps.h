/**
 * @file    bitmaps.h
 * @brief   Declarations for bitmap graphics.
 * @details This file defines the data structures, identifiers (IDs), and
 *          external data array for bitmap images used in the application.
 *          The actual bitmap data is defined in `bitmaps.c`.
 *
 * @date    05.01.2026
 * @author  J. Mullink
 */

#ifndef __BITMAPS_H
#define __BITMAPS_H

#include "stm32f4xx.h"

//--------------------------------------------------------------
// Bitmap Definitions
//--------------------------------------------------------------

/**
 * @brief A special color value used to represent a transparent pixel in a bitmap.
 * @details When drawing a bitmap, pixels with this color value will not be
 *          drawn to the screen, allowing the background to show through.
 */
#define BITMAP_TRANSPARENT_COLOR 0xFE

/**
 * @brief Structure to define a bitmap image.
 */
typedef struct {
    const uint16_t width;   /*!< Width of the bitmap in pixels. */
    const uint16_t height;  /*!< Height of the bitmap in pixels. */
    const uint8_t (*data)[16]; /*!< Pointer to the bitmap's 2D pixel data array. */
} Bitmap_t;

/**
 * @brief Enumeration of available bitmap IDs.
 * @details Use these IDs with the `UB_VGA_DrawBitmap` function to draw the
 *          corresponding image.
 */
typedef enum {
    BITMAP_ARROW_UP = 0,    /*!< ID for the 'up arrow' bitmap. */
    BITMAP_ARROW_RIGHT,     /*!< ID for the 'right arrow' bitmap. */
    BITMAP_ARROW_DOWN,      /*!< ID for the 'down arrow' bitmap. */
    BITMAP_ARROW_LEFT,      /*!< ID for the 'left arrow' bitmap. */
    BITMAP_SMILEY_ANGRY,    /*!< ID for the 'angry smiley' bitmap. */
    BITMAP_SMILEY_HAPPY,    /*!< ID for the 'happy smiley' bitmap. */
    // Add more bitmap IDs here as needed
    NUM_BITMAPS             /*!< The total number of available bitmaps. Must be the last entry. */
} BITMAP_ID;

/**
 * @brief Extern declaration for the global array of bitmaps.
 * @details This array holds all the `Bitmap_t` structures. The actual
 *          definition and initialization of this array is in `bitmaps.c`.
 */
extern const Bitmap_t vga_bitmaps[];

#endif // __BITMAPS_H
