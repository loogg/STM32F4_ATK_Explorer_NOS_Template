#include "system.h"
#include "task_run.h"

extern void cdc_acm_init(uint8_t busid, uint32_t reg_base);
extern void cdc_acm_multi_init(uint8_t busid, uintptr_t reg_base);
extern void cdc_acm_data_send_with_dtr_test(uint8_t busid);
extern void cdc_acm_multi_data_send_test(uint8_t busid, uint8_t ep_idx);

#define TASK_RUN_PERIOD 500

static int cdc_acm_send_entry(struct task_pcb *task) {
    cdc_acm_multi_data_send_test(0, 0);
    cdc_acm_multi_data_send_test(0, 1);
    cdc_acm_multi_data_send_test(0, 2);

    return 0;
}

void usb_sample_init(void) {
#if USB_LOW_USE_FS
    // cdc_acm_init(0, USB_OTG_FS_PERIPH_BASE);
    cdc_acm_multi_init(0, USB_OTG_FS_PERIPH_BASE);
#else
    // cdc_acm_init(0, USB_OTG_HS_PERIPH_BASE);
    cdc_acm_multi_init(0, USB_OTG_HS_PERIPH_BASE);
#endif /* USB_LOW_USE_FS */

    task_init(TASK_USBD_CDC_ACM, cdc_acm_send_entry, NULL, TASK_RUN_PERIOD);
    task_start(TASK_USBD_CDC_ACM);
}
