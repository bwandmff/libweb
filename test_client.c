#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static int destroy_flag = 0;
static int connection_completed = 0;
static int callback_count = 0;

enum demo_protocols {
    PROTOCOL_HTTP = 0,
    PROTOCOL_EXAMPLE,
    PROTOCOL_COUNT
};

static int callback_example(struct lws *wsi, enum lws_callback_reasons reason, 
                          void *user, void *in, size_t len)
{
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("Client connection established\n");
            lws_callback_on_writable(wsi);
            break;
            
        case LWS_CALLBACK_CLIENT_RECEIVE:
            printf("Received from server: %.*s\n", (int)len, (char *)in);
            destroy_flag = 1;
            break;
            
        case LWS_CALLBACK_CLIENT_WRITEABLE:
            if (callback_count == 0) {
                unsigned char buf[LWS_SEND_BUFFER_PRE_PADDING + 512 + LWS_SEND_BUFFER_POST_PADDING];
                int len = sprintf((char *)&buf[LWS_SEND_BUFFER_PRE_PADDING], "Hello Server!");
                if (lws_write(wsi, &buf[LWS_SEND_BUFFER_PRE_PADDING], len, LWS_WRITE_TEXT) < 0) {
                    printf("Failed to send message\n");
                    return -1;
                }
                printf("Sent message to server: Hello Server!\n");
                callback_count++;
            }
            break;
            
        case LWS_CALLBACK_CLOSED:
        case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
            printf("Connection closed or error\n");
            destroy_flag = 1;
            break;
            
        default:
            break;
    }
    
    return 0;
}

static struct lws_protocols protocols[] = {
    {
        "example-protocol",
        callback_example,
        0,
        0,
    },
    {
        NULL, NULL, 0, 0
    }
};

int main(int argc, char **argv)
{
    struct lws_context_creation_info info;
    struct lws_client_connect_info i;
    struct lws_context *context;
    struct lws *wsi;
    int n = 0;

    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.iface = NULL;
    info.protocols = protocols;
    info.extensions = NULL;
    info.gid = -1;
    info.uid = -1;
    info.options = 0;

    context = lws_create_context(&info);
    if (!context) {
        printf("Context creation failed\n");
        return 1;
    }

    memset(&i, 0, sizeof(i));
    i.context = context;
    i.address = "localhost";
    i.port = 8080;
    i.path = "/";
    i.host = i.address;
    i.origin = i.address;
    i.protocol = "example-protocol";

    wsi = lws_client_connect_via_info(&i);
    if (!wsi) {
        printf("Client connection failed\n");
        lws_context_destroy(context);
        return 1;
    }

    printf("Connecting to server...\n");
    
    while (n >= 0 && !destroy_flag) {
        n = lws_service(context, 50);
    }

    lws_context_destroy(context);
    printf("Test completed\n");
    return 0;
}