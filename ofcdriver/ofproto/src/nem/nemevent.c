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



/* File  :      nemevent.c
 * Author:      Atmaram G <atmaram.g@freescale.com>
 * Date:   08/23/2013
 * Description: A network map_entry is logical mapping between network elements.NEM maintains multiple databases containing mapping information of network elements. This file proves notification APIs, interested applications will get notified when mapping entry created in database or unmapping is done.
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

#define NEM_MAX_NOTIFICATION_ENTRIES       128
#define NEM_MAX_NOTIFICATION_STATIC_ENTRIES   32 
#define NEM_MAX_DOMAINS 32

uint64_t domain_handles_g[NEM_MAX_DOMAINS];
uint8_t domain_count_g=0;
/******************************************************************************
 * * * * * * * * * * * * * * NEM Events * * * * * * * * * * * * * * * * * * * *
 *******************************************************************************/

void nem_dprm_domain_notifier_handler(uint32_t notification_type,
      uint64_t domain_handle,
      struct   dprm_domain_notification_data notification_data,
      void     *callback_arg1,
      void     *callback_arg2);
int32_t
nem_dp_ready_event_handler(uint64_t  controller_handle,
      uint64_t   domain_handle,
      uint64_t   datapath_handle,
      void       *cbk_arg1,
      void       *cbk_arg2);
int32_t
nem_dp_detach_event_handler(uint64_t  controller_handle,
      uint64_t   domain_handle,
      uint64_t   datapath_handle,
      void       *cbk_arg1,
      void       *cbk_arg2);
void nem_namespace_notification_handler(uint32_t notification_type,
      struct   dprm_namespace_notification_data notification_data,
      void     *callback_arg1,
      void     *callback_arg2);
void nem_namespace_create_event_handler( struct   dprm_namespace_notification_data *dprm_event_data_p);
void nem_namespace_delete_event_handler(  struct   dprm_namespace_notification_data *dprm_event_data_p);
void nem_namespace_datapath_attach_event_handler( struct   dprm_namespace_notification_data *dprm_event_data_p);
void nem_namespace_datapath_detach_event_handler( struct   dprm_namespace_notification_data *dprm_event_data_p);
void nem_namespace_port_attach_event_handler( struct   dprm_namespace_notification_data *dprm_event_data_p);
void nem_namespace_port_detach_event_handler( struct   dprm_namespace_notification_data *dprm_event_data_p);

int32_t nem_send_dp_and_ns_active_event(uint64_t dp_id, char *domain_name);
int32_t nem_send_dp_and_ns_deactive_event(uint64_t dp_id, char *domain_name);
/******************************************************************************
 * * * * * * * * * * * * * * RCU Functions * * * * * * * * * * * * * * * * * * * 
 *******************************************************************************/
void nem_free_notfication_appentry_rcu(struct rcu_head *app_entry_rcu_p);

/******************************************************************************
 * * * * * * * * * * * * * * Global Variables * * * * * * * * * * * * * * * * * * 
 *******************************************************************************/
void   *nem_notifications_mempool_g;
of_list_t nem_notifications[NEM_MAX_EVENT];
uint32_t nem_root_ns_fd_g=0;
extern struct of_nem_iface_callbacks *nem_iface_callbacks_p;

/******************************************************************************
 * * * * * * * * * * * * * * Function Definitionis* * * * * * * * * * * * * * * * *
 *******************************************************************************/
int32_t nem_event_init()
{
   int32_t status=NEM_SUCCESS;
   int32_t ret_value;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   {
      /* * * * * * * * * * * * Mempool Initialization * * * * * * * * * * * * * * * * * * */
      ret_value=nem_event_mempool_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Event Mempool  initialization failed");
         status=NEM_FAILURE;
         break;
      }

      /* * * * * * * * * * * * Event Registration * * * * * * * * * * * * * * * * * * */
      ret_value=nem_event_register();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Event Registration failed");
         status=NEM_FAILURE;
         break;
      }

     ret_value=nem_root_ns_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Root Namespace Initialization  failed");
         status=NEM_FAILURE;
         break;
      }
   }while(0);

   return status;
}

int32_t nem_event_mempool_init()
{
   int32_t  ii,ret_value,status=NEM_SUCCESS;
   uint32_t notification_entries_max,notification_static_entries;
   struct mempool_params mempool_info={};   

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   do
   {
      nem_get_notification_mempoolentries(&notification_entries_max,&notification_static_entries);

      mempool_info.mempool_type = MEMPOOL_TYPE_HEAP;
      mempool_info.block_size = sizeof(struct nem_notification_app_hook_info);
      mempool_info.max_blocks = notification_entries_max;
      mempool_info.static_blocks = notification_static_entries;
      mempool_info.threshold_min = notification_static_entries/10;
      mempool_info.threshold_max = notification_static_entries/3;
      mempool_info.replenish_count = notification_static_entries/10;
      mempool_info.b_memset_not_required =  FALSE;
      mempool_info.b_list_per_thread_required =  FALSE;
      mempool_info.noof_blocks_to_move_to_thread_list = 0;

      ret_value = mempool_create_pool("nem_notifications_pool", &mempool_info,
            &nem_notifications_mempool_g);
      if(ret_value != MEMPOOL_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "mempool creation failed");
         status=NEM_FAILURE;
         break;
      }

      for(ii=0;ii<NEM_MAX_EVENT;ii++)
      {
         OF_LIST_INIT(nem_notifications[ii],nem_free_notfication_appentry_rcu);
      }

   }while(0);

   return status;
}

void nem_get_notification_mempoolentries(uint32_t *notification_entries_max,uint32_t *notification_static_entries)
{
   *notification_entries_max    = NEM_MAX_NOTIFICATION_ENTRIES;
   *notification_static_entries = NEM_MAX_NOTIFICATION_STATIC_ENTRIES;
}


void nem_free_notfication_appentry_rcu(struct rcu_head *app_entry_rcu_p)
{
   struct nem_notification_app_hook_info *app_entry_p;
   uint16_t offset;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entry");

   if(app_entry_rcu_p)
   {
      offset = NEM_NOTIFICATIONS_APP_HOOK_RCU_HEAD_OFFSET;
      app_entry_p = (struct nem_notification_app_hook_info *) ( (char *)app_entry_rcu_p -offset);
      mempool_release_mem_block(nem_notifications_mempool_g,(uchar8_t*)app_entry_p,app_entry_p->heap_b);
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "memory released");
      return;
   }

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "NULL Passed for deletion");
}


int32_t of_nem_register_nem_notifications( uint32_t  notification_type, of_nem_notifier_fn nem_notifier_fn, void * callback_arg1, void *callback_arg2)
{

   struct  nem_notification_app_hook_info *app_entry_p;
   uint8_t heap_b;
   int32_t ret_value,status=NEM_SUCCESS;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");

   do
   {
      if((notification_type < NEM_FIRST_EVENT) || (notification_type > NEM_MAX_EVENT))
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "invalid notification type");
         status= NEM_ERROR_INVALID_NOTIFICATION_TYPE;
         break;
      }
      if(nem_notifier_fn == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "invalid notification type");
         status=NEM_ERROR_NULL_CALLBACK_NOTIFICATION_HOOK;
         break;
      }

      ret_value = mempool_get_mem_block(nem_notifications_mempool_g,(uchar8_t**)&app_entry_p,&heap_b);
      if(ret_value != MEMPOOL_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "MEMPOOL get block failed");
         status= NEM_ERROR_NO_MEM;
         break;
      }
      app_entry_p->call_back = (void*)nem_notifier_fn;
      app_entry_p->cbk_arg1  = callback_arg1;
      app_entry_p->cbk_arg2  = callback_arg2;
      app_entry_p->heap_b    = heap_b;

      /* Add Application to the list of NEM Applications maintained per notification type. */
OF_APPEND_NODE_TO_LIST(nem_notifications[notification_type],app_entry_p,NEM_NOTIFICATIONS_APP_HOOK_LISTNODE_OFFSET);
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "successfully added for notification type %d", notification_type);

   }while(0);

   return status;

}

int32_t of_nem_deregister_notifications( uint32_t  notification_type, of_nem_notifier_fn nem_notifier_fn)
{

   struct  nem_notification_app_hook_info *app_entry_p,*prev_app_entry_p=NULL;
   int32_t status=NEM_FAILURE;
   int8_t offset=NEM_NOTIFICATIONS_APP_HOOK_LISTNODE_OFFSET;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");

   do
   {

      if((notification_type < NEM_FIRST_EVENT) || (notification_type > NEM_MAX_EVENT))
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "invalid notification type");
         status= NEM_ERROR_INVALID_NOTIFICATION_TYPE;
         break;
      }

      if(nem_notifier_fn == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "invalid notification type");
         status=NEM_ERROR_NULL_CALLBACK_NOTIFICATION_HOOK;
         break;
      }

      CNTLR_RCU_READ_LOCK_TAKE();
      OF_LIST_SCAN(nem_notifications[notification_type],app_entry_p,struct nem_notification_app_hook_info *,offset )
      {
	      if(app_entry_p->call_back != nem_notifier_fn)
	      {
		      prev_app_entry_p = app_entry_p;
		      continue;
	      }
	      OF_REMOVE_NODE_FROM_LIST(nem_notifications[notification_type],app_entry_p,prev_app_entry_p,offset );
	      status=NEM_SUCCESS;
	      break;
      }
      CNTLR_RCU_READ_LOCK_RELEASE();

   }while(0);

   return status;

}

int32_t nem_event_register()
{
   int32_t ret_value,status=NEM_SUCCESS;
   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");

   do
   {
      ret_value = dprm_register_namespace_notifications (DPRM_NAMESPACE_ALL_NOTIFICATIONS,
            nem_namespace_notification_handler,
            NULL,
            NULL);
      if(ret_value != DPRM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "registration for namespace notification failed");
         status= NEM_FAILURE;
         break;
      }

      ret_value = dprm_register_forwarding_domain_notifications (DPRM_DOMAIN_ADDED,
            nem_dprm_domain_notifier_handler,
            NULL,
            NULL);
      if(ret_value != DPRM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "registration for namespace notification failed");
         status= NEM_FAILURE;
         break;
      }

   }while(0);

   return status;
}

void nem_dprm_domain_notifier_handler(uint32_t notification_type,
      uint64_t domain_handle,
      struct   dprm_domain_notification_data notification_data,
      void     *callback_arg1,
      void     *callback_arg2)
{
   uint8_t priority=15;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "notification type %d", notification_type);
   switch(notification_type)
   {
      case  DPRM_DOMAIN_ADDED :
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "domain %s added notification",notification_data.domain_name);
         if (cntlr_register_asynchronous_event_hook(domain_handle,
                  DP_READY_EVENT,
                  priority,
                  nem_dp_ready_event_handler,
                  NULL, NULL) == OF_FAILURE)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR,"Register DP Ready event cbk failed");
         }

         if (cntlr_register_asynchronous_event_hook(domain_handle,
                  DP_DETACH_EVENT,
                  priority,
                  nem_dp_detach_event_handler,
                  NULL, NULL) == OF_FAILURE)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR,"Register DP Detach event cbk failed");
         }
          domain_handles_g[domain_count_g++]=domain_handle;
         break;
      default:
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "unknown notification ");
         break;
   }
   return;
}

   int32_t
nem_dp_ready_event_handler(uint64_t  controller_handle,
      uint64_t   domain_handle,
      uint64_t   datapath_handle,
      void       *cbk_arg1,
      void       *cbk_arg2)
{
   int32_t ret_value,status=NEM_SUCCESS;
   uint64_t dp_id;
   struct dprm_domain_entry  *domain_info_p=NULL;
   char domain_name[NEM_MAX_IFACE_NAME_LEN];

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");

   do
   {
      CNTLR_RCU_READ_LOCK_TAKE();
      ret_value=dprm_get_datapath_id_byhandle( datapath_handle,&dp_id);
      if(ret_value != DPRM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "get DPID failed for handle %llx",datapath_handle);
         status=NEM_SUCCESS;
         break;
      }
      ret_value = get_domain_byhandle(domain_handle, &domain_info_p);
      if(ret_value != DPRM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "get domain failed for handle %llx",domain_handle);
         status=NEM_SUCCESS;
         break;
      }
      strncpy(domain_name, domain_info_p->name, NEM_MAX_IFACE_NAME_LEN);
      //send_notification:
      /* * * * * * * * * * * * Send Notifications to Applications * * * * * * * * * * * * * * * * * */
      ret_value= nem_send_dp_and_ns_active_event(dp_id, domain_name);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "nem_send_dp_and_ns_active_event failed");
         status=NEM_FAILURE;
         break;
      }
   }while(0);

   CNTLR_RCU_READ_LOCK_RELEASE();
   return status;
}

   int32_t
nem_dp_detach_event_handler(uint64_t  controller_handle,
      uint64_t   domain_handle,
      uint64_t   datapath_handle,
      void       *cbk_arg1,
      void       *cbk_arg2)
{
   int32_t ret_value,status=NEM_SUCCESS;
   uint64_t dp_id;
   struct dprm_domain_entry  *domain_info_p=NULL;
   char domain_name[NEM_MAX_IFACE_NAME_LEN];

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");

   do
   {
      CNTLR_RCU_READ_LOCK_TAKE();
      ret_value=dprm_get_datapath_id_byhandle( datapath_handle,&dp_id);
      if(ret_value != DPRM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "get DPID failed for handle %llx",datapath_handle);
         status=NEM_SUCCESS;
         break;
      }
      ret_value = get_domain_byhandle(domain_handle, &domain_info_p);
      if(ret_value != DPRM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "get domain failed for handle %llx",domain_handle);
         status=NEM_SUCCESS;
         break;
      }
      strncpy(domain_name, domain_info_p->name, NEM_MAX_IFACE_NAME_LEN);
      //send_notification:
      /* * * * * * * * * * * * Send Notifications to Applications * * * * * * * * * * * * * * * * * */
      ret_value= nem_send_dp_and_ns_deactive_event(dp_id, domain_name);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "nem_send_dp_and_ns_deactive_event failed");
         status=NEM_FAILURE;
         break;
      }
   }while(0);

   CNTLR_RCU_READ_LOCK_RELEASE();
   return status;
}



void nem_namespace_notification_handler(uint32_t notification_type,
      struct  dprm_namespace_notification_data dprm_event_data,
      void     *callback_arg1,
      void     *callback_arg2)
{
   //int32_t ret_value,status=NEM_SUCCESS;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "notification type %d", notification_type);

   switch(notification_type)
   {
      case  DPRM_NAMESPACE_CREATED :
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "namespace creation notification");
         nem_namespace_create_event_handler(&dprm_event_data);
         break;
      case DPRM_NAMESPACE_REMOVED:
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "namespace removed notification");
         nem_namespace_delete_event_handler(&dprm_event_data);
         break;
      case DPRM_NAMESPACE_DATAPATH_ATTACHED:
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "namespace datapath attached notification");
         nem_namespace_datapath_attach_event_handler(&dprm_event_data);
         break;
      case DPRM_NAMESPACE_DATAPATH_DETACHED:
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "namespace datapath detached notification");
         nem_namespace_datapath_detach_event_handler(&dprm_event_data);
         break;
      case DPRM_NAMESPACE_PORT_ATTACHED:
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "namespace port attached notification");
         nem_namespace_port_attach_event_handler(&dprm_event_data);
         break;
      case DPRM_NAMESPACE_PORT_DETACHED:
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "namespace port detached notification");
         nem_namespace_port_detach_event_handler(&dprm_event_data);
         break;
      default:
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "unknown notification ");
         break;
   }

   return;
}

void nem_namespace_create_event_handler(struct   dprm_namespace_notification_data *dprm_event_data_p)
{
   struct    nem_notification_app_hook_info  *app_entry_p;
   struct of_nem_notification_data notification_data={};
   int32_t ret_value;
   uint32_t port_id;
   uint32_t peth_id;
   uint32_t ns_id;
   uint64_t dp_id;
   uint16_t dp_ns_id;
   char namespace[NEM_MAX_NS_NAME_LEN];
   char port_name[NEM_MAX_IFACE_NAME_LEN];
   char peth_name[NEM_MAX_IFACE_NAME_LEN];

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");


   do
   {
      if (dprm_event_data_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "event notification data  is NULL");
         break;
      }
      strcpy(namespace, dprm_event_data_p->namespace_name);
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Namespace  %s",namespace);

      /* * * * * * * * * * * * Generate  NSID * * * * * * * * * * * * * * * * * */
      ret_value=nem_validate_namespace_and_generate_nsid(namespace, &ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Namespace validation failed");
         break;
      }

      /* * * * * * * * * * * * Map Namespace to NSID * * * * * * * * * * * * * * * * * */
      ret_value=nem_ns_and_nsid_mapping_handler(namespace, ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM NS AND NSID Mapping failed");
         break;
      }


      //map_dpid_dpns:
      /* * * * * * * * * * * * Get attached DP information * * * * * * * * * * * * * * * * * */
      ret_value=dprm_get_dpid_from_ns_attributevalue(namespace, &dp_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "No DPID attached to this namespace %s", namespace);
         break;
      }
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "DPID %llx attached to this namespace %s",dp_id, namespace);

      /* * * * * * * * * * * * Map DPID to dpnsid * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_dpnsid_mapping_handler(dp_id, namespace, &dp_ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM DPID and DPNSID Mapping failed");
         break;
      }

      //map_dpid_ns:
      /* * * * * * * * * * * * Map in Database for NSID and DPID * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_nsid_mapping_handler( dp_id, dp_ns_id, ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "dpid and nsid maping   failed");
         break;
      }


      //map_port_peth:
      /* * * * * * * * * * * * Get attached Port information * * * * * * * * * * * * * * * * * */
      ret_value=dprm_get_dpid_and_portname_from_ns_attributevalue(namespace, &dp_id, port_name, &port_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "No Port is  attached to this namespace %s", namespace);
         break;
      }
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "DPID %llx Port %s id %d attached to this namespace %s",dp_id, port_name,port_id, namespace);

      ret_value=nem_validate_dpid_portid_and_create_peth(dp_id, port_id, namespace, ns_id, peth_name, &peth_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "validation and creation of peth failed");
         break;
      }

      /* * * * * * * * * * * * Mapping Peth Database  * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_peth_mapping_handler( dp_id, port_id, ns_id, peth_name, peth_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Map dpid and peth failed");
         break;
      }
      strcpy(  notification_data.port_name, port_name);
      strcpy(  notification_data.peth_name, peth_name);

      //send_notification:
      /* * * * * * * * * * * * Send Notifications to Applications * * * * * * * * * * * * * * * * * */
      notification_data.dp_id=dp_id;
      strcpy( notification_data.namespace_name, namespace);
      notification_data.ns_id=ns_id;

      CNTLR_RCU_READ_LOCK_TAKE();
      OF_LIST_SCAN(nem_notifications[NEM_NS_CREAT_EVENT],app_entry_p,struct nem_notification_app_hook_info*,NEM_NOTIFICATIONS_APP_HOOK_LISTNODE_OFFSET )
      {
         ((of_nem_notifier_fn)(app_entry_p->call_back))(NEM_NS_CREAT_EVENT,
         notification_data,
         app_entry_p->cbk_arg1,
         app_entry_p->cbk_arg2);
      }
      CNTLR_RCU_READ_LOCK_RELEASE();
   }while(0);

   return;
}

void nem_namespace_delete_event_handler(struct   dprm_namespace_notification_data *dprm_event_data_p)
{
   int32_t ret_value;
   struct of_nem_notification_data notification_data={};
   struct  nem_notification_app_hook_info *app_entry_p;
   char namespace[NEM_MAX_NS_NAME_LEN];
   char port_name[NEM_MAX_IFACE_NAME_LEN];
   char peth_name[NEM_MAX_IFACE_NAME_LEN];
   uint32_t ns_id;
   uint32_t port_id;
   uint64_t dp_id;
   uint16_t dp_ns_id;
   uint32_t peth_id;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");

   do
   {
      if (dprm_event_data_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "event notification data  is NULL");
         break;
      }

      strcpy(namespace, dprm_event_data_p->namespace_name);
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Namespace  %s",namespace);

      /* * * * * * * * * * * * Get Namespace ID * * * * * * * * * * * * * * * * * */
      ret_value=of_nem_get_nsid_from_namespace(namespace, &ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "of_nem_get_nsid_from_namespace failed");
         break;
      }
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "ns id for namespace %s is %d", namespace, ns_id);

      /* * * * * * * * * * * * UnMap in Database Namespace and NSID * * * * * * * * * * * * * * * * * */
      ret_value=nem_ns_and_nsid_unmapping_handler(namespace, ns_id);
      if(ret_value != NEM_SUCCESS)

      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM unMapping in database failed");
         break;
      }


      /* * * * * * * * * * * * Get DPID to whom Namespace has been attached * * * * * * * * * * * * * * * * * */
      ret_value=dprm_get_dpid_from_ns_attributevalue(namespace, &dp_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_NOTICE, "No DPID attached to this namespace %s", namespace);
         break;
      }
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "DPID %llx attached to this namespace %s",dp_id, namespace);

      /* * * * * * * * * * * * unmapping dpnsid  * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_dpnsid_unmapping_handler(dp_id, namespace, &dp_ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "unmapping dpnsid failed for dpid %llx namespace %d", dp_id,namespace);
         break;
      }

      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_INFO, "dpid %llx dp_ns_id %d", dp_id,dp_ns_id);

      /* * * * * * * * * * * * unmapping dpid nsid  * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_nsid_unmapping_handler(dp_id, ns_id, dp_ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "unmapping dpid-nsid failed for dpid %llx dp_ns_id %d, ns_id %d", dp_id,dp_ns_id, ns_id);
         break;
      }


      /* * * * * * * * * * * * Get attached Port information * * * * * * * * * * * * * * * * * */
      ret_value=dprm_get_dpid_and_portname_from_ns_attributevalue(namespace, &dp_id, port_name, &port_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "No Port is  attached to this namespace %s", namespace);
         break;
      }
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "DPID %llx Port %s portid %d attached to this namespace %s",dp_id, port_name,port_id, namespace);


      /* * * * * * * * * * * * Unmap dpid, port and peth  * * * * * * * * * * * * * * * * * */
      ret_value=of_nem_get_nsid_and_peth_from_dpid_and_port_id(dp_id, port_id, &ns_id,peth_name, &peth_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "get pethid and peth name failed");
         break;
      }
      ret_value=nem_dpid_peth_unmapping_handler(dp_id, port_id, peth_name);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "unampping  failed for dpid %llx portid %d", dp_id, port_id);
         break;
      }

      /* * * * * * * * * * * * delete peth  * * * * * * * * * * * * * * * * * */
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "peth deletion for dpid %llx portid %d", dp_id, port_id);
      if (!nem_iface_callbacks_p)
      {
	      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "peth not registered");
	      break;
      }		
      ret_value=nem_iface_callbacks_p->iface_delete(namespace,ns_id,peth_id);
      if(ret_value != NEM_SUCCESS)
      {
        OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "peth deletion failed for dpid %llx portid %d , peth_id %d", dp_id, port_id,peth_id);
         break;
      }

      /* * * * * * * * * * * * Send Notifications to Applications * * * * * * * * * * * * * * * * * */
      strcpy(  notification_data.namespace_name, namespace);
      notification_data.ns_id=ns_id;
      notification_data.dp_id=dp_id;

      CNTLR_RCU_READ_LOCK_TAKE();
      OF_LIST_SCAN(nem_notifications[NEM_NS_REMOVE_EVENT],app_entry_p,struct nem_notification_app_hook_info*,NEM_NOTIFICATIONS_APP_HOOK_LISTNODE_OFFSET )
      {
         ((of_nem_notifier_fn)(app_entry_p->call_back))(NEM_NS_REMOVE_EVENT,
         notification_data,
         app_entry_p->cbk_arg1,
         app_entry_p->cbk_arg2);
      }
      CNTLR_RCU_READ_LOCK_RELEASE();

   }while(0);

   return;
}

void nem_namespace_datapath_attach_event_handler(struct   dprm_namespace_notification_data *dprm_event_data_p)
{
   struct    nem_notification_app_hook_info  *app_entry_p;
   struct of_nem_notification_data notification_data={};
   int32_t ret_value;
   char namespace[NEM_MAX_NS_NAME_LEN];
   uint32_t ns_id;
   uint16_t dp_ns_id;
   uint64_t dp_id;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");

   do
   {
      if (dprm_event_data_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "event notification data  is NULL");
         break;
      }
      strcpy(namespace, dprm_event_data_p->namespace_name);
      dp_id= dprm_event_data_p->dp_id;
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "Namespace  %s dpid %llx ",namespace, dp_id);



      /* * * * * * * * * * * * Get Namespace ID * * * * * * * * * * * * * * * * * */
      ret_value=of_nem_get_nsid_from_namespace(namespace, &ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "of_nem_get_nsid_from_namespace failed");
         break;
      }
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "ns id for namespace %s is %d", namespace, ns_id);

      /* * * * * * * * * * * * Map in Database for NSID and DPID * * * * * * * * * * * * * * * * * */
      /* * * * * * * * * * * * Map DPID to dpnsid * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_dpnsid_mapping_handler(dp_id, namespace, &dp_ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM DPID and DPNSID Mapping failed");
         break;
      }

      ret_value=nem_dpid_nsid_mapping_handler(dp_id, dp_ns_id,ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "dpid and nsid mapping  failed");
         break;
      }


      /* * * * * * * * * * * * Send Notifications to Applications * * * * * * * * * * * * * * * * * */
      strcpy(  notification_data.namespace_name, namespace);
      notification_data.ns_id=ns_id;
      notification_data.dp_id=dp_id;

      CNTLR_RCU_READ_LOCK_TAKE();
      OF_LIST_SCAN(nem_notifications[NEM_NS_CREAT_EVENT],app_entry_p,struct nem_notification_app_hook_info*,NEM_NOTIFICATIONS_APP_HOOK_LISTNODE_OFFSET )
      {
         ((of_nem_notifier_fn)(app_entry_p->call_back))(NEM_NS_CREAT_EVENT,
         notification_data,
         app_entry_p->cbk_arg1,
         app_entry_p->cbk_arg2);
      }

      CNTLR_RCU_READ_LOCK_RELEASE();


   }while(0);

   return;
}

void nem_namespace_datapath_detach_event_handler(struct   dprm_namespace_notification_data *dprm_event_data_p)
{
   struct    nem_notification_app_hook_info  *app_entry_p;
   struct of_nem_notification_data notification_data={};
   int32_t ret_value;
   char namespace[NEM_MAX_NS_NAME_LEN];
   uint64_t dp_id;
   uint16_t dp_ns_id;
   uint32_t ns_id;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");

   do
   {
      if (dprm_event_data_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "event notification data  is NULL");
         break;
      }
      strcpy(namespace, dprm_event_data_p->namespace_name);
      dp_id= dprm_event_data_p->dp_id;
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "Namespace  %s dpid %llx ",namespace, dp_id);


      /* * * * * * * * * * * * Get Namespace ID * * * * * * * * * * * * * * * * * */
      ret_value=of_nem_get_nsid_from_namespace(namespace, &ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "of_nem_get_nsid_from_namespace failed");
         break;
      }
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "ns id for namespace %s is %d", namespace, ns_id);

      /* * * * * * * * * * * * unMap DPID and DPNSID  * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_dpnsid_unmapping_handler(dp_id, namespace, &dp_ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM UnMapping in database DPID TO NSID failed");
         break;
      }


      /* * * * * * * * * * * * unMap DPID to NSID  * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_nsid_unmapping_handler(dp_id, ns_id, dp_ns_id );
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM UnMapping in database DPID TO NSID failed");
         break;
      }


      /* * * * * * * * * * * * Send Notifications to Applications * * * * * * * * * * * * * * * * * */
      strcpy(  notification_data.namespace_name, namespace);
      notification_data.ns_id=ns_id;
      notification_data.dp_id=dp_id;

      CNTLR_RCU_READ_LOCK_TAKE();
      OF_LIST_SCAN(nem_notifications[NEM_NS_REMOVE_EVENT],app_entry_p,struct nem_notification_app_hook_info*,NEM_NOTIFICATIONS_APP_HOOK_LISTNODE_OFFSET )
      {
         ((of_nem_notifier_fn)(app_entry_p->call_back))(NEM_NS_CREAT_EVENT,
         notification_data,
         app_entry_p->cbk_arg1,
         app_entry_p->cbk_arg2);
      }
      CNTLR_RCU_READ_LOCK_RELEASE();
   }while(0);

   return;
}

void nem_namespace_port_attach_event_handler(struct   dprm_namespace_notification_data *dprm_event_data_p)
{
   struct    nem_notification_app_hook_info  *app_entry_p;
   struct of_nem_notification_data notification_data={};
   int32_t ret_value;
   char namespace[NEM_MAX_NS_NAME_LEN];
   char port_name[NEM_MAX_IFACE_NAME_LEN];
   char peth_name[NEM_MAX_IFACE_NAME_LEN];
   uint64_t dp_id;
   uint32_t ns_id;
   uint32_t port_id;
   uint32_t peth_id;
   char system_cmd[100]={};
   uint64_t  dp_handle;
   struct   dprm_datapath_entry  *datapath_info=NULL;  

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");

   do
   {
      if (dprm_event_data_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "event notification data  is NULL");
         break;
      }
      strcpy(namespace, dprm_event_data_p->namespace_name);
      strcpy(port_name, dprm_event_data_p->port_name);
      port_id=dprm_event_data_p->port_id;
      dp_id=dprm_event_data_p->dp_id;
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "Namespace  %s portname %s port_id %d dpid %llx ",namespace, port_name,port_id, dp_id);


      /* * * * * * * * * * * * Validate* * * * * * * * * * * * * * * * * */
      /* * * * * * * * * * * * Get Namespace ID * * * * * * * * * * * * * * * * * */
      ret_value=of_nem_get_nsid_from_namespace(namespace, &ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "of_nem_get_nsid_from_namespace failed");
         break;
      }
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "ns id for namespace %s is %d", namespace, ns_id);


      ret_value=nem_validate_dpid_portid_and_create_peth(dp_id, port_id, namespace, ns_id, peth_name, &peth_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "validate port  failed");
         break;
      }

      /* * * * * * * * * * * * Mapping Peth Database  * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_peth_mapping_handler(dp_id, port_id, ns_id, peth_name, peth_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Map dpid and peth failed");
         break;
      }

      /*Assign Mac Address and make status as UP only incase of DP is active*/
      ret_value = dprm_get_datapath_handle(dp_id, &dp_handle);
      if(ret_value == NEM_SUCCESS)
      {
              CNTLR_RCU_READ_LOCK_TAKE();
	      ret_value = get_datapath_byhandle(dp_handle, &datapath_info);
	      if(ret_value == NEM_SUCCESS) 
              {
                if(datapath_info->is_dp_active == TRUE)
	        {
		      sprintf(system_cmd, "ip netns exec %s ifconfig %s hw ether %02x:%02x:%02x:%02x:%02x:%02x", namespace, peth_name, dprm_event_data_p->hw_addr[0], dprm_event_data_p->hw_addr[1],dprm_event_data_p->hw_addr[2], dprm_event_data_p->hw_addr[3], dprm_event_data_p->hw_addr[4], dprm_event_data_p->hw_addr[5]);

		      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG,"executing cmd %s ",system_cmd);

		      ret_value=system(system_cmd);

		      if (ret_value != NEM_SUCCESS)
		      {
			      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG,"cmd %s  failed",system_cmd);
		      }
		      sprintf(system_cmd, "ip netns exec %s ifconfig %s up", namespace, peth_name);
		      ret_value=system(system_cmd);
		      if (ret_value != NEM_SUCCESS)
		      {
			      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG,"cmd %s  failed",system_cmd);
		      }
                }
                else
                {
                   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG,"is_dp_active is false");
                }
	      }
              CNTLR_RCU_READ_LOCK_RELEASE();
      }
      /* * * * * * * * * * * * Send Notifications to Applications * * * * * * * * * * * * * * * * * */
      strcpy(  notification_data.namespace_name, namespace);
      //      strcpy(  notification_data.port_name, port_name);
      strcpy(  notification_data.peth_name, peth_name);
      notification_data.dp_id=dp_id;
      notification_data.ns_id=ns_id;
      notification_data.port_id=port_id;

      CNTLR_RCU_READ_LOCK_TAKE();
      OF_LIST_SCAN(nem_notifications[NEM_PETH_CREAT_EVENT],app_entry_p,struct nem_notification_app_hook_info*,NEM_NOTIFICATIONS_APP_HOOK_LISTNODE_OFFSET )
      {
         ((of_nem_notifier_fn)(app_entry_p->call_back))(NEM_PETH_CREAT_EVENT,
         notification_data,
         app_entry_p->cbk_arg1,
         app_entry_p->cbk_arg2);
      }
      CNTLR_RCU_READ_LOCK_RELEASE();
   }while(0);

   return;
}

void nem_namespace_port_detach_event_handler(struct   dprm_namespace_notification_data *dprm_event_data_p)
{
   struct    nem_notification_app_hook_info  *app_entry_p;
   struct of_nem_notification_data notification_data={};
   int32_t ret_value;
   char namespace[NEM_MAX_NS_NAME_LEN];
   uint64_t dp_id;
   uint32_t ns_id;
   uint32_t port_id;
   uint32_t peth_id;
   char port_name[NEM_MAX_IFACE_NAME_LEN];
   char peth_name[NEM_MAX_IFACE_NAME_LEN];

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");

   do
   {
      if (dprm_event_data_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "event notification data  is NULL");
         break;
      }
      strcpy(namespace, dprm_event_data_p->namespace_name);
      strcpy(port_name, dprm_event_data_p->port_name);
      dp_id=dprm_event_data_p->dp_id;
      port_id=dprm_event_data_p->port_id;
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "Namespace  %s portname %s dpid %llx ",namespace, port_name, dp_id);


      /* * * * * * * * * * * * Get Namespace Info * * * * * * * * * * * * * * * * * */

      /* * * * * * * * * * * * Get Namespace ID * * * * * * * * * * * * * * * * * */
      ret_value=of_nem_get_nsid_from_namespace(namespace, &ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "of_nem_get_nsid_from_namespace failed");
         break;
      }
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "ns id for namespace %s is %d", namespace, ns_id);

      ret_value=of_nem_get_nsid_and_peth_from_dpid_and_port_id(dp_id, port_id, &ns_id,peth_name, &peth_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "get pethid and peth name failed");
         break;
      }

      /* * * * * * * * * * * * get peth  * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_peth_unmapping_handler(dp_id, port_id,peth_name);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "unampping  failed for dpid %llx portname %s", dp_id, port_name);
         break;
      }

      /******Deleting peth interface *******/
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "peth deletion for dpid %llx portid %d", dp_id, port_id);
      if (!nem_iface_callbacks_p)
      {
	      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "peth not registered");
	      break;
      }		
      ret_value=nem_iface_callbacks_p->iface_delete(namespace,ns_id,peth_id);
      if(ret_value != NEM_SUCCESS)
      {
        OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "peth deletion failed for dpid %llx portid %d , peth_id %d", dp_id, port_id,peth_id);
         break;
      }

      /* * * * * * * * * * * * Send Notifications to Applications * * * * * * * * * * * * * * * * * */
      strcpy(  notification_data.namespace_name, namespace);
      //      strcpy(  notification_data.port_name, port_name);
      strcpy(notification_data.peth_name, peth_name);
      notification_data.dp_id=dp_id;
      notification_data.ns_id=ns_id;
      notification_data.port_id=port_id;
      CNTLR_RCU_READ_LOCK_TAKE();
      OF_LIST_SCAN(nem_notifications[NEM_PETH_REMOVE_EVENT],app_entry_p,struct nem_notification_app_hook_info*,NEM_NOTIFICATIONS_APP_HOOK_LISTNODE_OFFSET )
      {
         ((of_nem_notifier_fn)(app_entry_p->call_back))(NEM_PETH_REMOVE_EVENT,
         notification_data,
         app_entry_p->cbk_arg1,
         app_entry_p->cbk_arg2);
      }
      CNTLR_RCU_READ_LOCK_RELEASE();
   }while(0);

   return;
}

int32_t nem_send_dp_and_ns_active_event(uint64_t dp_id, char *domain_name)
{
   struct    nem_notification_app_hook_info  *app_entry_p;
   struct of_nem_notification_data notification_data={};
   int32_t ret_value,status=NEM_SUCCESS;
   struct of_nem_ns_info ns_info;
   struct of_nem_dp_ns_info dp_ns_info;
   uint16_t dp_ns_id;
   uint16_t max_dp_ns_id;
   uint64_t map_entry_handle;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");

   do
   {
      /* * * * * * * * * * * * Get Max DP-NS-ID for DP-ID * * * * * * * * * * * * * * * * * */
      ret_value=nem_get_max_dpnsid_for_dpid(dp_id, &max_dp_ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "get max dpnsid failed for dpid %llx",dp_id);
         status=NEM_SUCCESS;
         break;
      }
      OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, " max dpnsid =%d for dpid %llx",max_dp_ns_id, dp_id);

      notification_data.dp_id=dp_id;
      strncpy(notification_data.domain_name, domain_name, NEM_MAX_IFACE_NAME_LEN);
      CNTLR_RCU_READ_LOCK_TAKE();
      for(dp_ns_id=0; dp_ns_id < max_dp_ns_id; dp_ns_id++)
      {

         ret_value=of_nem_dpid_2_nsid_db_get_map_entry_handle(dp_id, dp_ns_id, &map_entry_handle);
         if ( ret_value != NEM_SUCCESS)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "resource doesnt exists");
            continue;
         }

         ret_value=  of_nem_dpid_2_nsid_db_get_exact_map_entry(map_entry_handle,
               &dp_ns_info);
         if ( ret_value != NEM_SUCCESS)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "resource doesnt exists, invalid handle");
            status = NEM_ERROR_INVALID_HANDLE;
            break;
         }

         ret_value=of_nem_nsid_2_ns_db_get_map_entry_handle(dp_ns_info.ns_id, &map_entry_handle);
         if ( ret_value != NEM_SUCCESS)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "resource doesnt exists");
            status = NEM_ERROR_INVALID_HANDLE;
            break;
         }

         ret_value=  of_nem_nsid_2_ns_db_get_exact_map_entry(map_entry_handle,
               &ns_info);
         if ( ret_value != NEM_SUCCESS)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "resource doesnt exists, invalid handle");
            status = NEM_ERROR_INVALID_HANDLE;
            break;
         }

         strcpy( notification_data.namespace_name, ns_info.name);
         notification_data.ns_id=dp_ns_info.ns_id;
         notification_data.dp_ns_id=dp_ns_info.dp_ns_id;

         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "sending dp and ns active event dp_id=%llx ns=%s nsid %d dpnsid %d",
               notification_data.dp_id,notification_data.namespace_name, notification_data.ns_id, notification_data.dp_ns_id);
         OF_LIST_SCAN(nem_notifications[NEM_DP_AND_NS_ACTIVE_EVENT],app_entry_p,struct nem_notification_app_hook_info*,NEM_NOTIFICATIONS_APP_HOOK_LISTNODE_OFFSET )
         {
            ((of_nem_notifier_fn)(app_entry_p->call_back))(NEM_DP_AND_NS_ACTIVE_EVENT,
            notification_data,
            app_entry_p->cbk_arg1,
            app_entry_p->cbk_arg2);
         }
      }
   CNTLR_RCU_READ_LOCK_RELEASE();
   }while(0);

   return status;
}

int32_t nem_send_dp_and_ns_deactive_event(uint64_t dp_id, char *domain_name)
{
   struct    nem_notification_app_hook_info  *app_entry_p;
   struct of_nem_notification_data notification_data={};
   int32_t ret_value,status=NEM_SUCCESS;
   struct of_nem_ns_info ns_info;
   struct of_nem_dp_ns_info dp_ns_info;
   uint16_t dp_ns_id;
   uint16_t max_dp_ns_id;
   uint64_t map_entry_handle;
   struct dprm_port_info port_info;
   uint64_t datapath_handle;
   uint64_t port_handle;
   char peth_name[NEM_MAX_IFACE_NAME_LEN];
   struct dprm_port_runtime_info runtime_info;
   uint32_t port_id;
   uint32_t peth_id;
   uint32_t nem_ns_id;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");

   do
   {
      /* * * * * * * * * * * * Get Max DP-NS-ID for DP-ID * * * * * * * * * * * * * * * * * */
         /*get datapath handle from datapath_id*/
         ret_value=dprm_get_datapath_handle(dp_id, &datapath_handle);
         if (ret_value != NEM_SUCCESS)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG,"Get DPRM Handle failed for dpid %llx", dp_id);
            status= NEM_FAILURE;
            break;
         }
      ret_value=nem_get_max_dpnsid_for_dpid(dp_id, &max_dp_ns_id);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "get max dpnsid failed for dpid %llx",dp_id);
         status=NEM_SUCCESS;
         break;
      }

      notification_data.dp_id=dp_id;
      strncpy(notification_data.domain_name, domain_name, NEM_MAX_IFACE_NAME_LEN);
      CNTLR_RCU_READ_LOCK_TAKE();
      for(dp_ns_id=0; dp_ns_id < max_dp_ns_id; dp_ns_id++)
      {

         ret_value=of_nem_dpid_2_nsid_db_get_map_entry_handle(dp_id, dp_ns_id, &map_entry_handle);
         if ( ret_value != NEM_SUCCESS)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "resource doesnt exists");
            continue;
         }

         ret_value=  of_nem_dpid_2_nsid_db_get_exact_map_entry(map_entry_handle,
               &dp_ns_info);
         if ( ret_value != NEM_SUCCESS)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "resource doesnt exists, invalid handle");
            status = NEM_ERROR_INVALID_HANDLE;
            break;
         }

         ret_value=of_nem_nsid_2_ns_db_get_map_entry_handle(dp_ns_info.ns_id, &map_entry_handle);
         if ( ret_value != NEM_SUCCESS)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "resource doesnt exists");
            status = NEM_ERROR_INVALID_HANDLE;
            break;
         }

         ret_value=  of_nem_nsid_2_ns_db_get_exact_map_entry(map_entry_handle,
               &ns_info);
         if ( ret_value != NEM_SUCCESS)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "resource doesnt exists, invalid handle");
            status = NEM_ERROR_INVALID_HANDLE;
            break;
         }
         /*get first port*/
         ret_value=dprm_get_first_datapath_port(datapath_handle, &port_info, &runtime_info, &port_handle);
	 if (ret_value != NEM_SUCCESS)
	 {
		 OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG,"Get frst DPRM port failed");
		 if(ret_value != DPRM_ERROR_NO_MORE_ENTRIES)
		 {      
			 status= NEM_FAILURE;
			 break;
		 }
	 }

         while (ret_value == NEM_SUCCESS)
         {
            port_id=port_info.port_id;

            ret_value=of_nem_get_nsid_and_peth_from_dpid_and_port_id(dp_id, port_id, &nem_ns_id, peth_name, &peth_id);
            if ( ret_value!= NEM_SUCCESS)
            {
               OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG,"peth might not be created for port_id %d", port_id);
               status= NEM_FAILURE;
               break;
            }
            else
            {
               OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "unampping  for dpid %llx portid %d", dp_id, port_id);
               ret_value=nem_dpid_peth_unmapping_handler(dp_id, port_id, peth_name);
               if(ret_value != NEM_SUCCESS)
               {
                  OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "unampping  failed for dpid %llx portid %d", dp_id, port_id);
                  status=NEM_FAILURE;
                  break;
               }

               /******Deleting peth*******/
               OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "peth deletion for dpid %llx portid %d , peth_id %d", dp_id, port_id,peth_id);
	       if (!nem_iface_callbacks_p)
	       {
		       OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "peth not registered");
		       status=NEM_FAILURE;
		       break;
	       }		
	       ret_value=nem_iface_callbacks_p->iface_delete(ns_info.name,dp_ns_info.ns_id,peth_id);
        //       ret_value=peth_delete_net_dev_iface (ns_info.name,dp_ns_info.ns_id,peth_id);
               if(ret_value != NEM_SUCCESS)
               {
                  OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "peth deletion failed for dpid %llx portid %d , peth_id %d", dp_id, port_id,peth_id);
                               status=NEM_FAILURE;
                                 break;
               }

               ret_value=dprm_get_next_datapath_port( datapath_handle, &port_info, &runtime_info, &port_handle); 
            }
         }
         strcpy( notification_data.namespace_name, ns_info.name);
         notification_data.ns_id=dp_ns_info.ns_id;
         notification_data.dp_ns_id=dp_ns_info.dp_ns_id;

         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "sending dp and ns deactive event dp_id=%llx ns=%s nsid %d dpnsid %d",
               notification_data.dp_id,notification_data.namespace_name, notification_data.ns_id, notification_data.dp_ns_id);
         OF_LIST_SCAN(nem_notifications[NEM_DP_AND_NS_DEACTIVE_EVENT],app_entry_p,struct nem_notification_app_hook_info*,NEM_NOTIFICATIONS_APP_HOOK_LISTNODE_OFFSET )
         {
            ((of_nem_notifier_fn)(app_entry_p->call_back))(NEM_DP_AND_NS_DEACTIVE_EVENT,
            notification_data,
            app_entry_p->cbk_arg1,
            app_entry_p->cbk_arg2);
         }
      }
      CNTLR_RCU_READ_LOCK_RELEASE();
   }while(0);

   return status;
}

int32_t  nem_root_ns_init()
{
int32_t status=OF_SUCCESS;

	OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "Entered");

	nem_root_ns_fd_g = open ("/proc/1/ns/net", O_RDWR|O_CREAT, S_IRWXU);
	if (nem_root_ns_fd_g < 0) 
	{
		OF_LOG_MSG (OF_NEM_LOG, OF_LOG_ERROR,
				"Failed to open /proc/1/ns/net fd");
		status=OF_FAILURE;
	}
	return status;
}

/******************************************************************************
 * * * * * * * * * * * * NEM Event DeInit* * * * * * * * * * * * * * * * * * * *
 *******************************************************************************/

int32_t nem_event_deinit()
{
   int32_t status=NEM_SUCCESS;
   int32_t ret_value;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   {
      /* * * * * * * * * * * * Event Registration * * * * * * * * * * * * * * * * * * */
      ret_value=nem_event_deregister();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Event De-Registration failed");
         status=NEM_FAILURE;
         //   break;
      }

      /* * * * * * * * * * * * Mempool De-Initialization * * * * * * * * * * * * * * * * * * */
      ret_value=nem_event_mempool_deinit();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Event Mempool De-initialization failed");
         status=NEM_FAILURE;
         // break;
      }

      /* * * * * * * * * * * *Root Ns Fd Close * * * * * * * * * * * * * * * * * * */
      ret_value=nem_root_ns_deinit();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Root Namespace De-Initialization  failed");
         status=NEM_FAILURE;
         break;
      }
   }while(0);

   return status;
}


int32_t nem_event_deregister()
{
   int32_t ret_value,status=NEM_SUCCESS;
   int8_t i;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");

   do
   {
      for (i =0; i < domain_count_g; i++)
      {
         uint8_t priority=15;
         ret_value=cntlr_deregister_asynchronous_event_hook(domain_handles_g[i],
               DP_READY_EVENT,
               priority
               );
         if(ret_value != OF_SUCCESS)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR,"De-Register DP Ready event cbk failed");
            status= NEM_FAILURE;
            //break;
         }

         ret_value=cntlr_deregister_asynchronous_event_hook(domain_handles_g[i],
               DP_DETACH_EVENT,
               priority
               );
         if(ret_value != OF_SUCCESS)
         {
            OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR,"De-Register DP Detach event cbk failed");
            status= NEM_FAILURE;
            //break;
         }
      }
      ret_value = dprm_deregister_namespace_notifications (DPRM_NAMESPACE_ALL_NOTIFICATIONS,
            nem_namespace_notification_handler
            );
      if(ret_value != DPRM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "De-registration for namespace notification failed");
         status= NEM_FAILURE;
         //break;
      }

      ret_value = dprm_deregister_domain_notifications (DPRM_DOMAIN_ADDED,
            nem_dprm_domain_notifier_handler
            );
      if(ret_value != DPRM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "De-registration for namespace notification failed");
         status= NEM_FAILURE;
         //break;
      }

   }while(0);

   return status;
}
int32_t nem_event_mempool_deinit()
{
   int32_t ret_value;
   int32_t status=NEM_SUCCESS;
   struct  nem_notification_app_hook_info *app_entry_p=NULL,*prev_app_entry_p=NULL;
   uint16_t offset=NEM_NOTIFICATIONS_APP_HOOK_LISTNODE_OFFSET;
   uint32_t notification_type;
   /** Delete hash table to contain MFC rules */
   do
   {

     CNTLR_RCU_READ_LOCK_TAKE();
     for (notification_type = NEM_FIRST_EVENT; notification_type < NEM_MAX_EVENT; notification_type++)
     {
       app_entry_p= (struct nem_notification_app_hook_info *)(((unsigned char *)OF_LIST_FIRST(nem_notifications[notification_type])) - offset);
       if (app_entry_p)
       {
         OF_REMOVE_NODE_FROM_LIST(nem_notifications[notification_type],app_entry_p,prev_app_entry_p,offset );
       }
     }
     CNTLR_RCU_READ_LOCK_RELEASE();
      ret_value=  mempool_delete_pool(nem_notifications_mempool_g);
      if( CNTLR_UNLIKELY(ret_value != NEM_SUCCESS) )
      {
         OF_LOG_MSG(OF_LOG_ARP_APP, OF_LOG_ERROR, "mchash_table_delete failed");
         status= OF_FAILURE;
         break;
      }
      nem_notifications_mempool_g=NULL;
   }while(0);

   return status;
}

int32_t  nem_root_ns_deinit()
{
   int32_t status=NEM_SUCCESS;
   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   close(nem_root_ns_fd_g);
   return status;
}
#endif /* CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT*/
