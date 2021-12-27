#include "msp.h"
#include <stdio.h>

#define DELAY_VALUE 100

void UART0_init(void);
void UART0_putchar(char c);
void UART0_puts(char* s);
void config_switches(void);
void config_leds(void);

static uint8_t volatile currentLED;
static char buffer[80];

int main() {	
	// Stop watchdog timer.
	WDT_A -> CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;

	// RED LED is selected by default.
	currentLED = 0;
	
	//intializes switches and LEDs.
	config_switches();
	config_leds();
	
	//Device interrupt configuration.
	P1IES |= (uint8_t)0x12;
	P1IFG &= (uint8_t)~0x12;
	P1IE |= (uint8_t)0x12;
	
	//UART0 initialization
	UART0_init(); 

	//NVIC configuration.
	// Configuring the buttons.
	NVIC_SetPriority(PORT1_IRQn,2);
	NVIC_ClearPendingIRQ(PORT1_IRQn);
	NVIC_EnableIRQ(PORT1_IRQn);

	// Configuring the UART.
	NVIC_SetPriority(EUSCIA0_IRQn,2);
	NVIC_ClearPendingIRQ(EUSCIA0_IRQn);
	NVIC_EnableIRQ(EUSCIA0_IRQn);
	
	//Globally enable interrupts in CPU.
	__ASM("CPSIE I");
	
	while (1){
		//Waiting for interrupts.
		__ASM("WFI");
	}
}

/* UART0 is connected to virtual COM port through the USB debug connection */
void UART0_init(void) {
    EUSCI_A0->CTLW0 |= 1;             /* put in reset mode for config */
    EUSCI_A0->MCTLW = 0;              /* disable oversampling */
    EUSCI_A0->CTLW0 = 0x0081;         /* 1 stop bit, no parity, SMCLK, 8-bit data */
    EUSCI_A0->BRW = 26;               /* 3000000 / 115200 = 26 */
    P1->SEL0 |= 0x0C;                 /* P1.3, P1.2 for UART */
    P1->SEL1 &= ~0x0C;
    EUSCI_A0->CTLW0 &= ~1;            /* take UART out of reset mode */
	  EUSCI_A0->IE |= 1;                /* enable receive interrupt */
}    

void UART0_putchar(char c) {
    while(!(EUSCI_A0->IFG&0x02)) { }  /* wait for transmit buffer empty */
    EUSCI_A0->TXBUF = c;              /* send a char */
}

void UART0_puts(char* s) {
    while (*s != 0)             /* if not end of string */
    UART0_putchar(*s++);        /* send the character through UART0 */
}

	
void config_switches(){	
	//sets function to GPIO
	P1->SEL0 &= (uint8_t)(~((1<<1)|(1<<4)));
	P1->SEL1 &= (uint8_t)(~((1<<1)|(1<<4)));
	
	//sets direction of switches to inputs
	P1->DIR &= (uint8_t)(~((1<<1)|(1<<4)));
	
	//sets resistor enable 
	P1->REN |= (uint8_t)((1<<1)|(1<<4));
	
	//sets restistor to pull-up
	P1->OUT |= (uint8_t)((1<<1)|(1<<4));
}

void config_leds(){	
	//sets function to GPIO
	P1->SEL0 &= (uint8_t)(~(1<<0));
	P1->SEL1 &= (uint8_t)(~(1<<0));
	
	//sets direction of LED pin to outputs
	P1->DIR |= (uint8_t)(1<<0);
	
	//sets drive strength to default (regular) since it can be regular or high
	P1->DS &= (uint8_t)(~(1<<0));
	
	//sets default state to off
	P1->OUT &= (uint8_t)(~(1<<0));
	
	//sets function to GPIO for all 3 pins
	P2->SEL0 &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	P2->SEL1 &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	
	//sets direction of LED pins to outputs
	P2->DIR |= (uint8_t)((1<<0)|(1<<1)|(1<<2));
	
	//sets drive strength to default (regular) since it can be regular or high
	P2->DS &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	
	//sets default state of all pins to off
	P2->OUT &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
}

void PORT1_IRQHandler(void){
	//Test for pin 3 interrupt flag
	if((P1IFG & (uint8_t) 0x02) != 0){
		P1IFG &= (uint8_t)~0x02;
		sprintf(buffer, "UP\n");
    UART0_puts(buffer);
	} else if((P1IFG & (uint8_t) 0x16) != 0){
		P1IFG &= (uint8_t)~0x16;
		sprintf(buffer, "DOWN\n");
		UART0_puts(buffer);
	}
	
	// Debouncing the LEDs.
	static uint16_t i = DELAY_VALUE;
  while (i--) {
		// Delay loop contains some asm code placeholder
		__ASM volatile (""); 
  }
	
	sprintf(buffer, "DONE\n");
	UART0_puts(buffer);
}

void EUSCIA0_IRQHandler(void){
	// Check for RX flag (receive complete)
	char rx_data;
	if(EUSCI_A0->IFG & 0x01){
		EUSCI_A0->IFG &= ~0x01; // clear flag
		rx_data = EUSCI_A0->RXBUF; // copy data
		
		if(rx_data == 'G'){
			P1 -> OUT |= (uint8_t) ((1 << 0));
		} 
	}
} // end uart rx int handler
