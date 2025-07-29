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
	unsigned int input_gpio = 192;   // GPIOY0
	unsigned int output_gpio = 194;  // GPIOY2
	struct gpiod_chip *chip;
	struct gpiod_line *input_line;
	struct gpiod_line *output_line;
	struct gpiod_line_event event;
	int ret;

	// 開啟 chip
	chip = gpiod_chip_open_by_name(chipname);
	if (!chip) {
		perror("Open chip failed");
		exit(1);
	}

	// 設定 GPIOY2 (#194) 為輸出（但不操作值）
	output_line = gpiod_chip_get_line(chip, output_gpio);
	if (!output_line) {
		perror("Get output line failed");
		gpiod_chip_close(chip);
		exit(1);
	}

	ret = gpiod_line_request_output(output_line, CONSUMER, 0); // 初始輸出為 0
	if (ret < 0) {
		perror("Request output line failed");
		gpiod_chip_close(chip);
		exit(1);
	}

	// 設定 GPIOY0 (#192) 為 rising-edge 偵測
	input_line = gpiod_chip_get_line(chip, input_gpio);
	if (!input_line) {
		perror("Get input line failed");
		gpiod_chip_close(chip);
		exit(1);
	}

	ret = gpiod_line_request_rising_edge_events(input_line, CONSUMER);
	if (ret < 0) {
		perror("Request rising edge events failed");
		gpiod_chip_close(chip);
		exit(1);
	}

	printf("Waiting for rising edge on GPIO %d...\n", input_gpio);

	while (1) {
		ret = gpiod_line_event_wait(input_line, NULL);  // 阻塞等待事件
		if (ret < 0) {
			perror("Event wait error");
			break;
		} else if (ret == 0) {
			printf("Timeout occurred, no event.\n");
			continue;
		}

		ret = gpiod_line_event_read(input_line, &event);
		if (ret < 0) {
			perror("Event read error");
			break;
		}

		if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
			printf("Rising edge detected on GPIO %d\n", input_gpio);
		}
	}

	gpiod_line_release(input_line);
	gpiod_line_release(output_line);
	gpiod_chip_close(chip);
	return 0;
}
