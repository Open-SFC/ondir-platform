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

#ifndef _OF_ARP_H
#define _OF_ARP_H

/****************************************************************************************
 * * * * * * * * * * * * * * * STRUCTURE DEFINITIONS * * * * * * * * * * * * * * * * *
 *****************************************************************************************/
#define IFACE_HWADDR_LEN 6
typedef struct arp_record
{
   uint8_t  dst_hwaddr[IFACE_HWADDR_LEN]; /* MAC address */
   uint32_t dest_ipaddr;/* IP Address */
   uint32_t flags; /*static, dynamic */
   uint16_t state; /* FREE, INCOPMPLETE, RESOLVED */
   uint32_t if_index; /* Router can be connected to different networks each with a different interface number*/

}arp_record_t;


/****************************************************************************************
 * * * * * * * * * * * * * * * FUNCTION PROTOTYPES * * * * * * * * * * * * * * * * * * *
 *****************************************************************************************/
int32_t arp_get_first_record(uint32_t ns_id,struct arp_record *arp_rec_p);
int32_t arp_get_next_record(uint32_t ns_id, struct arp_record *arp_rec_p);
int32_t arp_get_exact_record(uint32_t ns_id, struct arp_record *arp_rec_p);

#endif
