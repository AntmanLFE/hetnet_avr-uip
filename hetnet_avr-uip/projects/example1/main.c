#include <stdio.h>
#include <stdlib.h>

#include <avr/io.h>
#include <util/delay.h>

#include "timer.h"
#include "global-conf.h"
#include "uip_arp.h"
#include "network.h"
#include "enc28j60.h"
#include "uart.h"

#include <string.h>
#define BUF ((struct uip_eth_hdr *)&uip_buf[0])


/* === MACROS AND DATA ===*/
/* --- LedPort handling ---*/
#define LED_bm (1<<PORTC0)
#define LEDPORT PORTC
#define LEDPORT_D DDRC
#define led_conf()    LEDPORT_D |= LED_bm
#define led_on() 	    LEDPORT |= LED_bm
#define led_off()     LEDPORT &= ~LED_bm
#define led_toggle()  LEDPORT ^= LED_bm

/* --- Protothread and timers ---*/
static struct pt blink_thread;
static struct timer blink_timer;
static struct pt greetings_thread;
static struct timer greeting_timer;

int setup;
uint16_t range;


/* === PROTOTHREADS  ===*/
/* ---  Blink protothread ---*/
static 
PT_THREAD(blink(void))
{
	PT_BEGIN(&blink_thread);

	led_on();
	timer_set(&blink_timer, CLOCK_CONF_SECOND/2);
	PT_WAIT_UNTIL(&blink_thread, 
			timer_expired(&blink_timer));

	led_off();
	timer_set(&blink_timer, CLOCK_CONF_SECOND/2);
	PT_WAIT_UNTIL(&blink_thread,
			timer_expired(&blink_timer));

	PT_END(&blink_thread);
}

/* ---Simple greetings protothread ---*/
static
PT_THREAD(greetings(void))
{
	PT_BEGIN(&greetings_thread);
	
	timer_set(&greeting_timer, CLOCK_CONF_SECOND*2);
	PT_WAIT_UNTIL(&greetings_thread,
			timer_expired(&greeting_timer));
	printf("Hello, AdmStaff!!\n");

	PT_END(&greetings_thread);
}
	


/* === MAIN === */
int main(int argc, char *argv[])
{
	/*9600, 2x, @8Mhz*/
	uint16_t brate = 103; 
	int i;

	/*--- device setup ---
	 *	- clock
	 *	- usart
	 *	- protothread
	 */

	clock_init();

	usart_init(brate);
	usart_redirect_stdout();
	printf("\n\n********** HETNETv0.1 ****************\n\n");
	printf("Starting uIP \n");
	printf("Configuring clock \n");
	led_conf();
	printf("Configuring led \n");


	PT_INIT(&blink_thread);
	PT_INIT(&greetings_thread);
	blink();

	while(1){
		blink();
		greetings();
	}
}
