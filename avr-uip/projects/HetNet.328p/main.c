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

/* --- Led handling functions ---*/
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


/* ---  Blink protothread ---*/
static 
PT_THREAD(blink(void))
{
	PT_BEGIN(&blink_thread);

	led_on();
	printf("led on!\n");
	timer_set(&blink_timer, CLOCK_CONF_SECOND);
	PT_WAIT_UNTIL(&blink_thread, 
			timer_expired(&blink_timer));

	led_off();
	printf("led off \n");
	timer_set(&blink_timer, CLOCK_CONF_SECOND);
	PT_WAIT_UNTIL(&blink_thread,
			timer_expired(&blink_timer));

	PT_END(&blink_thread);
}

/* === MAIN === */
int main(int argc, char *argv[])
{
	/*9600, 2x, @8Mhz*/
	uint16_t brate = 103; 

	/*--- device setup ---*/

	clock_init();
	led_conf();
	usart_init(brate);
	usart_redirect_stdout();
	printf("Starting uIP \n");
	PT_INIT(&blink_thread);

	for(;;){
		blink();
	}
}
