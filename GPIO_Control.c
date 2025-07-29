#include <gpiod.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef CONSUMER
#define CONSUMER "RisingEdgeDetector"
#endif

int main()
{
	const char *chipname = "gpiochip0";
	unsigned int line_num = 192;  // GPIOY0 = GPIO 192
	struct gpiod_chip *chip;
	struct gpiod_line *line;
	struct gpiod_line_event event;
	int ret;

	// 開啟 chip
	chip = gpiod_chip_open_by_name(chipname);
	if (!chip) {
		perror("Open chip failed");
		exit(1);
	}

	// 取得 line
	line = gpiod_chip_get_line(chip, line_num);
	if (!line) {
		perror("Get line failed");
		gpiod_chip_close(chip);
		exit(1);
	}

	// 設定為 rising-edge 偵測模式
	ret = gpiod_line_request_rising_edge_events(line, CONSUMER);
	if (ret < 0) {
		perror("Request rising edge events failed");
		gpiod_chip_close(chip);
		exit(1);
	}

	printf("Waiting for rising edge on GPIO %d...\n", line_num);

	while (1) {
		ret = gpiod_line_event_wait(line, NULL);  // 等待事件（阻塞）
		if (ret < 0) {
			perror("Event wait error");
			break;
		} else if (ret == 0) {
			printf("Timeout occurred, no event.\n");
			continue;
		}

		ret = gpiod_line_event_read(line, &event);
		if (ret < 0) {
			perror("Event read error");
			break;
		}

		if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
			printf("Rising edge detected on GPIO %d\n", line_num);
		}
	}

	gpiod_line_release(line);
	gpiod_chip_close(chip);
	return 0;
}
