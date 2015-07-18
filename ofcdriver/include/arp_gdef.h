/**
 * Copyright (c) 2012, 2013  Freescale.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 **/
#ifndef __ARP_GDEF_H
#define __ARP_GDEF_H
#define ND_MAX_IFACE_NAME_LEN  16
#define IFACE_NAME_LEN        (32)
#define IFACE_HWADDR_LEN 6



/**************************************************************************//**
@Description   ARP packet structure used to send ARP Request to DP
*//***************************************************************************/
struct arp_pkt
{
  uint32_t uiNsid; /*Namespace ID */
  uint16_t usHwAddrtype; /*Hardware type - 1 for Ethernet */
  uint16_t usProtoType; /*Protocol type - 0x0800 for ip, 0x0806 for arp */
  uint8_t ucHwaddrLen; /*Hardware address length - 6 for ethernet */
  uint8_t ucProtoLen; /*length of address used by upper layer protocol, 4 for ipv4*/
  uint16_t ucOpcode; /* 1 for ARP request and 2 for ARP reply */ 
  uint8_t SenderHwAddr[IFACE_HWADDR_LEN];
  uint32_t uiSenderAddr;    
  uint8_t TargetHwAddr[IFACE_HWADDR_LEN];
  uint32_t uiTargetAddr; 
  char cIfaceName[IFACE_NAME_LEN]; /*Interface Name */
  uint32_t uiPort; /*port on which the response/request should be sent. will be outport for arp reply and inport for arp request*/
};

struct arpapp_arp_pkt
{
  uint32_t nsid; /*Namespace ID */
  uint32_t ifid;  /*Interface ID*/
  //uint32_t ip_addr;
  //char mac_addr[IFACE_HWADDR_LEN];/*Destination MAC Address */
  char peth_name[ND_MAX_IFACE_NAME_LEN];  /*Interface ID*/
  struct pkt_mbuf *pPkt; /*PSP buf that contains entire IP Packet received from DP*/
};
/**************************************************************************//**
@Description   The IP Pkt whose mac address is not resolved in the DP will send the IP pkt.
	       This structure is for IP packet received from DP for MAC Address resolution.
*//***************************************************************************/
struct arpapp_ip_pkt
{
  uint32_t nsid; /*Namespace ID */
  uint32_t gwipaddr; /*gateway IP Address  -- deepthi modified fron destipaddr*/
  uint32_t ifid;  /*Interface ID*/ 
  char peth_name[ND_MAX_IFACE_NAME_LEN];  /*Interface ID*/
  struct pkt_mbuf *pkt_mbuf_p; /*PSP buf that contains entire IP Packet received from DP*/
  void *opaque_p;
  uint64_t dp_id;
  uint32_t in_port;
  of_list_node_t           node_entry;
};
#if 0
struct arpapp_pkt_in_data{
  uint64_t dp_id;
  uint32_t port_id;
};
#endif
#define OF_ARP_PKT_IN_LISTNODE_OFFSET offsetof(struct arpapp_ip_pkt, node_entry)
#endif
