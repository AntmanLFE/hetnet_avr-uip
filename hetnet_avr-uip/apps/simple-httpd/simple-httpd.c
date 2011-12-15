#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "simple-httpd.h"
#include "uip.h"

static int handle_connection(struct simple_httpd_state *s);

void simple_httpd_init(void)
{
	printf("Init simple Http App\n");
	uip_listen(HTONS(80));
}


/* === This is application entry point === */
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
	printf("Handle Connection\n");
  PSOCK_BEGIN(&s->p);
  PSOCK_SEND_STR(&s->p, "HTTP/1.0 200 OK\r\n");
  PSOCK_SEND_STR(&s->p, "Content-Type: text/html\r\n");
  PSOCK_SEND_STR(&s->p, "\r\n");
  PSOCK_SEND_STR(&s->p, "Hello World, From a simple httpd.");
  PSOCK_SEND_STR(&s->p, "\r\n");
  PSOCK_CLOSE(&s->p);
  PSOCK_END(&s->p);
}

