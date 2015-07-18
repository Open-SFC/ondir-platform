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

/******************************************************************************
* Copyright 2013-2014, Freescale Semiconductor, Inc. All rights reserved.
*******************************************************************************/
/*File: ipsec_be_ldef.h
 *Author: Narayana  Rao KVV <narayana.k@freescale.com>
 * Description:
 * This file contails IPSec application registered with Forward Domain 
 */
#ifndef IPSEC_LDEF_H
#define IPSEC_LDEF_H

#define  FSLX_DP_NS_ID_FIELD_LEN    6

/* IPSec Bind Object Cookie format */

/* SPD Policy Bind Object */
/* |    Policy Id   | Sub_type | Reserved |  */
/* 0                31         39             63 */  

/* SA Bind Object */
/* |     SPI        | Sub_type |   Reserved   |  */
/* 0                31          39             63 */  

enum ipsec_bind_obj_app_sub_type {
  IPSEC_BO_APP_SUB_TYPE_SPD_IN = 1,
  IPSEC_BO_APP_SUB_TYPE_SPD_OUT,
  IPSEC_BO_APP_SUB_TYPE_SA_IN,
  IPSEC_BO_APP_SUB_TYPE_SA_OUT,
  IPSEC_BO_MAX_APPL_ID 
};

enum ofp_ipsec_net_type{
  OFP_IPSEC_ADDR_TYPE_IPV4 = 1,
  OFP_IPSEC_ADDR_TYPE_IPV6 = 2
};

struct ofp_ipsec_address{
  enum ofp_ipsec_net_type  type; 
  union {
  uint32_t ipv4;
  uint8_t  ipv6[16]; 
  };
};

struct ipsec_sa_id_info
{
  uint32_t spi;
  uint8_t  sec_protocol;
  struct ofp_ipsec_address dest_addr;
};

struct ipsec_db_datapath_info {
  uint32_t            max_pol_buckets;
  /* Inbound SPD Policy */
  struct mchash_table *in_pol_table; 
  /* Outbound SPD Policy */
  struct mchash_table *out_pol_table; 

  uint32_t            max_sa_buckets;
  /* Inbound MC SA Table with SPI as Hash Index */
  struct mchash_table *in_sa_table; 
  /* Outbound MC SA Table with SPI as Hash Index */
  struct mchash_table *out_sa_table;
};

struct ipsec_db_datapath_entry {
  struct mchash_table_link     next_link;
  uint64_t            datapath_id;
  uint32_t            ns_id; 
  uint16_t            dp_ns_id; 
  uint8_t             redside_frag_is_enable;  
  struct ipsec_db_datapath_info ipsec_dp;
  struct saferef      dp_saferef;
  uint8_t             is_heap;
};
#define IPSEC_DB_DP_TBL_LINK_OFFSET offsetof(struct ipsec_db_datapath_entry,next_link)

/* Inbound/Outbound SPD Policy Array Based Indirection */
struct ipsec_db_policy_entry {
  struct mchash_table_link next_link;
  struct rcu_head       rcu_head;
  struct saferef        safe_ref;
  uint32_t              id;
  int32_t               priority;
  of_list_t             sa_list;
  uint32_t              bind_obj_id;
  uint8_t               is_heap;
};
#define IPSEC_DB_POL_TBL_LINK_OFFSET offsetof(struct ipsec_db_policy_entry, next_link)

struct ipsec_pol_sa_entry {
  of_list_node_t  next_node;
  struct rcu_head rcu_head;                           
  uint8_t         is_heap;
  struct saferef  safe_ref;
};

/* Inbound/Outbound MC SA Table with SPI as Hash Index */
struct ipsec_db_sa_entry {
  struct mchash_table_link next_link;
  struct ipsec_sa_id_info sa_id;
  uint32_t            bind_obj_id;
  struct saferef      safe_ref;
  uint8_t             is_heap;
};
#define IPSEC_DB_SA_TBL_LINK_OFFSET offsetof(struct ipsec_db_sa_entry, next_link)

enum peth_info_flags{
  IP_VERSION_IPV4,
  IP_VERSION_IPV6
};

struct peth_headroom_info
{
  uint8_t table_id;
  uint8_t event_type;
  enum peth_info_flags flags;
  union{
    uint32_t policy_id;
    struct ipsec_sa_id_info sa_id;
  };
};

#define IPV4_BITS_TO_MASK(bits, mask) \
{\
  if(bits > 32) mask = 0xffffffff; \
  else if(bits == 0) mask = 0x0; \
  else mask = ((~(uint32_t)0) << (32 - bits)); \
}

#define IPSEC_DECRYPTED_PACKET 1
#define IPSEC_ENCRYPTED_PACKET 2
#define IPSEC_ENCRYPT_AND_SEND_PACKET 3
#endif /* IPSEC_LDEF_H */

