#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 4096

// 交通灯状态定义
typedef enum {
    TL_RED,
    TL_GREEN,
    TL_YELLOW
} tl_state_t;

// 全局交通灯模拟数据
typedef struct {
    tl_state_t ns_state; // 南北向状态
    int ns_timer;        // 南北向倒计时
    tl_state_t ew_state; // 东西向状态
    int ew_timer;        // 东西向倒计时
} traffic_sim_t;

static traffic_sim_t g_traffic = { TL_GREEN, 30, TL_RED, 33 };

// 客户端连接信息结构
typedef struct client_info {
    struct lws *wsi;
    char name[64];
    struct client_info *next;
} client_info_t;

static client_info_t *clients_head = NULL;

static void add_client(struct lws *wsi, const char *name) {
    client_info_t *new_client = malloc(sizeof(client_info_t));
    if (new_client) {
        new_client->wsi = wsi;
        strncpy(new_client->name, name ? name : "Anonymous", sizeof(new_client->name) - 1);
        new_client->next = clients_head;
        clients_head = new_client;
    }
}

static void remove_client(struct lws *wsi) {
    client_info_t *current = clients_head, *prev = NULL;
    while (current) {
        if (current->wsi == wsi) {
            if (prev) prev->next = current->next;
            else clients_head = current->next;
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

static void broadcast_message(const char *message) {
    client_info_t *current = clients_head;
    size_t len = strlen(message);
    unsigned char buf[LWS_PRE + BUFFER_SIZE];
    memcpy(&buf[LWS_PRE], message, len);

    while (current) {
        lws_write(current->wsi, &buf[LWS_PRE], len, LWS_WRITE_TEXT);
        current = current->next;
    }
}

// 模拟器逻辑：每秒更新一次灯态
static void update_traffic_sim() {
    // 更新南北向
    g_traffic.ns_timer--;
    if (g_traffic.ns_timer <= 0) {
        if (g_traffic.ns_state == TL_GREEN) { g_traffic.ns_state = TL_YELLOW; g_traffic.ns_timer = 3; }
        else if (g_traffic.ns_state == TL_YELLOW) { g_traffic.ns_state = TL_RED; g_traffic.ns_timer = 30; }
        else { g_traffic.ns_state = TL_GREEN; g_traffic.ns_timer = 30; }
    }

    // 更新东西向 (逻辑相反)
    g_traffic.ew_timer--;
    if (g_traffic.ew_timer <= 0) {
        if (g_traffic.ew_state == TL_RED) { g_traffic.ew_state = TL_GREEN; g_traffic.ew_timer = 30; }
        else if (g_traffic.ew_state == TL_GREEN) { g_traffic.ew_state = TL_YELLOW; g_traffic.ew_timer = 3; }
        else { g_traffic.ew_state = TL_RED; g_traffic.ew_timer = 30; }
    }

    // 构造 JSON 并广播
    char json[256];
    const char *colors[] = {"red", "green", "yellow"};
    snprintf(json, sizeof(json), 
        "{\"type\":\"traffic_update\",\"ns\":{\"color\":\"%s\",\"timer\":%d},\"ew\":{\"color\":\"%s\",\"timer\":%d}}",
        colors[g_traffic.ns_state], g_traffic.ns_timer,
        colors[g_traffic.ew_state], g_traffic.ew_timer);
    
    broadcast_message(json);
}

static int callback_example(struct lws *wsi, enum lws_callback_reasons reason,
                           void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            add_client(wsi, NULL);
            break;
        case LWS_CALLBACK_CLOSED:
            remove_client(wsi);
            break;
        case LWS_CALLBACK_RECEIVE:
            // 原有逻辑保留，也可扩展
            break;
        default: break;
    }
    return 0;
}

static int callback_http(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len) {
    if (reason == LWS_CALLBACK_HTTP) {
        lws_serve_http_file(wsi, "www/index.html", "text/html", NULL, 0);
    }
    return 0;
}

static struct lws_protocols protocols[] = {
    { "http", callback_http, 0, 0 },
    { "example-protocol", callback_example, 0, 1024 },
    { NULL, NULL, 0, 0 }
};

int main(int argc, char **argv) {
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = 8080;
    info.protocols = protocols;
    info.gid = -1; info.uid = -1;

    struct lws_context *context = lws_create_context(&info);
    if (!context) return -1;

    time_t last_tick = 0;
    while (1) {
        lws_service(context, 50);
        
        time_t now = time(NULL);
        if (now > last_tick) {
            update_traffic_sim();
            last_tick = now;
        }
    }

    lws_context_destroy(context);
    return 0;
}
