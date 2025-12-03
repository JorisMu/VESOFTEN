//--------------------------------------------------------------
// File     : main.c
// Datum    : 30.03.2016
// Version  : 1.0
// Autor    : UB
// mods by	: J.F. van der Bent
// CPU      : STM32F4
// IDE      : CooCox CoIDE 1.7.x
// Module   : CMSIS_BOOT, M4_CMSIS_CORE
// Function : VGA_core DMA LIB 320x240, 8bit color
//--------------------------------------------------------------

#include "main.h"
#include "stm32_ub_vga_screen.h"
#include <math.h>
#include "logic.h"

#include <stdio.h>

#include <stdint.h>

#define GPIOD_BASE 0x40020C00
#define RCC_BASE   0x40023800

#define RCC_AHB1ENR (*(volatile uint32_t*)(RCC_BASE + 0x30))
#define GPIOD_MODER (*(volatile uint32_t*)(GPIOD_BASE + 0x00))
#define GPIOD_ODR   (*(volatile uint32_t*)(GPIOD_BASE + 0x14))

void LED_Init(void) {
    // 1️⃣ Clock enable voor GPIOD
    RCC_AHB1ENR |= (1 << 3); // GPIODEN = 1

    // 2️⃣ PD12–PD15 als output
    for (int i = 12; i <= 15; i++) {
        GPIOD_MODER &= ~(3 << (i*2)); // clear MODER
        GPIOD_MODER |=  (1 << (i*2)); // output mode
    }
}

void LED_On(int led) {
    // LED 0 = PD12, 1 = PD13, 2 = PD14, 3 = PD15
    if (led < 0 || led > 3) return;

    // Eerst alle LEDjes uit
    GPIOD_ODR &= ~((1 << 12) | (1 << 13) | (1 << 14) | (1 << 15));

    // Dan het juiste LEDje aan
    GPIOD_ODR |= (1 << (12 + led));
}



int main(void)
{
	SystemInit(); // System speed to 168MHz

	UB_VGA_Screen_Init(); // Init VGA-Screen

	UB_VGA_FillScreen(VGA_COL_BLUE);

	LED_Init();   // LED configureren
	//char kleur[20] = "blauw";

	LED_On(lijn(60,60, 60, 60, "blauw", 3));     // LED aanzetten

  while(1)
  {

  }
}
