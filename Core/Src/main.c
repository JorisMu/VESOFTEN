/**
 ******************************************************************************
 * @file    main.c
 * @author  J. Mullink
 * @version V2.1
 * @date    05-Jan-2026
 * @brief   Main program body for the VGA driver demo.
 ******************************************************************************
 */

#include "main.h"
#include "stm32_ub_vga_screen.h"
#include <math.h>

void RunFeatureDemo(void);

/**
  * @brief  The application entry point.
  * @details Initializes the system and the VGA driver, then runs a demo
  *          showcasing the library's features. Enters an infinite loop.
  * @retval int
  */
int main(void)
{
	SystemInit(); // Configure the system clock to 168MHz

	UB_VGA_Screen_Init(); // Initialize the VGA screen driver

	RunFeatureDemo(); // Draw all the test graphics

  while(1)
  {
    // The main loop is empty as the demo is static and drawn only once.
  }
}

/**
  * @brief  Runs a demonstration of the VGA library's drawing capabilities.
  * @details This function clears the screen and draws various primitives,
  *          shapes, and text to test and showcase the driver's features,
  *          including clipping, styled text, and filled shapes.
  * @retval None
  */
void RunFeatureDemo(void)
{
	// 1. Fill screen with a background color
	UB_VGA_FillScreen(VGA_COL_BLUE);

	// 2. Test Filled Shapes
	UB_VGA_FillCircle(40, 40, 30, VGA_COL_YELLOW);
	UB_VGA_FillRectangle(250, 20, 50, 40, VGA_COL_GREEN);

	// 3. Test Thick Lines (new algorithm)
	UB_VGA_DrawLine(10, 230, 110, 100, VGA_COL_LIGHT_CYAN, 1);
	UB_VGA_DrawLine(20, 230, 120, 100, VGA_COL_LIGHT_CYAN, 4);
	UB_VGA_DrawLine(30, 230, 130, 100, VGA_COL_LIGHT_CYAN, 8);
	UB_VGA_DrawLine(40, 230, 140, 100, VGA_COL_CYAN, 12);


	// 4. Test Clipping
	VGA_Rect clip_rect = {80, 60, 160, 120};
	UB_VGA_SetClipRect(&clip_rect);

	// This red line should only be visible inside the clipping rectangle
	UB_VGA_DrawLine(0, 0, 319, 239, VGA_COL_RED, 3);
	// This text should be clipped
	UB_VGA_DrawText(60, 100, VGA_COL_WHITE, "Clipped Text", "default", 2, TEXT_STYLE_NORMAL, NULL);

	// Reset clipping to draw anywhere on screen again
	UB_VGA_ResetClipRect();
	// Draw a white border to show where the clipping rect was
	UB_VGA_DrawRectangle(clip_rect.x, clip_rect.y, clip_rect.width, clip_rect.height, VGA_COL_WHITE);


	// 5. Test Text Styles & Bounding Box feature
	VGA_Rect bbox; // Struct to hold the bounding box

	// Normal text
	UB_VGA_DrawText(120, 10, VGA_COL_WHITE, "Normal Text", "default", 1, TEXT_STYLE_NORMAL, NULL);

	// Bold text
	UB_VGA_DrawText(10, 80, VGA_COL_YELLOW, "Bold!", "default", 2, TEXT_STYLE_BOLD, NULL);

	// Italic text (using 'arial' font)
	UB_VGA_DrawText(200, 80, VGA_COL_LIGHT_GREEN, "Italic!", "arial", 2, TEXT_STYLE_ITALIC, NULL);

	// Bold, Italic, and Bounding Box
	UB_VGA_DrawText(140, 200, VGA_COL_LIGHT_MAGENTA, "Bounds Test", "arial", 2, TEXT_STYLE_BOLD | TEXT_STYLE_ITALIC, &bbox);

	// Draw a rectangle around the text we just drew using the returned bounding box
	UB_VGA_DrawRectangle(bbox.x - 2, bbox.y - 2, bbox.width + 4, bbox.height + 4, VGA_COL_GREY);
}
