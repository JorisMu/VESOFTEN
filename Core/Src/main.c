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
	SystemInit(); // System speed to 168MHz
	UB_VGA_Screen_Init(); // Init VGA-Screen
	USART2_Init();

    while (1)
    {
        USART2_BUFFER(); 
    }
}


