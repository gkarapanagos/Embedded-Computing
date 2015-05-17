#include <msp430.h>
#include <stdint.h>
#include "inc\hw_memmap.h"
#include "driverlibHeaders.h"
#include "CTS_Layer.h"
#include "grlib.h"
#include "LcdDriver/Dogs102x64_UC1701.h"
#include "peripherals.h"
#include "HAL_Cma3000.h"

void drawMarble(int x, int y, int check);
void buzzeron(int pitch);

int main(void){

	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer
	//Perform initializations (see peripherals.c)
	configTouchPadLEDs();
	configDisplay();
	configCapButtons();
	_BIS_SR(GIE);
	int xoffset, yoffset, zoffset;
	int Xg , Yg , Zg;
	int check;
	check = 0 ;
	CAP_BUTTON keypressed_state;
	P1SEL &= ~(BIT0|BIT1|BIT2);  // Select P1.0 for digital IO
	P1DIR &= ~(BIT0|BIT1|BIT2);
	Cma3000_init();
	xoffset = Cma3000_readRegister(DOUTX);
	yoffset = Cma3000_readRegister(DOUTY);
	zoffset = 0;
	Cma3000_setAccel_offset(xoffset, yoffset, zoffset);
	while(1){
		Cma3000_readAccel_offset();
		Xg = Cma3000_xAccel;
		Yg = Cma3000_yAccel;
		Zg = Cma3000_zAccel;
		keypressed_state = CapButtonRead();
		if(keypressed_state==1) {check=0;}
		if(keypressed_state==2) {check=1;}
		drawMarble(Xg, Yg, check);

}
}

void drawMarble(int x, int y, int check){
	int maxX, minX, maxY, minY;
	int xPOS, yPOS;
	maxY = 68;
	minY = -62;
	minX = -80;
	maxX = 78;
	if( x >= 0 )
	xPOS = 51 - 51 * x / maxX;
	if ( x < 0)
	xPOS = 51 + 51 * x / minX;
	if( y >= 0 )
	yPOS = 32 * y / maxY + 32;
	if ( y < 0)
	yPOS = 32 - 32 * y / minY;
	GrClearDisplay(&g_sContext);
	if(yPOS>=45){
		if(xPOS>72){
			xPOS=72;
		}
	}
	if(xPOS>=85){
			if(yPOS>32){
				yPOS=32;
			}
		}

	if((yPOS<=45)&&(yPOS>=35)&&(xPOS>=75)&&(xPOS<=85)){
		if(yPOS>=42) {yPOS=42; }
		if(xPOS>=82) {xPOS=82; }
}

	GrCircleFill(&g_sContext, xPOS, yPOS, 3);
	GrLineDraw(&g_sContext, 85, 35, 85, 45 );
	GrLineDraw(&g_sContext, 75, 45, 85, 45 );
	GrLineDraw(&g_sContext, 75, 45, 75, 60 );
	GrLineDraw(&g_sContext, 85, 35, 100, 35 );
	GrFlush(&g_sContext);
	int pitch;
	pitch = 30 * (yPOS + xPOS) / 166 + 45;
	if (check==0) { buzzeron(pitch); }
	if (check==1) { BuzzerOff(); }
}

void buzzeron(int pitch)
{
	// Initialize PWM output on P7.5, which corresponds to TB0.3
	P7SEL |= BIT5; // Select peripheral output mode for P7.5
	P7DIR |= BIT5;

	TB0CTL  = (TBSSEL__ACLK|ID__1|MC__UP);  // Configure Timer B0 to use ACLK, divide by 1, up mode
	TB0CTL  &= ~TBIE; 						// Explicitly Disable timer interrupts for safety

	TB0CCR0   = pitch; 					// Set the PWM period in ACLK ticks
	TB0CCTL0 &= ~CCIE;					// Disable timer interrupts

	// Configure CC register 3, which is connected to our PWM pin TB0.3
	TB0CCTL3  = OUTMOD_7;					// Set/reset mode for PWM
	TB0CCTL3 &= ~CCIE;						// Disable capture/compare interrupts
	TB0CCR3   = TB0CCR0/2; 					// Configure a 50% duty cycle
}




