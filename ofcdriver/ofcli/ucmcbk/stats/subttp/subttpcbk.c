/* 
 *
 * Copyright  2012, 2013  Freescale Semiconductor
 *
 *
 * Licensed under the Apache License, version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

/*
 *
 * File name:subttpcbk.c
 * Author:
 * Date: 
 * Description: 
 * 
 */

#ifdef OF_CM_SUPPORT

#include"openflow.h"
#include"cbcmninc.h"
#include "cntrucminc.h"
#include "cntrlappcmn.h"
#include "cmgutil.h"

int32_t subttp_stats_cbk_init();

#ifdef CNTRL_INFRA_LIB_SUPPORT
void cntlr_shared_lib_init()
{
   int32_t retval=OF_SUCCESS;

   CM_CBK_DEBUG_PRINT("Loading  Subttp stats");

   do
   {
      /** Initialize Subttp Stats */
      retval=subttp_stats_cbk_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "Subttp Stats initialization failed");
         break;
      }

   }while(0);

   return;
}
#endif

int32_t subttp_stats_cbk_init()
{

   int32_t status=OF_SUCCESS;
   int32_t retval;


   CM_CBK_DEBUG_PRINT( "entered");

   do
   {

      retval=fwconn_ttp_stats_appl_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "Subttp Stats cbk initialization failed");
         status=OF_FAILURE;
         break;
      }
      retval=arpsubttp_stats_appl_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "Subttp Stats cbk initialization failed");
         status=OF_FAILURE;
         break;
      }
      retval=route_ttp_stats_appl_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "Subttp Stats CBK  initialization failed");
         status=OF_FAILURE;
         break;
      }
      retval=fwfwd_ttp_stats_appl_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "Subttp Stats CBK initialization failed");
         status=OF_FAILURE;
         break;
      }

   } while(0);
   CM_CBK_DEBUG_PRINT( "subttp stats success");

   return status;
}

#ifdef CNTRL_INFRA_LIB_SUPPORT
void cntlr_shared_lib_deinit()
{

   CM_CBK_DEBUG_PRINT("UnLoading  Subttp stats cbk");

   do
   {
      /** De-Initialize Subttp Infrastructure */
      /* we are not initializing any memory/pools to free */
#if 0
      retval=of_nem_cbk_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "Network Element Mapper CBK initialization failed");
         status= OF_FAILURE;
         break;
      }
#endif
   }while(0);

   return;
}
#endif
#endif /* OF_CM_SUPPORT */
