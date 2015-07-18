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
*******************************************************************************
* File:ttprm_api.c
* Description: This file implements the ttprm module init routines.
* Authors:      Varun Kumar<B36461@freescale.com>
* History:
* 29 Jan 2014 - Varun Kumar - Initial Implementation.
*******************************************************************************/
#include "controller.h"
#include "cntlr_event.h"
#include "cntlr_tls.h"
#include "cntrl_queue.h"
#include "cntlr_transprt.h"
#include "of_multipart.h"
#include "fsl_ext.h"
#include "dprmldef.h"
#include "dprm.h"
#include "ttprmgdef.h"
#include "ttprmldef.h"


#define round_up(v,s)   ((((v) + (s) - 1) / (s)) * (s))
extern struct   mchash_table *ttprm_table_g;
extern void    *ttprm_mempool_g;
extern void    *ttprm_domain_mempool_g;
extern void    *ttprm_table_mempool_g;
extern void    *ttprm_table_match_field_mempool_g;
extern uint32_t ttprm_buckets_g;


extern inline int32_t cntlr_send_msg_to_dp(struct of_msg     *msg,
                     struct dprm_datapath_entry* datapath,
                     cntlr_conn_table_t         *conn_table,
                     cntlr_conn_node_saferef_t  *conn_safe_ref,
                     void                       *callback_fn,
                     void                       *clbk_arg1,
                     void                       *clbk_arg2);

int32_t get_ttprm_node_byname(char *main_ttp_name_p, 
                              struct ttprm_entry **ttprm_entry_p_p);

/************************************************************************************************/
int32_t ttprm_register(char *main_ttp_name_p,  uint64_t *output_ttprm_handle_p)
{
  int32_t   ret_value=OF_SUCCESS;
  struct    ttprm_entry *ttprm_node_p=NULL,*ttprm_entry_scan_p=NULL;
  uint8_t   heap_b, status_b = FALSE;
  uchar8_t* hashobj_p = NULL;
  uint32_t  hashkey, offset, index, magic,tbl_id,msgno;


  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "main_ttp_name:%s",main_ttp_name_p);

  if(ttprm_table_g == NULL)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "No hash table\r\n");
    return CNTLR_TTP_FAILURE;
  }

  if((main_ttp_name_p == NULL))
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Invalid ttprm parameters");
    return CNTLR_TTP_FAILURE;
  }

  CNTLR_RCU_READ_LOCK_TAKE();

  /** calculate hash key for the Virtual network name **/
  hashkey = ttprm_get_hashval_byname(main_ttp_name_p, ttprm_buckets_g);
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "hashkey:%d",hashkey);

  offset = TTPRM_NODE_OFFSET;

  /** Scan ttprm hash table for the name **/
  MCHASH_BUCKET_SCAN(ttprm_table_g, 
                     hashkey, 
                     ttprm_entry_scan_p, 
                     struct ttprm_entry*, 
                     offset)
  {
    if(strcmp(ttprm_entry_scan_p->main_ttp_name, main_ttp_name_p) != 0)
      continue;
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
               "Duplicate main ttprm name %s", main_ttp_name_p);
    ret_value = CNTLR_TTP_FAILURE;
    goto TTPRM_EXIT;
  }

  /** Create memory for the entry */
  ret_value = mempool_get_mem_block(ttprm_mempool_g, 
                                    (uchar8_t **)&ttprm_node_p, &heap_b);
  if(ret_value != MEMPOOL_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "memory block allocation failed.");
    ret_value = CNTLR_TTP_FAILURE;
    goto TTPRM_EXIT;
  }

  memset(ttprm_node_p , 0 , sizeof(struct ttprm_entry));
  ttprm_node_p->heap_b = heap_b;
  strcpy(ttprm_node_p->main_ttp_name, main_ttp_name_p);


  /*  init for tables and domains */
  OF_LIST_INIT(ttprm_node_p->table_list,  ttprm_tbl_node_free_entry_rcu);
  OF_LIST_INIT(ttprm_node_p->domain_list, ttprm_domain_node_free_entry_rcu);

  for(tbl_id =0;tbl_id<CNTLR_TTPRM_MAX_TBLS;tbl_id++)
  {
    for(msgno = 1; msgno <= OF_MAX_NUMBER_ASYNC_MSGS; msgno++)
    { 
      memset(&((ttprm_node_p->async_msg_cbks[tbl_id]).event_info[msgno]),
             0,
             sizeof(struct async_event_info));

      (ttprm_node_p->async_msg_cbks[tbl_id]).event_info[msgno].type = msgno;
      OF_LIST_INIT((ttprm_node_p->async_msg_cbks[tbl_id]).event_info[msgno].app_list, 
                   NULL)
    }
  }


  hashobj_p = (uchar8_t *)ttprm_node_p + offset;

  MCHASH_APPEND_NODE(ttprm_table_g, 
                     hashkey, 
                     ttprm_node_p, 
                     index, 
                     magic, 
                     hashobj_p, 
                     status_b);
  if(FALSE == status_b)
  {
    OF_LOG_MSG(OF_LOG_TTP,OF_LOG_DEBUG, " failed to append ttprm node to hash table");
    ret_value = CNTLR_TTP_FAILURE;
    goto TTPRM_EXIT;
  }

  *output_ttprm_handle_p = magic;
  *output_ttprm_handle_p = ((*output_ttprm_handle_p <<32) | (index));

  ttprm_node_p->magic = magic;
  ttprm_node_p->index = index;
  (ttprm_node_p->main_ttp_handle) = *output_ttprm_handle_p;
  CNTLR_RCU_READ_LOCK_RELEASE();
  OF_LOG_MSG(OF_LOG_TTP,OF_LOG_NOTICE, 
             " registered successfully with ttprm", main_ttp_name_p);
  return CNTLR_TTP_SUCCESS;

TTPRM_EXIT:
  if(ttprm_node_p)
  {
    mempool_release_mem_block(ttprm_mempool_g, 
                              (uchar8_t *)ttprm_node_p, 
                              ttprm_node_p->heap_b);
  }
  CNTLR_RCU_READ_LOCK_RELEASE();
  OF_LOG_MSG(OF_LOG_TTP,OF_LOG_DEBUG, "ttprm registration failed.");
  return ret_value;
}

/*******************************************************************************************************/
int32_t get_ttprm_table_lst(char *main_ttp_name_p, 
                            of_list_t **ttprm_table_list_p_p, 
                            uint32_t *no_of_tables_p)
{
  int32_t ret_val;
  struct ttprm_entry *ttprm_entry_p = NULL;
  CNTLR_RCU_READ_LOCK_TAKE();
  ret_val = get_ttprm_node_byname(main_ttp_name_p, &ttprm_entry_p);
  if(ret_val!=CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "inavild main-ttp name:%s.",main_ttp_name_p);
   CNTLR_RCU_READ_LOCK_RELEASE();
   return CNTLR_TTP_FAILURE;
  } 
  *ttprm_table_list_p_p=&(ttprm_entry_p->table_list);
  *no_of_tables_p=ttprm_entry_p->number_of_tables;
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_SUCCESS;
}
       


/*******************************************************************************************************/
int32_t get_ttprm_node_byname(char *main_ttp_name_p, 
                              struct ttprm_entry **ttprm_entry_p_p)
{
  uint32_t hashkey;
  uint16_t offset;
  struct ttprm_entry *ttprm_entry_p = NULL;

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "main_ttp_name:%s",main_ttp_name_p);

  if(ttprm_table_g == NULL)
    return CNTLR_TTP_FAILURE;

  hashkey = ttprm_get_hashval_byname(main_ttp_name_p, ttprm_buckets_g);
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "hashkey:%d",hashkey);

  CNTLR_RCU_READ_LOCK_TAKE();
  offset = TTPRM_NODE_OFFSET;
  MCHASH_BUCKET_SCAN(ttprm_table_g, 
                     hashkey, 
                     ttprm_entry_p, 
                     struct ttprm_entry*,
                     offset)
  {
    if(strcmp(main_ttp_name_p, ttprm_entry_p->main_ttp_name))
      continue;  
    *ttprm_entry_p_p = ttprm_entry_p;
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_SUCCESS;
  }
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_FAILURE;
}
       

/******************************************************************************/
int32_t add_domain_to_ttprm(char *main_ttp_name_p, 
                            char *domain_name_p, 
                            uint64_t domain_handle)
{
  int32_t ret_val;
  struct  ttprm_entry *ttprm_entry_p=NULL;
  struct  ttprm_domain_entry *ttprm_domain_node_p=NULL;
  uint8_t heap_b;

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Entered");

  CNTLR_RCU_READ_LOCK_TAKE();
  ret_val = get_ttprm_node_byname(main_ttp_name_p, &ttprm_entry_p);
  if(ret_val!=CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, 
               "%s not registered with ttprm ",main_ttp_name_p);
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_SUCCESS;	
  }

  /** Create memory for the domain */
  ret_val = mempool_get_mem_block(ttprm_domain_mempool_g, 
                                  (uchar8_t **)&ttprm_domain_node_p, 
                                  &heap_b);
  if(ret_val != MEMPOOL_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "memory block allocation failed.");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }
  ttprm_domain_node_p->heap_b = heap_b;
  strcpy(ttprm_domain_node_p->domain_name, domain_name_p);
  ttprm_domain_node_p->domain_handle=domain_handle;
  OF_APPEND_NODE_TO_LIST(ttprm_entry_p->domain_list,
                         ttprm_domain_node_p,
                         TTPRM_DOMAIN_LST_NODE_OFFSET);
  ttprm_entry_p->number_of_domains++;
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "added domain successfully");
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_SUCCESS;
}

/******************************************************************************/
int32_t detach_domain_from_ttprm(char *main_ttp_name_p, char *domain_name_p)
{
  int32_t  ret_val;
  struct   ttprm_domain_entry *ttprm_domain_entry_p=NULL,
                              *ttprm_domain_prev_node_p=NULL;
  uint32_t ttprm_domain_offset=TTPRM_DOMAIN_LST_NODE_OFFSET;
  struct   ttprm_entry *ttprm_entry_p=NULL;

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
             "ttprm_name:%s, domain_name:%s",main_ttp_name_p, domain_name_p);
  CNTLR_RCU_READ_LOCK_TAKE();
  ret_val = get_ttprm_node_byname(main_ttp_name_p, &ttprm_entry_p);
  if(ret_val!=CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "Not registered with ttprm");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }

  OF_LIST_SCAN(ttprm_entry_p->domain_list, 
               ttprm_domain_entry_p, 
               struct  ttprm_domain_entry*,
               ttprm_domain_offset)
  {
    if(!strcmp(domain_name_p, ttprm_domain_entry_p->domain_name))
    {
      OF_REMOVE_NODE_FROM_LIST(ttprm_entry_p->domain_list, 
                               ttprm_domain_entry_p, 
                               ttprm_domain_prev_node_p,
                               ttprm_domain_offset);
      ttprm_entry_p->number_of_domains--;
      CNTLR_RCU_READ_LOCK_RELEASE();
      return CNTLR_TTP_SUCCESS;
    }
    ttprm_domain_prev_node_p=ttprm_domain_entry_p;
  }
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_FAILURE;
}

/*****************************************************************************/
int32_t alloc_ttp_tbl_mem_block(struct  ttprm_table_entry **tbl_entry_p_p)
{
  struct ttprm_table_entry *tbl_entry_p=NULL;
  int32_t ret_val;
  uint8_t heap_b = TRUE;

  CNTLR_RCU_READ_LOCK_TAKE();
  ret_val = mempool_get_mem_block(ttprm_table_mempool_g, 
                                  (uchar8_t **)&tbl_entry_p, 
                                  &heap_b);
  if(ret_val != MEMPOOL_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_ARP_SUB_TTP, OF_LOG_ERROR, "mempool get block failed.");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return  OF_FAILURE;
  }
  /** init match field list **/
  OF_LIST_INIT(tbl_entry_p->match_fields_list, 
               ttp_free_table_match_field_entry_rcu);
  tbl_entry_p->heap_b = heap_b;
  *tbl_entry_p_p = tbl_entry_p;
  CNTLR_RCU_READ_LOCK_RELEASE();
  return OF_SUCCESS;
}

void list_ttprm_tbls(char *main_ttp_name_p)
{
  int32_t ret_val;
  struct  ttprm_entry *ttprm_entry_p=NULL;
  struct  ttprm_table_entry *cur_tbl_entry_p=NULL;
  uint32_t offset;

  CNTLR_RCU_READ_LOCK_TAKE();
  ret_val = get_ttprm_node_byname(main_ttp_name_p, &ttprm_entry_p);
  if(ret_val!=CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, 
               "not registered with ttprm.",main_ttp_name_p);
    CNTLR_RCU_READ_LOCK_RELEASE();
    return ;
  }
  /** add tables to ttprm in ascending order **/
  offset = TTPRM_TBL_LST_NODE_OFFSET;
  OF_LIST_SCAN(ttprm_entry_p->table_list, 
               cur_tbl_entry_p, 
               struct ttprm_table_entry*, 
               offset)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
               "id:%d name:%s rows:%d clmns:%d",
                cur_tbl_entry_p->table_id,cur_tbl_entry_p->table_name,
                cur_tbl_entry_p->max_rows,cur_tbl_entry_p->max_columns);
  }
  CNTLR_RCU_READ_LOCK_RELEASE();
  return;
}

/**************************************************************************************/
int32_t ttprm_register_table(char *main_ttp_name_p, 
			    uint32_t match_fields_cnt,
			    struct ttprm_tbl_match_fields_cfg *match_fields_info_p,
			    struct ttprm_tbl_cfg *tbl_cfg_info_p)
{
  int32_t ret_val,status=CNTLR_TTP_SUCCESS,ii;
  struct  ttprm_table_entry *tbl_entry_p=NULL;

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "Entered");
  CNTLR_RCU_READ_LOCK_TAKE();
  do
  {
    ret_val = alloc_ttp_tbl_mem_block(&tbl_entry_p);
    if(CNTLR_UNLIKELY(ret_val != MEMPOOL_SUCCESS))
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "mempool get block failed");
      status=CNTLR_TTP_FAILURE;
      break;
    }
    memset(tbl_entry_p, 0 ,sizeof(struct  ttprm_table_entry));
    tbl_entry_p->table_id = tbl_cfg_info_p->tbl_id;
    strcpy(tbl_entry_p->table_name, tbl_cfg_info_p->tbl_name);
    tbl_entry_p->max_rows=tbl_cfg_info_p->max_rows;
    tbl_entry_p->max_columns = match_fields_cnt;

    for(ii=0;ii<match_fields_cnt;ii++)
    {
      if(match_fields_info_p[ii].is_wc == TRUE)
        tbl_entry_p->no_of_wcs++;
      if(match_fields_info_p[ii].is_lpm == TRUE)
        tbl_entry_p->no_of_lpms++;
    }
    if(tbl_entry_p->no_of_wcs)
      tbl_entry_p->is_wc_present=TRUE;
    if(tbl_entry_p->no_of_lpms)
      tbl_entry_p->is_lpm_present=TRUE;  

    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "tbl_entry_p->no_of_wcs:%d,\
	tbl_entry_p->no_of_lpms:%d",tbl_entry_p->no_of_wcs,tbl_entry_p->no_of_lpms);

    ret_val = ttprm_register_matchfields(&(tbl_entry_p->match_fields_list),
					  match_fields_info_p,
		  		          match_fields_cnt);
    if(CNTLR_UNLIKELY((ret_val != CNTLR_TTP_SUCCESS)))
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "add match-fields to table failed.");
      status = CNTLR_TTP_FAILURE;
      break;
    }

    ret_val = add_table_to_ttprm(main_ttp_name_p, &tbl_entry_p);
    if(CNTLR_UNLIKELY(ret_val != CNTLR_TTP_SUCCESS))
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "add self filter table to ttprm  failed.");
      status = CNTLR_TTP_FAILURE;
      break;
    }  
  }while(0);
  CNTLR_RCU_READ_LOCK_RELEASE();
  return status;
}

/**************************************************************************************/
int32_t add_table_to_ttprm(char *main_ttp_name_p, 
                           struct ttprm_table_entry **tbl_entry_p_p)
{
  int32_t ret_val;
  struct  ttprm_entry *ttprm_entry_p=NULL;
  struct  ttprm_table_entry *prev_tbl_entry_p=NULL,*cur_tbl_entry_p=NULL;
  uint32_t offset;

  CNTLR_RCU_READ_LOCK_TAKE();

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Entered");
  ret_val = get_ttprm_node_byname(main_ttp_name_p, &ttprm_entry_p);
  if(ret_val!=CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, 
               "Not registered with ttprm",main_ttp_name_p);
    CNTLR_RCU_READ_LOCK_RELEASE();
   return CNTLR_TTP_FAILURE;
  }
  /** add tables to ttprm in ascending order **/
  offset = TTPRM_TBL_LST_NODE_OFFSET;
  OF_LIST_SCAN(ttprm_entry_p->table_list, 
               cur_tbl_entry_p, 
               struct ttprm_table_entry*, 
               offset)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
               "table_id:%d",cur_tbl_entry_p->table_id);

    if(((*tbl_entry_p_p)->table_id) < (cur_tbl_entry_p->table_id))
    {
      OF_LIST_INSERT_NODE(ttprm_entry_p->table_list,
                          prev_tbl_entry_p,
                          *tbl_entry_p_p,
                          cur_tbl_entry_p,offset);

      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
                 "inserted table id:%d",((*tbl_entry_p_p)->table_id));

      ttprm_entry_p->async_msg_cbks[(*tbl_entry_p_p)->table_id].is_valid=TRUE;
      ttprm_entry_p->number_of_tables++;
      CNTLR_RCU_READ_LOCK_RELEASE();
      return CNTLR_TTP_SUCCESS;
    }
    prev_tbl_entry_p = cur_tbl_entry_p;
  }
  OF_APPEND_NODE_TO_LIST(ttprm_entry_p->table_list, *tbl_entry_p_p, offset);
  ttprm_entry_p->number_of_tables++;
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
             "number_of_tables:%d",ttprm_entry_p->number_of_tables);
  ttprm_entry_p->async_msg_cbks[(*tbl_entry_p_p)->table_id].is_valid=TRUE;
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_SUCCESS;
}

/******************************************************************************/
int32_t add_match_field_to_table(of_list_t *ttp_table_list_p,
                        uint32_t match_field,
                        uint8_t is_wild_card,
                        uint8_t is_lpm
                        )
{
  uint8_t heap_b;
  int32_t ret_value;
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Entered");
  struct  table_match_field_entry *table_match_field_node_p;

  CNTLR_RCU_READ_LOCK_TAKE();

  /** Create memory for the table matchfields */
  ret_value = mempool_get_mem_block(ttprm_table_match_field_mempool_g,
                                    (uchar8_t **)&table_match_field_node_p,
                                    &heap_b);
  if(ret_value != MEMPOOL_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "memory block allocation failed.");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }
   /* heap_b is not assigned to the node */
  table_match_field_node_p->heap_b= heap_b;
  table_match_field_node_p->match_field = match_field;
  table_match_field_node_p->is_wild_card = is_wild_card;
  table_match_field_node_p->is_lpm= is_lpm;
  OF_APPEND_NODE_TO_LIST((*ttp_table_list_p),
                         table_match_field_node_p,
                         TTPRM_TBL_MATCH_FIELD_LST_NODE_OFFSET);

  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_SUCCESS;
}
/******************************************************************************/
int32_t ttprm_register_matchfields(of_list_t *ttp_table_list_p,
  			  struct ttprm_tbl_match_fields_cfg *match_fields_info_p,
			  uint32_t match_fields_cnt)
{
  uint8_t heap_b,ii;
  int32_t ret_value,status=CNTLR_TTP_SUCCESS;
  struct  table_match_field_entry *table_match_field_node_p=NULL;

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Entered");
  CNTLR_RCU_READ_LOCK_TAKE();
  do
  {
    for(ii=0;ii<match_fields_cnt;ii++)
    {
      /** Create memory for the table matchfields */
      ret_value = mempool_get_mem_block(ttprm_table_match_field_mempool_g,
                                    (uchar8_t **)&table_match_field_node_p,
                                    &heap_b);
      if(ret_value != MEMPOOL_SUCCESS)
      {
        OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "mem block alloc failed for match field");
        status=CNTLR_TTP_FAILURE;
        break;
      }
      /* heap_b is not assigned to the node */
      table_match_field_node_p->heap_b= heap_b;
      table_match_field_node_p->match_field = match_fields_info_p[ii].match_field;
      table_match_field_node_p->is_wild_card = match_fields_info_p[ii].is_wc;
      table_match_field_node_p->is_lpm = match_fields_info_p[ii].is_lpm;
      OF_APPEND_NODE_TO_LIST((*ttp_table_list_p), 
			     table_match_field_node_p,
                             TTPRM_TBL_MATCH_FIELD_LST_NODE_OFFSET);
    }
  }while(0);
 CNTLR_RCU_READ_LOCK_RELEASE();
  return status;
}
/*******************************************************************************/
int32_t del_match_fields_from_table(struct ttprm_table_entry *ttprm_table_entry_p)
{
  int32_t ret_val;
  struct  table_match_field_entry *table_match_field_entry_p=NULL, 
                                  *prev_table_match_field_entry_p = NULL;

  uchar8_t tbl_match_field_offset;
  tbl_match_field_offset = TTPRM_TBL_MATCH_FIELD_LST_NODE_OFFSET;

  CNTLR_RCU_READ_LOCK_TAKE();

  OF_LIST_SCAN(ttprm_table_entry_p->match_fields_list, 
               table_match_field_entry_p, 
               struct  table_match_field_entry*,
               tbl_match_field_offset)
  {
    OF_REMOVE_NODE_FROM_LIST(ttprm_table_entry_p->match_fields_list, 
                             table_match_field_entry_p, 
                             prev_table_match_field_entry_p,
                             tbl_match_field_offset);
  }
  ret_val = mempool_release_mem_block(ttprm_mempool_g,
                                      (uchar8_t*)ttprm_table_entry_p,
                                     ttprm_table_entry_p->heap_b);
  if(ret_val != MEMPOOL_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, 
               "Failed to release memory block.");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;	
  }
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_SUCCESS;
}

/**********************************************************************************/
int32_t del_table_from_ttprm(char *main_ttp_name_p, uint8_t ttprm_tbl_id)
{
  int32_t ret_val;
  struct  ttprm_table_entry *ttprm_table_entry_p=NULL, 
                            *prev_ttprm_table_entry_p = NULL;
  struct  table_match_field_entry *table_match_field_entry_p=NULL, 
                            *prev_table_match_field_entry_p = NULL;
  struct ttprm_entry *ttprm_entry_p=NULL;

  uchar8_t ttp_tbl_offset;
  ttp_tbl_offset = TTPRM_TBL_LST_NODE_OFFSET;

  uchar8_t tbl_match_field_offset;
  tbl_match_field_offset = TTPRM_TBL_MATCH_FIELD_LST_NODE_OFFSET;

  CNTLR_RCU_READ_LOCK_TAKE();
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "mian_ttp_name:%s",main_ttp_name_p);
  ret_val = get_ttprm_node_byname(main_ttp_name_p, &ttprm_entry_p);
  if(ret_val!=CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "Not registered with ttprm");

    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }
  OF_LIST_SCAN(ttprm_entry_p->table_list, 
               ttprm_table_entry_p, 
               struct ttprm_table_entry *, 
               ttp_tbl_offset )
  {
    if(ttprm_table_entry_p->table_id != ttprm_tbl_id)continue;
    OF_LIST_SCAN(ttprm_table_entry_p->match_fields_list, 
                 table_match_field_entry_p, 
                 struct  table_match_field_entry*,
                 tbl_match_field_offset)
    {
      OF_REMOVE_NODE_FROM_LIST(ttprm_table_entry_p->match_fields_list, 
                               table_match_field_entry_p, 
                               prev_table_match_field_entry_p,
                               tbl_match_field_offset);
    }
    OF_REMOVE_NODE_FROM_LIST(ttprm_entry_p->table_list,
                             ttprm_table_entry_p,
                             prev_ttprm_table_entry_p,
                             ttp_tbl_offset );
    ttprm_entry_p->number_of_tables--;
    break;
  }
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "success");
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_SUCCESS;
}

#if 0
/***************************************************************************/
void cntlr_ttp_domain_event_cbk(uint32_t notification_type,
                             uint64_t domain_handle,
                             struct   dprm_domain_notification_data notification_data,
                             void     *callback_arg1,
                             void     *callback_arg2)
{
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "domain name = %s and Table Type Pattern=%s",notification_data.domain_name, notification_data.ttp_name);
 switch(notification_type)
  {
    case DPRM_DOMAIN_ADDED:
    {
        OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG,"DPRM_DOMAIN_ADDED event");
        if(add_domain_to_ttp(notification_data.ttp_name, notification_data.domain_name, domain_handle)!=CNTLR_TTP_SUCCESS)
        {
          OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR,"Add domain to ttp failed!.");
          return;
        }
        OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG,"Domain added to ttp.");
        if(cntlr_register_asynchronous_event_hook(domain_handle,
              DP_READY_EVENT,
              16,
              cntlr_ttp_dp_ready_event,
              NULL, NULL) == OF_FAILURE)
        {
          OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR,"Register DP Ready event cbk failed");
        }
    }
    break;
   case DPRM_DOMAIN_DELETE:
      //need to unreg handler for DP_READY_EVENT
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG,"DPRM_DOMAIN_DELETE event");
      {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG,"DPRM_DOMAIN_DELETED");
        break;
      }
  }
}
#endif

/*********************************************************************/
int32_t get_number_of_ttp_tables(char *main_ttp_name_p, uint32_t *no_of_tbls_p)
{
  int32_t ret_val;
  struct ttprm_entry *ttprm_entry_p=NULL;
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "main_ttp_name:%s",main_ttp_name_p);
  CNTLR_RCU_READ_LOCK_TAKE();
  ret_val = get_ttprm_node_byname(main_ttp_name_p, &ttprm_entry_p);
  if(ret_val != CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "get_ttprm_node_byname failed.");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }  
  *no_of_tbls_p = ttprm_entry_p->number_of_tables;  
    CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_SUCCESS;
}

int32_t get_ttprm_async_event_info(char *main_ttp_name_p, 
                                   uint8_t tbl_id, 
                                   struct async_event_info **event_info_p_p)
{
  int32_t ret_val;
  struct ttprm_entry *ttprm_entry_p=NULL;
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "main_ttp_name:%s",main_ttp_name_p);

  CNTLR_RCU_READ_LOCK_TAKE();
  ret_val = get_ttprm_node_byname(main_ttp_name_p, &ttprm_entry_p);
  if(ret_val != CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "ttp not registered.");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }  
  if((ttprm_entry_p->async_msg_cbks[tbl_id]).is_valid)
  {
    *event_info_p_p = ((ttprm_entry_p->async_msg_cbks[tbl_id]).event_info);
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_SUCCESS;
  }  
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Invalid table id:%d",tbl_id);
  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_FAILURE;
}

int32_t cntlr_ttprm_tbl_feature_len(char *main_ttp_name_p, uint16_t *msg_len_p)
{
  int32_t ret_val;
  uint16_t msg_len=0, 
           tbl_ftr_req_len =0,
           match_prop_len=0, 
           wc_prop_len=0,
           lpm_prop_len=0;
  struct   ttprm_table_entry *ttprm_table_entry_p  = NULL;
  uchar8_t ttprm_tbl_offset = TTPRM_TBL_LST_NODE_OFFSET;
  struct ttprm_entry *ttprm_entry_p=NULL;

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Entered");
  ret_val = get_ttprm_node_byname(main_ttp_name_p, &ttprm_entry_p);
  if(ret_val != CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "ttp not registered");
    return CNTLR_TTP_FAILURE;
  }
  msg_len = OFU_TABLE_FEATURES_REQ_MESSAGE_LEN;//multipart req hdr.
  OF_LIST_SCAN(ttprm_entry_p->table_list, 
               ttprm_table_entry_p, 
               struct ttprm_table_entry *, 
               ttprm_tbl_offset)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Table = %s ",ttprm_table_entry_p->table_name);
    if(!(ttprm_table_entry_p->max_columns))
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
                 "Table (%s) has no match fields.",ttprm_table_entry_p->table_name);
      continue;
    }

    /* Add Table Feature Header Len */
    tbl_ftr_req_len = sizeof(struct ofp_table_features);
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
           "tbl_ftr_hdr_len:%d",tbl_ftr_req_len);

    /* Propery len  . match fields have property header and no of match fields.*/
    match_prop_len = \
       ((sizeof(struct ofp_table_feature_prop_oxm)) + 
       ((ttprm_table_entry_p->max_columns)*sizeof(uint32_t)));

    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
               "match_prop_len:%d,round_up(match_prop_len):%d",
                match_prop_len,round_up(match_prop_len,8));

    tbl_ftr_req_len += round_up(match_prop_len, 8);

    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
               "match fields:%d, match_hdr_len:%d,tbl_ftr_req_len:%d",
                ttprm_table_entry_p->max_columns,
                sizeof(struct ofp_table_feature_prop_oxm),tbl_ftr_req_len);

        /* if wc entries presents, add to len */
    if(ttprm_table_entry_p->is_wc_present == TRUE)
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
                 "wild card entry presents in the table");
      wc_prop_len =\
      ((sizeof(struct ofp_table_feature_prop_oxm)) +
      ((ttprm_table_entry_p->no_of_wcs)*sizeof(uint32_t)));

      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
                 "wc_prop_len:%d,round_up(wc_prop_len):%d",
                  wc_prop_len,round_up(wc_prop_len,8));

      tbl_ftr_req_len += round_up(wc_prop_len, 8);
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
                 "no of wcs:%d,wc_hdr_len:%d,tbl_ftr_req_len:%d",
                  ttprm_table_entry_p->no_of_wcs,
                  sizeof(struct ofp_table_feature_prop_oxm),
                  tbl_ftr_req_len);
    }
     /* lpm entries presents add to len */
    if(ttprm_table_entry_p->is_lpm_present == TRUE)
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "lpm entry presents in the table");
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
                 "no of lpms:%d",ttprm_table_entry_p->no_of_lpms);
      lpm_prop_len = \
      ((sizeof(struct ofp_table_feature_prop_experimenter)) +
       (ttprm_table_entry_p->no_of_lpms*sizeof(uint32_t)));

     tbl_ftr_req_len += round_up(lpm_prop_len, 8);

      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
                "lpm_prop_len:%d,round_up(lpm_prop_len):%d",
                 lpm_prop_len,round_up(lpm_prop_len,8));

      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
                 "no of lpms:%d,lpm_hdr_len:%d,tbl_ftr_req_len:%d",
                 ttprm_table_entry_p->no_of_lpms,
                 (sizeof(struct ofp_table_feature_prop_experimenter)),
                 tbl_ftr_req_len);
    }
    msg_len += tbl_ftr_req_len ;
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "tbl_ftr_req_len :%d",tbl_ftr_req_len);
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "msg_len :%d",msg_len);
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "########################");
  }
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "total msg_len:%d",msg_len);
  printf( "total msg_len:%d",msg_len);
  *msg_len_p = msg_len;
 return CNTLR_TTP_SUCCESS;
}

int32_t cntlr_ttprm_build_tbl_ftr_msg(char *main_ttp_name_p, struct  of_msg  *msg)
{
  uint32_t ii;
  int32_t ret_val=CNTLR_TTP_SUCCESS;
  struct  ofp_multipart_request    *multipart_tbl_feature_req_msg_p=NULL;
  struct  ofp_table_features       *tbl_ftr_req_p=NULL;
  struct  ttprm_table_entry        *ttprm_table_entry_p = NULL;
  struct  table_match_field_entry  *table_match_field_entry_p = NULL;
  struct ttprm_entry *ttprm_entry_p=NULL;
  uchar8_t ttp_tbl_offset;
  ttp_tbl_offset = TTPRM_TBL_LST_NODE_OFFSET;
  uchar8_t tbl_match_field_offset;
  tbl_match_field_offset = TTPRM_TBL_MATCH_FIELD_LST_NODE_OFFSET;

  CNTLR_RCU_READ_LOCK_TAKE();
  ret_val = get_ttprm_node_byname(main_ttp_name_p, &ttprm_entry_p);
  if(ret_val != CNTLR_TTP_SUCCESS)
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "get_ttprm_node_byname failed.");
    CNTLR_RCU_READ_LOCK_RELEASE();
    return CNTLR_TTP_FAILURE;
  }
 
  /*Build Message Content */
  /* Multipart request header*/
  multipart_tbl_feature_req_msg_p =\
       (struct ofp_multipart_request*)(msg->desc.start_of_data);
  multipart_tbl_feature_req_msg_p->header.version  = OFP_VERSION;
  multipart_tbl_feature_req_msg_p->header.type     = OFPT_MULTIPART_REQUEST;
  multipart_tbl_feature_req_msg_p->header.xid      = 0;
  multipart_tbl_feature_req_msg_p->type     = htons(OFPMP_TABLE_FEATURES);
  msg->desc.data_len = sizeof(struct ofp_multipart_request);

  tbl_ftr_req_p =\
   (struct ofp_table_features*)(multipart_tbl_feature_req_msg_p->body);

  OF_LIST_SCAN(ttprm_entry_p->table_list, 
               ttprm_table_entry_p, 
               struct ttprm_table_entry *, 
               ttp_tbl_offset)
  {
    if(!(ttprm_table_entry_p->max_columns))
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "Table(id:%d,name:%s) null",
                 ttprm_table_entry_p->table_id,ttprm_table_entry_p->table_name);
        continue;
    }

    /* Table feature msg hdr */

    tbl_ftr_req_p->table_id = ttprm_table_entry_p->table_id;
    /* table name */
    strncpy(tbl_ftr_req_p->name, 
            ttprm_table_entry_p->table_name, OFP_MAX_TABLE_NAME_LEN);     
    tbl_ftr_req_p->metadata_match=0xffffffffffffffff;
    tbl_ftr_req_p->metadata_write=0xffffffffffffffff;
    /* max rows presents in the table */
    tbl_ftr_req_p->max_entries = htonl(ttprm_table_entry_p->max_rows);
    //tbl_ftr_req_p->config = OFPTC_TABLE_MISS_CONTROLLER;
    tbl_ftr_req_p->length =  sizeof(struct ofp_table_features);
   
     OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
                "Table name = %s ",ttprm_table_entry_p->table_name);
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
                "tbl_ftr_hdr_len:%d",tbl_ftr_req_p->length);

    /*Add  match fields property to the table feature message*/
    struct ofp_table_feature_prop_oxm *oxm_capabilities = \
            (struct ofp_table_feature_prop_oxm *)(tbl_ftr_req_p->properties);

    oxm_capabilities->type = htons(OFPTFPT_MATCH);
    oxm_capabilities->length = sizeof(struct ofp_table_feature_prop_oxm);
    ii=0;
    OF_LIST_SCAN(ttprm_table_entry_p->match_fields_list, 
                 table_match_field_entry_p, 
                 struct  table_match_field_entry*,
                 tbl_match_field_offset)
    {
      oxm_capabilities->oxm_ids[ii] = \
        htonl(table_match_field_entry_p->match_field);    
      oxm_capabilities->length += sizeof(uint32_t);
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
                 "oxm_capabilities->oxm_ids[ii]:%x\n",oxm_capabilities->oxm_ids[ii]);
      ii++;
    }
    tbl_ftr_req_p->length +=  round_up(oxm_capabilities->length,8);
    oxm_capabilities->length = htons(oxm_capabilities->length);
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
               "oxm_capabilities->length:%d",oxm_capabilities->length);

    if(ttprm_table_entry_p->is_wc_present == TRUE)
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "wild card present in the table");
      /* wild card field property */  
      oxm_capabilities =\
 (struct ofp_table_feature_prop_oxm *)((char*)tbl_ftr_req_p + tbl_ftr_req_p->length);     

      oxm_capabilities->type = OFPTFPT_WILDCARDS;
      oxm_capabilities->length = sizeof(struct ofp_table_feature_prop_oxm);      
      ii=0;
      OF_LIST_SCAN(ttprm_table_entry_p->match_fields_list, 
                   table_match_field_entry_p, 
                   struct  table_match_field_entry*,
                   tbl_match_field_offset)
      {
         if(table_match_field_entry_p->is_wild_card == TRUE)
        {
          oxm_capabilities->oxm_ids[ii] = htonl(table_match_field_entry_p->match_field);        
          oxm_capabilities->length += sizeof(uint32_t);
          OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
                     "oxm_capabilities->oxm_ids:%x",oxm_capabilities->oxm_ids[ii]);
          ii++;
        }
      } 
      tbl_ftr_req_p->length += round_up(oxm_capabilities->length,8);
      oxm_capabilities->length = htons(oxm_capabilities->length);
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
                 "oxm_capabilities->length:%d",oxm_capabilities->length);
    }


    if(ttprm_table_entry_p->is_lpm_present == TRUE)
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "lpm  present in the table");
      struct ofp_table_feature_prop_experimenter *oxm_lpm_capabilities=NULL;

      oxm_lpm_capabilities =\
 (struct ofp_table_feature_prop_experimenter *)((char*)tbl_ftr_req_p + tbl_ftr_req_p->length);     

      oxm_lpm_capabilities->type = htons(OFPTFPT_EXPERIMENTER);
      oxm_lpm_capabilities->length =\
      sizeof(struct ofp_table_feature_prop_experimenter);
      ii=0;
      oxm_lpm_capabilities->experimenter = htonl(FSLX_VENDOR_ID);
      oxm_lpm_capabilities->exp_type = htonl(FSLXTFPT_LPM);     
      OF_LIST_SCAN(ttprm_table_entry_p->match_fields_list, 
                   table_match_field_entry_p, 
                   struct  table_match_field_entry*,
                   tbl_match_field_offset)
      {
        if(table_match_field_entry_p->is_lpm == TRUE)
        {
          oxm_lpm_capabilities->experimenter_data[ii] =\
          htonl(table_match_field_entry_p->match_field);
  
          oxm_lpm_capabilities->length += sizeof(uint32_t);
          OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
           "oxm_lpm_capabilities->experimenter_data[ii]:%x",
            oxm_lpm_capabilities->experimenter_data[ii]);
          ii++;
        }
      } 
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
      "oxm_lpm_capabilities->length:%d",oxm_lpm_capabilities->length);

      tbl_ftr_req_p->length += round_up(oxm_lpm_capabilities->length,8);
      oxm_lpm_capabilities->length = htons(oxm_lpm_capabilities->length);
    }
    msg->desc.data_len += tbl_ftr_req_p->length;

    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
              "tbl_ftr_req_len:%d,msg_desc_len:%d",
               tbl_ftr_req_p->length,msg->desc.data_len);

    tbl_ftr_req_p->length= htons(tbl_ftr_req_p->length);
    tbl_ftr_req_p = (struct ofp_table_features*)((msg->desc.start_of_data) + (msg->desc.data_len));    
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "*************************");
  }

  multipart_tbl_feature_req_msg_p->header.length = htons(msg->desc.data_len);
 OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, 
             "table_feature_msg_len:%d",
            multipart_tbl_feature_req_msg_p->header.length);
// printf("table_feature_msg_len:%d",multipart_tbl_feature_req_msg_p->header.length);

  CNTLR_RCU_READ_LOCK_RELEASE();
  return CNTLR_TTP_SUCCESS;

}


