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
 * File name:limcbk.c
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

int32_t lim_stats_cbk_init();

#ifdef CNTRL_INFRA_LIB_SUPPORT
void cntlr_shared_lib_init()
{
   int32_t retval=OF_SUCCESS;

   CM_CBK_DEBUG_PRINT("Loading  Lim stats");

   do
   {
      /** Initialize LIM Stats */
      retval=lim_stats_cbk_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "LIM Stats initialization failed");
         break;
      }

   }while(0);

   return;
}
#endif

int32_t lim_stats_cbk_init()
{

   int32_t status=OF_SUCCESS;
   int32_t retval;


   CM_CBK_DEBUG_PRINT( "entered");

   do
   {

      retval=route_stats_appl_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "LIM Stats CBK  initialization failed");
         status=OF_FAILURE;
         break;
      }
      retval=arp_stats_appl_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "LIM Stats cbk initialization failed");
         status=OF_FAILURE;
         break;
      }
      retval=fwconn_stats_appl_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "LIM Stats cbk initialization failed");
         status=OF_FAILURE;
         break;
      }
      retval=fw_fwdstats_appl_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "LIM Stats CBK initialization failed");
         status=OF_FAILURE;
         break;
      }

   } while(0);

   return status;
}

#ifdef CNTRL_INFRA_LIB_SUPPORT
void cntlr_shared_lib_deinit()
{

   CM_CBK_DEBUG_PRINT("UnLoading  LIM stats cbk");

   do
   {
      /** De-Initialize LIM Infrastructure */
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
