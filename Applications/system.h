#ifndef __SYSTEM_H
#define __SYSTEM_H

#include <stm32f4xx.h>
#include <stdint.h>
#include <stdio.h>

#define SOFTWARE_VERSION "1.0.1"

#define SYSTEM_USING_CONSOLE
#ifdef SYSTEM_USING_CONSOLE
#define SYSTEM_PRINTF printf
#define SYSTEM_USING_CONSOLE_DEBUG
#define SYSTEM_USING_CONSOLE_DEBUG_COLOR
#define SYSTEM_USING_CONSOLE_SHELL
#else
#define SYSTEM_PRINTF(...)
#endif

#define USB_LOW_USE_FS 0

#if USB_LOW_USE_FS
#define CONFIG_USBDEV_EP_NUM 4
/* ---------------- DWC2 Configuration ---------------- */
/* (5 * number of control endpoints + 8) + ((largest USB packet used / 4) + 1 for
 * status information) + (2 * number of OUT endpoints) + 1 for Global NAK
 */
#define CONFIG_USB_DWC2_RXALL_FIFO_SIZE (256 / 4)
/* IN Endpoints Max packet Size / 4 */
#define CONFIG_USB_DWC2_TX0_FIFO_SIZE (64 / 4)
#define CONFIG_USB_DWC2_TX1_FIFO_SIZE (128 / 4)
#define CONFIG_USB_DWC2_TX2_FIFO_SIZE (64 / 4)
#define CONFIG_USB_DWC2_TX3_FIFO_SIZE (64 / 4)
#define CONFIG_USB_DWC2_TX4_FIFO_SIZE (0 / 4)
#define CONFIG_USB_DWC2_TX5_FIFO_SIZE (0 / 4)
#define CONFIG_USB_DWC2_TX6_FIFO_SIZE (0 / 4)
#define CONFIG_USB_DWC2_TX7_FIFO_SIZE (0 / 4)
#define CONFIG_USB_DWC2_TX8_FIFO_SIZE (0 / 4)
#else
#define CONFIG_USBDEV_EP_NUM 6
#define CONFIG_USB_DWC2_DMA_ENABLE
#endif

void system_delay_us(uint32_t us);
int system_init(void);

#endif
