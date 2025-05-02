#ifndef __MBTCP_MASTER_H
#define __MBTCP_MASTER_H

#include "lwip/timeouts.h"
#include "lwip/ip_addr.h"
#include "lwip/mem.h"
#include "lwip/err.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include <agile_modbus.h>
#include "system.h"

enum {
    MBTCP_MASTER_STATE_CLOSED = 0,
    MBTCP_MASTER_STATE_CONNECTING,
    MBTCP_MASTER_STATE_CONNECTED,
};

enum {
    MBTCP_REQUEST_STATE_NULL = 0,
    MBTCP_REQUEST_STATE_SEND,
    MBTCP_REQUEST_STATE_PENDING,
    MBTCP_REQUEST_STATE_SUCCESS,
    MBTCP_REQUEST_STATE_FAILED,
};

enum {
    MBTCP_MASTER_OPT_OK = 0,
    MBTCP_MASTER_OPT_BUSY,
    MBTCP_MASTER_OPT_FAIL,
    MBTCP_MASTER_OPT_TIMEOUT,
};

typedef struct _mbtcp_master_request {
    uint8_t state;
    int send_len;
    int recv_len;
    uint32_t timeout;
} mbtcp_master_request_t;

typedef struct _mbtcp_master {
    uint8_t ctx_sendbuf[AGILE_MODBUS_MAX_ADU_LENGTH];
    uint8_t ctx_readbuf[AGILE_MODBUS_MAX_ADU_LENGTH];
    agile_modbus_tcp_t ctx_tcp;
    struct tcp_pcb *conn;
    ip_addr_t remote_ip;
    uint16_t remote_port;
    mbtcp_master_request_t req;
    uint32_t timeout;
    uint8_t state;
} mbtcp_master_t;

int mbtcp_master_start(mbtcp_master_t *master, const char *ip, uint16_t port);
int mbtcp_master_stop(mbtcp_master_t *master);
int mbtcp_master_read_registers(mbtcp_master_t *master, int addr, int nb, uint16_t *dest, uint32_t timeout);

#endif
