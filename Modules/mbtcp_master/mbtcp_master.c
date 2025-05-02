#include "mbtcp_master.h"
#include <string.h>

#define DBG_TAG "mbtcp.master"
#define DBG_LVL DBG_INFO
#include <agile_dbg.h>

#define MBTCP_MASTER_CYCLE_TIMEOUT     5
#define MBTCP_MASTER_RECONNECT_TIMEOUT 100
#define MBTCP_MASTER_CONNECT_TIMEOUT   1000

static void mbtcp_master_close(mbtcp_master_t *master);

static err_t _tcp_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    mbtcp_master_t *master = (mbtcp_master_t *)arg;
    agile_modbus_t *ctx = &master->ctx_tcp._ctx;

    if (p == NULL) {
        LOG_E("Received NULL pbuf, remote has closed connection.");
        mbtcp_master_close(master);
        return ERR_ABRT;
    } else {
        if (err != ERR_OK) {
            LOG_E("Recv err: %d", err);
            pbuf_free(p);
            mbtcp_master_close(master);
            return ERR_ABRT;
        }

        tcp_recved(tpcb, p->tot_len);
        if (master->req.state == MBTCP_REQUEST_STATE_PENDING) {
            int copy_len = ctx->read_bufsz - master->req.recv_len;
            if (copy_len > p->tot_len) {
                copy_len = p->tot_len;
            }

            if (copy_len > 0) {
                pbuf_copy_partial(p, ctx->read_buf + master->req.recv_len, copy_len, 0);
                master->req.recv_len += copy_len;
            }

            int rc = agile_modbus_deserialize_raw_response(ctx, master->req.recv_len);
            if (rc >= 0) {
                master->req.state = MBTCP_REQUEST_STATE_SUCCESS;
            } else if (rc != -1 || master->req.recv_len >= ctx->read_bufsz) {
                master->req.state = MBTCP_REQUEST_STATE_FAILED;
            }
        }
        pbuf_free(p);
    }

    return ERR_OK;
}

static err_t _tcp_connected_cb(void *arg, struct tcp_pcb *tpcb, err_t err) {
    mbtcp_master_t *master = (mbtcp_master_t *)arg;

    if (err != ERR_OK) {
        LOG_E("Connect failed.");
        return err;
    }

    tcp_recv(tpcb, _tcp_recv_cb);

    master->state = MBTCP_MASTER_STATE_CONNECTED;
    LOG_I("Connected to %s:%d", ipaddr_ntoa(&tpcb->remote_ip), tpcb->remote_port);

    return ERR_OK;
}

static void _tcp_err_cb(void *arg, err_t err) {
    mbtcp_master_t *master = (mbtcp_master_t *)arg;

    LOG_E("TCP error: %d", err);

    master->conn = NULL;
    mbtcp_master_close(master);
}

static void mbtcp_master_connect(mbtcp_master_t *master) {
    err_t err;

    master->conn = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (master->conn == NULL) {
        LOG_E("Create tcp connection failed.");
        goto tcp_fail;
    }

    tcp_arg(master->conn, master);
    err = tcp_bind(master->conn, IP_ADDR_ANY, 0);
    if (err != ERR_OK) {
        LOG_E("Bind tcp connection failed.");
        goto tcp_fail;
    }

    err = tcp_connect(master->conn, &master->remote_ip, master->remote_port, _tcp_connected_cb);
    if (err != ERR_OK) {
        LOG_E("Connect to %s:%d failed.", ipaddr_ntoa(&master->remote_ip), master->remote_port);
        goto tcp_fail;
    }
    tcp_err(master->conn, _tcp_err_cb);

    master->timeout = HAL_GetTick() + MBTCP_MASTER_CONNECT_TIMEOUT;
    master->state = MBTCP_MASTER_STATE_CONNECTING;
    return;

tcp_fail:
    mbtcp_master_close(master);
}

static void mbtcp_master_close(mbtcp_master_t *master) {
    if (master->conn != NULL) {
        struct tcp_pcb *pcb = master->conn;
        master->conn = NULL;

        tcp_arg(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_err(pcb, NULL);
        tcp_poll(pcb, NULL, 0);
        tcp_sent(pcb, NULL);
        tcp_abort(pcb);
    }
    master->timeout = HAL_GetTick() + MBTCP_MASTER_RECONNECT_TIMEOUT;
    master->state = MBTCP_MASTER_STATE_CLOSED;

    if (master->req.state == MBTCP_REQUEST_STATE_PENDING) {
        master->req.state = MBTCP_REQUEST_STATE_FAILED;
    }
}

static void _cycle_timer(void *arg) {
    mbtcp_master_t *master = (mbtcp_master_t *)arg;
    agile_modbus_t *ctx = &master->ctx_tcp._ctx;

    switch (master->state) {
        case MBTCP_MASTER_STATE_CLOSED: {
            if ((HAL_GetTick() - master->timeout) < (HAL_MAX_DELAY / 2)) {
                mbtcp_master_connect(master);
            }
        } break;

        case MBTCP_MASTER_STATE_CONNECTING: {
            if ((HAL_GetTick() - master->timeout) < (HAL_MAX_DELAY / 2)) {
                LOG_E("Connecting timeout, now close.");
                mbtcp_master_close(master);
            }
        } break;

        case MBTCP_MASTER_STATE_CONNECTED: {
            if (master->req.state == MBTCP_REQUEST_STATE_SEND) {
                err_t err = tcp_write(master->conn, ctx->send_buf, master->req.send_len, TCP_WRITE_FLAG_COPY);
                if (err == ERR_OK) {
                    tcp_output(master->conn);
                    master->req.state = MBTCP_REQUEST_STATE_PENDING;
                } else {
                    master->req.state = MBTCP_REQUEST_STATE_FAILED;
                    mbtcp_master_close(master);
                }
            }
        } break;

        default:
            mbtcp_master_close(master);
            break;
    }

    sys_timeout(MBTCP_MASTER_CYCLE_TIMEOUT, _cycle_timer, master);
}

int mbtcp_master_start(mbtcp_master_t *master, const char *ip, uint16_t port) {
    if (master == NULL) return -1;
    if (master->state != MBTCP_MASTER_STATE_CLOSED) return -1;

    memset(master, 0, sizeof(mbtcp_master_t));
    agile_modbus_tcp_init(&master->ctx_tcp, master->ctx_sendbuf, sizeof(master->ctx_sendbuf), master->ctx_readbuf, sizeof(master->ctx_readbuf));
    ipaddr_aton(ip, &master->remote_ip);
    master->remote_port = port;

    mbtcp_master_connect(master);
    sys_timeout(MBTCP_MASTER_CYCLE_TIMEOUT, _cycle_timer, master);

    return 0;
}

int mbtcp_master_stop(mbtcp_master_t *master) {
    if (master == NULL) return -1;

    mbtcp_master_close(master);
    sys_untimeout(_cycle_timer, master);

    return 0;
}

int mbtcp_master_read_registers(mbtcp_master_t *master, int addr, int nb, uint16_t *dest, uint32_t timeout) {
    if (master == NULL) return MBTCP_MASTER_OPT_FAIL;

    agile_modbus_t *ctx = &master->ctx_tcp._ctx;
    int status = MBTCP_MASTER_OPT_BUSY;

    if (master->req.state == MBTCP_REQUEST_STATE_NULL) {
        int rc = agile_modbus_serialize_read_registers(ctx, addr, nb);
        if (rc < 0) return MBTCP_MASTER_OPT_FAIL;
        master->req.send_len = rc;
        master->req.recv_len = 0;
        master->req.state = MBTCP_REQUEST_STATE_SEND;
        master->req.timeout = HAL_GetTick() + timeout;
    }

    if (master->req.state == MBTCP_REQUEST_STATE_SUCCESS) {
        if (dest != NULL) {
            agile_modbus_deserialize_read_registers(ctx, master->req.recv_len, dest);
        }
        status = MBTCP_MASTER_OPT_OK;
    } else if (master->req.state == MBTCP_REQUEST_STATE_FAILED) {
        status = MBTCP_MASTER_OPT_FAIL;
    } else {
        if ((HAL_GetTick() - master->req.timeout) < (HAL_MAX_DELAY / 2)) {
            status = MBTCP_MASTER_OPT_TIMEOUT;
        }
    }

    if (status != MBTCP_MASTER_OPT_BUSY) {
        master->req.state = MBTCP_REQUEST_STATE_NULL;
    }

    return status;
}
