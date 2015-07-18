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

#ifndef __IPV4_FWD_TTP_H__
#define __IPV4_FWD_TTP_H__
//#include "sd_router_ttp.h"
#if 1
#define IPV4_FWD_TTP_IN_PORT_CLASSIFY_TABLE_ID	(0)
#define IPV4_FWD_TTP_ETH_FILTERING_TABLE_ID	(1)
#define IPV4_FWD_TTP_ETH_CLASSIFY_TABLE_ID	(2)
#define IPV4_FWD_TTP_ARP_INBOUND_TABLE_ID	(3)
#define IPV4_FWD_TTP_IPV4_CLASSIFY_TABLE_ID	(4)
#define IPV4_FWD_TTP_IPV4_PBR_TABLE_ID		(7)
#define IPV4_FWD_TTP_IPV4_ROUTING_TABLE_ID	8
//#define IPV4_FWD_TTP_IPV4_MCAST_GROUP_TABLE_ID  (10)
//#define IPV4_FWD_TTP_IPV4_MCAST_ROUTING_TABLE_ID (11)

//#define IPSEC_APP_SELF_CLASSIFICATION_TABLE_ID   12
//#define IPSEC_APP_PACKET_CLASSIFICATION_TABLE_ID 13
//#define IPSEC_APP_UDP_ENCAP_NATT_TABLE_ID        14
//#define IPSEC_APP_IN_SA_TABLE_ID                 15
//#define IPSEC_APP_IN_POLICY_TABLE_ID             16
//#define IPSEC_APP_OUT_POLICY_TABLE_ID            18
//#define IPSEC_APP_OUT_SA_TABLE_ID                19


#define IPV4_FWD_TTP_ARP_RESOLUTION_TABLE_ID	 20
#if 1
/*Table id20:QOS Filter Table */
//#define SD_ROUTER_TTP_QOS_FILTER_TBL_ID            21
/*Table id21:QOS config Table */
//#define SD_ROUTER_TTP_QOS_CONFIG_TBL_ID            22
#endif
#endif


//#define IPV4_FWD_APP_PKT_IN_PRIORITY     (2)


#endif /* __IPV4_FWD_TTP_H__ */
