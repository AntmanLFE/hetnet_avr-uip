#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/eeprom.h>

#include "httpd_custom.h"
#include "uip.h"

static int handle_connection(struct simple_httpd_state *s);
static char message[] = "Simple httpd hello. stored in EEPROM: %d\r\n";
uint8_t EEMEM data;
volatile static uint8_t data_sram;

void simple_httpd_init(void)
{
	printf("Init simple Http App\n");
	eeprom_write_byte(&data, 10);
	uip_listen(HTONS(80));
}

#if defined PORT_APP_MAPPER
void simple_httpd_appcall(void)
{
	struct simple_httpd_state *s = &(httpd_state_list[0]);
#else
void simple_httpd_appcall(void)
{
	struct simple_httpd_state *s = &(uip_conn->appstate);
#endif
  if(uip_connected()) {
		PSOCK_INIT(&s->p, NULL, 0);
  }

  handle_connection(s);
}

static int handle_connection(struct simple_httpd_state *s)
{
	/*get value stored in eeprom and build string*/
	data_sram = eeprom_read_byte((uint8_t *)&data);
	sprintf(message,message, data_sram);

	printf("Handle Connection\n");
  PSOCK_BEGIN(&s->p);
  PSOCK_SEND_STR(&s->p, "HTTP/1.0 200 OK\r\n");
  PSOCK_SEND_STR(&s->p, "Content-Type: text/plain\r\n");
  PSOCK_SEND_STR(&s->p, "\r\n");
  PSOCK_SEND_STR(&s->p, message);
  PSOCK_CLOSE(&s->p);
  PSOCK_END(&s->p);
}

