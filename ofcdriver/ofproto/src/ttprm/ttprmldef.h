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

/*******************************************************************************
* Copyright 2013-2014, Freescale Semiconductor, Inc. All rights reserved.
********************************************************************************
* File       :ttprmldef.h
* Description:This file contains the ttprm module structures,function prototypes
* Authors:      Varun Kumar<B36461@freescale.com>
* Modifier:
* History
* 29 Jan 2014 - Varun Kumar - Initial Implementation.
*********************************************************************************/

#ifndef __TTPRM_LDEF_H__
#define __TTPRM_LDEF_H__

/** max no of tables may present in ttprm node**/
#define CNTLR_MAX_NUMBER_OF_TTPS 8
#define CNTLR_TTPRM_MAX_TBLS   48
#define CNTLR_TTPRM_MAX_MATCH_FIELDS   48

#define TTPRM_MAX_ENTRIES       64
#define TTPRM_STATIC_ENTRIES    16
#define TTPRM_TABLE_SIZE        16


#if 0
/* MAXIMUM Number of async messages or events supported*/
#define OF_FIRST_ASYNC_EVENT     OF_ASYNC_MSG_PACKET_IN_EVENT /* ASSUMPTION  - This value is always 1*/
#define OF_LAST_ASYNC_EVENT      OF_ASYNC_MSG_ERROR_EVENT
#define OF_MAX_NUMBER_ASYNC_MSGS (OF_LAST_ASYNC_EVENT - OF_FIRST_ASYNC_EVENT) + 1
#endif

/** structure having table id and registered callbacks for  tabled id **/
struct ttprm_async_msg_cbks
{
  /** table id **/
//  uint8_t tbl_id;
  uint8_t is_valid;
  /**  callbacks **/
  struct async_event_info event_info[OF_MAX_NUMBER_ASYNC_MSGS + 1];
};


struct ttprm_entry
{
  char    main_ttp_name[CNTLR_MAX_TTP_NAME_LEN];
  /* number of tables attached to ttp */
  uint32_t number_of_tables;
  /* table list */
  of_list_t table_list;
  /* number of domains attached to ttp*/
  uint32_t number_of_domains;
  /* domains list */
  of_list_t domain_list;
  struct mchash_table_link ttprm_link;
  /** table ids and registered callbacks **/
  struct ttprm_async_msg_cbks async_msg_cbks[CNTLR_TTPRM_MAX_TBLS];
  uint32_t magic;
  uint32_t index;
  uint64_t main_ttp_handle;
  uint8_t  heap_b;
};
#define TTPRM_NODE_OFFSET offsetof(struct ttprm_entry, ttprm_link)

void    ttprm_get_mempoolentries(uint32_t *max_entries, uint32_t *static_entries);
int32_t ttprm_mempool_init();
int32_t ttprm_hash_table_init();
int32_t ttprm_init();
int32_t ttprm_deinit();
int32_t ttprm_domain_mempool_init();
int32_t ttprm_table_mempool_init();
int32_t ttprm_match_field_mempool_init();
void    ttprm_free_node_entry_rcu(struct rcu_head *ttprm_entry_p);




#define TTPRM_DOBBS_MIX(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}
#define TTPRM_DOBBS_WORDS4(word_a, word_b, word_c, word_d, initval, \
    hashmask, hash) \
{ \
  register unsigned long temp_a, temp_b, temp_c; \
  temp_a = temp_b = 0x9e3779b9;  /* the golden ratio; an arbitrary value */\
  temp_c = initval;         /* random value*/ \
  \
  temp_a += word_a; \
  temp_b += word_b; \
  temp_c += word_c; \
  \
  TTPRM_DOBBS_MIX(temp_a, temp_b, temp_c); \
  \
  temp_a += word_d; \
  TTPRM_DOBBS_MIX(temp_a, temp_b, temp_c); \
  \
  hash = temp_c & (hashmask-1); \
}

#define TTPRM_COMPUTE_HASH(param1,param2,parm3,parm4,param5,hashkey) \
  TTPRM_DOBBS_WORDS4(param1,param2,param3,param4,0xa2956174,param5,hashkey)
//uint32_t ttprm_get_hashval_byname(char* name_p,uint32_t no_of_buckets);

#endif

