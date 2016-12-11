#include "espressif/esp_common.h"
#include "esp/uart.h"

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "lwip/api.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "ssid_config.h"

#define PORT 5568
#define SACNLENGTH 638

void task(void *pvParameters) {
	printf("Open server in 10 seconds.\r\n");
	vTaskDelay(10000);

	struct netconn *conn;
	err_t err;

	/* Create a new connection handle */
	conn = netconn_new(NETCONN_UDP);
	if(!conn) {
		printf("Error: Failed to allocate socket.\r\n");
		return;
	}

	/* Bind to port with default IP address */
	err = netconn_bind(conn, IP_ADDR_ANY, PORT);
	if(err != ERR_OK) {
		printf("Error: Failed to bind socket. err=%d\r\n", err);
		return;
	}

	printf("Listening for connections.\r\n");

	while(1) {
		//printf("Loop\r\n");

		struct netbuf *inbuf;

		err = netconn_recv(conn, &inbuf);

		if(err != ERR_OK) {
			printf("Error: Failed to receive packet. err=%d\r\n", err);
			continue;
		}

		char* buf;
		u16_t buflen;

		netbuf_data(inbuf, (void**)&buf, &buflen);
		if (buflen == SACNLENGTH) {
			printf("Received Len %d\n", buflen);
			//printf("%s\n", buf);

			int channel1;
			channel1 = buf[126];
			printf("Value %d\n\n", channel1);
		}

		netbuf_delete(inbuf);
	}
}

void user_init(void) {
    uart_set_baud(0, 115200);
    printf("SDK version:%s\n", sdk_system_get_sdk_version());

    struct sdk_station_config config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
    };
    sdk_wifi_set_opmode(STATION_MODE);
    sdk_wifi_station_set_config(&config);

	xTaskCreate(task, "task", 768, NULL, 8, NULL);
}

