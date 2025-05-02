#include "com_tcp.h"
#include "tcp_server.h"
#include <string.h>
#include "task_run.h"

#define DBG_TAG "com_tcp"
#define DBG_LVL DBG_INFO
#include <agile_dbg.h>

typedef struct _com_tcp_client {
    tcp_server_session_t *session;
    uint8_t push_flag;
    uint8_t err_cnt;
} com_tcp_client_t;

static tcp_server_t _server = {0};
static com_tcp_client_t _clients[2] = {0};

static uint8_t _tmp_send_buf[1070];

static int _accept_cb(tcp_server_session_t *session) {
    for (int i = 0; i < sizeof(_clients) / sizeof(_clients[0]); i++) {
        if (_clients[i].session == NULL) {
            memset(&_clients[i], 0, sizeof(com_tcp_client_t));
            _clients[i].session = session;
            session->user_data = &_clients[i];
            return 0;
        }
    }

    return -1;
}

static int _close_cb(tcp_server_session_t *session) {
    com_tcp_client_t *client = (com_tcp_client_t *)session->user_data;
    if (client) {
        client->session = NULL;
    }

    return 0;
}

static int _recv_cb(tcp_server_session_t *session, struct pbuf *p) {
    com_tcp_client_t *client = (com_tcp_client_t *)session->user_data;
    if (client == NULL) {
        LOG_E("Client not found.");
        return -1;
    }

    char buf[32] = {0};
    pbuf_copy_partial(p, buf, sizeof(buf) - 1, 0);
    buf[sizeof(buf) - 1] = 0;

    if (strncmp(buf, "push", 4) == 0) {
        client->push_flag = 1;
    } else if (strncmp(buf, "stop", 4) == 0) {
        client->push_flag = 0;
    } else {
        LOG_E("Unknown command");
    }

    return 0;
}

#define TASK_RUN_PERIOD 50

static int com_tcp_entry(struct task_pcb *task) {
    for (int i = 0; i < sizeof(_clients) / sizeof(_clients[0]); i++) {
        if (_clients[i].session == NULL) continue;
        if (!_clients[i].push_flag) continue;

        err_t err = tcp_server_session_send(_clients[i].session, _tmp_send_buf, sizeof(_tmp_send_buf));
        if (err == ERR_OK) {
            _clients[i].err_cnt = 0;
        } else {
            _clients[i].err_cnt++;
            if (_clients[i].err_cnt > 5) {
                LOG_E("Send failed %d times, closing session.", _clients[i].err_cnt);
                tcp_server_session_close(_clients[i].session);
                _clients[i].session = NULL;
                _clients[i].err_cnt = 0;
            }
        }
    }

    return 0;
}

int com_tcp_init(void) {
    for (int i = 0; i < sizeof(_tmp_send_buf); i++) {
        _tmp_send_buf[i] = i + 1;
    }

    _server.port = 5000;
    _server.accept_cb = _accept_cb;
    _server.close_cb = _close_cb;
    _server.recv_cb = _recv_cb;

    LWIP_ASSERT("tcp_server_start: unexpected return value", tcp_server_start(&_server) == 0);

    task_init(TASK_COM_TCP, com_tcp_entry, NULL, TASK_RUN_PERIOD);
    task_start(TASK_COM_TCP);

    return 0;
}

