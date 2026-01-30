#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 4096

// 客户端连接信息结构
typedef struct client_info {
    struct lws *wsi;
    char name[64];
    struct client_info *next;
} client_info_t;

// 协议相关的用户数据
struct per_session_data__example {
    struct per_session_data__example *pss_list;
    struct lws *wsi;
    int expecting_pong_response;  // 标记是否需要发送pong响应
};

// 全局客户端链表
static client_info_t *clients_head = NULL;

// 添加客户端到列表
static void add_client(struct lws *wsi, const char *name) {
    client_info_t *new_client = malloc(sizeof(client_info_t));
    if (new_client) {
        new_client->wsi = wsi;
        strncpy(new_client->name, name ? name : "Anonymous", sizeof(new_client->name) - 1);
        new_client->name[sizeof(new_client->name) - 1] = '\0';
        new_client->next = clients_head;
        clients_head = new_client;
        lwsl_user("Client added: %s\n", new_client->name);
    }
}

// 移除客户端
static void remove_client(struct lws *wsi) {
    client_info_t *current = clients_head;
    client_info_t *prev = NULL;
    
    while (current) {
        if (current->wsi == wsi) {
            if (prev) {
                prev->next = current->next;
            } else {
                clients_head = current->next;
            }
            lwsl_user("Client removed: %s\n", current->name);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

// 广播消息给所有客户端
static int broadcast_message(const char *message, size_t length) {
    client_info_t *current = clients_head;
    int result = 0;
    
    while (current) {
        if (lws_write(current->wsi, (unsigned char *)message, length, LWS_WRITE_TEXT) < 0) {
            lwsl_err("Failed to write to client %s\n", current->name);
            result = -1;
        }
        current = current->next;
    }
    return result;
}

// 回显协议回调
static int callback_example(struct lws *wsi, enum lws_callback_reasons reason,
                           void *user, void *in, size_t len) {
    struct per_session_data__example *pss = (struct per_session_data__example *)user;
    unsigned char buffer[BUFFER_SIZE + LWS_SEND_BUFFER_PRE_PADDING + LWS_SEND_BUFFER_POST_PADDING];
    unsigned char *p = &buffer[LWS_SEND_BUFFER_PRE_PADDING];
    int n, m;

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            lwsl_user("WebSocket connection established\n");
            // 添加新客户端
            add_client(wsi, NULL);
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE: {
            if (pss->expecting_pong_response) {
                // 发送pong响应
                const char *pong_msg = "{\"type\":\"pong\",\"data\":\"Server pong\"}";
                if (lws_write(wsi, (unsigned char *)pong_msg, strlen(pong_msg), LWS_WRITE_TEXT) < 0) {
                    lwsl_err("Failed to write pong message\n");
                    return -1;
                }
                pss->expecting_pong_response = 0; // 重置标记
                
                // 然后再发送received确认
                const char *received_msg = "{\"type\":\"received\",\"data\":\"received\"}";
                if (lws_write(wsi, (unsigned char *)received_msg, strlen(received_msg), LWS_WRITE_TEXT) < 0) {
                    lwsl_err("Failed to write received confirmation\n");
                    return -1;
                }
            } else {
                // 发送received确认
                const char *received_msg = "{\"type\":\"received\",\"data\":\"received\"}";
                if (lws_write(wsi, (unsigned char *)received_msg, strlen(received_msg), LWS_WRITE_TEXT) < 0) {
                    lwsl_err("Failed to write received confirmation\n");
                    return -1;
                }
            }
            break;
        }

        case LWS_CALLBACK_RECEIVE: {
            lwsl_user("Received data: %s\n", (char *)in);
            
            struct per_session_data__example *pss = (struct per_session_data__example *)user;
            
            // 检查是否是特殊命令
            if (strncmp((char *)in, "ping", 4) == 0) {
                // 设置标记，稍后发送pong和received响应
                pss->expecting_pong_response = 1;
                if (lws_callback_on_writable(wsi) < 0) {
                    lwsl_err("Failed to request writable callback\n");
                    return -1;
                }
            } else {
                // 设置标记，稍后发送received响应
                pss->expecting_pong_response = 0;
                if (lws_callback_on_writable(wsi) < 0) {
                    lwsl_err("Failed to request writable callback\n");
                    return -1;
                }
            }
            break;
        }

        case LWS_CALLBACK_CLOSED:
            lwsl_user("WebSocket connection closed\n");
            remove_client(wsi);
            break;

        default:
            break;
    }

    return 0;
}

// HTTP协议回调
static int callback_http(struct lws *wsi, enum lws_callback_reasons reason,
                         void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_HTTP:
            lwsl_user("HTTP request received\n");
            
            // 发送主页
            if (lws_serve_http_file(wsi, "www/index.html", "text/html", NULL, 0) < 0) {
                lwsl_err("Failed to serve index.html\n");
                return -1;
            }
            break;

        case LWS_CALLBACK_HTTP_BODY:
            // 处理HTTP POST请求体
            lwsl_user("HTTP body: %s\n", (char *)in);
            break;

        case LWS_CALLBACK_HTTP_FILE_COMPLETION:
            // 文件发送完成，关闭连接
            return -1;

        default:
            break;
    }

    return 0;
}

// 协议定义
static struct lws_protocols protocols[] = {
    {
        "http-only",        // 协议名称
        callback_http,      // HTTP回调函数
        0,                  // 用户数据大小
        0,                  // 接收缓冲区大小
    },
    {
        "example-protocol", // 协议名称
        callback_example,   // WebSocket回调函数
        sizeof(struct per_session_data__example), // 用户数据大小
        1024,               // 接收缓冲区大小
    },
    {
        NULL, NULL, 0, 0    // 结束标记
    }
};

// 扩展选项 - 用于提供静态文件
static const struct lws_extension exts[] = {
    {
        "permessage-deflate",
        lws_extension_callback_pm_deflate,
        "permessage-deflate; client_max_window_bits"
    },
    {
        NULL, NULL, NULL
    }
};

int main(int argc, char **argv) {
    struct lws_context_creation_info info;
    struct lws_context *context;
    int n = 0;

    // 设置服务器参数
    memset(&info, 0, sizeof(info));
    info.port = 8080;
    info.iface = NULL;
    info.protocols = protocols;
    info.extensions = exts;
    info.ssl_cert_filepath = NULL;
    info.ssl_private_key_filepath = NULL;
    info.ssl_ca_filepath = NULL;
    info.ssl_cipher_list = "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:DHE-RSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384";
    info.gid = -1;
    info.uid = -1;
    info.options = 0;
    info.max_http_header_pool = 1;
    info.ka_time = 0;
    info.ka_probes = 0;
    info.ka_interval = 0;

    // 创建上下文
    context = lws_create_context(&info);
    if (!context) {
        lwsl_err("libwebsockets init failed\n");
        return -1;
    }

    lwsl_user("Starting server on port %d\n", info.port);
    lwsl_user("Visit http://localhost:8080 in your browser\n");

    // 事件循环
    while (n >= 0) {
        n = lws_service(context, 1000); // 1秒超时
    }

    // 清理
    lws_context_destroy(context);

    return 0;
}