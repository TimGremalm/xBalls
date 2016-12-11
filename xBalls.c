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

#include "ws2812_i2s/ws2812_i2s.h"

#define PORT 5568
#define SACNLENGTH 638
const uint32_t led_number = 24;

static ws2812_pixel_t getColor(char red, char green, char blue) {
	ws2812_pixel_t color = {0, 0, 0};
	color.red = red;
	color.green = green;
	color.blue = blue;
	return color;
}

void task(void *pvParameters) {
	printf("Init xBalls WS2812 LEDs.\r\n");
	ws2812_pixel_t pixels[led_number];
	memset(pixels, 0, sizeof(ws2812_pixel_t) * led_number);
	ws2812_i2s_init(led_number);

	printf("Open server in 10 seconds.\r\n");
	vTaskDelay(1000);

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
			//printf("Received Len %d\n", buflen);
			//printf("%s\n", buf);

			int channelOffset = 126;
			for (int i = 0; i < led_number; i++) {
				char red = buf[i * 3 + channelOffset];
				char green = buf[i * 3 + channelOffset + 1];
				char blue = buf[i * 3 + channelOffset + 2];

				pixels[i] = getColor(red, green, blue);
			}

			ws2812_i2s_update(pixels);

			for (int i = 0; i < 1; i++) {
				printf("LED %d - %d %d %d\n", i, pixels[i].red, pixels[i].green, pixels[i].blue);
			}
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

