//--------------------------------------------------------------
// File     : bitmaps.h
//--------------------------------------------------------------

#ifndef __BITMAPS_H
#define __BITMAPS_H

#include "stm32f4xx.h"

//--------------------------------------------------------------
// Bitmap Definitions
//--------------------------------------------------------------
#define BITMAP_TRANSPARENT_COLOR 0xFE

typedef struct {
    uint16_t width;
    uint16_t height;
    const uint8_t (*data)[16]; // Pointer to an array of 16 uint8_t
} Bitmap_t;

// Enum for bitmap IDs
typedef enum {
    BITMAP_ARROW_UP = 0,
    BITMAP_ARROW_RIGHT,
    BITMAP_ARROW_DOWN,
    BITMAP_ARROW_LEFT,
    BITMAP_SMILEY_ANGRY,
    BITMAP_SMILEY_HAPPY,
    // Add more as needed
    NUM_BITMAPS
} BITMAP_ID;

// Extern declaration of the bitmap array
// The actual data is in bitmaps.c
extern const Bitmap_t vga_bitmaps[];

#endif // __BITMAPS_H
