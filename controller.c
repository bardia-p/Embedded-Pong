#include "msp.h"
#include <stdio.h>

/* Constants */
#define DELAY_VALUE 100 	//Used for the debouncing
#define RGB_LED_MASK 0x07 	//Used for the RGB LED

/* Function Prototypes */
// I/O configuration functions
void config_switches(void);
void config_leds(void);
void config_interrupts(void);

// Timer configuration
void config_TA0(void);

// UART configuration functions
void UART0_init(void);
void UART0_putchar(char c);
void UART0_puts(char* s);

// Helper functions
void handleRGBState(void);
void handleScoring(void);

/* Global Variables */
static char buffer[80]; 			//Keeps track of the serial buffer
static volatile uint8_t scoreTimerCount = 0; 	//Keeps track of interrupt count for the score LED.

/*
The main function for the program
*/
int main() {	
	// Stop watchdog timer.
	WDT_A -> CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
	
	//intializes switches and LEDs.
	config_switches();
	config_leds();
	
	// Configure interrupts
  	config_interrupts();
	
	// Configure Timer A0
	config_TA0();
	
	//UART0 initialization
	UART0_init(); 

	//NVIC configuration.
	// Configuring the buttons.
	NVIC_SetPriority(PORT1_IRQn,2);
	NVIC_ClearPendingIRQ(PORT1_IRQn);
	NVIC_EnableIRQ(PORT1_IRQn);

	// Configure NVIC for Timer A0
	NVIC_SetPriority(TA0_N_IRQn, 2);
	NVIC_ClearPendingIRQ(TA0_N_IRQn);
	NVIC_EnableIRQ(TA0_N_IRQn);
	
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


										/* Configurations */
/* 
Configure the switches (P1.1 and P1.4) as inputs, using pull-up internal resistors. 
*/
void config_switches(){	
	// Set function to GPIO (P1.1 and P1.4)
	P1->SEL0 &= (uint8_t)(~((1<<1)|(1<<4)));
	P1->SEL1 &= (uint8_t)(~((1<<1)|(1<<4)));

	// Set direction of switches to inputs
	P1->DIR &= (uint8_t)(~((1<<1)|(1<<4)));
	
	// Set pull resistor enable 
	P1->REN |= (uint8_t)((1<<1)|(1<<4));
	
	// Set resistor to pull-up
	P1->OUT |= (uint8_t)((1<<1)|(1<<4));
}

/*
Configure LEDs (P1.0 and P2.0, P2.1, P2.2) as outputs.
*/
void config_leds(){	
  /* LEDs P1.0 */
	// Set function to GPIO
	P1->SEL0 &= (uint8_t)(~(1<<0));
	P1->SEL1 &= (uint8_t)(~(1<<0));

	// Set direction of LED pin to outputs
	P1->DIR |= (uint8_t)(1<<0);
	
	// Set drive strength to default (regular) since it can be regular or high
	P1->DS &= (uint8_t)(~(1<<0));
	
	// Set default state of all pins to off
	P1->OUT &= (uint8_t)(~(1<<0));

    	// Disable interrupts
	P1->IE &= (uint8_t)(~(1<<0)); 

  /* LEDs P2.0, P2.1, P2.2 */
	// Set function to GPIO for all 3 pins
	P2->SEL0 &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	P2->SEL1 &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	
	// Set direction of LED pin to outputs
	P2->DIR |= (uint8_t)((1<<0)|(1<<1)|(1<<2));
	
	// Set drive strength to default (regular) since it can be regular or high
	P2->DS &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
	
	// Initialize LEDs states (all turned off)
	P2->OUT &= (uint8_t)(~((1<<0)|(1<<1)|(1<<2)));
}

/*
Configure switch interrupts at port level (device), NVIC, and CPU.
*/
void config_interrupts(void) {
  // Configure interrupts on falling edge
	P1->IES |= (uint8_t)((1<<1)|(1<<4));
	
	// Clear interrupt flags
	P1->IFG &= (uint8_t)~((1<<1)|(1<<4));
	
	// Interrupt pin enabled
	P1->IE |= (uint8_t)((1<<1)|(1<<4));
}

/*
Configure Timer A0.
*/
void config_TA0(void) {
  TA0CTL &= (uint16_t)(~((1<<5)|(1<<4)));   	// Stop the timer
	TA0CTL &= (uint16_t)(~(1<<0));          // Clear interrupt flag TAIFG
	TA0CCR0 = (uint16_t)(16384);            // Holds the upper limit value 1s
	TA0CTL |= (uint16_t)((1<<1));           // Interrupt enable TAIE
	TA0CTL |= (uint16_t)((1<<4));           // Up count Mode Control
	TA0CTL |= (uint16_t)((1<<8));           // ACLK as source for timer -> 32.768 kHz
	TA0CTL &= (uint16_t)(~(1<<9));		// Clear clock source TASSEL
}

/*
UART0 is connected to virtual COM port through the USB debug connection
*/
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

/*
Puts a character in the serial port
*/
void UART0_putchar(char c) {
    while(!(EUSCI_A0->IFG&0x02)) { }  /* wait for transmit buffer empty */
    EUSCI_A0->TXBUF = c;              /* send a char */
}

/*
Puts a string in the serial port 
*/
void UART0_puts(char* s) {
    while (*s != 0)             /* if not end of string */
    UART0_putchar(*s++);        /* send the character through UART0 */
}

										/* Interrupt Handlers */

/*
Port 1 interrupt handler
*/
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
	
	// Debouncing loop to make sure the button press was registered.
	static uint16_t i = DELAY_VALUE;
  	while (i--) {
		// Delay loop contains some asm code placeholder
		__ASM volatile (""); 
  	}
	
	sprintf(buffer, "DONE\n");
	UART0_puts(buffer);
}

/*
Timer A0 interrupt request handler.
*/
void TA0_N_IRQHandler(void) {
	// Clear the interrupt flag
	TA0CTL &= (uint16_t)(~(1<<0));
	
	// Change the state of the RGB LED
	handleRGBState();
	// Handles resetting the scoring LED.
	handleScoring();
}

/*
Interrupt handler for UART messages
*/
void EUSCIA0_IRQHandler(void){
	// Check for RX flag (receive complete)
	char rx_data;
	if(EUSCI_A0->IFG & 0x01){
		EUSCI_A0->IFG &= ~0x01; // clear flag
		rx_data = EUSCI_A0->RXBUF; // copy data
		
		// Turns the RED LED on to indicate someone scored a goal.
		if(rx_data == 'G'){
			scoreTimerCount = (uint8_t)1;
			P1 -> OUT |= (uint8_t) ((1 << 0));
		} 
	}
} // end uart rx int handler

										/* Helper functions */
/*
RGB_LED has 8 states, helper function to handle bitwise clearing and setting.
*/
void handleRGBState(void) {
	volatile uint8_t count = (P2->OUT & RGB_LED_MASK);
	count++;
	// Clear previous state
	P2->OUT &= ~(RGB_LED_MASK);
	// Set new state
	P2->OUT |= count & RGB_LED_MASK;
}

/*
Handles setting and resetting the score LED.
*/
void handleScoring(void){
	if (scoreTimerCount != (uint8_t)0){
		scoreTimerCount++;
	
		// Resets the score LED every 3 interrupts.
		if (scoreTimerCount == (uint8_t)4){
			scoreTimerCount = (uint8_t)0;
			P1 -> OUT &= (uint8_t) (~(1 << 0));
		}
	}
}
