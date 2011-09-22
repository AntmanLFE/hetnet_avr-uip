#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/eeprom.h>

#include "uip.h"
#include "httpd_custom.h"
#include "http-strings.h"


static int handle_connection(struct simple_httpd_state *s);
//static int handle_input(struct simple_httpd_state *);
//static int handle_output(struct simple_httpd_state *);

static char message[] = "The number stored in in EEPROM is: %d\r\n";
uint8_t EEMEM data;
volatile static uint8_t data_sram;

static  
PT_THREAD(handle_output(struct simple_httpd_state *s))
{
	PSOCK_BEGIN(&s->sockout);
	/*get value stored in eeprom and build string*/
	data_sram = eeprom_read_byte((uint8_t *)&data);
	sprintf(message,message, data_sram);

	printf("handle out1\n");

	PSOCK_SEND_STR(&s->sockout, "<html><head><title>:: A web page in an AVR ::</title> </head><body>");
	printf("handle out2\n");
	PSOCK_SEND_STR(&s->sockout, message);
	printf("handle out3\n");
	PSOCK_SEND_STR(&s->sockout, "\r\n");
	PSOCK_SEND_STR(&s->sockout, "<br> You have asked for: ");
	switch (s->state){
		case STATE_OUTPUT_TEMP:
			PSOCK_SEND_STR(&s->sockout, "Temperature</br>\n");
			PSOCK_SEND_STR(&s->sockout, "<br>Temp = 1</br>\n");
			break;
		case STATE_OUTPUT_DIST: 
			PSOCK_SEND_STR(&s->sockout, "Distance</br>\n");
			PSOCK_SEND_STR(&s->sockout, "<br>Dist = 2</br>\n");
			break;
	}

	/*
	PSOCK_SEND_STR(&s->sockout, "<form> Cambia numero: <input type=\"text\" />");
	PSOCK_SEND_STR(&s->sockout, "<input type=\"submit\" value=\"Submit\" /> </form><br>");
	*/
	PSOCK_SEND_STR(&s->sockout, "</body>");
	PSOCK_CLOSE(&s->sockout);
	s->state=DATA_SENT;
	PSOCK_END(&s->sockout);
}
 
static 
PT_THREAD(handle_input(struct simple_httpd_state *s))
{
  PSOCK_BEGIN(&s->sockin);

  PSOCK_READTO(&s->sockin, ISO_space);
	printf("%s", s->buffin);
  
  if(strncmp(s->buffin, http_get, 4) != 0) {
    PSOCK_CLOSE_EXIT(&s->sockin);
  }
	/*here we get the GET parameter (to next space)*/
  PSOCK_READTO(&s->sockin, ISO_space);
	printf("\n%s", s->buffin);

	/*
	 * now sockin contains the get argouments
	 * Get couples of 'key=value' strings  in the sockin buff
	 * function is defined in websrv_helper_functions library
	char get_args_buf[4];
	find_key_val(&s->buffin, &get_args_buf, 4, "args");
	printf("\nKey value = %s\n", get_args_buf);
	*/

	/* Parse the string until we find the '=' char*/
	printf("Parse string\n");
	char *get_arg = (char *) &s->buffin;
	
	while ( *(get_arg++) != '=' );
	printf("string parsed: %s\n", get_arg);

  if(s->buffin[0] != ISO_slash) {
    PSOCK_CLOSE_EXIT(&s->sockin);
  }
  
	/* Determinate the output state*/
	if ( !strncmp(get_arg, GET_REQ_TEMP,
				strlen(GET_REQ_TEMP) )){
		s->state = STATE_OUTPUT_TEMP;

	}else if ( !strncmp(get_arg, GET_REQ_DIST,
				strlen(GET_REQ_DIST) )){
		s->state = STATE_OUTPUT_DIST;

	}else{
		/*default is temp*/
		s->state = STATE_OUTPUT_TEMP;
	}

  while(1) {
    PSOCK_READTO(&s->sockin, ISO_nl);
		printf("handle in5\n");
  }
  
	printf("handle in7\n");
  PSOCK_END(&s->sockin);
	printf("handle in8\n");
}

void simple_httpd_init(void)
{
	//printf("Init simple Http App\n");
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
	if (uip_closed() || uip_aborted() || uip_timedout()){
	}else if(uip_connected()) {
		printf("Connected\n");
		PSOCK_INIT(&s->sockin, s->buffin, sizeof(s->buffin));
		PSOCK_INIT(&s->sockout, s->buffout, sizeof(s->buffout));

		PT_INIT(&s->handle_output);
		PT_INIT(&s->handle_input);

		s->state = CONNECTED;
  }else if(s != NULL){
		if (uip_poll())
			{;} /*implement a timer*/
 		handle_connection(s);
	}else
		uip_abort();
}

int
handle_connection(struct simple_httpd_state *s)
{	
	if (uip_aborted() || uip_timedout() || uip_closed()) {;
	}
	else if (uip_rexmit()){
		printf("rexmit\n");
	}else if (uip_newdata()){
		printf("newdata\n");
	}else if (uip_acked()){
		printf("ack\n");
	}else if (uip_connected()){
		printf("connected 2\n");
	}else if (uip_poll()){
		printf("poll\n");
	}

  handle_input(s);
  if(s->state == STATE_OUTPUT) {
    handle_output(s);
 }
	return 0;
}
#if 0
	/*get value stored in eeprom and build string*/
	data_sram = eeprom_read_byte((uint8_t *)&data);
	sprintf(message,message, data_sram);

	PSOCK_BEGIN(&s->p);

	if (uip_aborted() || uip_timedout() || uip_closed()) {;
	}
	else if (uip_rexmit()){
		printf("rexmit\n");
	}else if (uip_newdata()){
		printf("newdata\n");
	}else if (uip_acked()){
		printf("ack\n");
	}else if (uip_connected()){
		printf("connected 2\n");
	}else if (uip_poll()){
		printf("poll\n");
	}

	printf("Handle Connection, state: %d\n", s->state);
	if (s->state == CONNECTED){
		PSOCK_SEND_STR(&s->p, "<html><head><title>:: A web page in an AVR ::</title> </head><body>");
		PSOCK_SEND_STR(&s->p, message);
		PSOCK_SEND_STR(&s->p, "\r\n");
		PSOCK_SEND_STR(&s->p, "<br>Cambia Numero in EEPROM.. <br>\n");

		PSOCK_SEND_STR(&s->p, "<form> Cambia numero: <input type=\"text\" />");
		PSOCK_SEND_STR(&s->p, "<input type=\"submit\" value=\"Submit\" /> </form><br>");

	//	PSOCK_READTO(&s->p, '\n');
	//	printf("User typed: %s", s->digit);
		PSOCK_SEND_STR(&s->p, "</body>");
		s->state = DATA_SENT; /*we have to wait an ack to do so*/
	}
	else if (s->state == DATA_SENT){
		PSOCK_READTO(&s->p, '\n');
		printf("User typed: %s", s->digit);

	}
#endif
