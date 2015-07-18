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

#ifndef _OF_MCAST_H
#define _OF_MCAST_H

/****************************************************************************************
 * * * * * * * * * * * * * * * STRUCTURE DEFINITIONS * * * * * * * * * * * * * * * * *
 *****************************************************************************************/


#define IP4_MCFWD_MAX_VIFS    32 

/** Local structure definition other than main structure **/

/* structure for configuring multicast forward flow  entries */
typedef struct of_mcast_info
{
  uint32_t mcastgrp; /* multicast group address */
  uint32_t src_ip; /* source address for the packet */
  uint32_t vif_id; /* index of the virtual network device in the vif table over which packets this MF entry should arrive */
  uint32_t minvif; /*minimum vif value, indexes to elements in the ttls list */
  uint32_t maxvif; /* maximum vif value, indexes to elements in the ttls list */
}of_mcast_info_t;


/****************************************************************************************
 * * * * * * * * * * * * * * * FUNCTION PROTOTYPES * * * * * * * * * * * * * * * * * * *
 *****************************************************************************************/
int32_t mcast_get_first_record(uint32_t ns_id,struct of_mcast_info *mcast_rec_p);
int32_t mcast_get_next_record(uint32_t ns_id, struct of_mcast_info *mcast_rec_p);
int32_t mcast_get_exact_record(uint32_t ns_id, struct of_mcast_info *mcast_rec_p);


#endif

