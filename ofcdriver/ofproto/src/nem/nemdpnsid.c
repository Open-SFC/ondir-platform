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

/* File  :      nemdp2ns.c
 * Author:      Atmaram G <atmaram.g@freescale.com>
 * Date:   08/23/2013
 * Description: A network map_entry is logical mapping between network elements.This file provides datapath ID to NamespaceID  mapping APIs. It also provides APIs to getfirst/getnext/getexact/delete map_entrys. NEM maintains  separate hash table and mempool list for map_entry maintanance for this database.
 */

#ifdef CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT

#include "controller.h"
#include "dprm.h"
#include "cntlr_epoll.h"
#include "cntrl_queue.h"
#include "of_utilities.h"
#include "of_msgapi.h"
#include "of_multipart.h"
#include "cntlr_tls.h"
#include "cntlr_transprt.h"
#include "cntlr_event.h"
#include "dprmldef.h"
#include "of_nem.h"
#include "nemldef.h"


void     *nem_dpid_and_dpnsid_db_mempool_g = NULL;
uint32_t nem_dpid_and_dpnsid_buckets_g;
struct   mchash_table* nem_dpid_and_dpnsid_db_table_g = NULL;
struct nem_dpid_ns_info of_nem_dpid_ns_info_g[DPRM_MAX_DATAPATH_ENTRIES];



void nem_free_dpid_and_dpnsid_dbentry_rcu(struct rcu_head *map_entry_rcu_p);

/******************************************************************************
 * * * * * * * * * * * * Namespace to NSID * * * * * * * * * * * * * * * * * * *
 *******************************************************************************/

int32_t nem_dpid_and_dpnsid_db_init()
{
   int32_t status=NEM_SUCCESS;
   int32_t ret_value;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   do
   {
      ret_value=nem_dpid_and_dpnsid_db_mempool_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Mempool initialization failed"); 
         status=NEM_FAILURE;
         break;
      }

      ret_value=nem_dpid_and_dpnsid_db_mchash_table_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Hash table initialization failed"); 
         status=NEM_FAILURE;
         break;
      }

   }while(0);

   return status;
}

int32_t nem_dpid_and_dpnsid_db_deinit()
{
   int32_t status=NEM_SUCCESS;
   int32_t ret_value;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   do
   {

      ret_value=nem_dpid_and_dpnsid_unmap_all_entries();
      if( CNTLR_UNLIKELY(ret_value != OF_SUCCESS) )
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Cleanup all entries failed");
         status=NEM_FAILURE;
       //  break;
      }

      ret_value=  mchash_table_delete(nem_dpid_and_dpnsid_db_table_g);
      if( CNTLR_UNLIKELY(ret_value != OF_SUCCESS) )
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Mempool De-initialization failed"); 
         status=NEM_FAILURE;
         break;
      }

      /** Delete mempools for MFC database */
      ret_value=mempool_delete_pool(nem_dpid_and_dpnsid_db_mempool_g);
      if( CNTLR_UNLIKELY(ret_value != OF_SUCCESS) )
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Hash table Deinitialization failed"); 
         status=NEM_FAILURE;
         break;
      }

   }while(0);

   return status;
}

int32_t nem_dpid_and_dpnsid_db_mempool_init()
{
   int32_t status=NEM_SUCCESS;
   uint32_t db_entries_max,db_static_entries;
   int32_t ret_value;
   struct mempool_params mempool_info={};

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   nem_dpid_and_nsid_db_get_map_entry_mempoolentries(&db_entries_max,&db_static_entries);

   mempool_info.mempool_type = MEMPOOL_TYPE_HEAP;
   mempool_info.block_size = sizeof(struct nem_dpid_and_dpnsid_map_entry);
   mempool_info.max_blocks = db_entries_max;
   mempool_info.static_blocks = db_static_entries;
   mempool_info.threshold_min = db_static_entries/10;
   mempool_info.threshold_max = db_static_entries/3;
   mempool_info.replenish_count = db_static_entries/10;
   mempool_info.b_memset_not_required =  FALSE;
   mempool_info.b_list_per_thread_required =  FALSE;
   mempool_info.noof_blocks_to_move_to_thread_list = 0;

   ret_value = mempool_create_pool("nem_dpid_and_dpnsid_db_pool", &mempool_info,
         &nem_dpid_and_dpnsid_db_mempool_g
         );

   if(ret_value != MEMPOOL_SUCCESS)
   {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "mempool creation failed"); 
      status=NEM_FAILURE;
   }

   return status;
}

int32_t nem_dpid_and_dpnsid_db_mchash_table_init()
{
   uint32_t db_entries_max,db_static_entries;
   int32_t ret_value;
   int32_t status=NEM_SUCCESS;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   nem_dpid_and_nsid_db_get_map_entry_mempoolentries(&db_entries_max,&db_static_entries);

   nem_dpid_and_dpnsid_buckets_g = ((db_entries_max / 5)+1);
   ret_value = mchash_table_create( nem_dpid_and_dpnsid_buckets_g,
         db_entries_max,
         nem_free_dpid_and_dpnsid_dbentry_rcu,
         &nem_dpid_and_dpnsid_db_table_g);
   if(ret_value != MCHASHTBL_SUCCESS)
   {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "hash table creation failed"); 
      status=NEM_FAILURE;
   }

   return status;

}


void nem_free_dpid_and_dpnsid_dbentry_rcu(struct rcu_head *map_entry_rcu_p)
{
   struct nem_dpid_and_dpnsid_map_entry *map_entry_p;
   uint16_t offset;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entry");

   if(map_entry_rcu_p)
   {
      offset = DPID_DPNSID_MAPNODE_HASH_TBLLNK_OFFSET;
      map_entry_p = (struct nem_dpid_and_dpnsid_map_entry *) ( (char *)map_entry_rcu_p -(RCUNODE_OFFSET_IN_MCHASH_TBLLNK)-offset);
      mempool_release_mem_block(nem_dpid_and_dpnsid_db_mempool_g,(uchar8_t*)map_entry_p,map_entry_p->heap_b);
      return;
   }

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "NULL Passed for deletion");
}

int32_t nem_map_dpid_to_dpnsid(struct of_nem_dpid_dpns_info *dp_dpns_info_p,uint64_t*  output_handle_p)
{
   int32_t  status,ret_value;
   uint32_t hashkey;
   struct   nem_dpid_and_dpnsid_map_entry* map_entry_p      = NULL; 
   uchar8_t  heap_b;
   uchar8_t* hashobj_p = NULL;
   uint32_t  index,magic;
   uint8_t status_b;


   status = NEM_SUCCESS;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   do
   {

      if(nem_dpid_and_dpnsid_db_table_g == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "NEM Namespace Table is NULL"); 
         status=NEM_FAILURE;
         break;
      }

      if(dp_dpns_info_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "input null"); 
         status=NEM_FAILURE;
         break;
      }
      if(dp_dpns_info_p->namespace_name == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "input null"); 
         status=NEM_FAILURE;
         break;
      }

      NEM_PRINT_DPID_AND_DPNSID_MAP_INFO(dp_dpns_info_p);



      hashkey = nem_get_hashval_by_dpid_and_nsname(dp_dpns_info_p->dp_id, dp_dpns_info_p->namespace_name, nem_dpid_and_dpnsid_buckets_g);  
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "hashkey = %d",hashkey); 

      ret_value=nem_dpid_and_dpnsid_db_get_map_entry_handle(dp_dpns_info_p->dp_id, dp_dpns_info_p->namespace_name, output_handle_p);
      if ( ret_value == NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "resource already exists"); 
         status = NEM_ERROR_DUPLICATE_RESOURCE;
         break;
      }

      ret_value = mempool_get_mem_block(nem_dpid_and_dpnsid_db_mempool_g,(uchar8_t**)&map_entry_p,&heap_b);
      if(ret_value != MEMPOOL_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "mempool get block failed"); 
         status = NEM_FAILURE;
         break;
      }

      /* copy parameters */

      map_entry_p->dp_ns_info.dp_id=dp_dpns_info_p->dp_id;
      map_entry_p->dp_ns_info.dp_ns_id=dp_dpns_info_p->dp_ns_id;
      strcpy(map_entry_p->dp_ns_info.namespace_name,dp_dpns_info_p->namespace_name);
      map_entry_p->heap_b = heap_b;

      CNTLR_RCU_READ_LOCK_TAKE();
      hashobj_p = (uchar8_t *)map_entry_p + DPID_DPNSID_MAPNODE_HASH_TBLLNK_OFFSET;
      MCHASH_APPEND_NODE(nem_dpid_and_dpnsid_db_table_g,hashkey,map_entry_p,index,magic,hashobj_p,status_b);
      if(FALSE == status_b)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "append node failed"); 
         status = NEM_FAILURE;
         CNTLR_RCU_READ_LOCK_RELEASE();
         break;
      }

      *output_handle_p = magic; 
      *output_handle_p = ((*output_handle_p <<32) | (index));

      map_entry_p->magic = magic;
      map_entry_p->index = index; 

      NEM_PRINT_DPID_AND_DPNSID_MAP_ENTRY(map_entry_p);


      CNTLR_RCU_READ_LOCK_RELEASE();
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "new map entry successfully created"); 

   }while(0);

   if ( status != NEM_SUCCESS)
   {
      if(map_entry_p) 
      {
         mempool_release_mem_block(nem_dpid_and_dpnsid_db_mempool_g,(uchar8_t *)map_entry_p,map_entry_p->heap_b);
      }
   }
   return status;
}

int32_t nem_update_dpid_to_dpnsid_map_entry(uint64_t handle,struct of_nem_dpid_dpns_info* dp_dpns_info_p)
{
   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "not support modification of map_entry"); 
   return NEM_FAILURE;
}

int32_t nem_unmap_dpid_to_dpnsid(uint64_t handle)
{
   uint32_t index,magic;
   int32_t status=NEM_ERROR_INVALID_HANDLE;
   struct   nem_dpid_and_dpnsid_map_entry* map_entry_p = NULL;
   uint8_t  status_b = FALSE;
   uint16_t offset;
  int32_t ret_value;


   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 
   magic = (uint32_t)(handle >>32);
   index = (uint32_t)handle;

   do
   {

      if(nem_dpid_and_dpnsid_db_table_g == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "NEM Namespace Table is NULL"); 
         status=NEM_FAILURE;
         break;
      }
      CNTLR_RCU_READ_LOCK_TAKE();

      MCHASH_ACCESS_NODE(nem_dpid_and_dpnsid_db_table_g,index,magic,map_entry_p,status_b);
      if(TRUE == status_b)
      {

         NEM_PRINT_DPID_AND_DPNSID_MAP_ENTRY(map_entry_p);

         status_b = FALSE;
         offset = DPID_DPNSID_MAPNODE_HASH_TBLLNK_OFFSET;
         ret_value=nem_delete_dpid_from_dpid_and_dpnsid_list(map_entry_p->dp_ns_info.dp_id);
         if(ret_value != NEM_SUCCESS)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "mempool get block failed"); 
            status = NEM_FAILURE;
            break;
         }
         MCHASH_DELETE_NODE_BY_REF(nem_dpid_and_dpnsid_db_table_g,index,magic,struct nem_dpid_and_dpnsid_map_entry *,offset,status_b);
//         CNTLR_RCU_READ_LOCK_RELEASE(); LSR
         if(status_b == TRUE)
         {
            status= NEM_SUCCESS;
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "deleted success from hash table");
         }
         else
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "delete failed  from hash table"); 
            status= NEM_FAILURE;
         }

      }
      CNTLR_RCU_READ_LOCK_RELEASE();

      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "map entry successfully deleted"); 
   }while(0);

   return status;
}

int32_t nem_dpid_and_dpnsid_unmap_all_entries()
{

   struct nem_dpid_and_dpnsid_map_entry* map_entry_p = NULL;
   uint32_t hashkey;
   int32_t status=NEM_SUCCESS;
   uint16_t offset;
   int32_t ret_value;
   int8_t status_b;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   if(nem_dpid_and_dpnsid_db_table_g == NULL)
   {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "NEM Namespace Table is NULL");
      return NEM_FAILURE;
   }

   CNTLR_RCU_READ_LOCK_TAKE();
   MCHASH_TABLE_SCAN(nem_dpid_and_dpnsid_db_table_g,hashkey)
   {
      offset = DPID_DPNSID_MAPNODE_HASH_TBLLNK_OFFSET;
      MCHASH_BUCKET_SCAN(nem_dpid_and_dpnsid_db_table_g,hashkey,map_entry_p,struct nem_dpid_and_dpnsid_map_entry *,offset)
      {
         ret_value=nem_delete_dpid_from_dpid_and_dpnsid_list(map_entry_p->dp_ns_info.dp_id);
         if(ret_value != NEM_SUCCESS)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "mempool get block failed"); 
            status = NEM_FAILURE;
            break;
         }
         MCHASH_DELETE_NODE_BY_REF(nem_dpid_and_dpnsid_db_table_g,map_entry_p->index,map_entry_p->magic,struct nem_dpid_and_dpnsid_map_entry *,offset,status_b);
         if(status_b == TRUE)
         {
            status= NEM_SUCCESS;
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "deleted success from hash table");
         }
         else
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "delete failed  from hash table");
            status= NEM_FAILURE;
         }
      }
   }
   CNTLR_RCU_READ_LOCK_RELEASE();

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "cleanup all entries success");
   return status;
}


int32_t nem_dpid_and_dpnsid_db_get_map_entry_handle(uint64_t dp_id, char *namespace_p, uint64_t* map_entry_handle_p)
{
   uint32_t hashkey;
   uint16_t offset;
   int32_t status= NEM_ERROR_INVALID_NAME;
   struct nem_dpid_and_dpnsid_map_entry* map_entry_p = NULL;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   { 
      if(nem_dpid_and_dpnsid_db_table_g == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "db table null");

         status= NEM_FAILURE;
         break;
      }
      hashkey = nem_get_hashval_by_dpid_and_nsname(dp_id, namespace_p, nem_dpid_and_dpnsid_buckets_g);

      CNTLR_RCU_READ_LOCK_TAKE();
      offset = DPID_DPNSID_MAPNODE_HASH_TBLLNK_OFFSET;
      MCHASH_BUCKET_SCAN(nem_dpid_and_dpnsid_db_table_g,hashkey,map_entry_p,struct nem_dpid_and_dpnsid_map_entry *,offset)
      {
         if(!((map_entry_p->dp_ns_info.dp_id == dp_id) && (!strcmp(map_entry_p->dp_ns_info.namespace_name, namespace_p))  ))
            continue;
//         CNTLR_RCU_READ_LOCK_RELEASE();

         *map_entry_handle_p = map_entry_p->magic;
         *map_entry_handle_p = ((*map_entry_handle_p <<32) | (map_entry_p->index));

         NEM_PRINT_DPID_AND_DPNSID_MAP_ENTRY(map_entry_p);
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "get handle success"); 
         status=NEM_SUCCESS;
         break;
      }
      CNTLR_RCU_READ_LOCK_RELEASE();
   }while(0);

   return status;
}


int32_t nem_dpid_and_dpnsid_db_get_first_map_entry(struct of_nem_dpid_dpns_info *dp_dpns_info_p,
      uint64_t  *map_entry_handle_p)
{

   struct nem_dpid_and_dpnsid_map_entry* map_entry_p = NULL;
   uint32_t hashkey;

   uint16_t offset;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 
   if(nem_dpid_and_dpnsid_db_table_g == NULL)
   {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "NEM Namespace Table is NULL"); 
      return NEM_FAILURE;
   }

   CNTLR_RCU_READ_LOCK_TAKE();
   MCHASH_TABLE_SCAN(nem_dpid_and_dpnsid_db_table_g,hashkey)
   {
      offset = DPID_DPNSID_MAPNODE_HASH_TBLLNK_OFFSET;
      MCHASH_BUCKET_SCAN(nem_dpid_and_dpnsid_db_table_g,hashkey,map_entry_p,struct nem_dpid_and_dpnsid_map_entry *,offset)
      {
         dp_dpns_info_p->dp_id=map_entry_p->dp_ns_info.dp_id;
         dp_dpns_info_p->dp_ns_id=map_entry_p->dp_ns_info.dp_ns_id;
         strcpy(dp_dpns_info_p->namespace_name,map_entry_p->dp_ns_info.namespace_name);

         *map_entry_handle_p = map_entry_p->magic;
         *map_entry_handle_p = ((*map_entry_handle_p <<32) | (map_entry_p->index));
         NEM_PRINT_DPID_AND_DPNSID_MAP_ENTRY(map_entry_p);
         CNTLR_RCU_READ_LOCK_RELEASE();
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "get first map_entry success"); 
         return NEM_SUCCESS;
      }
   }
   CNTLR_RCU_READ_LOCK_RELEASE();

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "get first map_entry failed"); 
   return NEM_ERROR_NO_MORE_ENTRIES;

}


int32_t nem_dpid_and_dpnsid_db_get_next_map_entry(struct of_nem_dpid_dpns_info *dp_dpns_info_p,
      uint64_t *map_entry_handle_p)
{
   struct nem_dpid_and_dpnsid_map_entry* map_entry_p = NULL;
   uint32_t hashkey;
   uint8_t current_entry_found_b;
   uint16_t offset;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 
   CNTLR_RCU_READ_LOCK_TAKE();
   current_entry_found_b = FALSE;

   MCHASH_TABLE_SCAN(nem_dpid_and_dpnsid_db_table_g,hashkey)
   {
      offset = DPID_DPNSID_MAPNODE_HASH_TBLLNK_OFFSET;
      MCHASH_BUCKET_SCAN(nem_dpid_and_dpnsid_db_table_g,hashkey,map_entry_p,struct nem_dpid_and_dpnsid_map_entry *,offset)
      {
         if(current_entry_found_b == FALSE)
         {

            if(!(( map_entry_p->magic  == ((uint32_t)(*map_entry_handle_p >>32)) ) &
                     ( map_entry_p->index  == ((uint32_t)(*map_entry_handle_p ) ))))
            {
               continue;
            }
            else
            {
               current_entry_found_b = TRUE;
               continue;
            }

            /*Skip the First matching DPID Info and Get the next DPID Info */
         }

         dp_dpns_info_p->dp_id=map_entry_p->dp_ns_info.dp_id;
         dp_dpns_info_p->dp_ns_id=map_entry_p->dp_ns_info.dp_ns_id;
         strcpy(dp_dpns_info_p->namespace_name,map_entry_p->dp_ns_info.namespace_name);

         *map_entry_handle_p = map_entry_p->magic;
         *map_entry_handle_p = ((*map_entry_handle_p <<32) | (map_entry_p->index));

         NEM_PRINT_DPID_AND_DPNSID_MAP_ENTRY(map_entry_p);
         CNTLR_RCU_READ_LOCK_RELEASE();
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "get next map_entry success"); 
         return NEM_SUCCESS;
      }
   }
   CNTLR_RCU_READ_LOCK_RELEASE();

   if(current_entry_found_b == TRUE)
   {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "no more entries"); 
      return NEM_ERROR_NO_MORE_ENTRIES;
   }
   else
   {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "get next map_entry failed"); 
      return NEM_ERROR_INVALID_HANDLE;
   }
}


int32_t  nem_dpid_and_dpnsid_db_get_exact_map_entry(uint64_t handle,
      struct of_nem_dpid_dpns_info* dp_dpns_info_p)
{
   uint32_t index,magic;
   struct   nem_dpid_and_dpnsid_map_entry* map_entry_p;
   uint8_t  status_b;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 
   magic = (uint32_t)(handle >>32);
   index = (uint32_t)handle;

   CNTLR_RCU_READ_LOCK_TAKE();

   MCHASH_ACCESS_NODE(nem_dpid_and_dpnsid_db_table_g,index,magic,map_entry_p,status_b);
   if(TRUE == status_b)
   {

      dp_dpns_info_p->dp_id=map_entry_p->dp_ns_info.dp_id;
      dp_dpns_info_p->dp_ns_id=map_entry_p->dp_ns_info.dp_ns_id;
      strcpy(dp_dpns_info_p->namespace_name,map_entry_p->dp_ns_info.namespace_name);
      NEM_PRINT_DPID_AND_DPNSID_MAP_ENTRY(map_entry_p);
      CNTLR_RCU_READ_LOCK_RELEASE();
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "get exact map_entry success"); 
      return NEM_SUCCESS;
   }
   CNTLR_RCU_READ_LOCK_RELEASE();

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "no map entry found with given handle"); 
   return NEM_ERROR_INVALID_HANDLE;
}


int32_t of_nem_get_dpnsid_from_dpid_and_namespace(uint64_t dp_id, char *namespace_p, uint16_t *dp_ns_id_p)
{
   int32_t  status,ret_value,hashkey;
   uint64_t map_entry_handle;
   struct of_nem_dpid_dpns_info dp_ns_info;

   status = NEM_SUCCESS;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   do
   {
      if(nem_dpid_and_dpnsid_db_table_g == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "NEM Namespace Table is NULL"); 
         status=NEM_FAILURE;
         break;
      }

      if(namespace_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "NEM Namespace Table is NULL"); 
         status=NEM_FAILURE;
         break;
      }

      hashkey = nem_get_hashval_by_dpid_and_nsname(dp_id, namespace_p,nem_dpid_and_dpnsid_buckets_g);  
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "hashkey = %d",hashkey); 

      ret_value=nem_dpid_and_dpnsid_db_get_map_entry_handle(dp_id, namespace_p, &map_entry_handle);
      if ( ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "resource doesnt exists"); 
         status = NEM_ERROR_INVALID_HANDLE;
         break;
      }

      ret_value=  nem_dpid_and_dpnsid_db_get_exact_map_entry(map_entry_handle,
            &dp_ns_info);
      if ( ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "resource doesnt exists, invalid handle"); 
         status = NEM_ERROR_INVALID_HANDLE;
         break;
      }

      NEM_PRINT_DPID_AND_DPNSID_MAP_INFO((&dp_ns_info));
      *dp_ns_id_p=dp_ns_info.dp_ns_id;

   }while(0);

   return status;
}


int32_t nem_validate_namespace_and_generate_dpnsid_for_dpid(uint64_t dp_id, char *namespace_p, uint16_t *dp_ns_id_p)
{
   int32_t  status,ret_value;
   uint16_t dp_ns_id;


   status = NEM_SUCCESS;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   {
      if(nem_dpid_and_dpnsid_db_table_g == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "NEM Namespace Table is NULL");
         status=NEM_FAILURE;
         break;
      }

      if(namespace_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "input null");
         status=NEM_FAILURE;
         break;
      }

      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "namespace %s", namespace_p);
      if(strlen(namespace_p) < 3)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "name length is not enough");
         status=NEM_ERROR_NAME_LENGTH_NOT_ENOUGH;
         break;
      }

      ret_value= of_nem_get_dpnsid_from_dpid_and_namespace(dp_id, namespace_p, &dp_ns_id );
      if ( ret_value == NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "resource already exists");
         status=NEM_ERROR_DUPLICATE_RESOURCE;
         break;
      }

      ret_value=nem_generate_dpnsid_for_dpid(dp_id, &dp_ns_id);
      if ( ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "generation of dpnsid failed");
         status=NEM_ERROR_INVALID_HANDLE;
         break;
      }

      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "new generated dp_ns_id=%d for dpid=%llx", dp_ns_id,dp_id);
      *dp_ns_id_p=dp_ns_id;

   }while(0);

   return status;
}

/******************************************************************************
 * * * * * * * * * * * *DPID and DPNSID list * * * * * * * * * * * * * * * * * 
 ******************************************************************************/

void nem_dpid_and_dpnsid_list_init()
{
   int32_t ii;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   for (ii =0; ii < DPRM_MAX_DATAPATH_ENTRIES; ii ++ )
   {
      of_nem_dpid_ns_info_g[ii].valid_b = FALSE;
      of_nem_dpid_ns_info_g[ii].dp_id = 0;
      of_nem_dpid_ns_info_g[ii].dp_ns_id = 0;
   }
   return;
}

int32_t nem_generate_dpnsid_for_dpid(uint64_t dp_id, uint16_t *dp_ns_id_p)
{
   int32_t ii;
   int32_t status=NEM_FAILURE;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   {
      for (ii =0; ii < DPRM_MAX_DATAPATH_ENTRIES; ii ++ )
      {
         if ( of_nem_dpid_ns_info_g[ii].valid_b == TRUE )
         {
            if ( of_nem_dpid_ns_info_g[ii].dp_id == dp_id )
            {
               *dp_ns_id_p=of_nem_dpid_ns_info_g[ii].dp_ns_id;
               status= NEM_SUCCESS;
               OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "dpid %llx new nsid  %d", of_nem_dpid_ns_info_g[ii].dp_id, of_nem_dpid_ns_info_g[ii].dp_ns_id);
               of_nem_dpid_ns_info_g[ii].dp_ns_id++;
               break;
            }
         }
      }
      if ( status == NEM_SUCCESS)
         break;
      for (ii =0; ii < DPRM_MAX_DATAPATH_ENTRIES; ii ++ )
      {
         if ( of_nem_dpid_ns_info_g[ii].valid_b == FALSE )
         {
            of_nem_dpid_ns_info_g[ii].valid_b = TRUE;
            of_nem_dpid_ns_info_g[ii].dp_id = dp_id;
            of_nem_dpid_ns_info_g[ii].dp_ns_id = 0;
            status=NEM_SUCCESS;
            *dp_ns_id_p=of_nem_dpid_ns_info_g[ii].dp_ns_id;
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "dpid %llx new nsid  %d", of_nem_dpid_ns_info_g[ii].dp_id, of_nem_dpid_ns_info_g[ii].dp_ns_id);
            of_nem_dpid_ns_info_g[ii].dp_ns_id++;
            break;
         }
      }

   }while(0);

   return status;

}

int32_t nem_get_max_dpnsid_for_dpid(uint64_t dp_id, uint16_t *dp_ns_id_p)
{
   int32_t ii;
   int32_t status=NEM_FAILURE;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   {
      for (ii =0; ii < DPRM_MAX_DATAPATH_ENTRIES; ii ++ )
      {
         if ( of_nem_dpid_ns_info_g[ii].valid_b == TRUE )
         {
            if ( of_nem_dpid_ns_info_g[ii].dp_id == dp_id )
            {
               *dp_ns_id_p=of_nem_dpid_ns_info_g[ii].dp_ns_id;
               status= NEM_SUCCESS;
               OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "dpid %llx new nsid  %d", of_nem_dpid_ns_info_g[ii].dp_id, of_nem_dpid_ns_info_g[ii].dp_ns_id);
               break;
            }
         }
      }

   }while(0);

   return status;

}
int32_t nem_delete_dpid_from_dpid_and_dpnsid_list(uint64_t dp_id)
{
   int32_t ii;
   int32_t status=NEM_FAILURE;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");

   do
   {
      for (ii =0; ii < DPRM_MAX_DATAPATH_ENTRIES; ii ++ )
      {
         if ( of_nem_dpid_ns_info_g[ii].valid_b == TRUE )
         {
            if ( of_nem_dpid_ns_info_g[ii].dp_id == dp_id )
            {
               of_nem_dpid_ns_info_g[ii].valid_b = FALSE;
               of_nem_dpid_ns_info_g[ii].dp_id = 0;
               of_nem_dpid_ns_info_g[ii].dp_ns_id = 0;
               status=NEM_SUCCESS;
               break;
            }
         }
      }

   }while(0);

   return status;
}

#endif /* CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT*/
