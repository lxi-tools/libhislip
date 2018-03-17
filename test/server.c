#include <stdio.h>
#include <hislip/server.h>
#include <hislip/common.h>

int hislip0_message_sync(void *buffer, int length)
{
    return 0;
    // How to send response? Maybe add some message id?
    // send_response(id, void *buffer, int length);
}

int hislip0_message_async(void *buffer, int length)
{
    return 0;
}

int main(void)
{
    int status;
    hs_server_t server;
    hs_server_config_t config;
    hs_subaddress_callbacks_t hislip0_callbacks;

    // Initialize server configuration
    hs_server_config_init(&config);

    // Configure server
    config.connections_max = 10;
    config.worker_threads_max = 4;
    config.worker_queue_depth_max = 20;
    config.payload_size_max = 0x100000; // 1 MB
    config.message_timeout = 3000; // 3 seconds

    // Initialize server
    hs_server_init(&server, &config);
    
    // Register server message handlers
    hislip0_callbacks.message_sync = hislip0_message_sync;
    hislip0_callbacks.message_async = hislip0_message_async;
    hs_server_register_subaddress(&server, "hislip0", &hislip0_callbacks);
    hs_server_register_subaddress(&server, "hislip1", &hislip0_callbacks);
    hs_server_register_subaddress(&server, "hislip2", &hislip0_callbacks);

    // Start server
    status = hs_server_run(&server);

    return status;
}
