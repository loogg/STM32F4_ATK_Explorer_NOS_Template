#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/netif.h"

#include "system.h"
#include "shell.h"

#ifdef SYSTEM_USING_CONSOLE_SHELL
void list_if(void) {
    uint32_t index;
    struct netif *netif;

    netif = netif_list;

    while (netif != NULL) {
        SYSTEM_PRINTF("network interface: %c%c%s\r\n", netif->name[0], netif->name[1], (netif == netif_default) ? " (Default)" : "");
        SYSTEM_PRINTF("MTU: %d\r\n", netif->mtu);
        SYSTEM_PRINTF("MAC: ");
        for (index = 0; index < netif->hwaddr_len; index++) SYSTEM_PRINTF("%02x ", netif->hwaddr[index]);
        SYSTEM_PRINTF("\r\nFLAGS:");
        if (netif->flags & NETIF_FLAG_UP)
            SYSTEM_PRINTF(" UP");
        else
            SYSTEM_PRINTF(" DOWN");
        if (netif->flags & NETIF_FLAG_LINK_UP)
            SYSTEM_PRINTF(" LINK_UP");
        else
            SYSTEM_PRINTF(" LINK_DOWN");
        if (netif->flags & NETIF_FLAG_ETHARP) SYSTEM_PRINTF(" ETHARP");
        if (netif->flags & NETIF_FLAG_BROADCAST) SYSTEM_PRINTF(" BROADCAST");
        if (netif->flags & NETIF_FLAG_IGMP) SYSTEM_PRINTF(" IGMP");
        SYSTEM_PRINTF("\r\n");
        SYSTEM_PRINTF("ip address: %s\r\n", ipaddr_ntoa(&(netif->ip_addr)));
        SYSTEM_PRINTF("gw address: %s\r\n", ipaddr_ntoa(&(netif->gw)));
        SYSTEM_PRINTF("net mask  : %s\r\n", ipaddr_ntoa(&(netif->netmask)));
#if LWIP_IPV6
        {
            ip6_addr_t *addr;
            int addr_state;
            int i;

            addr = (ip6_addr_t *)&netif->ip6_addr[0];
            addr_state = netif->ip6_addr_state[0];

            SYSTEM_PRINTF("\r\nipv6 link-local: %s state:%02X %s\r\n", ip6addr_ntoa(addr), addr_state,
                          ip6_addr_isvalid(addr_state) ? "VALID" : "INVALID");

            for (i = 1; i < LWIP_IPV6_NUM_ADDRESSES; i++) {
                addr = (ip6_addr_t *)&netif->ip6_addr[i];
                addr_state = netif->ip6_addr_state[i];

                SYSTEM_PRINTF("ipv6[%d] address: %s state:%02X %s\r\n", i, ip6addr_ntoa(addr), addr_state,
                              ip6_addr_isvalid(addr_state) ? "VALID" : "INVALID");
            }
        }
        SYSTEM_PRINTF("\r\n");
#endif /* LWIP_IPV6 */
        netif = netif->next;
    }

#if LWIP_DNS
    {
        const ip_addr_t *ip_addr;

        for (index = 0; index < DNS_MAX_SERVERS; index++) {
            ip_addr = dns_getserver(index);
            SYSTEM_PRINTF("dns server #%d: %s\r\n", index, inet_ntoa(ip_addr));
        }
    }
#endif /**< #if LWIP_DNS */
}
SHELL_EXPORT_CMD(list_if, list_if, list network interface);
#endif /* SYSTEM_USING_CONSOLE_SHELL */
