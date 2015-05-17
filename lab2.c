#include <msp430.h>
#include <stdint.h>
#include "inc\hw_memmap.h"
#include "driverlibHeaders.h"
#include "CTS_Layer.h"
#include "grlib.h"
#include "LcdDriver/Dogs102x64_UC1701.h"
#include "peripherals.h"



#define START 0;
int leap_cnt=0;
unsigned int timer;
void buttonPushed(int delay)
{
	int i;

	if( (P1IN & BIT7) ==00000000) {
		GrClearDisplay(&g_sContext);
		GrStringDrawCentered(&g_sContext, "Button 1 Pressed", AUTO_STRING_LENGTH, 51, 16, TRANSPARENT_TEXT);
		GrFlush(&g_sContext);
	}
	if( (P2IN & BIT2) ==0000000) {
		GrClearDisplay(&g_sContext);
		GrStringDrawCentered(&g_sContext, "Button 2 Pressed", AUTO_STRING_LENGTH, 51, 16, TRANSPARENT_TEXT);
		GrFlush(&g_sContext);
	}

	for(i=0; i<delay; i++) {}
	GrClearDisplay(&g_sContext);
	GrFlush(&g_sContext);
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

void configLED1_3(char inbits){

	if( (inbits & BIT0) == 00000001 )
	{	P1OUT = P1OUT | BIT0;	}
	else { P1OUT &= ~(BIT0); }

	if(inbits & BIT1)
	{	P8OUT |= BIT1;	}
	else { P8OUT &=  ~(BIT1); }

	if(inbits & BIT2)
	{	P8OUT |= BIT2;	}
	else { P8OUT &=  ~(BIT2); }

}

void stoptimerA2(int reset)
{
TA2CTL = MC_0; // stop timer
TA2CCTL0 &= ~CCIE; // TA2CCR0 interrupt disabled
if(reset)
timer=0;
}

#pragma vector=TIMER2_A0_VECTOR
__interrupt void Timer_A2_ISR(void)
{
	if(leap_cnt<1024)
	{
	timer++;
	leap_cnt++;
	}
	else
	{timer+=2;
	leap_cnt=0;
	}
}

void runtimerA2(void)
{

TA2CTL = TASSEL_1 + MC_1 + ID_0;
TA2CCR0 = 163; // 327+1 ACLK tics = ~1/100 seconds
TA2CCTL0 = CCIE; // TA2CCR0 interrupt enabled
}

int chkBtns1()
{
	P1SEL &= ~BIT7;
	P1DIR &= ~BIT7;
	P1OUT |= BIT7;
	P1REN |= BIT7;
	//Logic 0 indicates button press
	int button = P1IN & BIT7;
	return button;
}
int chkBtns2()
{
	P2SEL &= ~BIT2;
	P2DIR &= ~BIT2;
	P2OUT |= BIT2;
	P2REN |= BIT2;
	//Logic 0 indicates button press
	int button = P2IN & BIT2;
	return button;
}

//MAIN FUNCTION
void main(void)
{
	int timer_on=0;
	_BIS_SR(GIE);
	int n[16];
	int k;
	for(k=0; k<16; k++)
	{
		n[k]=0;
	}
	//function definitions
	void buttonPushed(int delay);
	void configLED1_3(char inbits);
	void runtimerA2(void);
	void stoptimerA2(int reset);
	__interrupt void Timer_A2_ISR(void);
	void buzzeron(int pitch);
	int chkBtns1();
	int chkBtns2();
	// Stop WDT
    WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer
    //Perform initializations (see peripherals.c)
	configTouchPadLEDs();
	configDisplay();
	configCapButtons();
	int i;
    // Variable to record button state for later
    CAP_BUTTON keypressed_state;
    int state = START;
    int count=0;
    int button1, button2;
    P8SEL = P8SEL & ~(BIT0|BIT1|BIT2);  // Select P1.0 for digital IO
    P8DIR |=  (BIT0|BIT1|BIT2);		// Set P1.7 to input direction
    P8OUT &=  ~(BIT0|BIT1|BIT2);

    P1SEL = P1SEL & ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7);  // Select P1.0 for digital IO
   // P1DIR &= ~(BIT7);			// Set P1.7 to input direction
    P1DIR |= (BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6);
   // P1REN = P1REN | (BIT7);
    P1OUT &= ~(BIT0);
   // P1OUT |= BIT7;

    P2SEL = P1SEL & ~(BIT0|BIT1|BIT2|BIT3|BIT4|BIT5|BIT6|BIT7);  // Select P1.0 for digital IO
    P2DIR |= (BIT0|BIT1|BIT3|BIT4|BIT5|BIT6);
    P2DIR &= ~(BIT2);			// Set P2.2 to input direction
    P2REN = P2REN | (BIT2);
    keypressed_state = CapButtonRead();
    while(1)
      {
    	 switch(state){
    	// CASE 0 - WELCOME SCREEN
    	case 0:
    	// clear display
    	GrClearDisplay(&g_sContext);
    	// write welcome message
    	GrStringDrawCentered(&g_sContext, "WELCOME TO", AUTO_STRING_LENGTH, 51, 16, TRANSPARENT_TEXT);
    	GrStringDrawCentered(&g_sContext, "MSP430 HERO", AUTO_STRING_LENGTH, 51, 32, TRANSPARENT_TEXT);
    	GrStringDrawCentered(&g_sContext, "- press button 1 -", AUTO_STRING_LENGTH, 51, 48, TRANSPARENT_TEXT);
    	// refresh screen
    	GrFlush(&g_sContext);
    	for(k=0; k<16; k++)
    		{
    			n[k]=0;
    		}
    	state = 1 ;
        break;


    	case 1:
    	button1 = !chkBtns1();
    	if(button1)
    	{
    	//starts the countdown if the button 1 is pressed
        // THREE
    	GrClearDisplay(&g_sContext);
    	GrStringDrawCentered(&g_sContext, "_____________________________", AUTO_STRING_LENGTH, 51, 5, TRANSPARENT_TEXT);
    	GrStringDrawCentered(&g_sContext, "              3             ", AUTO_STRING_LENGTH, 51, 25, TRANSPARENT_TEXT);
    	GrStringDrawCentered(&g_sContext, "_____________________________", AUTO_STRING_LENGTH, 51, 45, TRANSPARENT_TEXT);
    	GrFlush(&g_sContext);
       	configLED1_3(00000001);
       	runtimerA2();
       	timer_on=1;
       	while(timer<100) {}
       	stoptimerA2(1);

    	// TWO
    	GrClearDisplay(&g_sContext);
    	GrStringDrawCentered(&g_sContext, "_____________________________", AUTO_STRING_LENGTH, 51, 5, TRANSPARENT_TEXT);
    	GrStringDrawCentered(&g_sContext, "              2!             ", AUTO_STRING_LENGTH, 51, 25, TRANSPARENT_TEXT);
    	GrStringDrawCentered(&g_sContext, "_____________________________", AUTO_STRING_LENGTH, 51, 45, TRANSPARENT_TEXT);
    	GrFlush(&g_sContext);
    	configLED1_3(11111010);
    	runtimerA2();
    	timer_on=1;
    	while(timer<100) {}
    	stoptimerA2(1);


    	// ONE
    	GrClearDisplay(&g_sContext);
    	GrStringDrawCentered(&g_sContext, "_____________________________", AUTO_STRING_LENGTH, 51, 5, TRANSPARENT_TEXT);
    	GrStringDrawCentered(&g_sContext, "              1!!            ", AUTO_STRING_LENGTH, 51, 25, TRANSPARENT_TEXT);
    	GrStringDrawCentered(&g_sContext, "_____________________________", AUTO_STRING_LENGTH, 51, 45, TRANSPARENT_TEXT);
    	GrFlush(&g_sContext);
    	configLED1_3(11111100);
    	runtimerA2();
    	timer_on=1;
    	while(timer<100) {}
    	stoptimerA2(1);

    	// GO
    	GrClearDisplay(&g_sContext);
    	GrStringDrawCentered(&g_sContext, "_____________________________", AUTO_STRING_LENGTH, 51, 5, TRANSPARENT_TEXT);
    	GrStringDrawCentered(&g_sContext, "             GOO!!           ", AUTO_STRING_LENGTH, 51, 25, TRANSPARENT_TEXT);
    	GrStringDrawCentered(&g_sContext, "_____________________________", AUTO_STRING_LENGTH, 51, 45, TRANSPARENT_TEXT);
    	GrFlush(&g_sContext);
    	configLED1_3(11111111);
    	runtimerA2();
    	timer_on=1;
    	while(timer<200) {}
    	stoptimerA2(1);
    	GrClearDisplay(&g_sContext);
    	GrFlush(&g_sContext);
    	configLED1_3(00000000);
    	state=2;
    	runtimerA2();
    	timer_on=1;
    	}
        break;

    	case 2:
    	keypressed_state = CapButtonRead();
    	//NOTE 1
    	while(timer<50)
    	{ buzzeron(63);
    	P1OUT |= BIT1;
    	keypressed_state = CapButtonRead();
    	if(keypressed_state==1) {n[0]=1;}}
    	while((timer>100)&&(timer<150))
    	{ BuzzerOff(); P1OUT &= 11100000;}
    	//NOTE 2
    	while((timer>150)&&(timer<200))
    	{keypressed_state = CapButtonRead(); buzzeron(56); P1OUT |= BIT2;
    	if(keypressed_state==2) {n[1]=1;}}
    	while((timer>200)&&(timer<300))
    	{ BuzzerOff(); P1OUT &= 11100000;}
    	//NOTE 3
    	while((timer>300)&&(timer<350))
    	{ keypressed_state = CapButtonRead();buzzeron(53); P1OUT |= BIT3;
    	if(keypressed_state==4) {n[2]=1;}}
    	while((timer>350)&&(timer<450))
    	{ BuzzerOff(); P1OUT &= 11100000; }
    	//NOTE 4
    	while((timer>450)&&(timer<500))
    	{ keypressed_state = CapButtonRead();buzzeron(63); P1OUT |= BIT1;
    	if(keypressed_state==1) {n[3]=1;}}
    	while((timer>500)&&(timer<550))
    	{ BuzzerOff(); P1OUT &= 11100000;}
    	//NOTE 5
    	while((timer>550)&&(timer<600))
    	{ keypressed_state = CapButtonRead();buzzeron(56); P1OUT |= BIT2;
    	if(keypressed_state==2) {n[4]=1;}}
    	while((timer>600)&&(timer<650))
    	{ BuzzerOff(); P1OUT &= 11100000;}
    	//NOTE 6
    	while((timer>650)&&(timer<700))
    	{ keypressed_state = CapButtonRead();buzzeron(47); P1OUT |= BIT4;
    	if(keypressed_state==8) {n[5]=1;}}
    	while((timer>750)&&(timer<800))
    	{ BuzzerOff(); P1OUT &= 11100000;}
    	//NOTE 7
    	while((timer>800)&&(timer<850))
    	{ keypressed_state = CapButtonRead();buzzeron(53); P1OUT |= BIT3;
    	if(keypressed_state==4) {n[6]=1;}}
    	while((timer>850)&&(timer<950))
    	{ BuzzerOff(); P1OUT &= 11100000;}
    	//NOTE 8
    	while((timer>950)&&(timer<1000))
    	{ keypressed_state = CapButtonRead();buzzeron(63); P1OUT |= BIT1;
    	if(keypressed_state==1) {n[7]=1;}}
    	while((timer>1000)&&(timer<1050))
    	{ BuzzerOff(); P1OUT &= 11100000;}
    	//NOTE 9
    	while((timer>1050)&&(timer<1100))
    	{ keypressed_state = CapButtonRead();buzzeron(56); P1OUT |= BIT2;
    	if(keypressed_state==2) {n[8]=1;}}
    	while((timer>1100)&&(timer<1150))
    	{ BuzzerOff(); P1OUT &= 11100000; }
    	//NOTE 10
    	while((timer>1150)&&(timer<1200))
    	{ keypressed_state = CapButtonRead();buzzeron(53); P1OUT |= BIT3;
    	if(keypressed_state==4) {n[9]=1;}}
    	while((timer>1200)&&(timer<1250))
    	{ BuzzerOff(); P1OUT &= 11100000; }
    	//NOTE 11
    	while((timer>1250)&&(timer<1300))
    	{ keypressed_state = CapButtonRead();buzzeron(56); P1OUT |= BIT2;
    	if(keypressed_state==2) {n[10]=1;}}
    	while((timer>1300)&&(timer<1350))
    	{ BuzzerOff(); P1OUT &= 11100000; }
    	//NOTE 12
    	while((timer>1350)&&(timer<1400))
    	{ keypressed_state = CapButtonRead();buzzeron(63); P1OUT |= BIT1;
    	if(keypressed_state==1) {n[11]=1;}}
    	if(timer>1400) {
    	BuzzerOff();
    	P1OUT &= 11110000;
    	stoptimerA2(0);
    	state = 3;
    	}
    	button2 = !chkBtns2();
    	if(button2) {state =0; BuzzerOff(); P1OUT &= 11100000; stoptimerA2(1);}
    	break;

    	case 3:
    	for(k=0; k<16; k++)
    	{
    	if (n[k]==1) {count++;}
    	}

    	if(count>8)
    	{
    	stoptimerA2(1);
    	state =4;
    	}
    	else
    	{
    	stoptimerA2(1);
    	state =5;
    	}
    	break;

    	case 4:
    	GrClearDisplay(&g_sContext);
    	GrStringDrawCentered(&g_sContext, "CONGRATULATIONS!", AUTO_STRING_LENGTH, 51, 5, TRANSPARENT_TEXT);
    	GrStringDrawCentered(&g_sContext, "YOU", AUTO_STRING_LENGTH, 51, 25, TRANSPARENT_TEXT);
    	GrStringDrawCentered(&g_sContext, "ROCK!", AUTO_STRING_LENGTH, 51, 45, TRANSPARENT_TEXT);
    	GrFlush(&g_sContext);
    	runtimerA2();
    	timer_on=1;
    	BuzzerOff();
    	while(timer<100)  {buzzeron(59); P1OUT |= 00011111; P8OUT |= (BIT2|BIT1);}
    	while((timer>90)&&(timer<150))  {BuzzerOff(); P1OUT &= 00000000; P8OUT &= 000;}
    	while((timer>149)&&(timer<200)) {buzzeron(56); P1OUT |= 00011111;P8OUT |= (BIT1|BIT2);}
    	while((timer>199)&&(timer<250))  {BuzzerOff(); P1OUT &= 00000000; P8OUT &= 000;}
    	while((timer>249)&&(timer<300)) {buzzeron(53); P1OUT |= 00011111;P8OUT |= (BIT1|BIT2);}
    	while((timer>299)&&(timer<350))  {BuzzerOff(); P1OUT &= 00000000; P8OUT &= 000;}
    	while((timer>349)&&(timer<500)) {buzzeron(50); P1OUT |= 00011111;P8OUT |= (BIT1|BIT2);}
    	while((timer>499)&&(timer<800)) {BuzzerOff(); state = 0; stoptimerA2(1); P1OUT &= 00000000; P8OUT &= 000;}
    	break;

    	case 5:
    	GrClearDisplay(&g_sContext);
    	GrStringDrawCentered(&g_sContext, "SORRY,", AUTO_STRING_LENGTH, 51, 5, TRANSPARENT_TEXT);
    	GrStringDrawCentered(&g_sContext, "TRY HARDER", AUTO_STRING_LENGTH, 51, 25, TRANSPARENT_TEXT);
    	GrStringDrawCentered(&g_sContext, "NEXT TIME...", AUTO_STRING_LENGTH, 51, 45, TRANSPARENT_TEXT);
    	GrFlush(&g_sContext);
    	runtimerA2();
    	timer_on=1;
    	BuzzerOff();
    	while(timer<100)  {buzzeron(50); P1OUT |= 00001001; P8OUT |= (BIT2|BIT1);}
    	while((timer>90)&&(timer<150))  {BuzzerOff(); P1OUT &= 00000000; P8OUT &= 000;}
    	while((timer>149)&&(timer<200)) {buzzeron(53); P1OUT |= 00000110; P8OUT |= (BIT2|BIT1);}
    	while((timer>199)&&(timer<250))  {BuzzerOff(); P1OUT &= 00000000; P8OUT &= 000;}
    	while((timer>249)&&(timer<350)) {buzzeron(59); P1OUT |= 00011111; P8OUT |= (BIT2|BIT1); }
    	while((timer>349)&&(timer<500)) {BuzzerOff(); state = 0; stoptimerA2(1); P1OUT &= 00000000; P8OUT &= 000;}
        break;


    	}


      }

}



