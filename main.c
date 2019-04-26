// Code to print to the LCD pixel display on the Educational BoosterPack

#include "msp430fr6989.h"
#include "Grlib/grlib/grlib.h"          // Graphics library (grlib)
#include "LcdDriver/lcd_driver.h"       // LCD driver
#include "logo.c"
#include <stdio.h>

#define redLED BIT0
#define greenLED BIT7
#define button BIT1

void config_ACLK_to_32KHz_crystal();

const tImage  logo4BPP_UNCOMP=
{
    IMAGE_FMT_4BPP_UNCOMP,
    128,
    128,
    16,
    palette_logo4BPP_UNCOMP,
    pixel_logo4BPP_UNCOMP,
};


Graphics_Context g_sContext;        // Declare a graphic library context
Graphics_Context my_sContext;        // Declare a graphic library context
Graphics_Context my_sContext2;        // Declare a graphic library context

volatile uint8_t counter=0;
char mystring[20];
unsigned int n;

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;     // Stop the Watchdog timer
    PM5CTL0 &= ~LOCKLPM5;         // Disable GPIO power-on default high-impedance mode

    P1DIR |= redLED;    P1OUT &= ~redLED;
    P9DIR |= greenLED;  P9OUT &= ~greenLED;
    P1DIR &= ~button; P1REN|=button; P1OUT|=button; // button, resistor, pullup
    P1IE  |= button;       // 1: enable interrupts
    P1IES |= button;       // 1: interrupt on falling edge
    P1IFG &= ~button;      // 0: clear the interrupt flags

    // Configure SMCLK to 8 MHz (used as SPI clock)
    CSCTL0 = CSKEY;                 // Unlock CS registers
    CSCTL3 &= ~(BIT4|BIT5|BIT6);    // DIVS=0
    CSCTL0_H = 0;                   // Relock the CS registers

    ////////////////////////////////////////////////////////////////////////////////////////////
    Crystalfontz128x128_Init();         // Initialize the display

    // Set the screen orientation
    Crystalfontz128x128_SetOrientation(0);

    // Initialize the context
    Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128);

    // Set background and foreground colors
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_GREEN);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);

    // Set the default font for strings
    GrContextFontSet(&g_sContext, &g_sFontFixed6x8);
    ////////////////////////////////////////////////////////////////////////////////////////////
    ///////////////////////////////// MY CONTEXT(S) ////////////////////////////////////////////
    // ------------------------------------(1)----------------------------------------------- //
    // Initialize the context
    Graphics_initContext(&my_sContext, &g_sCrystalfontz128x128);

    // Set background and foreground colors
    Graphics_setBackgroundColor(&my_sContext, GRAPHICS_COLOR_GREEN);
    Graphics_setForegroundColor(&my_sContext, GRAPHICS_COLOR_RED);

    // Set the default font for strings
    GrContextFontSet(&my_sContext, &g_sFontFixed6x8);

    // ------------------------------------(2)----------------------------------------------- //
    // Initialize the context
    Graphics_initContext(&my_sContext2, &g_sCrystalfontz128x128);

    // Set background and foreground colors
    Graphics_setBackgroundColor(&my_sContext2, GRAPHICS_COLOR_GREEN);
    Graphics_setForegroundColor(&my_sContext2, GRAPHICS_COLOR_YELLOW);

    // Set the default font for strings
    GrContextFontSet(&my_sContext2, &g_sFontFixed6x8);

    ////////////////////////////////////////////////////////////////////////////////////////////


    // Configure ACLK to the 32 KHz crystal
    config_ACLK_to_32KHz_crystal();

    // Configure Channel 0 for up mode with interrupt
    TA0CCR0 = 4915;           // Fill to get .3 second @ 24 KHz

    // Timer_A configuration (fill the line below)
    // Use ACLK, divide by 1, Up mode, TAR cleared
    TA0CTL = TASSEL_1 | ID_1 | MC_1 | TACLR | TAIE;
    TA0CTL &= ~TAIFG;

    firstFrame();
    printC();
    _low_power_mode_3();

//    while(1){}


}

void printC(){
    if( (P9OUT & greenLED) != 0 ) return;
    if( ++counter==0 ){
        sprintf(mystring, "   0   ");
        Graphics_drawStringCentered(&g_sContext, mystring, AUTO_STRING_LENGTH, 64, 80, OPAQUE_TEXT);
    }

    sprintf(mystring, "%d", counter);
    Graphics_drawStringCentered(&g_sContext, mystring, AUTO_STRING_LENGTH, 64, 80, OPAQUE_TEXT);
}

void secFrame(){
    Graphics_drawImage(&g_sContext, &logo4BPP_UNCOMP, 0, 0);
}

void firstFrame(){
    // Clear the screen
    Graphics_clearDisplay(&my_sContext2);

    Graphics_drawStringCentered(&g_sContext, "Bernardin D.", AUTO_STRING_LENGTH, 64, 15, OPAQUE_TEXT);

    Graphics_drawStringCentered(&g_sContext, "Welcome to", AUTO_STRING_LENGTH, 64, 30, OPAQUE_TEXT);

    sprintf(mystring, "EEL 4742 Lab!");
    Graphics_drawStringCentered(&g_sContext, mystring, AUTO_STRING_LENGTH, 64, 55, OPAQUE_TEXT);

    Graphics_fillCircle(&g_sContext, 32 - 16, 100, 10);
    Graphics_drawCircle(&g_sContext, 64 - 16, 100, 10);

    Graphics_Rectangle rect;
    int rect_centerX = 96 - 16, rect_width = 20;
    int rect_centerY = 100, rect_height = 16;
    rect.xMin = rect_centerX - (rect_width/2);
    rect.yMin = rect_centerY - (rect_height/2);
    rect.xMax = rect_centerX + (rect_width/2);
    rect.yMax = rect_centerY + (rect_height/2);
    Graphics_setClipRegion(&my_sContext, &rect);
    Graphics_fillRectangle(&my_sContext, &rect);

    Graphics_Rectangle rect2;
    int rect2_centerX = 128 - 16;
    int rect2_centerY = 100;
    rect2.xMin = rect2_centerX - (rect_width/2);
    rect2.yMin = rect2_centerY - (rect_height/2);
    rect2.xMax = rect2_centerX + (rect_width/2);
    rect2.yMax = rect2_centerY + (rect_height/2);
    Graphics_setClipRegion(&my_sContext2, &rect2);
    Graphics_drawRectangle(&my_sContext2, &rect2);

    Graphics_drawLineH(&g_sContext, 5, 124, 115);
}

#pragma vector = PORT1_VECTOR
__interrupt void mine() {
    // If on First Frame, which to logo and toggle LED
    if( (P9OUT & greenLED) == 0 ){
        P9OUT ^= greenLED;
        secFrame();
    }else{
        P9OUT ^= greenLED;
        firstFrame();
    }

    P1IFG &= !button;

}

// ISR of Timer (A1 vector)
#pragma vector = TIMER0_A1_VECTOR
__interrupt void T0A0_ISR() {
    printC();

    // Clear flag
    TA0CTL &= ~TAIFG;
}

void config_ACLK_to_32KHz_crystal() {
    // By default, ACLK runs on LFMODCLK at 5MHz/128 = 39 KHz

    // Reroute pins to LFXIN/LFXOUT functionality
    PJSEL1 &= ~BIT4;
    PJSEL0 |= BIT4;

    // Wait until the oscillator fault flags remain cleared
    CSCTL0 = CSKEY;             // Unlock CS registers
    do {
    CSCTL5 &= ~LFXTOFFG; // Local fault flag
    SFRIFG1 &= ~OFIFG; // Global fault flag
    } while((CSCTL5 & LFXTOFFG) != 0);
    CSCTL0_H = 0; // Lock CS registers
    return;
}
