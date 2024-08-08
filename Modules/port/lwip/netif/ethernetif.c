/**
 * @file
 * Ethernet Interface Skeleton
 *
 */

/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"

#if 1 /* don't build, this is only a skeleton, see previous comment */

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/ethip6.h"
#include "lwip/etharp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include <string.h>
#include "drv_eth.h"
#include "task_run.h"
#include "system.h"

#define DBG_TAG "ethif"
#define DBG_LVL DBG_INFO
#include <agile_dbg.h>

#define ETH_RX_DUMP
// #define ETH_TX_DUMP

/* Define those to better describe your network interface. */
#define IFNAME0 's'
#define IFNAME1 't'

/*Static IP ADDRESS: IP_ADDR0.IP_ADDR1.IP_ADDR2.IP_ADDR3 */
#define IP_ADDR0 (uint8_t)192
#define IP_ADDR1 (uint8_t)168
#define IP_ADDR2 (uint8_t)2
#define IP_ADDR3 (uint8_t)32

/*NETMASK*/
#define NETMASK_ADDR0 (uint8_t)255
#define NETMASK_ADDR1 (uint8_t)255
#define NETMASK_ADDR2 (uint8_t)255
#define NETMASK_ADDR3 (uint8_t)0

/*Gateway Address*/
#define GW_ADDR0 (uint8_t)192
#define GW_ADDR1 (uint8_t)168
#define GW_ADDR2 (uint8_t)2
#define GW_ADDR3 (uint8_t)1

#if LWIP_DHCP
#define DHCP_OFF              (uint8_t)0
#define DHCP_START            (uint8_t)1
#define DHCP_WAIT_ADDRESS     (uint8_t)2
#define DHCP_ADDRESS_ASSIGNED (uint8_t)3
#define DHCP_TIMEOUT          (uint8_t)4
#define DHCP_LINK_DOWN        (uint8_t)5

#define MAX_DHCP_TRIES 4
#endif /* LWIP_DHCP */

struct netif gnetif;

struct ethernetif {
    struct eth_addr *ethaddr;
#if LWIP_DHCP
    uint8_t dhcp_state;
#endif /* LWIP_DHCP */
};

#if defined(ETH_RX_DUMP) || defined(ETH_TX_DUMP)
#define __is_print(ch) ((unsigned int)((ch) - ' ') < 127u - ' ')
static void dump_hex(const uint8_t *ptr, size_t buflen)
{
    unsigned char *buf = (unsigned char *)ptr;
    int i, j;

    SYSTEM_PRINTF("\r\n\r\n");

    for (i = 0; i < buflen; i += 16)
    {
        SYSTEM_PRINTF("%08X: ", i);

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                SYSTEM_PRINTF("%02X ", buf[i + j]);
            else
                SYSTEM_PRINTF("   ");
        SYSTEM_PRINTF(" ");

        for (j = 0; j < 16; j++)
            if (i + j < buflen)
                SYSTEM_PRINTF("%c", __is_print(buf[i + j]) ? buf[i + j] : '.');
        SYSTEM_PRINTF("\r\n");
    }
}
#endif

/* Forward declarations. */
static void ethernetif_input(struct netif *netif);

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void low_level_init(struct netif *netif) {
    HAL_StatusTypeDef hal_eth_init_status;

    hal_eth_init_status = drv_eth_init();
    if (hal_eth_init_status == HAL_OK) {
        LOG_I("low init link up");
        netif->flags |= NETIF_FLAG_LINK_UP;
    }

    /* set MAC hardware address length */
    netif->hwaddr_len = ETHARP_HWADDR_LEN;

    /* set MAC hardware address */
    netif->hwaddr[0] = EthHandle.Init.MACAddr[0];
    netif->hwaddr[1] = EthHandle.Init.MACAddr[1];
    netif->hwaddr[2] = EthHandle.Init.MACAddr[2];
    netif->hwaddr[3] = EthHandle.Init.MACAddr[3];
    netif->hwaddr[4] = EthHandle.Init.MACAddr[4];
    netif->hwaddr[5] = EthHandle.Init.MACAddr[5];

    /* maximum transfer unit */
    netif->mtu = 1500;

    /* device capabilities */
    /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
    netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

#if LWIP_IPV6 && LWIP_IPV6_MLD
    /*
     * For hardware/netifs that implement MAC filtering.
     * All-nodes link-local is handled by default, so we must let the hardware know
     * to allow multicast packets in.
     * Should set mld_mac_filter previously. */
    if (netif->mld_mac_filter != NULL) {
        ip6_addr_t ip6_allnodes_ll;
        ip6_addr_set_allnodes_linklocal(&ip6_allnodes_ll);
        netif->mld_mac_filter(netif, &ip6_allnodes_ll, NETIF_ADD_MAC_FILTER);
    }
#endif /* LWIP_IPV6 && LWIP_IPV6_MLD */

    /* Do whatever else is needed to initialize interface. */
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become available since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */

static err_t low_level_output(struct netif *netif, struct pbuf *p) {
    struct pbuf *q;
    err_t errval;
    uint8_t *buffer = (uint8_t *)(EthHandle.TxDesc->Buffer1Addr);
    __IO ETH_DMADescTypeDef *DmaTxDesc;
    uint32_t framelength = 0;
    uint32_t bufferoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t payloadoffset = 0;
    HAL_StatusTypeDef hal_eth_trans_status;
    DmaTxDesc = EthHandle.TxDesc;
    bufferoffset = 0;

#if ETH_PAD_SIZE
    pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif

    for (q = p; q != NULL; q = q->next) {
        /* Send the data from the pbuf to the interface, one pbuf at a
           time. The size of the data in each pbuf is kept in the ->len
           variable. */
        /* Is this buffer available? If not, goto error */
        if ((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET) {
            LOG_E("buffer not valid");
            errval = ERR_USE;
            goto error;
        }

        /* Get bytes in current lwIP buffer */
        byteslefttocopy = q->len;
        payloadoffset = 0;

        /* Check if the length of data to copy is bigger than Tx buffer size*/
        while ((byteslefttocopy + bufferoffset) > ETH_TX_BUF_SIZE) {
            /* Copy data to Tx buffer*/
            memcpy((uint8_t *)((uint8_t *)buffer + bufferoffset), (uint8_t *)((uint8_t *)q->payload + payloadoffset),
                   (ETH_TX_BUF_SIZE - bufferoffset));

            /* Point to next descriptor */
            DmaTxDesc = (ETH_DMADescTypeDef *)(DmaTxDesc->Buffer2NextDescAddr);

            /* Check if the buffer is available */
            if ((DmaTxDesc->Status & ETH_DMATXDESC_OWN) != (uint32_t)RESET) {
                LOG_E("dma tx desc buffer is not valid");
                errval = ERR_USE;
                goto error;
            }

            buffer = (uint8_t *)(DmaTxDesc->Buffer1Addr);

            byteslefttocopy = byteslefttocopy - (ETH_TX_BUF_SIZE - bufferoffset);
            payloadoffset = payloadoffset + (ETH_TX_BUF_SIZE - bufferoffset);
            framelength = framelength + (ETH_TX_BUF_SIZE - bufferoffset);
            bufferoffset = 0;
        }

        /* Copy the remaining bytes */
        memcpy((uint8_t *)((uint8_t *)buffer + bufferoffset), (uint8_t *)((uint8_t *)q->payload + payloadoffset), byteslefttocopy);
        bufferoffset = bufferoffset + byteslefttocopy;
        framelength = framelength + byteslefttocopy;
    }

#ifdef ETH_TX_DUMP
    dump_hex(buffer, p->tot_len);
#endif

    LOG_D("transmit frame length :%d", framelength);

    /* wait for unlocked */
    while (EthHandle.Lock == HAL_LOCKED);

    /* Prepare transmit descriptors to give to DMA */
    hal_eth_trans_status = HAL_ETH_TransmitFrame(&EthHandle, framelength);
    if (hal_eth_trans_status != HAL_OK) {
        LOG_E("eth transmit frame faild: %d", hal_eth_trans_status);
    }

    errval = ERR_OK;

error:

    /* When Transmit Underflow flag is set, clear it and issue a Transmit Poll Demand to resume transmission */
    if ((EthHandle.Instance->DMASR & ETH_DMASR_TUS) != (uint32_t)RESET) {
        /* Clear TUS ETHERNET DMA flag */
        EthHandle.Instance->DMASR = ETH_DMASR_TUS;

        /* Resume DMA transmission*/
        EthHandle.Instance->DMATPDR = 0;
    }

    MIB2_STATS_NETIF_ADD(netif, ifoutoctets, p->tot_len);
    if (((u8_t *)p->payload)[0] & 1) {
        /* broadcast or multicast packet*/
        MIB2_STATS_NETIF_INC(netif, ifoutnucastpkts);
    } else {
        /* unicast packet */
        MIB2_STATS_NETIF_INC(netif, ifoutucastpkts);
    }
    /* increase ifoutdiscards or ifouterrors on error */
    if (errval != ERR_OK) {
        MIB2_STATS_NETIF_INC(netif, ifouterrors);
    }

#if ETH_PAD_SIZE
    pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    LINK_STATS_INC(link.xmit);

    return errval;
}

/**
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return a pbuf filled with the received packet (including MAC header)
 *         NULL on memory error
 */
static struct pbuf *low_level_input(struct netif *netif) {
    struct pbuf *p = NULL;
    struct pbuf *q;
    uint16_t len;
    uint8_t *buffer;
    __IO ETH_DMADescTypeDef *dmarxdesc;
    uint32_t bufferoffset = 0;
    uint32_t payloadoffset = 0;
    uint32_t byteslefttocopy = 0;
    uint32_t i = 0;

    if (HAL_ETH_GetReceivedFrame(&EthHandle) != HAL_OK) {
        LOG_D("receive frame faild");
        return NULL;
    }

    /* Obtain the size of the packet and put it into the "len"
       variable. */
    len = EthHandle.RxFrameInfos.length;
    buffer = (uint8_t *)EthHandle.RxFrameInfos.buffer;

    LOG_D("receive frame len : %d", len);

    if (len > 0) {
#if ETH_PAD_SIZE
        len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

        /* We allocate a pbuf chain of pbufs from the pool. */
        p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
    }

    if (p != NULL) {
#if ETH_PAD_SIZE
        pbuf_remove_header(p, ETH_PAD_SIZE); /* drop the padding word */
#endif

#ifdef ETH_RX_DUMP
        dump_hex(buffer, p->tot_len);
#endif

        dmarxdesc = EthHandle.RxFrameInfos.FSRxDesc;
        bufferoffset = 0;

        /* We iterate over the pbuf chain until we have read the entire
         * packet into the pbuf. */
        for (q = p; q != NULL; q = q->next) {
            /* Read enough bytes to fill this pbuf in the chain. The
             * available data in the pbuf is given by the q->len
             * variable.
             * This does not necessarily have to be a memcpy, you can also preallocate
             * pbufs for a DMA-enabled MAC and after receiving truncate it to the
             * actually received size. In this case, ensure the tot_len member of the
             * pbuf is the sum of the chained pbuf len members.
             */
            byteslefttocopy = q->len;
            payloadoffset = 0;

            /* Check if the length of bytes to copy in current pbuf is bigger than Rx buffer size */
            while ((byteslefttocopy + bufferoffset) > ETH_RX_BUF_SIZE) {
                /* Copy data to pbuf */
                memcpy((uint8_t *)((uint8_t *)q->payload + payloadoffset), (uint8_t *)((uint8_t *)buffer + bufferoffset),
                       (ETH_RX_BUF_SIZE - bufferoffset));

                /* Point to next descriptor */
                dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
                buffer = (uint8_t *)(dmarxdesc->Buffer1Addr);

                byteslefttocopy = byteslefttocopy - (ETH_RX_BUF_SIZE - bufferoffset);
                payloadoffset = payloadoffset + (ETH_RX_BUF_SIZE - bufferoffset);
                bufferoffset = 0;
            }

            /* Copy remaining data in pbuf */
            memcpy((uint8_t *)((uint8_t *)q->payload + payloadoffset), (uint8_t *)((uint8_t *)buffer + bufferoffset), byteslefttocopy);
            bufferoffset = bufferoffset + byteslefttocopy;
        }

        MIB2_STATS_NETIF_ADD(netif, ifinoctets, p->tot_len);
        if (((u8_t *)p->payload)[0] & 1) {
            /* broadcast or multicast packet*/
            MIB2_STATS_NETIF_INC(netif, ifinnucastpkts);
        } else {
            /* unicast packet*/
            MIB2_STATS_NETIF_INC(netif, ifinucastpkts);
        }
#if ETH_PAD_SIZE
        pbuf_add_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

        LINK_STATS_INC(link.recv);
    } else {
        LINK_STATS_INC(link.memerr);
        LINK_STATS_INC(link.drop);
        MIB2_STATS_NETIF_INC(netif, ifindiscards);
    }

    /* Release descriptors to DMA */
    /* Point to first descriptor */
    dmarxdesc = EthHandle.RxFrameInfos.FSRxDesc;
    /* Set Own bit in Rx descriptors: gives the buffers back to DMA */
    for (i = 0; i < EthHandle.RxFrameInfos.SegCount; i++) {
        dmarxdesc->Status |= ETH_DMARXDESC_OWN;
        dmarxdesc = (ETH_DMADescTypeDef *)(dmarxdesc->Buffer2NextDescAddr);
    }

    /* Clear Segment_Count */
    EthHandle.RxFrameInfos.SegCount = 0;

    /* When Rx Buffer unavailable flag is set: clear it and resume reception */
    if ((EthHandle.Instance->DMASR & ETH_DMASR_RBUS) != (uint32_t)RESET) {
        /* Clear RBUS ETHERNET DMA flag */
        EthHandle.Instance->DMASR = ETH_DMASR_RBUS;
        /* Resume DMA reception */
        EthHandle.Instance->DMARPDR = 0;
    }
    return p;
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void ethernetif_input(struct netif *netif) {
    struct pbuf *p;

    /* move received packet into a new pbuf */
    p = low_level_input(netif);
    /* if no packet could be read, silently ignore this */
    if (p != NULL) {
        /* pass all packets to ethernet_input, which decides what packets it supports */
        if (netif->input(p, netif) != ERR_OK) {
            LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
            pbuf_free(p);
            p = NULL;
        }
    }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
static err_t ethernetif_init(struct netif *netif) {
    struct ethernetif *ethernetif;

    LWIP_ASSERT("netif != NULL", (netif != NULL));

    ethernetif = mem_malloc(sizeof(struct ethernetif));
    if (ethernetif == NULL) {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
        return ERR_MEM;
    }

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

    /*
     * Initialize the snmp variables and counters inside the struct netif.
     * The last argument should be replaced with your link speed, in units
     * of bits per second.
     */
    MIB2_INIT_NETIF(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

    netif->state = ethernetif;
    netif->name[0] = IFNAME0;
    netif->name[1] = IFNAME1;
    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
#if LWIP_IPV4
    netif->output = etharp_output;
#endif /* LWIP_IPV4 */
#if LWIP_IPV6
    netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
    netif->linkoutput = low_level_output;

    ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);
#if LWIP_DHCP
    ethernetif->dhcp_state = DHCP_OFF;
#endif /* LWIP_DHCP */

    /* initialize the hardware */
    low_level_init(netif);

    return ERR_OK;
}

void ethernetif_notify_conn_changed(struct netif *netif) {
#if !LWIP_DHCP
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;
#endif

    if (netif_is_link_up(netif)) {
        LOG_I("The network cable is now connected");

#if LWIP_DHCP
        /* Update DHCP state machine */
        ethernetif->dhcp_state = DHCP_START;
#else
        IP_ADDR4(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
        IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
        IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);

        netif_set_addr(netif, &ipaddr, &netmask, &gw);

        uint8_t iptxt[20];
        snprintf((char *)iptxt, sizeof(iptxt), "%s", ip4addr_ntoa((const ip4_addr_t *)&netif->ip_addr));
        LOG_I("Static IP address: %s", iptxt);
#endif /* LWIP_DHCP */
    } else {
#if LWIP_DHCP
        struct ethernetif *ethernetif = netif->state;
        /* Update DHCP state machine */
        ethernetif->dhcp_state = DHCP_LINK_DOWN;
#endif /* LWIP_DHCP */

        LOG_I("The network cable is not connected");
    }
}

static void ethernetif_update_config(struct netif *netif) {
    __IO uint32_t tickstart = 0;
    uint32_t regvalue = 0;

    LOG_I("update ethernet config");

    if (netif_is_link_up(netif)) {
        /* Restart the auto-negotiation */
        if (EthHandle.Init.AutoNegotiation != ETH_AUTONEGOTIATION_DISABLE) {
            /* Enable Auto-Negotiation */
            HAL_ETH_WritePHYRegister(&EthHandle, PHY_BCR, PHY_AUTONEGOTIATION);

            /* Get tick */
            tickstart = HAL_GetTick();

            /* Wait until the auto-negotiation will be completed */
            do {
                HAL_ETH_ReadPHYRegister(&EthHandle, PHY_BSR, &regvalue);

                /* Check for the Timeout ( 1s ) */
                if ((HAL_GetTick() - tickstart) > 1000) {
                    /* In case of timeout */
                    goto error;
                }

            } while (((regvalue & PHY_AUTONEGO_COMPLETE) != PHY_AUTONEGO_COMPLETE));

            /* Read the result of the auto-negotiation */
            HAL_ETH_ReadPHYRegister(&EthHandle, PHY_SR, &regvalue);

            /* Configure the MAC with the Duplex Mode fixed by the auto-negotiation process */
            if ((regvalue & PHY_DUPLEX_STATUS) != (uint32_t)RESET) {
                /* Set Ethernet duplex mode to Full-duplex following the auto-negotiation */
                EthHandle.Init.DuplexMode = ETH_MODE_FULLDUPLEX;
            } else {
                /* Set Ethernet duplex mode to Half-duplex following the auto-negotiation */
                EthHandle.Init.DuplexMode = ETH_MODE_HALFDUPLEX;
            }
            /* Configure the MAC with the speed fixed by the auto-negotiation process */
            if (regvalue & PHY_SPEED_STATUS) {
                /* Set Ethernet speed to 10M following the auto-negotiation */
                EthHandle.Init.Speed = ETH_SPEED_10M;
            } else {
                /* Set Ethernet speed to 100M following the auto-negotiation */
                EthHandle.Init.Speed = ETH_SPEED_100M;
            }
        } else /* AutoNegotiation Disable */
        {
        error:
            /* Check parameters */
            assert_param(IS_ETH_SPEED(EthHandle.Init.Speed));
            assert_param(IS_ETH_DUPLEX_MODE(EthHandle.Init.DuplexMode));

            /* Set MAC Speed and Duplex Mode to PHY */
            HAL_ETH_WritePHYRegister(&EthHandle, PHY_BCR, ((uint16_t)(EthHandle.Init.DuplexMode >> 3) | (uint16_t)(EthHandle.Init.Speed >> 1)));
        }

        /* ETHERNET MAC Re-Configuration */
        HAL_ETH_ConfigMAC(&EthHandle, (ETH_MACInitTypeDef *)NULL);

        /* Restart MAC interface */
        HAL_ETH_Start(&EthHandle);
    } else {
        /* Stop MAC interface */
        HAL_ETH_Stop(&EthHandle);
    }

    ethernetif_notify_conn_changed(netif);
}

#if LWIP_DHCP

static int dhcp_entry(struct task_pcb *task) {
    struct netif *netif = task->user_data;
    struct ethernetif *ethernetif = netif->state;

    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;
    struct dhcp *dhcp;
    uint8_t iptxt[20];

    switch (ethernetif->dhcp_state) {
        case DHCP_START: {
            ip_addr_set_zero_ip4(&netif->ip_addr);
            ip_addr_set_zero_ip4(&netif->netmask);
            ip_addr_set_zero_ip4(&netif->gw);
            ethernetif->dhcp_state = DHCP_WAIT_ADDRESS;
            dhcp_start(netif);

            LOG_I("  State: Looking for DHCP server ...");
        } break;

        case DHCP_WAIT_ADDRESS: {
            if (dhcp_supplied_address(netif)) {
                ethernetif->dhcp_state = DHCP_ADDRESS_ASSIGNED;

                snprintf((char *)iptxt, sizeof(iptxt), "%s", ip4addr_ntoa((const ip4_addr_t *)&netif->ip_addr));
                LOG_I("IP address assigned by a DHCP server: %s", iptxt);
            } else {
                dhcp = (struct dhcp *)netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

                /* DHCP timeout */
                if (dhcp->tries > MAX_DHCP_TRIES) {
                    ethernetif->dhcp_state = DHCP_TIMEOUT;

                    /* Stop DHCP */
                    dhcp_stop(netif);

                    /* Static address used */
                    IP_ADDR4(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
                    IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
                    IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
                    netif_set_addr(netif, &ipaddr, &netmask, &gw);

                    snprintf((char *)iptxt, sizeof(iptxt), "%s", ip4addr_ntoa((const ip4_addr_t *)&netif->ip_addr));
                    LOG_I("DHCP Timeout !!");
                    LOG_I("Static IP address: %s", iptxt);
                }
            }
        } break;
        case DHCP_LINK_DOWN: {
            /* Stop DHCP */
            dhcp_stop(netif);
            ethernetif->dhcp_state = DHCP_OFF;
        } break;
        default:
            break;
    }

    return 0;
}

#endif /* LWIP_DHCP */

enum {
    PHY_LINK = (1 << 0),
    PHY_100M = (1 << 1),
    PHY_FULL_DUPLEX = (1 << 2),
};

#define TASK_ETHERNETIF_LINK_RUN_PERIOD 500
static int phy_linkchange_entry(struct task_pcb *task) {
    static uint8_t phy_speed = 0;

    struct netif *netif = task->user_data;
    uint8_t phy_speed_new = 0;
    uint32_t phyreg = 0;

    if (HAL_ETH_ReadPHYRegister(&EthHandle, PHY_BSR, &phyreg) != HAL_OK) {
        LOG_E("read phy basic reg faild");
        return 0;
    }

    LOG_D("phy basic status reg is 0x%X", phyreg);

    if (phyreg & (PHY_LINKED_STATUS | PHY_AUTONEGO_COMPLETE)) {
        phyreg = 0;

        phy_speed_new |= PHY_LINK;

        if (HAL_ETH_ReadPHYRegister(&EthHandle, PHY_SR, &phyreg) != HAL_OK) {
            LOG_E("read phy status reg faild");
            return 0;
        }
        LOG_D("phy control status reg is 0x%X", phyreg);

        if ((phyreg & PHY_SPEED_STATUS) != PHY_SPEED_STATUS) {
            phy_speed_new |= PHY_100M;
        }

        if (phyreg & PHY_DUPLEX_STATUS) {
            phy_speed_new |= PHY_FULL_DUPLEX;
        }
    }

    if (phy_speed != phy_speed_new) {
        phy_speed = phy_speed_new;
        if (phy_speed & PHY_LINK) {
            LOG_I("link up");
            if (phy_speed & PHY_100M) {
                LOG_I("100Mbps");
            } else {
                LOG_I("10Mbps");
            }

            if (phy_speed & PHY_FULL_DUPLEX) {
                LOG_I("full-duplex");
            } else {
                LOG_I("half-duplex");
            }

            netif_set_link_up(netif);
        } else {
            LOG_I("link down");
            netif_set_link_down(netif);
        }
    }

    return 0;
}

#define TASK_ETHERNETIF_INPUT_RUN_PERIOD 0
static int ethernetif_input_entry(struct task_pcb *task) {
    struct netif *netif = task->user_data;

    ethernetif_input(netif);

    return 0;
}

int ethernetif_system_init(void) {
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;

#if LWIP_DHCP
    ip_addr_set_zero_ip4(&ipaddr);
    ip_addr_set_zero_ip4(&netmask);
    ip_addr_set_zero_ip4(&gw);
#else
    IP_ADDR4(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
    IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
    IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
#endif /* LWIP_DHCP */

    /* Add the network interface */
    netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, ethernetif_init, ethernet_input);

    /*  Registers the default network interface */
    netif_set_default(&gnetif);

    /* When the netif is fully configured this function must be called */
    netif_set_up(&gnetif);

    /* Set the link callback function, this function is called on change of link status*/
    netif_set_link_callback(&gnetif, ethernetif_update_config);

    task_init(TASK_ETHERNETIF_INPUT, ethernetif_input_entry, &gnetif, TASK_ETHERNETIF_INPUT_RUN_PERIOD);
    task_start(TASK_ETHERNETIF_INPUT);

    task_init(TASK_ETHERNETIF_LINK, phy_linkchange_entry, &gnetif, TASK_ETHERNETIF_LINK_RUN_PERIOD);
    task_start(TASK_ETHERNETIF_LINK);

#if LWIP_DHCP
    task_init(TASK_ETHERNETIF_DHCP, dhcp_entry, &gnetif, DHCP_FINE_TIMER_MSECS);
    task_start(TASK_ETHERNETIF_DHCP);
#endif /* LWIP_DHCP */

    return 0;
}

#endif /* 0 */
