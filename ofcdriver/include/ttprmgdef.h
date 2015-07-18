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

/**************************************************************************
* File name: ttprmgdef.h
* Author: Varun Kumar Reddy <B36461@freescale.com>
* Date:   
* Description: 
*       This file contains the global ttprm module  macros,data structures 
*	and functions.
***************************************************************************/
#ifndef __TTPRM_GDEF_H__
#define __TTPRM_GDEF_H__

#define CNTLR_TTP_SUCCESS  0//success return
#define CNTLR_TTP_FAILURE -1//failure return

/** ttp name len in ttprm node **/
#define CNTLR_MAX_TTP_NAME_LEN  64



enum ttprm_table_types
{
  TABLE_EXACT_MATCH=0,
  ACL,
  LPM
};

enum match_field_types
{
  MATCH_EXACT_MATCH=0,
};


/* Applications detils for the given async message event. 
 * Appliations might register to revieve events with priority. 
 **/
struct async_event_info
{
  uint8_t   type; /** Message or event type*/
  /* Details of each application registered, 
   * list of nodes type 'struct of_app_hook_info' values*/
  of_list_t app_list;
};

struct ttprm_tbl_cfg
{
  uint8_t tbl_id;
  char tbl_name[OFP_MAX_TABLE_NAME_LEN];
  uint32_t max_rows, max_columns;
  uint8_t is_lpm_present,is_wc_present;
  uint32_t no_of_lpms,no_of_wcs;
};


struct ttprm_table_entry
{
  uint8_t         table_id;
  char            table_name[OFP_MAX_TABLE_NAME_LEN];
  uint32_t        max_rows;
  uint32_t        max_columns;// no of match fileds
  uint8_t         is_wc_present;//true if wildcards presents in the table.
  uint32_t        no_of_wcs;//no of wild card entries.
  uint8_t         is_lpm_present;//true if lpm presents in the table.
  uint32_t        no_of_lpms;//no of lpm entries.
  of_list_t       match_fields_list;     
  uint64_t        ttprm_handle;//main-ttprm or sub-ttp any one.
  of_list_node_t  next_node;
  struct rcu_head rcu_head;
  uint8_t         heap_b;
};

#define TTPRM_TBL_LST_NODE_OFFSET offsetof(struct ttprm_table_entry,next_node)
#define TTPRM_TBL_LST_RCU_OFFSET  offsetof(struct ttprm_table_entry,rcu_head)

struct ttprm_tbl_match_fields_cfg
{
  uint32_t match_field;
  uint8_t  is_wc;
  uint8_t  is_lpm;
};

struct table_match_field_entry
{
  of_list_node_t  next_node;
  struct rcu_head rcu_head;
  uint8_t         heap_b;
  uint32_t        match_field;
  //uint32_t match_type;
  uint8_t        is_wild_card;
  uint8_t        is_lpm;
};

#define TTPRM_TBL_MATCH_FIELD_LST_NODE_OFFSET     \
            offsetof(struct table_match_field_entry, next_node)
#define TTPRM_TBL_MATCH_FIELD_LST_RCU_HEAD_OFFSET \
            offsetof(struct table_match_field_entry, rcu_head)

struct ttprm_domain_entry
{
  of_list_node_t  next_node;
  struct rcu_head rcu_head;
  uint8_t         heap_b;
  char            domain_name[DPRM_MAX_FORWARDING_DOMAIN_LEN];
  uint64_t        domain_handle;
};
#define TTPRM_DOMAIN_LST_NODE_OFFSET     \
        offsetof(struct ttprm_domain_entry, next_node)
#define TTPRM_DOMAIN_LST_RCU_HEAD_OFFSET \
        offsetof(struct ttprm_domain_entry, rcu_head)


struct ttprm_config_info
{
  char     name[CNTLR_MAX_TTP_NAME_LEN];
  uint32_t no_of_tbls;
  uint32_t no_of_domains;
};

struct ttprm_tbl_info  
{
  uint8_t tbl_id;
  char tbl_name[OFP_MAX_TABLE_NAME_LEN];
  uint32_t max_rows,max_columns;
};
struct  match_field_info  
{
  uint32_t match_field_id;
  uint8_t is_wild_card; 
  uint8_t is_lpm ;
};




int32_t add_domain_to_ttprm(char *ttp_name_p, 
                            char *domain_name_p, 
                            uint64_t domain_handle);

int32_t detach_domain_from_ttprm(char *ttp_name_p, char *domain_name_p);
int32_t alloc_ttprm_tbl_mem_block(struct ttprm_table_entry **tbl_entry_p_p);
int32_t add_match_field_to_table(of_list_t *ttprm_table_list_p,
                                 uint32_t match_field,
                                 uint8_t is_wild_card,
                                 uint8_t is_lpm);
int32_t ttprm_tbl_match_field_list_deinit(char *ttp_name_p, uint8_t tbl_id);




int32_t
cntlr_ttp_dp_ready_event(uint64_t  controller_handle,
                         uint64_t   domain_handle,
                         uint64_t   datapath_handle,
                         void       *cbk_arg1,
                         void       *cbk_arg2);



void ttprm_tbl_node_free_entry_rcu(struct rcu_head *table_entry_p);
void ttp_free_table_match_field_entry_rcu(
                        struct rcu_head *table_match_filed_entry_p);
void ttprm_domain_node_free_entry_rcu(struct rcu_head *domain_entry_p);
int32_t add_table_to_ttprm(char *main_ttp_name_p, 
                           struct ttprm_table_entry **tbl_entry_p_p);

void
cntlr_handle_tbl_ftr_rpl_msg(struct of_msg *msg,
                             uint64_t controller_handle,
                             uint64_t domain_handle,
                             uint64_t datapath_handle,
                             uint8_t  table_id,
                             void  *cbk_arg1,
                             void  *cbk_arg2,
                             uint8_t status,
                             struct   ofp_table_features  *tbl_ftr_rply_p,
                             uint8_t more);

int32_t get_ttprm_async_event_info(char *main_ttp_name_p, 
                                   uint8_t tbl_id, 
                                   struct async_event_info **event_info_p_p);

int32_t ttprm_register(char *main_ttp_name_p, uint64_t *output_ttprm_handle_p);

int32_t alloc_ttp_tbl_mem_block(struct ttprm_table_entry **tbl_entry_p_p);

int32_t ttp_tbl_match_field_list_deinit(char *main_ttp_name_p, 
                                        uint8_t ttprm_tbl_id);

int32_t del_match_fields_from_table(
             struct  ttprm_table_entry *ttprm_table_entry_p);

int32_t del_table_from_ttprm(char *main_ttp_name_p, uint8_t ttprm_tbl_id);

int32_t get_ttprm_table_lst(char *main_ttp_name_p, 
                            of_list_t **ttprm_table_list_p, 
                            uint32_t *no_of_tables_p);

int32_t cntlr_ttprm_tbl_feature_len(char *main_ttp_name_p, 
                                    uint16_t *msg_len_p);

int32_t cntlr_ttprm_build_tbl_ftr_msg(char *main_ttp_name_p, 
                                      struct  of_msg  *msg);

void list_ttprm_tbls(char *main_ttp_name_p);

uint32_t ttprm_get_hashval_byname(char* name_p,uint32_t no_of_buckets);

int32_t ttprm_register_table(char *main_ttp_name_p, 
                            uint32_t match_fields_cnt,
                            struct ttprm_tbl_match_fields_cfg *match_fields_info_p,
                            struct ttprm_tbl_cfg *tbl_cfg_info_p);

int32_t ttprm_register_matchfields(of_list_t *ttp_table_list_p,
        struct ttprm_tbl_match_fields_cfg *match_fields_info_p,
 			          uint32_t match_fields_cnt);


#endif//__TTPGDEF_API_H


