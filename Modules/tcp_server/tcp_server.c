#include "tcp_server.h"
#include <string.h>

#define DBG_TAG "tcp_server"
#define DBG_LVL DBG_INFO
#include <agile_dbg.h>

static void _add_session(tcp_server_t *server, tcp_server_session_t *session) {
    session->server = server;
    session->next = server->session;
    server->session = session;
}

static void _remove_session(tcp_server_session_t *session) {
    tcp_server_t *server = session->server;
    tcp_server_session_t *prev = NULL;
    tcp_server_session_t *curr = server->session;

    while (curr != NULL) {
        if (curr == session) {
            if (prev == NULL) {
                server->session = curr->next;
            } else {
                prev->next = curr->next;
            }
            break;
        }
        prev = curr;
        curr = curr->next;
    }
}

static void _session_close(tcp_server_session_t *session) {
    if (session->pcb != NULL) {
        struct tcp_pcb *pcb = session->pcb;
        session->pcb = NULL;

        tcp_arg(pcb, NULL);
        tcp_recv(pcb, NULL);
        tcp_err(pcb, NULL);
        tcp_poll(pcb, NULL, 0);
        tcp_sent(pcb, NULL);
        tcp_abort(pcb);
    }
    if (session->server->close_cb != NULL) {
        session->server->close_cb(session);
    }
    _remove_session(session);
    mem_free(session);
}

static err_t _tcp_recv_cb(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    tcp_server_session_t *session = (tcp_server_session_t *)arg;

    if (p == NULL) {
        LOG_E("Received NULL pbuf, remote has closed connection.");
        _session_close(session);
        return ERR_ABRT;
    } else {
        if (err != ERR_OK) {
            LOG_E("Recv err: %d", err);
            pbuf_free(p);
            _session_close(session);
            return ERR_ABRT;
        }

        int rc = 0;
        tcp_recved(tpcb, p->tot_len);
        if (session->server->recv_cb != NULL) {
            rc = session->server->recv_cb(session, p);
        }
        pbuf_free(p);

        if (rc != 0) {
            LOG_E("Receive callback failed: %d", rc);
            _session_close(session);
            return ERR_ABRT;
        }
    }

    return ERR_OK;
}

static void _tcp_err_cb(void *arg, err_t err) {
    tcp_server_session_t *session = (tcp_server_session_t *)arg;

    LOG_E("TCP error: %d", err);

    session->pcb = NULL;
    _session_close(session);
}

static err_t _tcp_accept_cb(void *arg, struct tcp_pcb *pcb, err_t err) {
    tcp_server_t *server = (tcp_server_t *)arg;

    if ((err != ERR_OK) || (pcb == NULL)) {
        return ERR_VAL;
    }

    tcp_server_session_t *session = (tcp_server_session_t *)mem_malloc(sizeof(tcp_server_session_t));
    if (session == NULL) {
        LOG_E("Session malloc failed.");
        return ERR_MEM;
    }
    memset(session, 0, sizeof(tcp_server_session_t));

    if (server->accept_cb != NULL) {
        int rc = server->accept_cb(session);
        if (rc != 0) {
            mem_free(session);
            return ERR_MEM;
        }
    }

    _add_session(server, session);
    session->pcb = pcb;

    tcp_arg(pcb, session);
    tcp_recv(pcb, _tcp_recv_cb);
    tcp_err(pcb, _tcp_err_cb);

    return ERR_OK;
}

err_t tcp_server_session_send(tcp_server_session_t *session, uint8_t *buf, uint32_t size) {
    if ((session == NULL) || (session->pcb == NULL)) return ERR_ARG;
    if (buf == NULL) return ERR_ARG;
    if (size == 0) return ERR_OK;

    err_t err = tcp_write(session->pcb, buf, size, TCP_WRITE_FLAG_COPY);
    if (err == ERR_OK) {
        tcp_output(session->pcb);
    } else {
        LOG_E("Send failed: %d", err);
    }

    return err;
}

void tcp_server_session_close(tcp_server_session_t *session) {
    if (session == NULL) return;
    _session_close(session);
}

int tcp_server_start(tcp_server_t *server) {
    err_t err;
    struct tcp_pcb *pcb = NULL;

    if (server == NULL) return -1;

    pcb = tcp_new_ip_type(IPADDR_TYPE_V4);
    if (pcb == NULL) return -1;

    ip_set_option(pcb, SOF_REUSEADDR);
    err = tcp_bind(pcb, IP_ADDR_ANY, server->port);
    if (err != ERR_OK) {
        LOG_E("Bind failed: %d", err);
        tcp_abort(pcb);
        pcb = NULL;
        return -1;
    }

    server->lpcb = tcp_listen(pcb);
    if (server->lpcb == NULL) {
        LOG_E("Listen failed.");
        tcp_abort(pcb);
        pcb = NULL;
        return -1;
    }
    tcp_arg(server->lpcb, server);
    tcp_accept(server->lpcb, _tcp_accept_cb);

    return 0;
}

int tcp_server_stop(tcp_server_t *server) {
    if (server == NULL) return -1;

    if (server->lpcb != NULL) {
        tcp_arg(server->lpcb, NULL);
        tcp_accept(server->lpcb, NULL);
        tcp_close(server->lpcb);
        server->lpcb = NULL;
    }

    while (server->session != NULL) {
        _session_close(server->session);
    }

    return 0;
}
