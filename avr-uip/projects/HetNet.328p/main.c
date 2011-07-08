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
		timer_set(&blink_timer, CLOCK_CONF_SECOND);
		PT_WAIT_UNTIL(&blink_thread, 
				timer_expired(&blink_timer));

		led_off();
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
	int i;
	uip_ipaddr_t ipaddr;
	struct uip_eth_addr 
		mac = {UIP_ETHADDR0, UIP_ETHADDR1, 
			UIP_ETHADDR2, UIP_ETHADDR3,
			UIP_ETHADDR4, UIP_ETHADDR5};
	struct timer periodic_timer, arp_timer;

	/*--- device setup ---*/
	clock_init();
	timer_set(&periodic_timer, CLOCK_SECOND / 2);
	timer_set(&arp_timer, CLOCK_SECOND * 10);

	usart_init(brate);
	usart_redirect_stdout();
	printf("Starting uIP \n");
	printf("Configuring clock \n");
	led_conf();
	printf("Configuring led \n");
	network_init();
	printf("Configuring network \n");
	uip_init();
	printf("Configuring uIP stack \n");

	printf("Set EHT mac Address\n");
	uip_setethaddr(mac);

	simple_httpd_init();
	printf("Configuring HTTPD daemon \n");
	
  uip_ipaddr(ipaddr, 192,168,5,10);
  uip_sethostaddr(ipaddr);
  uip_ipaddr(ipaddr, 192,168,5,5);
  uip_setdraddr(ipaddr);
  uip_ipaddr(ipaddr, 255,255,255,0);
  uip_setnetmask(ipaddr);

	PT_INIT(&blink_thread);
	blink();


	while(1){
		blink();
		uip_len = network_read();

		if(uip_len > 0) {
			if(BUF->type == htons(UIP_ETHTYPE_IP)){
				uip_arp_ipin();
				uip_input();
				if(uip_len > 0) {
					uip_arp_out();
					network_send();
				}
			}else if(BUF->type == htons(UIP_ETHTYPE_ARP)){
				uip_arp_arpin();
				if(uip_len > 0){
					network_send();
				}
			}

		}else if(timer_expired(&periodic_timer)) {
			timer_reset(&periodic_timer);

			for(i = 0; i < UIP_CONNS; i++) {
				uip_periodic(i);
				if(uip_len > 0) {
					uip_arp_out();
					network_send();
				}
			}

			#if UIP_UDP
			for(i = 0; i < UIP_UDP_CONNS; i++) {
				uip_udp_periodic(i);
				if(uip_len > 0) {
					uip_arp_out();
					network_send();
				}
			}
			#endif /* UIP_UDP */

			if(timer_expired(&arp_timer)) {
				timer_reset(&arp_timer);
				uip_arp_timer();
			}
		}
	}
}
