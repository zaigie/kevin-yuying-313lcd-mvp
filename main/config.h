#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <driver/gpio.h>

#define DISPLAY_WIDTH 376
#define DISPLAY_HEIGHT 960
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY true

#define DISPLAY_OFFSET_X 0
#define DISPLAY_OFFSET_Y 0

#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_4
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false

#endif // _CONFIG_H_