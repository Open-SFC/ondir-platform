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

/* File  :      nemdb.c
 * Author:      Atmaram G <atmaram.g@freescale.com>
 * Date:   08/23/2013
 * Description: A network map_entry is logical mapping between network elements. Network Element Mapper module maintains mappings between different entities.This file provides initialization different Mapping databases.
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




int32_t nem_db_init()
{
   int32_t status=NEM_SUCCESS;
   int32_t ret_value;


   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 
   do
   {

      /* * * * * * * * * * * * Namespace to NSID * * * * * * * * * * * * * * * * * * */
      ret_value=nem_ns_2_nsid_db_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Namespace 2 Nsid DB initialization failed");
         status=NEM_FAILURE;
         break;
      }

      /* * * * * * * * * * * * NSID to Namespace * * * * * * * * * * * * * * * * * * */
      ret_value=nem_nsid_2_ns_db_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM NSID 2 Namespace DB initialization failed");
         status=NEM_FAILURE;
         break;
      }

      /* * * * * * * * * * * * DPID to NSID * * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_2_nsid_db_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM DPID 2 Nsid DB initialization failed");
         status=NEM_FAILURE;
         break;
      }

      /* * * * * * * * * * * * NSID to DPID * * * * * * * * * * * * * * * * * * */
      ret_value=nem_nsid_2_dpid_db_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM NSID 2 DPID  DB initialization failed");
         status=NEM_FAILURE;
         break;
      }

      /* * * * * * * * * * * * DPID-PORTName to NSID-Peth * * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_2_peth_db_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM DPID 2 Peth DB initialization failed");
         status=NEM_FAILURE;
         break;
      }

      /* * * * * * * * * * * * NSID-Peth to DPID-PORTName  * * * * * * * * * * * * * * * * * * */
      ret_value=nem_peth_2_dpid_db_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Peth 2 DPID DB initialization failed");
         status=NEM_FAILURE;
         break;
      }

      /* * * * * * * * * * * * DPID and DPNSID list * * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_and_dpnsid_db_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM DPID 2 DPNSID initialization failed");
         status=NEM_FAILURE;
         break;
      }
      nem_dpid_and_dpnsid_list_init();

   }while(0);

   return status;

}

int32_t nem_db_deinit()
{
   int32_t status=NEM_SUCCESS;
   int32_t ret_value;


   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 
   do
   {

      /* * * * * * * * * * * * Namespace to NSID * * * * * * * * * * * * * * * * * * */
      ret_value=nem_ns_2_nsid_db_deinit();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Namespace 2 Nsid DB deinitialization failed");
         status=NEM_FAILURE;
        // break;
      }

      /* * * * * * * * * * * * NSID to Namespace * * * * * * * * * * * * * * * * * * */
      ret_value=nem_nsid_2_ns_db_deinit();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM NSID 2 Namespace DB deinitialization failed");
         status=NEM_FAILURE;
         //break;
      }

      /* * * * * * * * * * * * DPID to NSID * * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_2_nsid_db_deinit();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM DPID 2 Nsid DB deinitialization failed");
         status=NEM_FAILURE;
         //break;
      }

      /* * * * * * * * * * * * NSID to DPID * * * * * * * * * * * * * * * * * * */
      ret_value=nem_nsid_2_dpid_db_deinit();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM NSID 2 DPID  DB deinitialization failed");
         status=NEM_FAILURE;
         //break;
      }

      /* * * * * * * * * * * * DPID-PORTName to NSID-Peth * * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_2_peth_db_deinit();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM DPID 2 Peth DB deinitialization failed");
         status=NEM_FAILURE;
        // break;
      }

      /* * * * * * * * * * * * NSID-Peth to DPID-PORTName  * * * * * * * * * * * * * * * * * * */
      ret_value=nem_peth_2_dpid_db_deinit();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Peth 2 DPID DB deinitialization failed");
         status=NEM_FAILURE;
         //break;
      }

      /* * * * * * * * * * * * DPID and DPNSID list * * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_and_dpnsid_db_deinit();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM DPID 2 DPNSID deinitialization failed");
         status=NEM_FAILURE;
         //break;
      }

   }while(0);

   return status;

}

/* * * * * * * * * * * * NSID and Namespace * * * * * * * * * * * * * * * * * * */

void nem_ns_and_nsid_db_get_map_entry_mempoolentries(uint32_t* db_entries_max,uint32_t* db_static_entries)
{
   *db_entries_max    = OF_NEM_MAX_NAMESPACE_ENTRIES;
   *db_static_entries = OF_NEM_MAX_NAMESPACE_STATIC_ENTRIES;
}

/* * * * * * * * * * * * DPID and Namespace * * * * * * * * * * * * * * * * * * */
void nem_dpid_and_nsid_db_get_map_entry_mempoolentries(uint32_t* db_entries_max,uint32_t* db_static_entries)
{
   *db_entries_max    = OF_NEM_MAX_NAMESPACE_ENTRIES;
   *db_static_entries = OF_NEM_MAX_NAMESPACE_STATIC_ENTRIES;
}

/* * * * * * * * * * * * DPID-PortName and NSID-Peth * * * * * * * * * * * * * * * * * * */
void nem_dpid_and_peth_db_get_map_entry_mempoolentries(uint32_t* db_entries_max,uint32_t* db_static_entries)
{
   *db_entries_max    = OF_NEM_MAX_NAMESPACE_ENTRIES;
   *db_static_entries = OF_NEM_MAX_NAMESPACE_STATIC_ENTRIES;
}

/*******************************************************************************
 * * * * * * * * * * * * DB MAPPING FUNCTIONS * * * * * * * * * * * * * * * * * * *
 ********************************************************************************/

int32_t nem_ns_and_nsid_mapping_handler(char *namespace_p, uint32_t ns_id)
{

   struct of_nem_ns_info ns_info={};
   int32_t status=NEM_SUCCESS;
   int32_t  ret_value;
   uint64_t nem_db_entry_handle;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   {

      if (namespace_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Namespace is NULL");
         status=NEM_FAILURE;
         break;
      }


      ns_info.id=ns_id;
      strcpy( ns_info.name, namespace_p);

      /* * * * * * * * * * * * Map Namespace to NSID * * * * * * * * * * * * * * * * * */
      ret_value=nem_map_ns_to_nsid(&ns_info, &nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Mapping in database NS TO NSID failed");
         status=NEM_FAILURE;
         break;
      }

      /* * * * * * * * * * * * Map NSID to Namespace * * * * * * * * * * * * * * * * * */
      ret_value=nem_map_nsid_to_ns(&ns_info, &nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Mapping in database NSID TO NS failed");
         status=NEM_FAILURE;
         break;
      }

   }while(0);

   return status;
}

int32_t nem_dpid_dpnsid_mapping_handler(uint64_t dp_id, char *namespace_p,  uint16_t *dp_ns_id_p)
{

   struct of_nem_dpid_dpns_info dp_dpns_info={};
   int32_t status=NEM_SUCCESS;
   int32_t  ret_value;
   uint64_t nem_db_entry_handle;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   {

      if (namespace_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Namespace is NULL");
         status=NEM_FAILURE;
         break;
      }

      ret_value=nem_validate_namespace_and_generate_dpnsid_for_dpid(dp_id, namespace_p, dp_ns_id_p);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "generation of dpnsid failed");
         status=NEM_FAILURE;
         break;
      }

      /* * * * * * * * * * * * Map Namespace to NSID * * * * * * * * * * * * * * * * * */
      dp_dpns_info.dp_id=dp_id;
      dp_dpns_info.dp_ns_id=*dp_ns_id_p;
      strcpy(dp_dpns_info.namespace_name, namespace_p);
      ret_value=nem_map_dpid_to_dpnsid(&dp_dpns_info, &nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Mapping in database DP TO dpNSID failed");
         status=NEM_FAILURE;
         break;
      }


   }while(0);

   return status;
}

int32_t nem_dpid_nsid_mapping_handler(uint64_t dp_id, uint16_t dp_ns_id, uint32_t ns_id)
{

   struct of_nem_dp_ns_info dp_ns_info={};
   int32_t status=NEM_SUCCESS;
   int32_t  ret_value;
   uint64_t nem_db_entry_handle;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   {
      dp_ns_info.dp_id=dp_id;
      dp_ns_info.ns_id=ns_id;
      dp_ns_info.dp_ns_id=dp_ns_id;

      /* * * * * * * * * * * * Map DPID to NamespaceID * * * * * * * * * * * * * * * * * */
      ret_value=nem_map_dpid_to_nsid(&dp_ns_info, &nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Mapping in database NS TO NSID failed");
         status=NEM_FAILURE;
         break;
      }

      /* * * * * * * * * * * * Map NSID to DPID * * * * * * * * * * * * * * * * * */
      ret_value=nem_map_nsid_to_dpid(&dp_ns_info, &nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Mapping in database NSID TO NS failed");
         status=NEM_FAILURE;
         break;
      }

   }while(0);

   return status;
}


int32_t nem_dpid_peth_mapping_handler(uint64_t dp_id, uint32_t port_id, uint32_t ns_id, char *peth_name_p, uint32_t peth_id)
{

   struct of_nem_dp_peth_info dp_peth_info={};
   int32_t status=NEM_SUCCESS;
   int32_t  ret_value;
   uint64_t nem_db_entry_handle;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   {
#if 0
      if(port_name_p == NULL )
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "port name is null");
         status=NEM_FAILURE;
         break;
      }
#endif
      if(peth_name_p == NULL )
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "peth name is null");
         status=NEM_FAILURE;
         break;
      }
      dp_peth_info.dp_id=dp_id;
      dp_peth_info.ns_id=ns_id;
      dp_peth_info.port_id=port_id;
      dp_peth_info.peth_id=peth_id;
//      strcpy(dp_peth_info.port_name, port_name_p);
      strcpy(dp_peth_info.peth_name, peth_name_p);


      /* * * * * * * * * * * * Map DPID To peth   * * * * * * * * * * * * * * * * * */
      ret_value=nem_map_dpid_to_peth(&dp_peth_info, &nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Mapping in database DPID to peth");
         status=NEM_FAILURE;
         break;
      }
  
     /* * * * * * * * * * * * Map NSID to peth   * * * * * * * * * * * * * * * * * */
      ret_value=nem_map_peth_to_dpid(&dp_peth_info, &nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM Mapping in database peth to dpid");
         status=NEM_FAILURE;
         break;
      }

   }while(0);

   return status;
}


/*******************************************************************************
 * * * * * * * * * * * * DB UN-MAPPING FUNCTIONS * * * * * * * * * * * * * * * * *
 ********************************************************************************/

int32_t nem_ns_and_nsid_unmapping_handler(char *namespace_p, uint32_t ns_id)
{

   struct of_nem_ns_info ns_info={};
   int32_t status=NEM_SUCCESS;
   int32_t  ret_value;
   uint64_t nem_db_entry_handle;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   {
      if (namespace_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Namespace is NULL");
         status=NEM_FAILURE;
         break;
      }

      ns_info.id=ns_id;
      strcpy( ns_info.name, namespace_p);

      /* * * * * * * * * * * * UnMap Namespace to NSID * * * * * * * * * * * * * * * * * */
      ret_value= of_nem_ns_2_nsid_db_get_map_entry_handle(namespace_p, &nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM  NSID to Namespace get map entry failed");
         status=NEM_FAILURE;
         break;
      }

      ret_value=nem_unmap_ns_to_nsid(nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM unMapping in database failed");
         status=NEM_FAILURE;
         break;
      }

      /* * * * * * * * * * * * UnMap NSID to Namespace * * * * * * * * * * * * * * * * * */
      ret_value=of_nem_nsid_2_ns_db_get_map_entry_handle(ns_info.id, &nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM  NSID to Namespace get map entry failed");
         status=NEM_FAILURE;
         break;
      }

      ret_value=
         of_nem_nsid_2_ns_db_get_exact_map_entry( nem_db_entry_handle,
               &ns_info);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM unMapping in database failed");
         status=NEM_FAILURE;
         break;
      }

      ret_value=nem_unmap_nsid_to_ns(nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM unMapping in database failed");
         status=NEM_FAILURE;
         break;
      }


   }while(0);

   return status;
}

int32_t nem_dpid_dpnsid_unmapping_handler(uint64_t dp_id, char *namespace_p, uint16_t *dp_ns_id_p)
{

   struct of_nem_dpid_dpns_info dp_dpns_info={};
   int32_t status=NEM_SUCCESS;
   int32_t  ret_value;
   uint64_t nem_db_entry_handle;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   {

      if (namespace_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Namespace is NULL");
         status=NEM_FAILURE;
         break;
      }


      /* * * * * * * * * * * * get p DPID to DPNSID from db * * * * * * * * * * * * * * * * * */
      ret_value=nem_dpid_and_dpnsid_db_get_map_entry_handle(dp_id, namespace_p,  &nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM  DPID to dpnsid get map entry failed");
         status=NEM_FAILURE;
         break;
      }

      ret_value=
         nem_dpid_and_dpnsid_db_get_exact_map_entry( nem_db_entry_handle,
               &dp_dpns_info);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM getexaqct failed");
         status=NEM_FAILURE;
         break;
      }

      /* * * * * * * * * * * * UnMap DPID to DPNSID  * * * * * * * * * * * * * * * * * */
      ret_value=nem_unmap_dpid_to_dpnsid(nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM unMapping in database failed");
         status=NEM_FAILURE;
         break;
      }

      *dp_ns_id_p=dp_dpns_info.dp_ns_id;

   }while(0);

   return status;
}

int32_t nem_dpid_nsid_unmapping_handler(uint64_t dp_id, uint32_t ns_id, uint16_t dp_ns_id)
{

   struct of_nem_dp_ns_info dp_ns_info={};
   int32_t status=NEM_SUCCESS;
   int32_t  ret_value;
   uint64_t nem_db_entry_handle;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   {

      /* * * * * * * * * * * * get p DPID to DPNSID from db * * * * * * * * * * * * * * * * * */
      ret_value=of_nem_dpid_2_nsid_db_get_map_entry_handle(dp_id, dp_ns_id,  &nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM  DPID to dpnsid get map entry failed");
         status=NEM_FAILURE;
         break;
      }

      ret_value=
         of_nem_dpid_2_nsid_db_get_exact_map_entry( nem_db_entry_handle,
               &dp_ns_info);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM getexaqct failed");
         status=NEM_FAILURE;
         break;
      }

      /* * * * * * * * * * * * UnMap DPID to NSID  * * * * * * * * * * * * * * * * * */
      ret_value=nem_unmap_dpid_to_nsid(nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM unMapping in database failed");
         status=NEM_FAILURE;
         break;
      }

      /* * * * * * * * * * * * UnMap NSID to DPID  * * * * * * * * * * * * * * * * * */
      ret_value=of_nem_nsid_2_dpid_db_get_map_entry_handle(ns_id,  &nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM  DPID to dpnsid get map entry failed");
         status=NEM_FAILURE;
         break;
      }
      ret_value=nem_unmap_nsid_to_dpid(nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM unMapping in database failed");
         status=NEM_FAILURE;
         break;
      }

   }while(0);

   return status;
}

int32_t nem_dpid_peth_unmapping_handler(uint64_t dp_id, uint32_t port_id, char *peth_name_p)
{

   struct of_nem_dp_peth_info dp_peth_info={};
   int32_t status=NEM_SUCCESS;
   int32_t  ret_value;
   uint64_t nem_db_entry_handle;

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered");
   do
   {
#if 0
      if (port_name_p == NULL)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "Namespace is NULL");
         status=NEM_FAILURE;
         break;
      }
#endif
      /* * * * * * * * * * * * get p DPID to DPNSID from db * * * * * * * * * * * * * * * * * */
      ret_value=of_nem_dpid_2_peth_db_get_map_entry_handle(dp_id, port_id,  &nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM  DPID to peth get map entry failed");
         status=NEM_FAILURE;
         break;
      }

      ret_value=
         of_nem_dpid_2_peth_db_get_exact_map_entry( nem_db_entry_handle,
               &dp_peth_info);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM getexaqct failed");
         status=NEM_FAILURE;
         break;
      }
      strcpy(peth_name_p, dp_peth_info.peth_name);

      /* * * * * * * * * * * * UnMap DPID to peth  * * * * * * * * * * * * * * * * * */
      ret_value=nem_unmap_dpid_to_peth(nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM unMapping in database failed");
         status=NEM_FAILURE;
         break;
       }
      /* * * * * * * * * * * * UnMap NSID to DPID  * * * * * * * * * * * * * * * * * */
      ret_value=of_nem_peth_2_dpid_db_get_map_entry_handle(dp_peth_info.ns_id, dp_peth_info.peth_id,  &nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM  peth 2 DPID  get map entry failed");
         status=NEM_FAILURE;
         break;
      }
/*      ret_value=nem_unmap_nsid_to_dpid(nem_db_entry_handle);
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM unMapping in database failed");
         status=NEM_FAILURE;
         break;
      }*/

   }while(0);

   return status;
}
#endif /* CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT*/
