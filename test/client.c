#include <stdio.h>
#include <hislip/common.h>
#include <hislip/client.h>

static void receive_handler(void *buffer, int length)
{
    printf("Received: %s\n", (char *)buffer);
}

int main(void)
{
    char buffer[1000];
    hs_client_t hislip0;

    // Connect to HiSlip server
    hislip0 = hs_connect("127.0.0.1", HISLIP_PORT, "hislip0", 1000);

    // Send SCPI command on sync channel
    //strcpy(buffer, "*IDN?");
    //hs_send_receive_sync(hislip0, buffer, strlen(buffer), 1000);

    // Send SCPI command on async channel
    //hs_send_receive_async(hislip0, buffer, strlen(buffer), 1000, receive_handler);

    // Disconnect
    hs_disconnect(hislip0);
}
