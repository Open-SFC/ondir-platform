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
 * File name: fwnat_cbkinit.c
 * Author: Jothirlatha J <B37228@freescale.com>
 * Date:   10/13/2014
 * Description: 
 * 
 */

#include "cbcmninc.h"
#include "cmgutil.h"
#include "cntrucminc.h"
//#include "common_nfapi.h"
#include "fw_nfapi.h"
#include "fw4_nfapi.h"
//#ifdef OF_CM_SUPPORT
//#ifdef CNTRL_FWNAT_SUPPORT

void cntlr_shared_lib_init()
{
   int32_t retval=OF_SUCCESS;

   CM_CBK_DEBUG_PRINT("Loading Firewall Nat ");

   do
   {
      /** Initialize FWNAT Infrastructure */
      retval=of_fwnat_cbk_init();
      if(retval != OF_SUCCESS)
      {
         CM_CBK_DEBUG_PRINT( "Firewall NAT  CBK initialization failed");
         break;
      }

   }while(0);

   return;
}

int32_t of_fwnat_cbk_init()
{
  int32_t status = OF_SUCCESS; 
  int32_t retval; 

  do
  {
    retval=of_fwnat_infilter_appl_ucmcbk_init();
    if(retval != OF_SUCCESS)
    {
      CM_CBK_DEBUG_PRINT( "FWNAT INPUT FILTER CBK  "
          "initialization failed");
      status=OF_FAILURE;
      break;
    }

    retval=of_fwnat_fwdfilter_appl_ucmcbk_init();
    if(retval != OF_SUCCESS)
    {
      CM_CBK_DEBUG_PRINT( "FWNAT INPUT FILTER CBK  "
          "initialization failed");
      status=OF_FAILURE;
      break;
    }
#ifdef CNTRL_FW4_CONNECTION_TEMPLATES_SUPPORT
    retval = of_fw4_conntemplates_appl_ucmcbk_init();
    if(retval != OF_SUCCESS)
    {
      CM_CBK_DEBUG_PRINT( "CONNECTION TEMPLATES CBK "
          "initialization failed");
      status=OF_FAILURE;
      break;
    }
#endif

  }while(0);

  return status; 
}

//#endif 
//#endif 
