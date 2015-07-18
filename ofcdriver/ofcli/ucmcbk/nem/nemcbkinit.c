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
 * File name: ns2nsidcbk.c
 * Author: G Atmaram <B38856@freescale.com>
 * Date:   08/23/2013
 * Description: 
 * 
 */
/* File  :      ns2nsidcbk.c
 * Author:      Atmaram G <atmaram.g@freescale.com>
 * Date:   08/23/2013
 * Description: A network map_entry is logical mapping between network elements. Network Element Mapper module maintains mappings between different entities.This file provides UCM support for Namespace to Namespace ID mapping APIs.
 */

#ifdef OF_CM_SUPPORT

#ifdef CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT

#include"openflow.h"
#include"cbcmninc.h"
#include "cntrucminc.h"
#include "cntrlappcmn.h"
#include "cmgutil.h"
#include"of_multipart.h"
#include"cmreg.h"

int32_t of_nem_cbk_init();

#ifdef CNTRL_INFRA_LIB_SUPPORT
void cntlr_shared_lib_init()
{
   int32_t retval=OF_SUCCESS;

   CM_CBK_DEBUG_PRINT("Loading  Network Element Mapper");

   do
   {
      /** Initialize NEM Infrastructure */
      retval=of_nem_cbk_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "Network Element Mapper CBK initialization failed");
         break;
      }

   }while(0);

   return;
}
#endif

int32_t of_nem_cbk_init()
{

   int32_t status=OF_SUCCESS;
   int32_t retval;


   CM_CBK_DEBUG_PRINT( "entered");

   do
   {

      retval=of_nem_ns2nsid_appl_ucmcbk_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "NEM NS2NSID CBK  initialization failed");
         status=OF_FAILURE;
         break;
      }
      retval=of_nem_nsid2ns_appl_ucmcbk_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "NEM NSID2NS cbk initialization failed");
         status=OF_FAILURE;
         break;
      }
      retval=of_nem_dpid2nsid_appl_ucmcbk_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "NEM DPID2NSID cbk initialization failed");
         status=OF_FAILURE;
         break;
      }
      retval=of_nem_nsid2dpid_appl_ucmcbk_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "NEM NSID2DPID CBK initialization failed");
         status=OF_FAILURE;
         break;
      }
      retval=of_nem_dpid2peth_appl_ucmcbk_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "NEM dpid2peth cbk initialization failed");
         status=OF_FAILURE;
         break;
      }
      retval=of_nem_peth2dpid_appl_ucmcbk_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "NEM peth2dpid cbk initialization failed");
         status=OF_FAILURE;
         break;
      }

   }while(0);

   return status;

}

#ifdef CNTRL_INFRA_LIB_SUPPORT
void cntlr_shared_lib_deinit()
{

   CM_CBK_DEBUG_PRINT("UnLoading  Network Element Mapper cbk");

   do
   {
      /** De-Initialize NEM Infrastructure */
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
#endif /* CNTRL_NETWORK_ELEMENT_MAPPER_SUPPORT */
#endif /* OF_CM_SUPPORT */
