//--------------------------------------------------------------
// File     : stm32_ub_vga_320x240.c
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.0
// Module   : GPIO, TIM, MISC, DMA
// Function : VGA out by GPIO (320x240 Pixel, 8bit color)
//
// signals  : PB11      = HSync-Signal
//            PB12      = VSync-Signal
//            PE8+PE9   = color Blue
//            PE10-PE12 = color Green
//            PE13-PE15 = color red
//
// uses     : TIM1, TIM2
//            DMA2, Channel6, Stream5
//--------------------------------------------------------------


//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "stm32_ub_vga_screen.h"
#include "bitmaps.h"
#include "fonts.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))

VGA_t VGA;
uint8_t VGA_RAM1[(VGA_DISPLAY_X+1)*VGA_DISPLAY_Y];
//--------------------------------------------------------------
// internal Functions
//--------------------------------------------------------------
void P_VGA_InitIO(void);
void P_VGA_InitTIM(void);
void P_VGA_InitINT(void);
void P_VGA_InitDMA(void);
static VGA_Status P_VGA_DrawSinglePixelLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint8_t color);


//--------------------------------------------------------------
// Init VGA-Module
//--------------------------------------------------------------
void UB_VGA_Screen_Init(void)
{
  uint16_t xp,yp;

  VGA.hsync_cnt=0;
  VGA.start_adr=0;
  VGA.dma2_cr_reg=0;

  UB_VGA_ResetClipRect();

  // RAM init total black
  for(yp=0;yp<VGA_DISPLAY_Y;yp++) {
    for(xp=0;xp<(VGA_DISPLAY_X+1);xp++) {
      VGA_RAM1[(yp*(VGA_DISPLAY_X+1))+xp]=0;
    }
  }

  // init IO-Pins
  P_VGA_InitIO();
  // init Timer
  P_VGA_InitTIM();
  // init DMA
  P_VGA_InitDMA();
  // init Interrupts
  P_VGA_InitINT();

  //-----------------------
  // Register swap and safe
  //-----------------------
  // content of CR-Register read and save
  VGA.dma2_cr_reg=DMA2_Stream5->CR;
}

//--------------------------------------------------------------
// Clipping functions
//--------------------------------------------------------------
void UB_VGA_SetClipRect(const VGA_Rect *rect)
{
    if (rect == NULL) {
        UB_VGA_ResetClipRect();
        return;
    }
    VGA.clip_rect.x = max(0, rect->x);
    VGA.clip_rect.y = max(0, rect->y);
    VGA.clip_rect.width = min(VGA_DISPLAY_X - VGA.clip_rect.x, rect->width);
    VGA.clip_rect.height = min(VGA_DISPLAY_Y - VGA.clip_rect.y, rect->height);
}

void UB_VGA_GetClipRect(VGA_Rect *rect)
{
    if (rect == NULL) return;
    *rect = VGA.clip_rect;
}

void UB_VGA_ResetClipRect(void)
{
    VGA.clip_rect.x = 0;
    VGA.clip_rect.y = 0;
    VGA.clip_rect.width = VGA_DISPLAY_X;
    VGA.clip_rect.height = VGA_DISPLAY_Y;
}


//--------------------------------------------------------------
// fill the DMA RAM buffer with one color
//--------------------------------------------------------------
VGA_Status UB_VGA_FillScreen(uint8_t color)
{
  // This function is fast, so we can bypass per-pixel clipping
  // and just fill the whole buffer.
  memset(VGA_RAM1, color, (VGA_DISPLAY_X+1)*VGA_DISPLAY_Y);
  // Ensure the last pixel of each line is black
  for(uint16_t yp=0; yp<VGA_DISPLAY_Y; yp++) {
      VGA_RAM1[(yp*(VGA_DISPLAY_X+1))+VGA_DISPLAY_X] = 0;
  }
  return VGA_SUCCESS;
}


//--------------------------------------------------------------
// put one Pixel on the screen with one color
// Important : the last Pixel+1 from every line must be black (don't know why??)
//--------------------------------------------------------------
VGA_Status UB_VGA_SetPixel(uint16_t xp, uint16_t yp, uint8_t color)
{
  // Check against screen boundaries
  if(xp >= VGA_DISPLAY_X || yp >= VGA_DISPLAY_Y) return VGA_ERROR_INVALID_COORDINATE;

  // Check against clipping rectangle
  if (xp < VGA.clip_rect.x || yp < VGA.clip_rect.y ||
      xp >= VGA.clip_rect.x + VGA.clip_rect.width ||
      yp >= VGA.clip_rect.y + VGA.clip_rect.height) {
      return VGA_SUCCESS; // Clipped, do not draw but not an error
  }

  // Write pixel to ram
  VGA_RAM1[(yp*(VGA_DISPLAY_X+1))+xp]=color;

  return VGA_SUCCESS;
}

VGA_Status UB_VGA_FastHLine(int32_t x0, int32_t y, int32_t x1, uint8_t color)
{
    if (y < VGA.clip_rect.y || y >= (VGA.clip_rect.y + VGA.clip_rect.height)) return VGA_SUCCESS;

    int32_t start_x = max(min(x0, x1), VGA.clip_rect.x);
    int32_t end_x = min(max(x0, x1), VGA.clip_rect.x + VGA.clip_rect.width - 1);

    if (start_x > end_x) return VGA_SUCCESS;

    uint32_t base_addr = y * (VGA_DISPLAY_X + 1);
    memset(&VGA_RAM1[base_addr + start_x], color, end_x - start_x + 1);

    return VGA_SUCCESS;
}

VGA_Status UB_VGA_FastVLine(int32_t x, int32_t y0, int32_t y1, uint8_t color)
{
    if (x < VGA.clip_rect.x || x >= (VGA.clip_rect.x + VGA.clip_rect.width)) return VGA_SUCCESS;

    int32_t start_y = max(min(y0, y1), VGA.clip_rect.y);
    int32_t end_y = min(max(y0, y1), VGA.clip_rect.y + VGA.clip_rect.height - 1);
    
    if (start_y > end_y) return VGA_SUCCESS;

    uint32_t line_addr_start = start_y * (VGA_DISPLAY_X + 1) + x;
    for (int32_t i = 0; i <= (end_y - start_y); i++) {
        VGA_RAM1[line_addr_start + i * (VGA_DISPLAY_X + 1)] = color;
    }
    return VGA_SUCCESS;
}

//--------------------------------------------------------------
// interne Funktionen
// init aller IO-Pins
//--------------------------------------------------------------
void P_VGA_InitIO(void)
{
  GPIO_InitTypeDef  GPIO_InitStructure;


  //---------------------------------------------
  // init RGB-Pins (PE8 - PE15)
  // as normal GPIOs
  //---------------------------------------------
 
  // Clock Enable
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

  // Config as Digital output
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 |
        GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOE, &GPIO_InitStructure);

  GPIOE->BSRRH = VGA_GPIO_HINIBBLE;


  //---------------------------------------------
  // init of the H-Sync Pin (PB11)
  // using Timer2 and CH4
  //---------------------------------------------

  // Clock Enable
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  // Config Pins as Digital-out
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  // alternative function connect with IO
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_TIM2);


  //---------------------------------------------
  // init of V-Sync Pin (PB12)
  // using GPIO
  //---------------------------------------------

  // Clock Enable
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

  // Config of the Pins as Digital out
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIOB->BSRRL = GPIO_Pin_12;
}


//--------------------------------------------------------------
// internal Function
// init Timer
//--------------------------------------------------------------
void P_VGA_InitTIM(void)
{
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  TIM_OCInitTypeDef  TIM_OCInitStructure;


  //---------------------------------------------
  // init of Timer1 for
  // Pixeldata via DMA
  //---------------------------------------------

  // Clock enable
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

  // Timer1 init
  TIM_TimeBaseStructure.TIM_Period =  VGA_TIM1_PERIODE;
  TIM_TimeBaseStructure.TIM_Prescaler = VGA_TIM1_PRESCALE;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);


  //---------------------------------------------
  // init Timer2
  // CH4 for HSYNC-Signal
  // CH3 for DMA Trigger start
  //---------------------------------------------

  // Clock enable
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

  // Timer2 init
  TIM_TimeBaseStructure.TIM_Period = VGA_TIM2_HSYNC_PERIODE;
  TIM_TimeBaseStructure.TIM_Prescaler = VGA_TIM2_HSYNC_PRESCALE;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

  // Timer2 Channel 3 ( for DMA Trigger start)
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = VGA_TIM2_HTRIGGER_START-VGA_TIM2_DMA_DELAY;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OC3Init(TIM2, &TIM_OCInitStructure);
  TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);

  // Timer2 Channel 4 (for HSYNC)
  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_Pulse = VGA_TIM2_HSYNC_IMP;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OC4Init(TIM2, &TIM_OCInitStructure);
  TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable);


  //---------------------------------------------
  // enable all Timers
  //---------------------------------------------

  // Timer1 enable
  TIM_ARRPreloadConfig(TIM1, ENABLE);

  // Timer2 enable
  TIM_ARRPreloadConfig(TIM2, ENABLE);
  TIM_Cmd(TIM2, ENABLE);

}

//--------------------------------------------------------------
// internal Function
// init Interrupts
//--------------------------------------------------------------
void P_VGA_InitINT(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  //---------------------------------------------
  // init from DMA Interrupt
  // for TransferComplete Interrupt
  // DMA2, Stream5, Channel6
  //---------------------------------------------

  DMA_ITConfig(DMA2_Stream5, DMA_IT_TC, ENABLE);

  // NVIC config
  NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream5_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);


  //---------------------------------------------
  // init of Timer2 Interrupt
  // for HSync-Counter using Update
  // for DMA Trigger START using CH3
  //---------------------------------------------

  TIM_ITConfig(TIM2,TIM_IT_CC3,ENABLE);

  // NVIC config
  NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}


//--------------------------------------------------------------
// internal Function
// init DMA
//--------------------------------------------------------------
void P_VGA_InitDMA(void)
{
  DMA_InitTypeDef DMA_InitStructure;

  //---------------------------------------------
  // DMA of Timer1 Update
  // (look at page 217 of the Ref Manual)
  // DMA=2, Channel=6, Stream=5
  //---------------------------------------------

  // Clock Enable (DMA)
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

  // DMA init (DMA2, Channel6, Stream5)
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

  // DMA-Timer1 enable
  TIM_DMACmd(TIM1,TIM_DMA_Update,ENABLE);
}



//--------------------------------------------------------------
// Interrupt of Timer2
//
//   CC3-Interrupt    -> starts from DMA
// Watch it.. higher troughput when interrupt flag is left alone
//--------------------------------------------------------------
void TIM2_IRQHandler(void)
{

  // Interrupt of Timer2 CH3 occurred (for Trigger start)
  TIM_ClearITPendingBit(TIM2, TIM_IT_CC3);

  VGA.hsync_cnt++;
  if(VGA.hsync_cnt>=VGA_VSYNC_PERIODE) {
    // -----------
    VGA.hsync_cnt=0;
    // Adresspointer first dot
    VGA.start_adr=(uint32_t)(&VGA_RAM1[0]);
  }

  // HSync-Pixel
  if(VGA.hsync_cnt<VGA_VSYNC_IMP) {
    // HSync low
    GPIOB->BSRRH = GPIO_Pin_12;
  }
  else {
    // HSync High
    GPIOB->BSRRL = GPIO_Pin_12;
  }

  // Test for DMA start
  if((VGA.hsync_cnt>=VGA_VSYNC_BILD_START) && (VGA.hsync_cnt<=VGA_VSYNC_BILD_STOP)) {
    // DMA2 init
	DMA2_Stream5->CR=VGA.dma2_cr_reg;
    // set address
    DMA2_Stream5->M0AR=VGA.start_adr;
    // Timer1 start
    TIM1->CR1|=TIM_CR1_CEN;
    // DMA2 enable
    DMA2_Stream5->CR|=DMA_SxCR_EN;

    // Test Adrespointer for high
    if((VGA.hsync_cnt & 0x01)!=0) {
      // inc after Hsync
      VGA.start_adr+=(VGA_DISPLAY_X+1);
    }
  }

}


//--------------------------------------------------------------
// DMA Interrupt ISR
//   after TransferCompleteInterrupt -> stop DMA
//
// still a bit buggy
//--------------------------------------------------------------
void DMA2_Stream5_IRQHandler(void)
{
  if(DMA_GetITStatus(DMA2_Stream5, DMA_IT_TCIF5))
  {
    // TransferInterruptComplete Interrupt from DMA2
    DMA_ClearITPendingBit(DMA2_Stream5, DMA_IT_TCIF5);

    // stop after all pixels => DMA Transfer stop
    // Timer1 stop
    TIM1->CR1&=~TIM_CR1_CEN;
    // DMA2 disable
    DMA2_Stream5->CR=0;
    // switch on black
    GPIOE->BSRRH = VGA_GPIO_HINIBBLE;
  }
}


VGA_Status UB_VGA_DrawBitmap(uint8_t id, uint16_t x_lup, uint16_t y_lup) {
    if (id >= NUM_BITMAPS) {
        return VGA_ERROR_INVALID_PARAMETER;
    }

    const Bitmap_t *bitmap = &vga_bitmaps[id];

    // Clipping is handled by SetPixel
    for (uint16_t y = 0; y < bitmap->height; y++) {
        for (uint16_t x = 0; x < bitmap->width; x++) {
            uint8_t color = bitmap->data[y][x];
            if (color != BITMAP_TRANSPARENT_COLOR) {
                UB_VGA_SetPixel(x_lup + x, y_lup + y, color);
            }
        }
    }
    return VGA_SUCCESS;
}

// Internal helper function to draw a 1-pixel thick line
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

VGA_Status UB_VGA_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t color, uint8_t thickness)
{
    if (thickness == 0) return VGA_ERROR_INVALID_PARAMETER;
    if (thickness == 1) {
        return P_VGA_DrawSinglePixelLine(x1, y1, x2, y2, color);
    }

    // Correct thick line implementation using filled circles at each point
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


VGA_Status UB_VGA_FillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color)
{
    if (width == 0 || height == 0) return VGA_ERROR_INVALID_PARAMETER;
    
    int32_t x_end = x + width;
    int32_t y_end = y + height;
    
    for (int32_t current_y = y; current_y < y_end; current_y++) {
        UB_VGA_FastHLine(x, current_y, x_end -1, color);
    }
    
    return VGA_SUCCESS;
}


VGA_Status UB_VGA_DrawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint8_t color)
{
    if (width == 0 || height == 0) return VGA_ERROR_INVALID_PARAMETER;

    uint16_t x2 = x + width - 1;
    uint16_t y2 = y + height - 1;

	UB_VGA_FastHLine(x, y, x2, color);      // Top
	UB_VGA_FastHLine(x, y2, x2, color);     // Bottom
	UB_VGA_FastVLine(x, y, y2, color);      // Left
	UB_VGA_FastVLine(x2, y, y2, color);     // Right
    
    return VGA_SUCCESS;
}

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

