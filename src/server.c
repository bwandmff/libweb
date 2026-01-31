#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#define BUFFER_SIZE 4096

typedef enum { TL_RED = 0, TL_GREEN = 1, TL_YELLOW = 2 } tl_state_t;

static struct {
    tl_state_t ns_state;
    int ns_timer;
    tl_state_t ew_state;
    int ew_timer;
} g_traffic = { TL_GREEN, 30, TL_RED, 33 };

static struct lws_context *g_context = NULL;
static volatile int g_force_exit = 0;
static const char *color_names[] = {"red", "green", "yellow"};

struct per_session_data_ws {
    int id;
};

static int client_id = 0;

static void update_traffic_sim(void) {
    g_traffic.ns_timer--;
    if (g_traffic.ns_timer <= 0) {
        if (g_traffic.ns_state == TL_GREEN) { g_traffic.ns_state = TL_YELLOW; g_traffic.ns_timer = 3; }
        else if (g_traffic.ns_state == TL_YELLOW) { g_traffic.ns_state = TL_RED; g_traffic.ns_timer = 30; }
        else { g_traffic.ns_state = TL_GREEN; g_traffic.ns_timer = 30; }
    }
    g_traffic.ew_timer--;
    if (g_traffic.ew_timer <= 0) {
        if (g_traffic.ew_state == TL_RED) { g_traffic.ew_state = TL_GREEN; g_traffic.ew_timer = 30; }
        else if (g_traffic.ew_state == TL_GREEN) { g_traffic.ew_state = TL_YELLOW; g_traffic.ew_timer = 3; }
        else { g_traffic.ew_state = TL_RED; g_traffic.ew_timer = 30; }
    }
}

static int callback_ws(struct lws *wsi, enum lws_callback_reasons reason,
                       void *user, void *in, size_t len) {
    struct per_session_data_ws *pss = (struct per_session_data_ws *)user;
    unsigned char buf[LWS_PRE + BUFFER_SIZE];
    char *p = (char *)&buf[LWS_PRE];
    int n;

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            pss->id = ++client_id;
            fprintf(stderr, "[WS#%d] Connected\n", pss->id);
            lws_callback_on_writable(wsi);
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
            n = snprintf(p, BUFFER_SIZE,
                "{\"type\":\"traffic_update\",\"ns\":{\"color\":\"%s\",\"timer\":%d},\"ew\":{\"color\":\"%s\",\"timer\":%d}}",
                color_names[g_traffic.ns_state], g_traffic.ns_timer,
                color_names[g_traffic.ew_state], g_traffic.ew_timer);
            fprintf(stderr, "[WS#%d] Sending: NS=%s/%d EW=%s/%d\n", pss->id,
                color_names[g_traffic.ns_state], g_traffic.ns_timer,
                color_names[g_traffic.ew_state], g_traffic.ew_timer);
            lws_write(wsi, (unsigned char *)p, n, LWS_WRITE_TEXT);
            break;

        case LWS_CALLBACK_CLOSED:
            fprintf(stderr, "[WS#%d] Disconnected\n", pss->id);
            break;

        default:
            break;
    }
    return 0;
}

// HTTP callback - 完全独立的处理
static int callback_http(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_HTTP: {
            char *uri = (char *)in;
            fprintf(stderr, "[HTTP] %s\n", uri);
            if (strcmp(uri, "/") == 0) {
                if (lws_serve_http_file(wsi, "www/index.html", "text/html", NULL, 0) < 0)
                    return -1;
            }
            // 返回0让lws继续处理
            return 0;
        }
        case LWS_CALLBACK_HTTP_FILE_COMPLETION:
            // 文件发送完成，完成HTTP事务
            lws_http_transaction_completed(wsi);
            return 0;
        default:
            break;
    }
    return 0;
}

// 协议定义 - 注意顺序和配置
static struct lws_protocols protocols[] = {
    {
        "http-only",
        callback_http,
        0,
        0,
    },
    {
        "traffic-protocol",
        callback_ws,
        sizeof(struct per_session_data_ws),
        BUFFER_SIZE,
    },
    { NULL, NULL, 0, 0 }
};

static void sigint_handler(int sig) { g_force_exit = 1; }

int main(void) {
    struct lws_context_creation_info info;
    signal(SIGINT, sigint_handler);
    
    memset(&info, 0, sizeof(info));
    info.port = 8080;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.options = 0;

    fprintf(stderr, "=== Traffic Light Server ===\n");
    fprintf(stderr, "Starting on port 8080...\n");
    
    g_context = lws_create_context(&info);
    if (!g_context) {
        fprintf(stderr, "Failed to create context\n");
        return -1;
    }
    
    fprintf(stderr, "Server running at http://localhost:8080\n");
    fprintf(stderr, "WebSocket protocol: traffic-protocol\n\n");

    time_t last_tick = time(NULL);
    
    while (!g_force_exit) {
        lws_service(g_context, 50);  // 50ms timeout
        
        time_t now = time(NULL);
        if (now > last_tick) {
            update_traffic_sim();
            last_tick = now;
            // 通知所有WebSocket客户端
            lws_callback_on_writable_all_protocol(g_context, &protocols[1]);
        }
    }
    
    lws_context_destroy(g_context);
    fprintf(stderr, "\nServer stopped.\n");
    return 0;
}
