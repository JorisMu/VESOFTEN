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

| Color Name      | Macro                | Hex Value | Binary (R3G3B2)   |
| --------------- | -------------------- | --------- | ----------------- |
| Black           | `VGA_COL_BLACK`      | `0x00`    | `000 000 00`      |
| Blue            | `VGA_COL_BLUE`       | `0x03`    | `000 000 11`      |
| Green           | `VGA_COL_GREEN`      | `0x1C`    | `000 111 00`      |
| Red             | `VGA_COL_RED`        | `0xE0`    | `111 000 00`      |
| White           | `VGA_COL_WHITE`      | `0xFF`    | `111 111 11`      |
| Cyan            | `VGA_COL_CYAN`       | `0x1F`    | `000 111 11`      |
| Magenta         | `VGA_COL_MAGENTA`    | `0xE3`    | `111 000 11`      |
| Yellow          | `VGA_COL_YELLOW`     | `0xFC`    | `111 111 00`      |
| Brown           | `VGA_COL_BROWN`      | `0xA0`    | `101 000 00`      |

---

## 4. API Functions

### System Functions

---

#### `void UB_VGA_Screen_Init(void)`

Initializes all necessary hardware for VGA signal generation, including GPIO pins, timers, DMA, and interrupts. It also clears the screen buffer to black. This function must be called once at the start of the application before any other drawing functions.

---

### Drawing Primitives

---

#### `void UB_VGA_FillScreen(uint8_t color)`

Fills the entire screen with a single color.

-   **`color`**: The color to fill the screen with. Use one of the `VGA_COL_*` macros.

---

#### `void UB_VGA_SetPixel(uint16_t xp, uint16_t yp, uint8_t color)`

Draws a single pixel at the specified coordinates.

-   **`xp`**: The x-coordinate (0 to 319).
-   **`yp`**: The y-coordinate (0 to 239).
-   **`color`**: The color of the pixel. Use one of the `VGA_COL_*` macros.

---

#### `void UB_VGA_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, uint8_t thickness)`

Draws a line between two points.

-   **`x1`, `y1`**: The starting coordinates of the line.
-   **`x2`, `y2`**: The ending coordinates of the line.
-   **`color`**: The color of the line.
-   **`thickness`**: The thickness of the line in pixels. A value of `0` or `1` results in a 1-pixel thick line.

---

#### `void UB_VGA_DrawRectangle(uint16_t x_lup, uint16_t y_lup, uint16_t width, uint16_t height, uint8_t color, uint8_t filled)`

Draws a rectangle. Internally, this function converts `width` and `height` to `x2` and `y2` (where `x2 = x_lup + width - 1`, `y2 = y_lup + height - 1`) for drawing.

-   **`x_lup`**: The x-coordinate of the top-left corner (linker_upper_pixel).
-   **`y_lup`**: The y-coordinate of the top-left corner (linker_upper_pixel).
-   **`width`**: The width of the rectangle.
-   **`height`**: The height of the rectangle.
-   **`color`**: The color of the rectangle.
-   **`filled`**: A flag to determine if the rectangle should be filled.
    -   `0`: The rectangle is drawn as an outline.
    -   `1`: The rectangle is filled with the specified color.
    *(Note: The internal implementation does not draw a separate 1px border when `filled` is 1. It simply fills the area.)*

---

#### `void UB_VGA_DrawCircle(uint16_t center_x, uint16_t center_y, uint16_t radius, uint8_t color)`

Draws a circle.

-   **`center_x`, `center_y`**: The coordinates of the center of the circle.
-   **`radius`**: The radius of the circle in pixels.
-   **`color`**: The color of the circle.

---

### Complex Drawing (Placeholders)

The following functions are placeholders and are not yet fully implemented. They require additional data (like font bitmaps or bitmap data) to be fully functional.

---

#### `void UB_VGA_DrawText(uint16_t x, uint16_t y, uint8_t color, const char* text, const char* font, uint8_t size, const char* style)`

*(Placeholder)* Intended to draw text on the screen.

-   **`x`**: The x-coordinate of the starting point for the text.
-   **`y`**: The y-coordinate of the starting point for the text.
-   **`color`**: The color of the text.
-   **`text`**: A pointer to the string of characters to be displayed.
-   **`font`**: A string indicating the desired font (e.g., "consolas", "arial").
-   **`size`**: An integer indicating the font size (e.g., `1`, `2`).
-   **`style`**: A string indicating the font style (e.g., "normaal", "vet", "cursief").

---

#### `void UB_VGA_DrawBitmap(uint8_t id, uint16_t x_lup, uint16_t y_lup)`

*(Placeholder)* Intended to draw a bitmap image on the screen.

-   **`id`**: An identifier for the bitmap to be drawn. Examples include: `0` for arrow up, `1` for arrow right, `2` for arrow down, `3` for arrow left, `4` for angry smiley, `5` for happy smiley. (Specific IDs depend on implementation).
-   **`x_lup`**: The x-coordinate of the top-left corner where the bitmap should be drawn (linker_upper_pixel).
-   **`y_lup`**: The y-coordinate of the top-left corner where the bitmap should be drawn (linker_upper_pixel).

