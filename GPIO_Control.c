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
	unsigned int output_gpio = 194;   // GPIOY0
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

	input_line = gpiod_chip_get_line(chip, input_gpio);
	if (!input_line) {
		perror("Get input line failed");
		gpiod_chip_close(chip);
		exit(1);
	}
	output_line = gpiod_chip_get_line(chip, output_gpio);
	if (!output_line) {
		perror("Get output line failed");
		gpiod_chip_close(chip);
		exit(1);
	}
	
	ret = gpiod_line_request_output(output_line, CONSUMER, 0);
	if (ret < 0) {
		perror("Request output_line as output failed\n");
		gpiod_line_release(output_line);
		return -1;
	}
	
	
	ret = gpiod_line_request_rising_edge_events(input_line, CONSUMER);
	if (ret < 0) {
		perror("Request rising edge events failed");
		gpiod_line_release(input_line);
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
		else 
		{
			rintf("Rising edge detected on GPIO %d\n");
		}
		if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
			printf("Rising edge detected on GPIO %d\n", input_gpio);
		}
	}

	gpiod_line_release(input_line);
	gpiod_chip_close(chip);
	return 0;
}
