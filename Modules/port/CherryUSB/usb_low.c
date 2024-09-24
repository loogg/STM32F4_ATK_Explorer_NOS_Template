#include "system.h"

void usb_dc_low_level_init(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* USER CODE BEGIN USB_OTG_FS_MspInit 0 */

    /* USER CODE END USB_OTG_FS_MspInit 0 */

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USB_OTG_FS GPIO Configuration
    PA11     ------> USB_OTG_FS_DM
    PA12     ------> USB_OTG_FS_DP
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Peripheral clock enable */
    __HAL_RCC_USB_OTG_FS_CLK_ENABLE();
    /* USB_OTG_FS interrupt Init */
    HAL_NVIC_SetPriority(OTG_FS_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(OTG_FS_IRQn);
    /* USER CODE BEGIN USB_OTG_FS_MspInit 1 */

    /* USER CODE END USB_OTG_FS_MspInit 1 */
}

void usb_dc_low_level_deinit(uint8_t busid) {
    /* USER CODE BEGIN USB_OTG_FS_MspDeInit 0 */

    /* USER CODE END USB_OTG_FS_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USB_OTG_FS_CLK_DISABLE();

    /**USB_OTG_FS GPIO Configuration
    PA11     ------> USB_OTG_FS_DM
    PA12     ------> USB_OTG_FS_DP
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);

    /* USB_OTG_FS interrupt DeInit */
    HAL_NVIC_DisableIRQ(OTG_FS_IRQn);
    /* USER CODE BEGIN USB_OTG_FS_MspDeInit 1 */

    /* USER CODE END USB_OTG_FS_MspDeInit 1 */
}

void OTG_FS_IRQHandler(void) {
    /* USER CODE BEGIN OTG_FS_IRQn 0 */

    /* USER CODE END OTG_FS_IRQn 0 */
    extern void USBD_IRQHandler(uint8_t busid);
    USBD_IRQHandler(0);
    /* USER CODE BEGIN OTG_FS_IRQn 1 */

    /* USER CODE END OTG_FS_IRQn 1 */
}