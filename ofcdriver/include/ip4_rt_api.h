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

#ifndef _OF_IPV4FWDRT_H
#define _OF_IPV4FWDRT_H

/****************************************************************************************
 * * * * * * * * * * * * * * * STRUCTURE DEFINITIONS * * * * * * * * * * * * * * * * *
 *****************************************************************************************/
//#define IP4_MCFWD_MAX_VIFS    32 

/** Local structure definition other than main structure **/

/* structure for configuring ipv4 route entries */

typedef struct of_ip4_fwd_route_info{
        uint32_t ip_addr;
        uint32_t net_mask;
        uint32_t metric;
}of_ip4_fwd_route_info_t;


/****************************************************************************************
 * * * * * * * * * * * * * * * FUNCTION PROTOTYPES * * * * * * * * * * * * * * * * * * *
 *****************************************************************************************/
int32_t of_ip4_fwd_route_get_first_record(uint32_t ns_id, struct of_ip4_fwd_route_info *ip4_fwd_route_rec_p);
int32_t of_ip4_fwd_route_get_next_record(uint32_t ns_id, struct of_ip4_fwd_route_info *ip4_fwd_route_rec_p);
int32_t of_ip4_fwd_route_get_exact_record(uint32_t ns_id, struct of_ip4_fwd_route_info *ip4_fwd_route_rec_p);

/*LIM Stats */
int32_t get_unicast_lim_stats(uint32_t ns_id, struct ip4_unicast_lim_stats *out_ip4_unicast_lim_stats_p);

#endif

