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
 * Description: A network map_entry is logical mapping between network elements.This file provides datapath ID and PortID to NamespaceID and PethInterface  mapping APIs. It also provides APIs to getfirst/getnext/getexact/delete map_entrys. NEM maintains  separate hash table and mempool list for map_entry maintanance for this database.
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


/******************************************************************************
 * * * * * * * * * * * * Global Variables * * * * * * * * * * * * * * * * * * *
 *******************************************************************************/
uint32_t   nem_peth_id_g=0;
void     *nem_dpid_2_peth_db_mempool_g = NULL;
uint32_t nem_dpid_2_peth_buckets_g;
struct   mchash_table* nem_dpid_2_peth_db_table_g = NULL;
extern uint32_t nem_root_ns_fd_g;
struct of_nem_iface_callbacks *nem_iface_callbacks_p=NULL;

/******************************************************************************
 * * * * * * * * * * * * Function Prototypes * * * * * * * * * * * * * * * * * *
 *******************************************************************************/

void nem_free_dpid_2_peth_dbentry_rcu(struct rcu_head *map_entry_rcu_p);

/******************************************************************************
 * * * * * * * * * * * * DPID to PETH * * * * * * * * * * * * * * * * * * *
 *******************************************************************************/

/******************************************************************************
 * * * * * * * * * * * * Function Defintions * * * * * * * * * * * * * * * * * *
 *******************************************************************************/
int32_t nem_dpid_2_peth_db_init()
{
   int32_t status=NEM_SUCCESS;
   int32_t ret_value;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   do
   {

      ret_value=nem_dpid_2_peth_db_mempool_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Mempool initialization failed"); 
         status=NEM_FAILURE;
         break;
      }

      ret_value=nem_dpid_2_peth_db_mchash_table_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Hash table initialization failed"); 
         status=NEM_FAILURE;
         break;
      }

   }while(0);

   return status;
}

int32_t nem_dpid_2_peth_db_deinit()
{
   int32_t status=NEM_SUCCESS;
   int32_t ret_value;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   do
   {

       ret_value=nem_dpid_to_peth_unmap_all_entries();
      if( CNTLR_UNLIKELY(ret_value != OF_SUCCESS) )
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Cleanup all entries failed");
         status=NEM_FAILURE;
       //  break;
      }

      ret_value=  mchash_table_delete(nem_dpid_2_peth_db_table_g);
      if( CNTLR_UNLIKELY(ret_value != OF_SUCCESS) )
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Mempool De-initialization failed"); 
         status=NEM_FAILURE;
         break;
      }

      /** Delete mempools for MFC database */
      ret_value=mempool_delete_pool(nem_dpid_2_peth_db_mempool_g);
      if( CNTLR_UNLIKELY(ret_value != OF_SUCCESS) )
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Hash table Deinitialization failed"); 
         status=NEM_FAILURE;
         break;
      }

   }while(0);

   return status;
}
int32_t nem_dpid_2_peth_db_mempool_init()
{
   int32_t status=NEM_SUCCESS;
   uint32_t db_entries_max,db_static_entries;
   int32_t ret_value;
   struct mempool_params mempool_info={};

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   nem_dpid_and_peth_db_get_map_entry_mempoolentries(&db_entries_max,&db_static_entries);

   mempool_info.mempool_type = MEMPOOL_TYPE_HEAP;
   mempool_info.block_size = sizeof(struct nem_dpid_and_peth_map_entry);
   mempool_info.max_blocks = db_entries_max;
   mempool_info.static_blocks = db_static_entries;
   mempool_info.threshold_min = db_static_entries/10;
   mempool_info.threshold_max = db_static_entries/3;
   mempool_info.replenish_count = db_static_entries/10;
   mempool_info.b_memset_not_required =  FALSE;
   mempool_info.b_list_per_thread_required =  FALSE;
   mempool_info.noof_blocks_to_move_to_thread_list = 0;

   ret_value = mempool_create_pool("nem_dpid_2_peth_db_pool", &mempool_info,
         &nem_dpid_2_peth_db_mempool_g
         );

   if(ret_value != MEMPOOL_SUCCESS)
   {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "mempool creation failed"); 
      status=NEM_FAILURE;
   }

   return status;
}

int32_t nem_dpid_2_peth_db_mchash_table_init()
{
   uint32_t db_entries_max,db_static_entries;
   int32_t ret_value;
   int32_t status=NEM_SUCCESS;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   nem_dpid_and_peth_db_get_map_entry_mempoolentries(&db_entries_max,&db_static_entries);

   nem_dpid_2_peth_buckets_g = ((db_entries_max / 5)+1);
   ret_value = mchash_table_create( nem_dpid_2_peth_buckets_g,
         db_entries_max,
         nem_free_dpid_2_peth_dbentry_rcu,
         &nem_dpid_2_peth_db_table_g);
   if(ret_value != MCHASHTBL_SUCCESS)
   {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "hash table creation failed"); 
      status=NEM_FAILURE;
   }


   return status;

}


void nem_free_dpid_2_peth_dbentry_rcu(struct rcu_head *map_entry_rcu_p)
{
   struct nem_dpid_and_peth_map_entry *map_entry_p;
   uint16_t offset;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entry");

   if(map_entry_rcu_p)
   {
      offset = DPID_PETH_MAPNODE_HASH_TBLLNK_OFFSET;
      map_entry_p = (struct nem_dpid_and_peth_map_entry *) ( (char *)map_entry_rcu_p -(RCUNODE_OFFSET_IN_MCHASH_TBLLNK)-offset);
      mempool_release_mem_block(nem_dpid_2_peth_db_mempool_g,(uchar8_t*)map_entry_p,map_entry_p->heap_b);
      return;
   }

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "NULL Passed for deletion");
}

int32_t nem_map_dpid_to_peth(struct of_nem_dp_peth_info *dp_peth_info_p,uint64_t*  output_handle_p)
{
   int32_t  status,ret_value;
   uint32_t hashkey;
   struct   nem_dpid_and_peth_map_entry* map_entry_p      = NULL; 
   uchar8_t  heap_b;
   uchar8_t* hashobj_p = NULL;
   uint32_t  index,magic;
   uint8_t status_b;


   status = NEM_SUCCESS;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   do
   {

      if(nem_dpid_2_peth_db_table_g == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Namespace Table is NULL"); 
         status=NEM_FAILURE;
         break;
      }

      if(dp_peth_info_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "input null"); 
         status=NEM_FAILURE;
         break;
      }
#if 0
      if(dp_peth_info_p->port_name == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "port name is  null"); 
         status=NEM_FAILURE;
         break;
      }
#endif
      NEM_PRINT_DPID_AND_PETH_MAP_INFO(dp_peth_info_p);


      hashkey = nem_get_hashval_by_dpid_and_portid(dp_peth_info_p->dp_id, dp_peth_info_p->port_id,nem_dpid_2_peth_buckets_g);  

      ret_value=of_nem_dpid_2_peth_db_get_map_entry_handle(dp_peth_info_p->dp_id, dp_peth_info_p->port_id, output_handle_p);
      if ( ret_value == NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "resource already exists"); 
         status = NEM_ERROR_DUPLICATE_RESOURCE;
         break;
      }

      ret_value = mempool_get_mem_block(nem_dpid_2_peth_db_mempool_g,(uchar8_t**)&map_entry_p,&heap_b);
      if(ret_value != MEMPOOL_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "mempool get block failed"); 
         status = NEM_FAILURE;
         break;
      }

      /* copy parameters */

      map_entry_p->dp_peth_info.dp_id=dp_peth_info_p->dp_id;
      map_entry_p->dp_peth_info.port_id=dp_peth_info_p->port_id;
      map_entry_p->dp_peth_info.peth_id=dp_peth_info_p->peth_id;
      //      strcpy(map_entry_p->dp_peth_info.port_name, dp_peth_info_p->port_name);
      strcpy(map_entry_p->dp_peth_info.peth_name, dp_peth_info_p->peth_name);
      map_entry_p->dp_peth_info.ns_id=dp_peth_info_p->ns_id;
      map_entry_p->heap_b = heap_b;

      CNTLR_RCU_READ_LOCK_TAKE();
      hashobj_p = (uchar8_t *)map_entry_p + DPID_PETH_MAPNODE_HASH_TBLLNK_OFFSET;
      MCHASH_APPEND_NODE(nem_dpid_2_peth_db_table_g,hashkey,map_entry_p,index,magic,hashobj_p,status_b);
      if(FALSE == status_b)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "append node failed"); 
         status = NEM_FAILURE;
         CNTLR_RCU_READ_LOCK_RELEASE();
         break;
      }

      *output_handle_p = magic; 
      *output_handle_p = ((*output_handle_p <<32) | (index));

      map_entry_p->magic = magic;
      map_entry_p->index = index; 

      NEM_PRINT_DPID_AND_PETH_MAP_ENTRY(map_entry_p);


      CNTLR_RCU_READ_LOCK_RELEASE();
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "new map entry successfully created"); 

   }while(0);

   if ( status != NEM_SUCCESS)
   {
      if(map_entry_p) 
      {
         mempool_release_mem_block(nem_dpid_2_peth_db_mempool_g,(uchar8_t *)map_entry_p,map_entry_p->heap_b);
      }
   }
   return status;
}

int32_t nem_update_dpid_to_peth_map_entry(uint64_t handle,struct of_nem_dp_peth_info* dp_peth_info_p)
{
   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "not support modification of map_entry"); 
   return NEM_FAILURE;
}

int32_t nem_unmap_dpid_to_peth(uint64_t handle)
{
   uint32_t index,magic;
   int32_t status=NEM_ERROR_INVALID_HANDLE;
   struct   nem_dpid_and_peth_map_entry* map_entry_p = NULL;
   uint8_t  status_b = FALSE;
   uint16_t offset;


   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 
   magic = (uint32_t)(handle >>32);
   index = (uint32_t)handle;

   do
   {

      if(nem_dpid_2_peth_db_table_g == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Namespace Table is NULL"); 
         status=NEM_FAILURE;
         break;
      }
      CNTLR_RCU_READ_LOCK_TAKE();

      MCHASH_ACCESS_NODE(nem_dpid_2_peth_db_table_g,index,magic,map_entry_p,status_b);
      if(TRUE == status_b)
      {

         NEM_PRINT_DPID_AND_PETH_MAP_ENTRY(map_entry_p);

         status_b = FALSE;
         offset = DPID_PETH_MAPNODE_HASH_TBLLNK_OFFSET;
         MCHASH_DELETE_NODE_BY_REF(nem_dpid_2_peth_db_table_g,index,magic,struct nem_dpid_and_peth_map_entry *,offset,status_b);
//         CNTLR_RCU_READ_LOCK_RELEASE();
         if(status_b == TRUE)
         {
            status= NEM_SUCCESS;
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "deleted success from hash table"); 
         }
         else
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "delete failed  from hash table"); 
            status= NEM_FAILURE;
         }

      }
      CNTLR_RCU_READ_LOCK_RELEASE();

      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "map entry successfully deleted"); 
   }while(0);

   return status;
}

int32_t nem_dpid_to_peth_unmap_all_entries()
{

   struct nem_dpid_and_peth_map_entry* map_entry_p = NULL;
   uint32_t hashkey;
   int32_t status=NEM_SUCCESS;
   uint16_t offset;
   int8_t status_b;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   if(nem_dpid_2_peth_db_table_g == NULL)
   {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "NEM Namespace Table is NULL");
      return NEM_FAILURE;
   }

   CNTLR_RCU_READ_LOCK_TAKE();
   MCHASH_TABLE_SCAN(nem_dpid_2_peth_db_table_g,hashkey)
   {
      offset = DPID_PETH_MAPNODE_HASH_TBLLNK_OFFSET;
      MCHASH_BUCKET_SCAN(nem_dpid_2_peth_db_table_g,hashkey,map_entry_p,struct nem_dpid_and_peth_map_entry *,offset)
      {
         MCHASH_DELETE_NODE_BY_REF(nem_dpid_2_peth_db_table_g,map_entry_p->index,map_entry_p->magic,struct nem_dpid_and_peth_map_entry *,offset,status_b);
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


int32_t of_nem_dpid_2_peth_db_get_map_entry_handle(uint64_t dp_id, uint32_t port_id, uint64_t* map_entry_handle_p)
{
   uint32_t hashkey;
   uint16_t offset;
   int32_t status= NEM_ERROR_INVALID_NAME;
   struct nem_dpid_and_peth_map_entry* map_entry_p = NULL;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   { 
      if(nem_dpid_2_peth_db_table_g == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "db table null");

         status= NEM_FAILURE;
         break;
      }
#if 0
      if(port_name == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "port name is  null"); 
         status=NEM_FAILURE;
         break;
      }
#endif
      hashkey = nem_get_hashval_by_dpid_and_portid(dp_id, port_id,nem_dpid_2_peth_buckets_g);

      CNTLR_RCU_READ_LOCK_TAKE();
      offset = DPID_PETH_MAPNODE_HASH_TBLLNK_OFFSET;
      MCHASH_BUCKET_SCAN(nem_dpid_2_peth_db_table_g,hashkey,map_entry_p,struct nem_dpid_and_peth_map_entry *,offset)
      {
         if(!((map_entry_p->dp_peth_info.dp_id == dp_id) && (map_entry_p->dp_peth_info.port_id == port_id)))
            continue;
//         CNTLR_RCU_READ_LOCK_RELEASE();

         *map_entry_handle_p = map_entry_p->magic;
         *map_entry_handle_p = ((*map_entry_handle_p <<32) | (map_entry_p->index));

         NEM_PRINT_DPID_AND_PETH_MAP_ENTRY(map_entry_p);
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "get handle success"); 
         status=NEM_SUCCESS;
         break;
      }
      CNTLR_RCU_READ_LOCK_RELEASE();
   }while(0);

   return status;
}


int32_t of_nem_dpid_2_peth_db_get_first_map_entry(struct of_nem_dp_peth_info *dp_peth_info_p,
      uint64_t  *map_entry_handle_p)
{

   struct nem_dpid_and_peth_map_entry* map_entry_p = NULL;
   uint32_t hashkey;

   uint16_t offset;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 
   if(nem_dpid_2_peth_db_table_g == NULL)
   {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Namespace Table is NULL"); 
      return NEM_FAILURE;
   }

   CNTLR_RCU_READ_LOCK_TAKE();
   MCHASH_TABLE_SCAN(nem_dpid_2_peth_db_table_g,hashkey)
   {
      offset = DPID_PETH_MAPNODE_HASH_TBLLNK_OFFSET;
      MCHASH_BUCKET_SCAN(nem_dpid_2_peth_db_table_g,hashkey,map_entry_p,struct nem_dpid_and_peth_map_entry *,offset)
      {
         dp_peth_info_p->dp_id=map_entry_p->dp_peth_info.dp_id;
         dp_peth_info_p->ns_id=map_entry_p->dp_peth_info.ns_id;
         dp_peth_info_p->peth_id=map_entry_p->dp_peth_info.peth_id;
         dp_peth_info_p->port_id=map_entry_p->dp_peth_info.port_id;
         //         strcpy ( dp_peth_info_p->port_name, map_entry_p->dp_peth_info.port_name);
         strcpy ( dp_peth_info_p->peth_name, map_entry_p->dp_peth_info.peth_name);

         *map_entry_handle_p = map_entry_p->magic;
         *map_entry_handle_p = ((*map_entry_handle_p <<32) | (map_entry_p->index));
         NEM_PRINT_DPID_AND_PETH_MAP_ENTRY(map_entry_p);
         CNTLR_RCU_READ_LOCK_RELEASE();
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "get first map_entry success"); 
         return NEM_SUCCESS;
      }
   }
   CNTLR_RCU_READ_LOCK_RELEASE();

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "get first map_entry failed"); 
   return NEM_ERROR_NO_MORE_ENTRIES;

}


int32_t of_nem_dpid_2_peth_db_get_next_map_entry(struct of_nem_dp_peth_info *dp_peth_info_p,
      uint64_t *map_entry_handle_p)
{
   struct nem_dpid_and_peth_map_entry* map_entry_p = NULL;
   uint32_t hashkey;
   uint8_t current_entry_found_b;
   uint16_t offset;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 
   CNTLR_RCU_READ_LOCK_TAKE();
   current_entry_found_b = FALSE;

   MCHASH_TABLE_SCAN(nem_dpid_2_peth_db_table_g,hashkey)
   {
      offset = DPID_PETH_MAPNODE_HASH_TBLLNK_OFFSET;
      MCHASH_BUCKET_SCAN(nem_dpid_2_peth_db_table_g,hashkey,map_entry_p,struct nem_dpid_and_peth_map_entry *,offset)
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

         dp_peth_info_p->dp_id=map_entry_p->dp_peth_info.dp_id;
         dp_peth_info_p->ns_id=map_entry_p->dp_peth_info.ns_id;
         dp_peth_info_p->port_id=map_entry_p->dp_peth_info.port_id;
         dp_peth_info_p->peth_id=map_entry_p->dp_peth_info.peth_id;
         //         strcpy (dp_peth_info_p->port_name, map_entry_p->dp_peth_info.port_name);
         strcpy ( dp_peth_info_p->peth_name, map_entry_p->dp_peth_info.peth_name);

         *map_entry_handle_p = map_entry_p->magic;
         *map_entry_handle_p = ((*map_entry_handle_p <<32) | (map_entry_p->index));

         NEM_PRINT_DPID_AND_PETH_MAP_ENTRY(map_entry_p);
         CNTLR_RCU_READ_LOCK_RELEASE();
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "get next map_entry success"); 
         return NEM_SUCCESS;
      }
   }
   CNTLR_RCU_READ_LOCK_RELEASE();

   if(current_entry_found_b == TRUE)
   {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_INFO, "no more entries"); 
      return NEM_ERROR_NO_MORE_ENTRIES;
   }
   else
   {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "get next map_entry failed"); 
      return NEM_ERROR_INVALID_HANDLE;
   }
}


int32_t  of_nem_dpid_2_peth_db_get_exact_map_entry(uint64_t handle,
      struct of_nem_dp_peth_info* dp_peth_info_p)
{
   uint32_t index,magic;
   struct   nem_dpid_and_peth_map_entry* map_entry_p;
   uint8_t  status_b;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 
   magic = (uint32_t)(handle >>32);
   index = (uint32_t)handle;

   CNTLR_RCU_READ_LOCK_TAKE();

   MCHASH_ACCESS_NODE(nem_dpid_2_peth_db_table_g,index,magic,map_entry_p,status_b);
   if(TRUE == status_b)
   {

      dp_peth_info_p->dp_id=map_entry_p->dp_peth_info.dp_id;
      dp_peth_info_p->ns_id=map_entry_p->dp_peth_info.ns_id;
      dp_peth_info_p->port_id=map_entry_p->dp_peth_info.port_id;
      dp_peth_info_p->peth_id=map_entry_p->dp_peth_info.peth_id;
      //strcpy ( dp_peth_info_p->port_name, map_entry_p->dp_peth_info.port_name);
      strcpy ( dp_peth_info_p->peth_name, map_entry_p->dp_peth_info.peth_name);
      NEM_PRINT_DPID_AND_PETH_MAP_ENTRY(map_entry_p);
      CNTLR_RCU_READ_LOCK_RELEASE();
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "get exact map_entry success"); 
      return NEM_SUCCESS;
   }
   CNTLR_RCU_READ_LOCK_RELEASE();

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "no map entry found with given handle"); 
   return NEM_ERROR_INVALID_HANDLE;
}


int32_t of_nem_get_nsid_and_peth_from_dpid_and_port_id(uint64_t dp_id, 
                                                       uint32_t port_id, 
                                                       uint32_t *ns_id_p, 
                                                       char *peth_name_p,
                                                       uint32_t *peth_id_p)
{
   int32_t  status,ret_value;
   uint64_t map_entry_handle;
   struct of_nem_dp_peth_info dp_peth_info;

   status = NEM_SUCCESS;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   do
   {
      if(nem_dpid_2_peth_db_table_g == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Namespace Table is NULL"); 
         status=NEM_FAILURE;
         break;
      }
#if 0
      if(port_name_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "input is  NULL"); 
         status=NEM_FAILURE;
         break;
      }
#endif
      if(peth_name_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "output is  NULL"); 
         status=NEM_FAILURE;
         break;
      }

      ret_value=of_nem_dpid_2_peth_db_get_map_entry_handle(dp_id, port_id, &map_entry_handle);
      if ( ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "resource doesnt exists"); 
         status = NEM_ERROR_INVALID_HANDLE;
         break;
      }

      ret_value=  of_nem_dpid_2_peth_db_get_exact_map_entry(map_entry_handle,
            &dp_peth_info);
      if ( ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "resource doesnt exists, invalid handle"); 
         status = NEM_ERROR_INVALID_HANDLE;
         break;
      }

      NEM_PRINT_DPID_AND_PETH_MAP_INFO((&dp_peth_info));
      
      *ns_id_p=dp_peth_info.ns_id;
      *peth_id_p=dp_peth_info.peth_id;
      strcpy( peth_name_p, dp_peth_info.peth_name);

   }while(0);

   return status;
}

int32_t nem_validate_dpid_portid_and_create_peth(uint64_t dp_id, uint32_t port_id, char *ns_name, uint32_t ns_id, char *peth_name_p, uint32_t *peth_id_p)
{
   int32_t  status,ret_value;
   uint64_t map_entry_handle;
   //uint32_t peth_id;
   int32_t peth_index;
   int32_t ns_fd;
   char command[128];
   char file_name[128];

   status = NEM_SUCCESS;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   {

      if(peth_name_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "peth input null");
         status=NEM_FAILURE;
         break;
      }
      //    OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "portname %s", port_name_p);

      ret_value=of_nem_dpid_2_peth_db_get_map_entry_handle(dp_id, port_id, &map_entry_handle);
      if ( ret_value == NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_INFO, "resource already exists");
         status = NEM_ERROR_DUPLICATE_RESOURCE;
         break;
      }

      sprintf(peth_name_p, "peth_%d", port_id);
#if 1
      if (!nem_iface_callbacks_p)
      {
	      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "peth interface creation failed");
	      status=NEM_ERROR_INVALID_HANDLE;
	      break;
      }
      ret_value=nem_iface_callbacks_p->iface_create(ns_name, ns_id, peth_name_p, &peth_index);
//      ret_value=peth_create_net_dev_iface(ns_name, ns_id, peth_name_p, &peth_index);
      if ( ret_value != OF_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "peth interface creation failed");
         status=NEM_ERROR_INVALID_HANDLE;
         break;
      }
#else
      NEM_GENERATE_PETH_ID(peth_id);
      peth_index=peth_id;
#endif
      ret_value=nem_set_cur_task_to_name_space(ns_name, &ns_fd);
      if ( ret_value != OF_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "set current task to namespace  failed");
         status=NEM_ERROR_INVALID_HANDLE;
         break;
      }
      memset(command,0,128);
      memset(file_name,0,128);
      sprintf(file_name, "/proc/sys/net/ipv4/neigh/%s/gc_stale_time",peth_name_p);
      sprintf(command, "echo 64800 > %s",file_name);
      OF_LOG_MSG(OF_RT_LOG, OF_LOG_DEBUG, "executing command %s",command);
      ret_value=system(command);
      if ( ret_value == OF_FAILURE)
      {
	      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_WARN, "system command failed %s",command);
              //status=NEM_FAILURE;
	      //break;
      }

      ret_value=nem_set_cur_task_to_root_name_space(nem_root_ns_fd_g, ns_fd);
      if ( ret_value != OF_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "set current task to rooot namespace  failed");
         status=NEM_ERROR_INVALID_HANDLE;
         break;
      }

      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "new peth interface %s peth index %d", peth_name_p, peth_index);
      *peth_id_p=peth_index;

   }while(0);

   return status;
}



int32_t of_nem_register_iface_callbacks(struct of_nem_iface_callbacks *iface_callbacks_p)
{
	int32_t                 status = OF_SUCCESS;

	OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_DEBUG,"entered");

	do
	{

		if ( iface_callbacks_p == NULL )
		{
			OF_LOG_MSG(OF_LOG_UTIL, OF_LOG_ERROR,"Invalid callback pointer");
			status = OF_FAILURE;
			break;
		}

		nem_iface_callbacks_p = iface_callbacks_p;
	}
	while(0);

	return status;
}

int32_t nem_execute_system_command(int32_t ns_id, char *sys_cmd)
{
  int32_t ret_value;
  int32_t status=OF_SUCCESS;
  int32_t  ns_fd;
  char ns_name[NEM_MAX_NS_NAME_LEN];

  do
  {

    if ( sys_cmd == NULL)
    {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "invalid input");
      status=NEM_ERROR_INVALID_HANDLE;
      break;
    }
    ret_value=of_nem_get_namespace_from_nsid(ns_id, ns_name);
    if ( ret_value != OF_SUCCESS)
    {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "set current task to namespace  failed");
      status=NEM_ERROR_INVALID_HANDLE;
      break;
    }

    ret_value=nem_set_cur_task_to_name_space(ns_name, &ns_fd);
    if ( ret_value != OF_SUCCESS)
    {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "set current task to namespace  failed");
      status=NEM_ERROR_INVALID_HANDLE;
      break;
    }
    ret_value=system(sys_cmd);
    if ( ret_value == OF_FAILURE)
    {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_WARN, "system command failed %s",sys_cmd);
      status=NEM_FAILURE;
      //break;
    }

    ret_value=nem_set_cur_task_to_root_name_space(nem_root_ns_fd_g, ns_fd);
    if ( ret_value != OF_SUCCESS)
    {
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "set current task to rooot namespace  failed");
      status=NEM_ERROR_INVALID_HANDLE;
      break;
    }

  }while(0);

  return status;
}
#endif /* CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT*/
