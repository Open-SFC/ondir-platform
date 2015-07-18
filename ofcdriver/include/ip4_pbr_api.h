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

#ifndef _OF_PBR_H
#define _OF_PBR_H

/****************************************************************************************
 * * * * * * * * * * * * * * * STRUCTURE DEFINITIONS * * * * * * * * * * * * * * * * *
 *****************************************************************************************/


//#define IP4_MCFWD_MAX_VIFS    32 

/** Local structure definition other than main structure **/


/* structure for configuring multicast forward flow  entries */
typedef struct of_ip4_fwd_pbr_db_entry_info
{
   uint16_t priority; /*!< Priority of the rule */
   uint32_t src_addr; /*!< Source IP address*/
   uint32_t src_mask; /*!< Source IP mask value */
   uint32_t dst_addr; /*!< Destination IP address*/
   uint32_t dst_mask; /*!< Destination IP mask value */
   uint16_t flags; /*!< valid in ifid, valid out ifid, permanent rule*/
   uint32_t rule_id; /*!< Target policy ID, if action is goto rule*/
   uint32_t rt_table_no; /*!< Route table number, if action is goto table */
   uint32_t in_port;
   uint32_t out_port;

}of_ip4_fwd_pbr_db_entry_info_t;


/****************************************************************************************
 * * * * * * * * * * * * * * * FUNCTION PROTOTYPES * * * * * * * * * * * * * * * * * * *
 *****************************************************************************************/
int32_t of_ip4_fwd_pbr_get_first_record(uint32_t ns_id,struct of_ip4_fwd_pbr_db_entry_info *mcast_rec_p);
int32_t of_ip4_fwd_pbr_get_next_record(uint32_t ns_id,struct of_ip4_fwd_pbr_db_entry_info *mcast_rec_p);
int32_t of_ip4_fwd_pbr_get_exact_record(uint32_t ns_id,struct of_ip4_fwd_pbr_db_entry_info *mcast_rec_p);


#endif

