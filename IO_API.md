# IO Layer API Reference

This document provides the API reference for the VGA IO layer, which is responsible for all low-level drawing operations and screen interactions.

---

## 1. Overview

The IO layer provides a set of functions to initialize the VGA display and draw pixels, shapes, and other graphical elements onto it. It directly interfaces with the hardware (GPIO, DMA, Timers) to generate the VGA signal.

---

## 2. Configuration

### Screen Dimensions

The display resolution is fixed and defined by the following constants:

-   `VGA_DISPLAY_X`: 320 pixels (width)
-   `VGA_DISPLAY_Y`: 240 pixels (height)

---

## 3. Color Palette

The color format is 8-bit R3G3B2 (3 bits for Red, 3 bits for Green, 2 bits for Blue). The following color constants are available:

| Color Name    | Macro                  | Hex Value | Binary (R3 G3 B2) |
| ------------- | --------------------- | --------- | -----------------       |
| Black         | `VGA_COL_BLACK`         | `0x00`      | `000 000 00`        |
| Blue          | `VGA_COL_BLUE         ` | `0x03`      | `000 000 11`        |
| Light Blue    | `VGA_COL_LIGHT_BLUE   ` | `0x5F`      | `010 111 11`        |
| Green         | `VGA_COL_GREEN        ` | `0x1C`      | `000 111 00`        |
| Light Green   | `VGA_COL_LIGHT_GREEN  ` | `0x9E`      | `100 111 10`        |
| Cyan          | `VGA_COL_CYAN         ` | `0x1F`      | `000 111 11`        |
| Light Cyan    | `VGA_COL_LIGHT_CYAN   ` | `0xDF`      | `110 111 11`        |
| Red           | `VGA_COL_RED          ` | `0xE0`      | `111 000 00`        |
| Light Red     | `VGA_COL_LIGHT_RED    ` | `0xF2`      | `111 100 10`        |
| Magenta       | `VGA_COL_MAGENTA      ` | `0xE3`      | `111 000 11`        |
| Light Magenta | `VGA_COL_LIGHT_MAGENTA` | `0xF7`      | `111 101 11`        |
| Brown         | `VGA_COL_BROWN        ` | `0x88`      | `100 010 00`        |
| Yellow        | `VGA_COL_YELLOW       ` | `0xFC`      | `111 111 00`        |
| Grey          | `VGA_COL_GREY         ` | `0x92`      | `100 100 10`        |
| White         | `VGA_COL_WHITE        ` | `0xFF`      | `111 111 11`        |

---

## 4. API Functions

### System Functions

---

#### `void UB_VGA_Screen_Init(void)`

Initializes all necessary hardware for VGA signal generation, including GPIO pins, timers, DMA, and interrupts. It also clears the screen buffer to black. This function must be called once at the start of the application before any other drawing functions.

---

### Return Values

All drawing functions return a `VGA_Status` enumeration to indicate the outcome of the operation.

```c
typedef enum {
    VGA_SUCCESS = 0,                // Operation successful
    VGA_ERROR_INVALID_COORDINATE,   // Coordinate out of bounds
    VGA_ERROR_INVALID_PARAMETER     // Other invalid parameter (e.g., radius=0)
} VGA_Status;
```

---

### Drawing Primitives

---

#### `VGA_Status UB_VGA_FillScreen(uint8_t color)`

Fills the entire screen with a single color.

-   **`color`**: The color to fill the screen with. Use one of the `VGA_COL_*` macros.
-   **Returns**: `VGA_SUCCESS`

---

#### `VGA_Status UB_VGA_SetPixel(uint16_t xp, uint16_t yp, uint8_t color)`

Draws a single pixel at the specified coordinates.

-   **`xp`**: The x-coordinate (0 to 319).
-   **`yp`**: The y-coordinate (0 to 239).
-   **`color`**: The color of the pixel. Use one of the `VGA_COL_*` macros.
-   **Returns**: 
    - `VGA_SUCCESS` on success.
    - `VGA_ERROR_INVALID_COORDINATE` if `xp` or `yp` are out of bounds.

---

#### `VGA_Status UB_VGA_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, uint8_t thickness)`

Draws a line between two points.

-   **`x1`, `y1`**: The starting coordinates of the line.
-   **`x2`, `y2`**: The ending coordinates of the line.
-   **`color`**: The color of the line.
-   **`thickness`**: The thickness of the line in pixels. A value of `0` or `1` results in a 1-pixel thick line.
-   **Returns**: 
    - `VGA_SUCCESS` on success.
    - `VGA_ERROR_INVALID_PARAMETER` if thickness is 0.

---

#### `VGA_Status UB_VGA_DrawRectangle(uint16_t x_lup, uint16_t y_lup, uint16_t width, uint16_t height, uint8_t color, uint8_t filled)`

Draws a rectangle.

-   **`x_lup`**: The x-coordinate of the top-left corner.
-   **`y_lup`**: The y-coordinate of the top-left corner.
-   **`width`**: The width of the rectangle.
-   **`height`**: The height of the rectangle.
-   **`color`**: The color of the rectangle.
-   **`filled`**: A flag to determine if the rectangle should be filled (`0` for outline, `1` for filled).
-   **Returns**: 
    - `VGA_SUCCESS` on success.
    - `VGA_ERROR_INVALID_COORDINATE` if the rectangle exceeds screen boundaries.
    - `VGA_ERROR_INVALID_PARAMETER` if width or height are 0.

---

#### `VGA_Status UB_VGA_DrawCircle(uint16_t center_x, uint16_t center_y, uint16_t radius, uint8_t color)`

Draws a circle.

-   **`center_x`, `center_y`**: The coordinates of the center of the circle.
-   **`radius`**: The radius of the circle in pixels.
-   **`color`**: The color of the circle.
-   **Returns**: 
    - `VGA_SUCCESS` on success.
    - `VGA_ERROR_INVALID_PARAMETER` if radius is 0.

---

### Complex Drawing

---

#### `VGA_Status UB_VGA_DrawText(uint16_t x, uint16_t y, uint8_t color, const char* text, const char* font, uint8_t size, const char* style)`

Draws text on the screen.

-   **`x`**, **`y`**: The starting coordinates of the text.
-   **`color`**: The color of the text.
-   **`text`**: The string to be displayed.
-   **`font`**: The desired font (e.g., "consolas", "arial").
-   **`size`**: A multiplier for the font size.
-   **`style`**: The font style (e.g., "vet" for bold, "cursief" for italic).
-   **Returns**: 
    - `VGA_SUCCESS` on success.
    - `VGA_ERROR_INVALID_PARAMETER` if the font or style is not found.

---

#### `VGA_Status UB_VGA_DrawBitmap(uint8_t id, uint16_t x_lup, uint16_t y_lup)`

Draws a pre-defined bitmap image.

-   **`id`**: The identifier for the bitmap.
-   **`x_lup`**, **`y_lup`**: The top-left coordinates for the bitmap.
-   **Returns**: 
    - `VGA_SUCCESS` on success.
    - `VGA_ERROR_INVALID_PARAMETER` if the bitmap ID is invalid.
    - `VGA_ERROR_INVALID_COORDINATE` if the bitmap exceeds screen boundaries.