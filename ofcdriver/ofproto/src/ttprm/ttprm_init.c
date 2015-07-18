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
* File:ttprm_init.c
* Description: This file implements the ttprm module init routines.
* Authors:      Varun Kumar<B36461@freescale.com>
* History:
* 29 Jan 2014 - Varun Kumar - Initial Implementation.
*******************************************************************************/
#include "controller.h"
#include "cntlr_event.h"
#include "cntlr_transprt.h"
#include "dprmldef.h"
#include "ttprmgdef.h"
#include "ttprmldef.h"



uint32_t ttprm_buckets_g;
struct   mchash_table *ttprm_table_g = NULL;
void    *ttprm_mempool_g = NULL;
void    *ttprm_domain_mempool_g = NULL;
void    *ttprm_table_mempool_g = NULL;
void    *ttprm_table_match_field_mempool_g = NULL;


/*********************************************************************************/
uint32_t ttprm_get_hashval_byname(char* name_p,uint32_t no_of_buckets)
{
  uint32_t hashkey,param1,param2,param3,param4;
  char name_hash[16];
  memset(name_hash,0,16);
  strncpy(name_hash,name_p,16);
  param1 = *(uint32_t *)(name_hash +0);
  param2 = *(uint32_t *)(name_hash +4);
  param3 = *(uint32_t *)(name_hash +8);
  param4 = *(uint32_t *)(name_hash +12);
  TTPRM_COMPUTE_HASH(param1,param2,param3,param4,no_of_buckets,hashkey);
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "name %s bucket %d hashkey %d",name_p, no_of_buckets, hashkey);
  return hashkey;
}

/******************************************************************************
Function:of_ctrl_arp_inbound_mempoolentries
*******************************************************************************/
void ttprm_get_mempoolentries(uint32_t *max_entries, uint32_t *static_entries)
{
  *max_entries    = TTPRM_MAX_ENTRIES;
  *static_entries = TTPRM_STATIC_ENTRIES;
}


/******************************************************************************
* Function : ttprm_mempool_init.
* Description:
*       ttprm  memory pool init.
******************************************************************************/
int32_t ttprm_mempool_init()
{
  int32_t  ret_val ;
  int32_t  status=CNTLR_TTP_SUCCESS;
  uint32_t db_entries_max, db_static_entries;
  struct mempool_params mempool_info={};

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Entered");
  do
  {
    ttprm_get_mempoolentries(&db_entries_max, &db_static_entries);
    mempool_info.mempool_type = MEMPOOL_TYPE_HEAP;
    mempool_info.block_size = sizeof(struct ttprm_entry);
    mempool_info.max_blocks = db_entries_max;
    mempool_info.static_blocks = db_static_entries;
    mempool_info.threshold_min = db_static_entries/10;
    mempool_info.threshold_max = db_static_entries/3;
    mempool_info.replenish_count = db_static_entries/10;
    mempool_info.b_memset_not_required = FALSE;
    mempool_info.b_list_per_thread_required = FALSE;
    mempool_info.noof_blocks_to_move_to_thread_list = 0;

    ret_val = mempool_create_pool("ttprm_pool", &mempool_info,
		    &ttprm_mempool_g
		    );
    if(ret_val != MEMPOOL_SUCCESS)
    {
	    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "mempool creation failed");
	    status=CNTLR_TTP_FAILURE;
    }
  }while(0);
  return status;
}

/******************************************************************************
* Function : ttprm_domain_mempool_init.
* Description:
*       ttprm domain memory pool init.
******************************************************************************/
int32_t ttprm_domain_mempool_init()
{
  int32_t  ret_val ;
  int32_t  status=CNTLR_TTP_SUCCESS;
  uint32_t db_entries_max, db_static_entries;
  struct mempool_params mempool_info={};

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Entered");
  do
  {
    ttprm_get_mempoolentries(&db_entries_max, &db_static_entries);
    mempool_info.mempool_type = MEMPOOL_TYPE_HEAP;
    mempool_info.block_size = sizeof(struct ttprm_domain_entry);
    mempool_info.max_blocks = db_entries_max;
    mempool_info.static_blocks = db_static_entries;
    mempool_info.threshold_min = db_static_entries/10;
    mempool_info.threshold_max = db_static_entries/3;
    mempool_info.replenish_count = db_static_entries/10;
    mempool_info.b_memset_not_required = FALSE;
    mempool_info.b_list_per_thread_required = FALSE;
    mempool_info.noof_blocks_to_move_to_thread_list = 0;

    ret_val = mempool_create_pool("ttprm_domain_pool", &mempool_info,
		    &ttprm_domain_mempool_g
		    );
    if(ret_val != MEMPOOL_SUCCESS)
    {
	    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "mempool creation failed");
	    status=CNTLR_TTP_FAILURE;
    }
  }while(0);
  return status;
}

/******************************************************************************
* Function : ttprm_table_mempool_init.
* Description:
*       ttprm table memory pool init.
******************************************************************************/
int32_t ttprm_table_mempool_init()
{
  int32_t  ret_val ;
  int32_t  status=CNTLR_TTP_SUCCESS;
  uint32_t db_entries_max, db_static_entries;
  struct mempool_params mempool_info={};

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Entered");
  do
  {
    
    db_entries_max = (CNTLR_MAX_NUMBER_OF_TTPS*CNTLR_TTPRM_MAX_TBLS);
    db_static_entries = db_entries_max/4;
    mempool_info.mempool_type = MEMPOOL_TYPE_HEAP;
    mempool_info.block_size = sizeof(struct ttprm_table_entry);
    mempool_info.max_blocks = db_entries_max;
    mempool_info.static_blocks = db_static_entries;
    mempool_info.threshold_min = db_static_entries/10;
    mempool_info.threshold_max = db_static_entries/3;
    mempool_info.replenish_count = db_static_entries/10;
    mempool_info.b_memset_not_required = FALSE;
    mempool_info.b_list_per_thread_required = FALSE;
    mempool_info.noof_blocks_to_move_to_thread_list = 0;

    ret_val = mempool_create_pool("ttprm_table_pool", &mempool_info,
		    &ttprm_table_mempool_g
		    );
    if(ret_val != MEMPOOL_SUCCESS)
    {
	    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "mempool creation failed");
	    status=CNTLR_TTP_FAILURE;
    }
  }while(0);
  return status;
}

/******************************************************************************
* Function : ttprm_match_field_mempool_init.
* Description:
*       ttprm match field memory pool init.
******************************************************************************/
int32_t ttprm_match_field_mempool_init()
{
  int32_t  ret_val ;
  int32_t  status=CNTLR_TTP_SUCCESS;
  uint32_t db_entries_max, db_static_entries;
  struct mempool_params mempool_info={};

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Entered");
  do
  {
    db_entries_max = (CNTLR_MAX_NUMBER_OF_TTPS*CNTLR_TTPRM_MAX_TBLS *CNTLR_TTPRM_MAX_MATCH_FIELDS);
    db_static_entries=db_entries_max/4;
    mempool_info.mempool_type = MEMPOOL_TYPE_HEAP;
    mempool_info.block_size = sizeof(struct table_match_field_entry);
    mempool_info.max_blocks = db_entries_max;
    mempool_info.static_blocks = db_static_entries;
    mempool_info.threshold_min = db_static_entries/10;
    mempool_info.threshold_max = db_static_entries/3;
    mempool_info.replenish_count = db_static_entries/10;
    mempool_info.b_memset_not_required = FALSE;
    mempool_info.b_list_per_thread_required = FALSE;
    mempool_info.noof_blocks_to_move_to_thread_list = 0;

    ret_val = mempool_create_pool("ttprm_match_field_pool", &mempool_info,
		    &ttprm_table_match_field_mempool_g
		    );
    if(ret_val != MEMPOOL_SUCCESS)
    {
	    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "mempool creation failed");
	    status=CNTLR_TTP_FAILURE;
    }
  }while(0);
  return status;
}
/*****************************************************************************
* Function:ttprm_free_node_entry_rcu
*       RCU to free subttp node entry.
*****************************************************************************/
void ttprm_free_node_entry_rcu(struct rcu_head *ttprm_entry_p)
{
  struct ttprm_entry *ttprm_entry_info_p=NULL;
  uint32_t offset=TTPRM_NODE_OFFSET;
  int32_t ret_val;

  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Entered");
  if(ttprm_entry_p)
  {
    ttprm_entry_info_p = (struct ttprm_entry *)(((char *)ttprm_entry_p - RCUNODE_OFFSET_IN_MCHASH_TBLLNK) - offset);
    ret_val = mempool_release_mem_block(ttprm_mempool_g, (uchar8_t*)ttprm_entry_info_p, ttprm_entry_info_p->heap_b);
    if(ret_val != MEMPOOL_SUCCESS)
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "Failed to release arp inbound subttp entry.");
    }
  }
  else
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Null Entry To Release.");
  }
}

/***************************************************************
 RCU to release domain memory block from the domain mempool 
****************************************************************/
void ttprm_domain_node_free_entry_rcu(struct rcu_head *domain_entry_p)
{
  struct ttprm_domain_entry  *domain_info_p;
  int32_t ret_value;
  if(domain_entry_p)
  {
    domain_info_p = (struct ttprm_domain_entry*)((char *)domain_entry_p - TTPRM_DOMAIN_LST_RCU_HEAD_OFFSET);
    ret_value = mempool_release_mem_block(ttprm_domain_mempool_g, (uchar8_t*)domain_info_p, domain_info_p->heap_b);
    if(ret_value != MEMPOOL_SUCCESS)
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "Failed to release ttp domain memory block.");
    }
    else
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "Released ttp domain memory block from the memory pool.");
    }

  }
  else
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "NULL passed for deletion.");
  }
}

/******************************************************************************/
/* RCU to release table memory block from the ttp table mempool */
void ttprm_tbl_node_free_entry_rcu(struct rcu_head *table_entry_p)
{
  struct ttprm_table_entry  *table_info_p=NULL;
  int32_t ret_value;
  if(table_entry_p)
  {
    table_info_p = (struct ttprm_table_entry*)((char *)table_entry_p - TTPRM_TBL_LST_RCU_OFFSET);
    ret_value = mempool_release_mem_block(ttprm_table_mempool_g, (uchar8_t*)table_info_p, table_info_p->heap_b);
    if(ret_value != MEMPOOL_SUCCESS)
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "Failed to release ttp table memory block.");
    }
    else
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "Released ttp table memory block from the memory pool.");
    }

  }
  else
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "NULL passed for deletion.");
  }
}

/******************************************************************************/
/* RCU to release table memory block from the ttp table mempool */
void ttp_free_table_match_field_entry_rcu(struct rcu_head *table_match_filed_entry_p)
{
  struct table_match_field_entry  *table_match_field_info_p=NULL;
  int32_t ret_value;
  if(table_match_filed_entry_p)
  {
    table_match_field_info_p = (struct table_match_field_entry*)((char *)table_match_filed_entry_p - TTPRM_TBL_MATCH_FIELD_LST_RCU_HEAD_OFFSET);
    ret_value = mempool_release_mem_block(ttprm_table_match_field_mempool_g, (uchar8_t*)table_match_filed_entry_p, table_match_field_info_p->heap_b);
    if(ret_value != MEMPOOL_SUCCESS)
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "Failed to release ttp table match field_memory block.");
    }
    else
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "Released ttp table match field memory block from the memory pool.");
    }

  }
  else
  {
    OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "NULL passed for deletion.");
  }
}
/******************************************************************************
* Function:ttprm_hash_table_init
* Description:
*       ttprm inbound hash table init
******************************************************************************/
int32_t ttprm_hash_table_init()
{
  int32_t ret_val, status=CNTLR_TTP_SUCCESS;
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Entered");
  do
  {
    ttprm_buckets_g =  1;
    ret_val = mchash_table_create(ttprm_buckets_g,
                                    TTPRM_TABLE_SIZE,
                                    ttprm_free_node_entry_rcu,
                                    &ttprm_table_g);
    if(ret_val != MCHASHTBL_SUCCESS)
    {
      OF_LOG_MSG(OF_LOG_TTP, OF_LOG_ERROR, "ttprm hash table creation failed");
      status = CNTLR_TTP_FAILURE;
      break;
    }
  }while(0);
  return status;
}


/***********************************************************
* Function: ttprm_init.
* Description:
*       ttprm initi routine.
*************************************************************/
int32_t ttprm_init()
{
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "Entered");

  
  /** Create mempool for ttp rm **/
  if(ttprm_mempool_init()!=CNTLR_TTP_SUCCESS)
  {
    return CNTLR_TTP_FAILURE;
  }

  /** Create hash table for arp inbound subttp **/
  if(ttprm_hash_table_init()!=CNTLR_TTP_SUCCESS)
  {
    return CNTLR_TTP_FAILURE;
  }

  /** Create mempool for ttprm domains **/
  if(ttprm_domain_mempool_init()!=CNTLR_TTP_SUCCESS)
  {
    return CNTLR_TTP_FAILURE;
  }

  /** Create mempool for ttprm tables **/
  if(ttprm_table_mempool_init()!=CNTLR_TTP_SUCCESS)
  {
    return CNTLR_TTP_FAILURE;
  }

  /** Create mempool for ttprm matchfields **/
  if(ttprm_match_field_mempool_init()!=CNTLR_TTP_SUCCESS)
  {
    return CNTLR_TTP_FAILURE;
  }
 
  OF_LOG_MSG(OF_LOG_TTP, OF_LOG_DEBUG, "ttprm init success");
  return CNTLR_TTP_SUCCESS;
}

int32_t ttprm_deinit()
{
  return CNTLR_TTP_SUCCESS; 
}



