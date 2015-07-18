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

/* File  :      dprmns.c
 * Author:      Atmaram G <atmaram.g@freescale.com>
 * Description: A network map_entry is logically another copy of the network stack, with its own routes, firewall rules, and network devices.This file provides API to create map_entry in linux. It also provides APIs to list/delete map_entrys. NEM maintains  separate hash list and mempool list for map_entry maintanance.
 */
#ifdef CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT
#include "controller.h"
#include "dprm.h"
#include "cntlr_epoll.h"
#include "cntrl_queue.h"
//#include "of_utilities.h"
#include "of_msgapi.h"
#include "of_multipart.h"
#include "cntlr_tls.h"
#include "cntlr_transprt.h"
#include "cntlr_event.h"
#include "dprmldef.h"
#include "of_nem.h"
#include "nemldef.h"

#ifdef CNTRL_INFRA_LIB_SUPPORT
void cntlr_shared_lib_init()
{
   int32_t ret_val=OF_SUCCESS; 

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG,"Loading  Network Element Mapper");

   do
   {
      /** Initialize NEM Infrastructure */
      ret_val=of_nem_init();
      if(ret_val != OF_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "Network Element Mapper initialization failed");
         break;
      }

   }while(0);

   return;
}
#endif

int32_t of_nem_init()
{

   int32_t status=NEM_SUCCESS;
   int32_t ret_value;


   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   do
   {
      ret_value= nem_db_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM DB initialization failed");
         status=NEM_FAILURE;
         break;
      }

      ret_value=nem_event_init();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM notifications initialization failed");
         status=NEM_FAILURE;
         break;
      }

   }while(0);

   return status;

}



#ifdef CNTRL_INFRA_LIB_SUPPORT
void cntlr_shared_lib_deinit()
{
   int32_t ret_val=OF_SUCCESS; 

   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG,"Unloading  Network Element Mapper");

   do
   {
      /** De-Initialize NEM Infrastructure */
      ret_val=of_nem_deinit();
      if(ret_val != OF_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "Network Element Mapper initialization failed");
         break;
      }

   }while(0);

   return;
}
#endif

int32_t of_nem_deinit()
{

   int32_t status=NEM_SUCCESS;
   int32_t ret_value;


   OF_LOG_MSG(OF_NEM_LOG, OF_LOG_DEBUG, "entered"); 

   do
   {
      ret_value=nem_event_deinit();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM notifications De-initialization failed");
         status=NEM_FAILURE;
         //break;
      }

      ret_value= nem_db_deinit();
      if(ret_value != NEM_SUCCESS)
      {
         OF_LOG_MSG(OF_NEM_LOG, OF_LOG_ERROR, "NEM DB De-initialization failed");
         status=NEM_FAILURE;
         //break;
      }


   }while(0);

   return status;

}

#endif /* CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT*/
