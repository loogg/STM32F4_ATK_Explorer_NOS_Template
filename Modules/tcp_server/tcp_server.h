#ifndef __TCP_SERVER_H
#define __TCP_SERVER_H

#include "lwip/timeouts.h"
#include "lwip/ip_addr.h"
#include "lwip/mem.h"
#include "lwip/err.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include <agile_modbus.h>
#include "system.h"

typedef struct _tcp_server tcp_server_t;

typedef struct _tcp_server_session {
    tcp_server_t *server;
    struct tcp_pcb *pcb;
    void *user_data;
    struct _tcp_server_session *next;
} tcp_server_session_t;

struct _tcp_server {
    struct tcp_pcb *lpcb;
    tcp_server_session_t *session;
    uint16_t port;
    int (*accept_cb)(tcp_server_session_t *session);
    int (*close_cb)(tcp_server_session_t *session);
    int (*recv_cb)(tcp_server_session_t *session, struct pbuf *p);
};

int tcp_server_start(tcp_server_t *server);
int tcp_server_stop(tcp_server_t *server);
err_t tcp_server_session_send(tcp_server_session_t *session, uint8_t *buf, uint32_t size);
void tcp_server_session_close(tcp_server_session_t *session);

#endif
