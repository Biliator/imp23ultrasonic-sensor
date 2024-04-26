/* Header file with all the essential definitions for a given type of MCU */
#include "MK60D10.h"

/* Macros for bit-level registers manipulation */
#define GPIO_PIN_MASK            0x1Fu
#define GPIO_PIN(x)              (((1)<<(x & GPIO_PIN_MASK)))

/* Constants specifying delay loop duration */
#define tdelay1 32768
#define tdelay2 10

/* Constants for activation of individual display positions */
#define C1_SEL 0xE000
#define C2_SEL 0xB000
#define C3_SEL 0x7000
#define C4_SEL 0xD000

/* Constants for showing of the digits */
#define N0 0xECC
#define N1 0x444
#define N2 0xBC4
#define N3 0xF44
#define N4 0x54C
#define N5 0xF48
#define N6 0xFC8
#define N7 0x644
#define N8 0xFCC
#define N9 0x74C

/* Bit-level masks that help to enable/disable DP segment on LED display */
#define MASK_DOT_ON 0x040
#define MASK_DOT_OFF 0xFBF


int show_dot = 0;
int display_selection =  1;
int number_test = 0;


/* Auxiliary global variable */
int counter = 0;


/* Just an ordinary delay loop */
void delay(long long bound) {

  long long i;
  for(i=0;i<bound;i++);
}


/* Let's turn off individual segments on the whole display */
void off() {

  PTA->PDOR = GPIO_PDOR_PDO(0x0000);

  PTD->PDOR = GPIO_PDOR_PDO(C1_SEL);
  PTD->PDOR = GPIO_PDOR_PDO(C2_SEL);
  PTD->PDOR = GPIO_PDOR_PDO(C3_SEL);
  PTD->PDOR = GPIO_PDOR_PDO(C4_SEL);
}


/* Basic initialization of GPIO features on PORTA and PORTD */
void ports_init (void)
{
  SIM->SCGC5 = ( SIM_SCGC5_PORTA_MASK | SIM_SCGC5_PORTD_MASK | SIM_SCGC5_PORTE_MASK);  // Turn on clocks for PORTA and PORTB

  /* Set corresponding PORTA pins for GPIO functionality */
  PORTA->PCR[2] = ( 0|PORT_PCR_MUX(0x01) ); // B
  PORTA->PCR[3] = ( 0|PORT_PCR_MUX(0x01) ); // F
  PORTA->PCR[6] = ( 0|PORT_PCR_MUX(0x01) ); // DP
  PORTA->PCR[7] = ( 0|PORT_PCR_MUX(0x01) ); // E
  PORTA->PCR[8] = ( 0|PORT_PCR_MUX(0x01) ); // G
  PORTA->PCR[9] = ( 0|PORT_PCR_MUX(0x01) ); // A
  PORTA->PCR[10] = ( 0|PORT_PCR_MUX(0x01) ); // C
  PORTA->PCR[11] = ( 0|PORT_PCR_MUX(0x01) ); // D
  PORTA->PCR[26] = ( 0|PORT_PCR_MUX(0x01) ); //
  PORTA->PCR[27] = ( 0|PORT_PCR_MUX(0x01) ); //

  /* Set corresponding PORTA port pins as outputs */
  PTA->PDDR = GPIO_PDDR_PDD( 0x8000FCC ); // "1" configures given pin as an output
  PTA->PDOR |= GPIO_PDOR_PDO( 0x000 );

   /* Set corresponding PORTD pins for GPIO functionality */
  PORTD->PCR[12] = ( 0|PORT_PCR_MUX(0x01) ); // C1
  PORTD->PCR[13] = ( 0|PORT_PCR_MUX(0x01) ); // C4
  PORTD->PCR[14] = ( 0|PORT_PCR_MUX(0x01) ); // C2
  PORTD->PCR[15] = ( 0|PORT_PCR_MUX(0x01) ); // C3

  /* Set corresponding PORTB port pins as outputs */
  PTD->PDDR = GPIO_PDDR_PDD( 0xF000 );
  PTD->PDOR |= GPIO_PDOR_PDO( 0xF000 );

  NVIC_DisableIRQ( PORTE_IRQn);  // Disable the eventual generation of the interrupt caused by the control button

  // Configure the Signal Multiplexer for SW6, configure SW6 to interrupt on falling edge
  PORTE->PCR[11] = PORT_PCR_MUX(0x01) | PORT_PCR_IRQC(0b1010); // SW6

  /* Let's clear any previously pending interrupt on PORTB and allow its subsequent generation */
  NVIC_ClearPendingIRQ( PORTE_IRQn);
  NVIC_EnableIRQ( PORTE_IRQn);
}


/* Single digit shown on a particular section of the display  */
void sn(int number, uint32_t display, int meter_dot) {

  uint32_t n;

  switch (number) {
    case 0:
      n = N0; break;
    case 1:
      n = N1; break;
    case 2:
      n = N2; break;
    case 3:
      n = N3; break;
    case 4:
      n = N4; break;
    case 5:
      n = N5; break;
    case 6:
      n = N6; break;
    case 7:
      n = N7; break;
    case 8:
      n = N8; break;
    case 9:
      n = N9; break;
    default:
      n = N0;
  }

  if (meter_dot)
    n |= MASK_DOT_ON;
  else
    n &= MASK_DOT_OFF;

  PTA->PDOR = GPIO_PDOR_PDO(n);
  PTD->PDOR = GPIO_PDOR_PDO(display);

  delay(10); //10
}

int turn_on = 0;
/*
 * vypne a zapne m��en� vzd�lenosti
 */
void PORTE_IRQHandler() {

	if (PORTE->ISFR & GPIO_PDIR_PDI(0x800)) {
	  if (!(PTE->PDIR & GPIO_PDIR_PDI(0x800))) {
	    if (!turn_on)
		  turn_on = 1;
		else
			turn_on = 0;
	  }
	  PORTE->PCR[11] |= PORT_PCR_ISF_MASK;  // Confirmation of interrupt after button press
	}
}

int main(void)
{
	ports_init();

	uint32_t pin_state = 0;
    int time = 0;

    for (;;) {
    	if (turn_on == 1)
    	{
    		// vy�le pulz pro zapo�et� m��en� o d�lce 10 us
			PTA->PDOR = GPIO_PDOR_PDO(0x8000000);
			delay(10);
			PTA->PDOR = GPIO_PDOR_PDO(0x0000000);

			// pokud je na Echo 0, tak �ekej
			while (!pin_state)
			{
				pin_state = (PTA->PDIR >> 26) & 0x01;
			}

			// pokud je na Echo 1, tak po��tej �as ve vno�en�m whilu, pot� vypi� v�sledek pouze
			// pokud je pulz o d�lce 100 us a� 25 ms

			while (pin_state)
			{
				pin_state = (PTA->PDIR >> 26) & 0x01;
				time++;
			}
			if (time < 25000 && time > 100) {
				// rychlost vyduchu je 331 m/s
				int distance = ((time / 2) * 331) / 1000;
				int p1 = (distance / 1000) % 10;
				int p2 = (distance / 100) % 10;
				int p3 = (distance / 10) % 10;
				int p4 = distance % 10;
				sn(p1, C1_SEL, 1);					// metr
				delay(10000);
				sn(p2, C2_SEL, 0);					// decimetr
				delay(10000);
				sn(p3, C3_SEL, 0);					// centimetr
				delay(10000);
				sn(p4, C4_SEL, 0);					// milimetr
				delay(10000);
			}
			time = 0;
    	}
    	else {
    		// vzpnut� stav
			sn(0, C1_SEL, 1);
    	}
    }

	return 0;
}
