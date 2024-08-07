#ifndef __DRV_ETH_H
#define __DRV_ETH_H

#include "main.h"

extern ETH_HandleTypeDef EthHandle;

HAL_StatusTypeDef drv_eth_init(void);

#endif /* __DRV_ETH_H */
