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
	LED_Init();   // LED configureren

	//clearscherm function
	if (clearscherm("grijs")==100) {
		LED_On(0);
	} else {
		LED_On(1);
	}

	if (wacht(1000)==100) {
			LED_On(0);
		} else {
			LED_On(1);
		}

	//lijn function
	/*if (lijn(3,5,30,30,"blauw", 5)==100) {
		LED_On(0);
	} else {
		LED_On(1);
	}*/

	//rechthoek function
	/*if (rechthoek(10,10,30,20,"zwart",1)==100) {
		LED_On(0);
	} else {
		LED_On(1);
	}*/

	if (figuur(160, 20, 250, 100, 210, 200, 110, 200, 70, 100, "rood") == OK) {
	    LED_On(0);
	} else {
	    LED_On(1);
	}

	/*if (cirkel(120,160,79,"groen")==100) {
		LED_On(0);
	} else {
		LED_On(1);
	}*/

	if (wacht(1000)==100) {
		LED_On(0);
	} else {
		LED_On(1);
	}

	/*if (tekst(10,20, "zwart","the quick brown fox jumps over the lazy dog", "consolas", 2, "vet")==100) {
		LED_On(0);
	} else {
		LED_On(1);
	}

	if (wacht(2000)==100) {
			LED_On(0);
		} else {
			LED_On(1);
		}*/

	if (bitmap(3,50,50)==100) {
			LED_On(0);
		} else {
			LED_On(1);
		}

	if (wacht(1000)==100) {
			LED_On(0);
		} else {
			LED_On(1);
		}

	if (herhaal(6, 3) == OK) {
	        LED_On(0); // Groen licht als het herhalen is gelukt
	    } else {
	        LED_On(1); // Rood licht bij een fout
	    }

	if (clearscherm("groen")==100) {
			LED_On(0);
		} else {
			LED_On(1);
		}

  while(1)
  {

  }
}
