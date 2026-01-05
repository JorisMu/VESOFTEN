/**
 ******************************************************************************
 * @file    stm32_ub_vga_screen.c
 * @author  J. Mullink
 * @version V1.0
 * @date    05-Jan-2026
 * @brief   A library for generating a 320x240 VGA signal with 8-bit color.
 *
 * @note    This library uses TIM1, TIM2, and DMA2 for signal generation.
 *          HSync is on PB11, VSync on PB12.
 *          Color data is output on PE8-PE15 (R3G3B2 format).
 ******************************************************************************
 */


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_vga_screen.h"
#include "bitmaps.h"
#include "fonts.h"
#include <stdlib.h>
#include <string.h>


//--------------------------------------------------------------
// Internal-only helper macros
//--------------------------------------------------------------
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))


//--------------------------------------------------------------
// Global variables
//--------------------------------------------------------------
VGA_t VGA;
uint8_t VGA_RAM1[VGA_RAM_SIZE];


//--------------------------------------------------------------
// Private function prototypes
//--------------------------------------------------------------
static void P_VGA_InitIO(void);
static void P_VGA_InitTIM(void);
static void P_VGA_InitINT(void);
static void P_VGA_InitDMA(void);
static VGA_Status P_VGA_DrawSinglePixelLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color);


/**
  * @brief  Initializes the VGA screen driver.
  * @details This function configures all necessary GPIOs, Timers, DMA, and interrupts
  *         to generate the VGA signal. It also clears the framebuffer.
  * @param  None
  * @retval None
  */
void UB_VGA_Screen_Init(void)
{
  uint32_t anfang = (uint32_t)(&VGA_RAM1[0]);

  VGA.hsync_cnt=0;
  VGA.start_adr=anfang;
  VGA.dma2_cr_reg=0;

  // Set the default clipping rectangle to the full screen size.
  UB_VGA_ResetClipRect();

  // Clear the framebuffer to black.
  memset(VGA_RAM1, 0x00, VGA_RAM_SIZE);

  // Initialize hardware peripherals.
  P_VGA_InitIO();
  P_VGA_InitTIM();
  P_VGA_InitDMA();
  P_VGA_InitINT();

  // Cache the initial value of the DMA_SxCR register for quick re-enabling.
  VGA.dma2_cr_reg=DMA2_Stream5->CR;
}

/**
  * @brief  Sets the clipping rectangle for all drawing operations.
  * @param  rect Pointer to a VGA_Rect structure defining the clipping area.
  *              If NULL, the clipping area is reset to the full screen.
  * @retval None
  */
void UB_VGA_SetClipRect(const VGA_Rect *rect)
{
    if (rect == NULL) {
        UB_VGA_ResetClipRect();
        return;
    }
    // Ensure the clipping rectangle is within the screen boundaries.
    VGA.clip_rect.x = max(0, rect->x);
    VGA.clip_rect.y = max(0, rect->y);
    VGA.clip_rect.width = min(VGA_DISPLAY_X - VGA.clip_rect.x, rect->width);
    VGA.clip_rect.height = min(VGA_DISPLAY_Y - VGA.clip_rect.y, rect->height);
}

/**
  * @brief  Retrieves the current clipping rectangle.
  * @param  rect Pointer to a VGA_Rect structure to store the current clipping area.
  * @retval None
  */
void UB_VGA_GetClipRect(VGA_Rect *rect)
{
    if (rect == NULL) return;
    *rect = VGA.clip_rect;
}

/**
  * @brief  Resets the clipping rectangle to the entire screen.
  * @param  None
  * @retval None
  */
void UB_VGA_ResetClipRect(void)
{
    VGA.clip_rect.x = 0;
    VGA.clip_rect.y = 0;
    VGA.clip_rect.width = VGA_DISPLAY_X;
    VGA.clip_rect.height = VGA_DISPLAY_Y;
}

/**
  * @brief  Fills the entire screen with a single color.
  * @param  color The 8-bit color to fill the screen with.
  * @retval VGA_Status status of the operation.
  */
VGA_Status UB_VGA_FillScreen(uint8_t color)
{
  // This function is fast, so we bypass per-pixel clipping and fill the buffer directly.
  memset(VGA_RAM1, color, VGA_RAM_SIZE);

  // The hardware requires the pixel after the last visible pixel of each line to be 0.
  for(uint16_t yp=0; yp<VGA_DISPLAY_Y; yp++) {
      VGA_RAM1[(yp*(VGA_DISPLAY_X+1))+VGA_DISPLAY_X] = 0;
  }
  return VGA_SUCCESS;
}

/**
  * @brief  Draws a single pixel at a specified coordinate.
  * @param  xp The x-coordinate of the pixel.
  * @param  yp The y-coordinate of the pixel.
  * @param  color The 8-bit color of the pixel.
  * @retval VGA_Status status of the operation.
  */
VGA_Status UB_VGA_SetPixel(uint16_t xp, uint16_t yp, uint8_t color)
{
  // Check against screen boundaries
  if(xp >= VGA_DISPLAY_X || yp >= VGA_DISPLAY_Y) return VGA_ERROR_INVALID_COORDINATE;

  // Check against the current clipping rectangle
  if (xp < VGA.clip_rect.x || yp < VGA.clip_rect.y ||
      xp >= VGA.clip_rect.x + VGA.clip_rect.width ||
      yp >= VGA.clip_rect.y + VGA.clip_rect.height) {
      return VGA_SUCCESS; // Clipped pixels are not an error.
  }

  // Write the pixel color to the framebuffer.
  VGA_RAM1[(yp*(VGA_DISPLAY_X+1))+xp]=color;

  return VGA_SUCCESS;
}

/**
  * @brief  Draws a fast horizontal line.
  * @param  x0 The starting x-coordinate.
  * @param  y  The y-coordinate.
  * @param  x1 The ending x-coordinate.
  * @param  color The 8-bit color of the line.
  * @retval VGA_Status status of the operation.
  */
VGA_Status UB_VGA_FastHLine(int32_t x0, int32_t y, int32_t x1, uint8_t color)
{
	// Clip the line vertically.
    if (y < VGA.clip_rect.y || y >= (VGA.clip_rect.y + VGA.clip_rect.height)) return VGA_SUCCESS;

    // Determine the visible portion of the line and clip horizontally.
    int32_t start_x = max(min(x0, x1), VGA.clip_rect.x);
    int32_t end_x = min(max(x0, x1), VGA.clip_rect.x + VGA.clip_rect.width - 1);

    if (start_x > end_x) return VGA_SUCCESS; // Nothing to draw.

    // Use memset for fast drawing of the horizontal line segment.
    uint32_t base_addr = y * (VGA_DISPLAY_X + 1);
    memset(&VGA_RAM1[base_addr + start_x], color, end_x - start_x + 1);

    return VGA_SUCCESS;
}

/**
  * @brief  Draws a fast vertical line.
  * @param  x  The x-coordinate.
  * @param  y0 The starting y-coordinate.
  * @param  y1 The ending y-coordinate.
  * @param  color The 8-bit color of the line.
  * @retval VGA_Status status of the operation.
  */
VGA_Status UB_VGA_FastVLine(int32_t x, int32_t y0, int32_t y1, uint8_t color)
{
	// Clip the line horizontally.
    if (x < VGA.clip_rect.x || x >= (VGA.clip_rect.x + VGA.clip_rect.width)) return VGA_SUCCESS;

    // Determine the visible portion of the line and clip vertically.
    int32_t start_y = max(min(y0, y1), VGA.clip_rect.y);
    int32_t end_y = min(max(y0, y1), VGA.clip_rect.y + VGA.clip_rect.height - 1);
    
    if (start_y > end_y) return VGA_SUCCESS; // Nothing to draw.

    // Draw the line pixel by pixel.
    uint32_t line_addr_start = start_y * (VGA_DISPLAY_X + 1) + x;
    for (int32_t i = 0; i <= (end_y - start_y); i++) {
        VGA_RAM1[line_addr_start + i * (VGA_DISPLAY_X + 1)] = color;
    }
    return VGA_SUCCESS;
}

/**
  * @brief  Initializes all GPIO pins required for VGA signal generation.
  * @note   Internal function.
  * @param  None
  * @retval None
  */
static void P_VGA_InitIO(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;

  // Enable clocks for GPIOE (color signals) and GPIOB (sync signals).
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  // Configure PE8-PE15 as 100MHz push-pull outputs for color data.
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 |
        GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  GPIOE->BSRRH = VGA_GPIO_HINIBBLE; // Set color lines low initially.

  // Configure PB11 (HSync) for alternate function mode with TIM2_CH4.
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_TIM2);

  // Configure PB12 (VSync) as a standard push-pull output.
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIOB->BSRRL = GPIO_Pin_12; // Set VSync high initially.
}


/**
  * @brief  Initializes TIM1 and TIM2 for pixel clock and sync signal generation.
  * @note   Internal function.
  * @param  None
  * @retval None
  */
static void P_VGA_InitTIM(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;

  // Enable timer clocks.
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  // Configure TIM1 to generate the DMA trigger for each pixel (pixel clock).
  TIM_TimeBaseStructure.TIM_Period =  VGA_TIM1_PERIODE;
  TIM_TimeBaseStructure.TIM_Prescaler = VGA_TIM1_PRESCALE;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
  TIM_ARRPreloadConfig(TIM1, ENABLE);

  // Configure TIM2 to generate HSync and the DMA start trigger.
  TIM_TimeBaseStructure.TIM_Period = VGA_TIM2_HSYNC_PERIODE;
  TIM_TimeBaseStructure.TIM_Prescaler = VGA_TIM2_HSYNC_PRESCALE;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  // Configure TIM2_CH3 to trigger the start of DMA transfer after the back porch.
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = VGA_TIM2_HTRIGGER_START-VGA_TIM2_DMA_DELAY;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OC3Init(TIM2, &TIM_OCInitStructure);
  TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);

  // Configure TIM2_CH4 to generate the HSync pulse on PB11.
  TIM_OCInitStructure.TIM_Pulse = VGA_TIM2_HSYNC_IMP;
  TIM_OC4Init(TIM2, &TIM_OCInitStructure);
  TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);

  // Enable TIM2.
  TIM_ARRPreloadConfig(TIM2, ENABLE);
  TIM_Cmd(TIM2, ENABLE);
}

/**
  * @brief  Initializes all interrupts required for VGA signal generation.
  * @note   Internal function.
  * @param  None
  * @retval None
  */
static void P_VGA_InitINT(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  // Configure DMA2_Stream5 transfer complete interrupt.
  DMA_ITConfig(DMA2_Stream5, DMA_IT_TC, ENABLE);
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  // Configure TIM2_CH3 interrupt to count HSync lines and manage VSync.
  TIM_ITConfig(TIM2,TIM_IT_CC3,ENABLE);
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_Init(&NVIC_InitStructure);
}


/**
  * @brief  Initializes DMA2_Stream5 to transfer framebuffer data to GPIOE.
  * @note   Internal function.
  * @param  None
  * @retval None
  */
static void P_VGA_InitDMA(void)
{
  DMA_InitTypeDef DMA_InitStructure;

  // Enable DMA2 clock.
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

  // Configure DMA2_Stream5, Channel 6 for memory-to-peripheral transfer.
  DMA_Cmd(DMA2_Stream5, DISABLE);
  DMA_DeInit(DMA2_Stream5);
  DMA_InitStructure.DMA_Channel = DMA_Channel_6;
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)VGA_GPIOE_ODR_ADDRESS;
  DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)&VGA_RAM1;
  DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
  DMA_InitStructure.DMA_BufferSize = VGA_DISPLAY_X+1;
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
  DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
  DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
  DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
  DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
  DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
  DMA_Init(DMA2_Stream5, &DMA_InitStructure);

  // Link TIM1 update event to the DMA request.
  TIM_DMACmd(TIM1,TIM_DMA_Update,ENABLE);
}


/**
  * @brief  TIM2 Interrupt Handler.
  * @details This IRQ is triggered on TIM2_CH3 compare match (start of horizontal line).
  *         It counts HSync lines to generate the VSync signal and initiates
  *         the DMA transfer for the visible portion of the screen.
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void)
{
  // Clear the interrupt pending bit for TIM2_CH3.
  TIM_ClearITPendingBit(TIM2, TIM_IT_CC3);

  VGA.hsync_cnt++;
  if(VGA.hsync_cnt >= VGA_VSYNC_PERIODE) {
    VGA.hsync_cnt=0;
    // Reset framebuffer pointer to the start of the screen.
    VGA.start_adr=(uint32_t)(&VGA_RAM1[0]);
  }

  // Generate VSync pulse.
  if(VGA.hsync_cnt < VGA_VSYNC_IMP) {
    GPIOB->BSRRH = GPIO_Pin_12; // VSync low
  }
  else {
    GPIOB->BSRRL = GPIO_Pin_12; // VSync high
  }

  // Check if the current line is within the visible area.
  if((VGA.hsync_cnt >= VGA_VSYNC_BILD_START) && (VGA.hsync_cnt <= VGA_VSYNC_BILD_STOP)) {
    // Re-enable DMA stream with cached register values.
	DMA2_Stream5->CR=VGA.dma2_cr_reg;
    // Set the memory source address for the DMA transfer.
    DMA2_Stream5->M0AR=VGA.start_adr;
    // Start TIM1 to generate pixel clock DMA requests.
    TIM1->CR1|=TIM_CR1_CEN;
    // Enable the DMA stream to start the transfer.
    DMA2_Stream5->CR|=DMA_SxCR_EN;

    // Increment framebuffer address for the next line, but only on every other HSync
    // because of interlacing-like behavior.
    if((VGA.hsync_cnt & 0x01)!=0) {
      VGA.start_adr += (VGA_DISPLAY_X+1);
    }
  }
}

/**
  * @brief  DMA2_Stream5 Interrupt Handler.
  * @details This IRQ is triggered when a line of pixels has been completely
  *         transferred to the GPIO port. It stops the pixel clock (TIM1)
  *         and DMA to wait for the next line trigger from TIM2.
  * @param  None
  * @retval None
  */
void DMA2_Stream5_IRQHandler(void)
{
  if(DMA_GetITStatus(DMA2_Stream5, DMA_IT_TCIF5))
  {
    // Clear the transfer complete interrupt flag.
    DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TCIF5);

    // Stop TIM1 and disable the DMA stream.
    TIM1->CR1&=~TIM_CR1_CEN;
    DMA2_Stream5->CR=0;
    // Set color output to black during the horizontal blanking interval.
    GPIOE->BSRRH = VGA_GPIO_HINIBBLE;
  }
}

/**
  * @brief  Draws a bitmap image on the screen.
  * @param  id The ID of the bitmap to draw (from bitmaps.h).
  * @param  x_lup The x-coordinate of the top-left corner.
  * @param  y_lup The y-coordinate of the top-left corner.
  * @retval VGA_Status status of the operation.
  */
VGA_Status UB_VGA_DrawBitmap(uint8_t id, uint16_t x_lup, uint16_t y_lup) {
    if (id >= NUM_BITMAPS) {
        return VGA_ERROR_INVALID_PARAMETER;
    }

    const Bitmap_t *bitmap = &vga_bitmaps[id];

    // Clipping is handled automatically by UB_VGA_SetPixel.
    for (uint16_t y = 0; y < bitmap->height; y++) {
        for (uint16_t x = 0; x < bitmap->width; x++) {
            uint8_t color = bitmap->data[y][x];
            if (color != BITMAP_TRANSPARENT_COLOR) { // Skip transparent pixels
                UB_VGA_SetPixel(x_lup + x, y_lup + y, color);
            }
        }
    }
    return VGA_SUCCESS;
}

/**
  * @brief  Draws a single-pixel-thick line using Bresenham's algorithm.
  * @note   Internal function. Coordinates are not clipped here.
  * @param  x1 Starting x-coordinate.
  * @param  y1 Starting y-coordinate.
  * @param  x2 Ending x-coordinate.
  * @param  y2 Ending y-coordinate.
  * @param  color 8-bit color of the line.
  * @retval VGA_Status status of the operation.
  */
static VGA_Status P_VGA_DrawSinglePixelLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color)
{
    int32_t dx = abs(x2 - x1);
    int32_t sx = x1 < x2 ? 1 : -1;
    int32_t dy = -abs(y2 - y1);
    int32_t sy = y1 < y2 ? 1 : -1;
    int32_t err = dx + dy;
    int32_t e2;

    for (;;) {
    	UB_VGA_SetPixel(x1, y1, color);
		if (x1 == x2 && y1 == y2) break;
		e2 = 2 * err;
		if (e2 >= dy) {
			err += dy;
			x1 += sx;
		}
		if (e2 <= dx) {
			err += dx;
			y1 += sy;
		}
    }
    return VGA_SUCCESS;
}

/**
  * @brief  Draws a line with a specified thickness.
  * @param  x1 Starting x-coordinate.
  * @param  y1 Starting y-coordinate.
  * @param  x2 Ending x-coordinate.
  * @param  y2 Ending y-coordinate.
  * @param  color 8-bit color of the line.
  * @param  thickness Thickness of the line in pixels.
  * @retval VGA_Status status of the operation.
  */
VGA_Status UB_VGA_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, uint8_t thickness)
{
    if (thickness == 0) return VGA_ERROR_INVALID_PARAMETER;
    if (thickness == 1) {
        return P_VGA_DrawSinglePixelLine(x1, y1, x2, y2, color);
    }

    // For thick lines, draw a filled circle at each point along the Bresenham path.
    // This produces a line with rounded ends and uniform thickness.
    int32_t dx = abs(x2 - x1);
    int32_t sx = x1 < x2 ? 1 : -1;
    int32_t dy = -abs(y2 - y1);
    int32_t sy = y1 < y2 ? 1 : -1;
    int32_t err = dx + dy;
    int32_t e2;
    uint16_t r = thickness / 2;

    int32_t current_x = x1;
    int32_t current_y = y1;

    for (;;) {
        UB_VGA_FillCircle(current_x, current_y, r, color);
        if (current_x == x2 && current_y == y2) break;
        e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            current_x += sx;
        }
        if (e2 <= dx) {
            err += dx;
            current_y += sy;
        }
    }
    return VGA_SUCCESS;
}

/**
  * @brief  Draws a filled rectangle.
  * @param  x The x-coordinate of the top-left corner.
  * @param  y The y-coordinate of the top-left corner.
  * @param  width The width of the rectangle.
  * @param  height The height of the rectangle.
  * @param  color The 8-bit fill color.
  * @retval VGA_Status status of the operation.
  */
VGA_Status UB_VGA_FillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color)
{
    if (width == 0 || height == 0) return VGA_ERROR_INVALID_PARAMETER;
    
    int32_t x_end = x + width;
    int32_t y_end = y + height;
    
    for (int32_t current_y = y; current_y < y_end; current_y++) {
        UB_VGA_FastHLine(x, current_y, x_end - 1, color);
    }
    
    return VGA_SUCCESS;
}

/**
  * @brief  Draws the outline of a rectangle.
  * @param  x The x-coordinate of the top-left corner.
  * @param  y The y-coordinate of the top-left corner.
  * @param  width The width of the rectangle.
  * @param  height The height of the rectangle.
  * @param  color The 8-bit color of the outline.
  * @retval VGA_Status status of the operation.
  */
VGA_Status UB_VGA_DrawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color)
{
    if (width == 0 || height == 0) return VGA_ERROR_INVALID_PARAMETER;

    uint16_t x2 = x + width - 1;
    uint16_t y2 = y + height - 1;

	UB_VGA_FastHLine(x, y, x2, color);      // Top
	UB_VGA_FastHLine(x, y2, x2, color);     // Bottom
	UB_VGA_FastVLine(x, y + 1, y2 - 1, color); // Left
	UB_VGA_FastVLine(x2, y + 1, y2 - 1, color);// Right
    
    return VGA_SUCCESS;
}

/**
  * @brief  Draws the outline of a circle.
  * @param  center_x The x-coordinate of the circle's center.
  * @param  center_y The y-coordinate of the circle's center.
  * @param  radius The radius of the circle.
  * @param  color The 8-bit color of the outline.
  * @retval VGA_Status status of the operation.
  */
VGA_Status UB_VGA_DrawCircle(uint16_t center_x, uint16_t center_y, uint16_t radius, uint8_t color)
{
    if (radius == 0) return VGA_ERROR_INVALID_PARAMETER;

    int32_t x = radius;
    int32_t y = 0;
    int32_t err = 0;

    while (x >= y)
    {
        UB_VGA_SetPixel(center_x + x, center_y + y, color);
        UB_VGA_SetPixel(center_x + y, center_y + x, color);
        UB_VGA_SetPixel(center_x - y, center_y + x, color);
        UB_VGA_SetPixel(center_x - x, center_y + y, color);
        UB_VGA_SetPixel(center_x - x, center_y - y, color);
        UB_VGA_SetPixel(center_x - y, center_y - x, color);
        UB_VGA_SetPixel(center_x + y, center_y - x, color);
        UB_VGA_SetPixel(center_x + x, center_y - y, color);

        if (err <= 0)
        {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0)
        {
            x -= 1;
            err -= 2*x + 1;
        }
    }
    return VGA_SUCCESS;
}

/**
  * @brief  Draws a filled circle.
  * @param  center_x The x-coordinate of the circle's center.
  * @param  center_y The y-coordinate of the circle's center.
  * @param  radius The radius of the circle.
  * @param  color The 8-bit fill color.
  * @retval VGA_Status status of the operation.
  */
VGA_Status UB_VGA_FillCircle(uint16_t center_x, uint16_t center_y, uint16_t radius, uint8_t color)
{
    if (radius == 0) return VGA_ERROR_INVALID_PARAMETER;

    int32_t x = radius;
    int32_t y = 0;
    int32_t err = 0;

    while (x >= y)
    {
        UB_VGA_FastHLine(center_x - x, center_y + y, center_x + x, color);
        UB_VGA_FastHLine(center_x - x, center_y - y, center_x + x, color);
        UB_VGA_FastHLine(center_x - y, center_y + x, center_x + y, color);
        UB_VGA_FastHLine(center_x - y, center_y - x, center_x + y, color);

        if (err <= 0)
        {
            y += 1;
            err += 2*y + 1;
        }
        if (err > 0)
        {
            x -= 1;
            err -= 2*x + 1;
        }
    }
    return VGA_SUCCESS;
}

/**
  * @brief  Draws text on the screen with various styling options.
  * @param  x The x-coordinate of the top-left starting point.
  * @param  y The y-coordinate of the top-left starting point.
  * @param  color The 8-bit color of the text.
  * @param  text The null-terminated string to draw.
  * @param  font_name The name of the font to use (e.g., "monospace", "arial").
  * @param  size The scaling factor for the font (e.g., 1, 2, 3...).
  * @param  style A bitmask for styling (TEXT_STYLE_NORMAL, TEXT_STYLE_BOLD, TEXT_STYLE_ITALIC).
  * @param  bounding_box Optional pointer to a VGA_Rect to store the dimensions of the rendered text.
  * @retval VGA_Status status of the operation.
  */
VGA_Status UB_VGA_DrawText(uint16_t x, uint16_t y, uint8_t color, const char* text, const char* font_name, uint8_t size, uint8_t style, VGA_Rect* bounding_box)
{
    // --- 1. Font Selection ---
    const FontDef_t* font_def = NULL;
    bool font_found = false;
    if (font_name != NULL && *font_name != '\0') {
        for (uint8_t i = 0; i < NUM_AVAILABLE_FONTS; i++) {
            if (strcmp(font_name, available_fonts[i].name) == 0) {
                font_def = available_fonts[i].font_def;
                font_found = true;
                break;
            }
        }
    } else {
        font_def = available_fonts[0].font_def; // Default font
        font_found = true;
    }

    if (!font_found || font_def == NULL) return VGA_ERROR_INVALID_PARAMETER;

    // --- 2. Style & Size Parameters ---
    bool is_bold = (style & TEXT_STYLE_BOLD);
    bool is_italic = (style & TEXT_STYLE_ITALIC);
    if (size == 0) size = 1;

    // --- 3. Bounding Box Initialization ---
    VGA_Rect bbox = { .x = x, .y = y, .width = 0, .height = 0 };
    bool first_char = true;

    uint16_t current_x = x;
    uint16_t current_y = y;

    // --- 4. Draw Loop ---
    while (*text) {
        char character = *text++;

        // Handle Control Characters
        if (character == '\n') {
            current_y += (font_def->height * size) + 2;
            current_x = x;
            continue;
        }
        if (character == '\r') continue;

        // Map character to a displayable one if it's outside the font range
        if ((uint8_t)character >= 128) character = '?';

        // --- 5. Retrieve Font Data ---
        uint8_t char_width = 0;
        const uint8_t* font_char_data = NULL;

        if (font_def->chars == NULL) { // Fixed-width font
            char_width = 5; // Default width for Consolas-like font
            font_char_data = font_consolas_data[(uint8_t)character];
        } else { // Proportional font
            const FontChar_t* font_char = &font_def->chars[(uint8_t)character];
            // Validate glyph data
            if (font_char != NULL && font_char->data != NULL && font_char->width > 0) {
                char_width = font_char->width;
                font_char_data = font_char->data;
            }
        }

        // Skip if character is not defined in the font
        if (font_char_data == NULL || char_width == 0) {
             current_x += (5 * size); // Advance by a default width
             continue;
        }
        
        uint16_t scaled_char_width = char_width * size;
        uint16_t scaled_font_height = font_def->height * size;

        // --- 6. Rendering ---
        for (uint8_t col = 0; col < char_width; col++) {
            uint8_t col_data = font_char_data[col];

            for (uint8_t row = 0; row < font_def->height; row++) {
                if ((col_data >> row) & 0x01) { // If pixel is set
                    int32_t x_offset = is_italic ? ((font_def->height - row) / 2) : 0;
                    
                    int32_t draw_x = current_x + (col * size) + x_offset;
                    int32_t draw_y = current_y + (row * size);

                    // Use FillRectangle for performance
                    uint8_t block_width = is_bold ? (size + 1) : size;
                    UB_VGA_FillRectangle(draw_x, draw_y, block_width, size, color);
                }
            }
        }

        // --- 7. Update Bounding Box ---
        int32_t italic_offset = is_italic ? (font_def->height / 2) : 0;
        int32_t char_render_width = scaled_char_width + (is_bold ? size : 0) + italic_offset;

        if (first_char) {
            bbox.x = current_x;
            bbox.y = current_y;
            bbox.width = char_render_width;
            bbox.height = scaled_font_height;
            first_char = false;
        } else {
            int32_t new_width = (current_x + char_render_width) - bbox.x;
            bbox.width = max(bbox.width, new_width);
            bbox.height = max(bbox.height, (current_y + scaled_font_height) - bbox.y);
        }

        // --- 8. Advance Cursor ---
        current_x += scaled_char_width + size; // Character width + 1px scaled spacing
        if (is_bold) current_x += size;

        // Wrap check
        if (current_x > VGA_DISPLAY_X - scaled_char_width) {
            current_y += scaled_font_height + 2;
            current_x = x;
        }
    }

    // --- 9. Finalize ---
    if (bounding_box != NULL) {
        *bounding_box = bbox;
    }

	return VGA_SUCCESS;
}

