#include "rl_net_lib.h"
#include "LPC17XX.h"

#include "HTTP_Server.h"



uint8_t net_mac_address	[6];
uint8_t net_ip_address	[4];


extern  LOCALM localm[];
#define LocM   localm[NETIF_ETH]



void get_network_data () {
	uint32_t sa_value;
	
	/* Ethernet MAC Addres */
	sa_value = LPC_EMAC->SA0;
	net_mac_address[4] = (uint8_t) sa_value;
  net_mac_address[5] = (uint8_t)(sa_value >> 8);
  sa_value = LPC_EMAC->SA1;
  net_mac_address[2] = (uint8_t) sa_value;
	net_mac_address[3] = (uint8_t)(sa_value >> 8);
  sa_value = LPC_EMAC->SA2;
	net_mac_address[0] = (uint8_t) sa_value;
  net_mac_address[1] = (uint8_t)(sa_value >> 8);
	
	/* IPv4 Address */
	net_ip_address[0] = LocM.IpAddr[0];
	net_ip_address[1] = LocM.IpAddr[1];
	net_ip_address[2] = LocM.IpAddr[2];
	net_ip_address[3] = LocM.IpAddr[3];
}
