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

#ifndef _OF_IPV4FWDGW_H
#define _OF_IPV4FWDGW_H
extern struct of_ip4_fwd_route_info *ip4_fwd_route_rec_p;
/****************************************************************************************
 * * * * * * * * * * * * * * * STRUCTURE DEFINITIONS * * * * * * * * * * * * * * * * *
 *****************************************************************************************/
//#define IP4_MCFWD_MAX_VIFS    32 

/** Local structure definition other than main structure **/

/* structure for configuring ipv4 route entries */

typedef struct of_ip4_fwd_gw_info{
        uint32_t gw;
        uint32_t port_id;
        uint32_t path_mtu;
}of_ip4_fwd_gw_info_t;


/****************************************************************************************
 * * * * * * * * * * * * * * * FUNCTION PROTOTYPES * * * * * * * * * * * * * * * * * * *
 *****************************************************************************************/
int32_t of_ip4_fwd_gw_get_first_record(uint32_t ns_id, struct of_ip4_fwd_route_info *ip4_fwd_route_rec_p,of_ip4_fwd_gw_info_t *ip4_fwd_gw_rec_p);
int32_t of_ip4_fwd_gw_next_record(uint32_t ns_id, struct of_ip4_fwd_route_info *ip4_fwd_route_rec_p, of_ip4_fwd_gw_info_t *ip4_fwd_gw_rec_p);
int32_t of_ip4_fwd_gw_exact_record(uint32_t ns_id, struct of_ip4_fwd_route_info *ip4_fwd_route_rec_p, of_ip4_fwd_gw_info_t *ip4_fwd_gw_rec_p);

#endif

