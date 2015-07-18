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

/**************************************************************************/
/*  Description : This file contains wrapper function related to the href */
/*                and will be used by the http server and all the modules */
/*                can include their entries. These wrapper is used for the*/
/*                lxpc system.                                            */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.20.04     dram     Modified during the code review.     */
/**************************************************************************/

#ifdef OF_HTTPD_SUPPORT

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "httpwrp.h"
/*#include "igwincld.h"*/
#include <dlfcn.h>


/****************************************************************
 * * * *  F u n c t i o n    I m p l e m e n t a t i o n  * * * *
 ****************************************************************/

/***********************************************************************
 * Function Name : Httpd_FilterHtmlFile                                *
 * Description   : Requested file name can be manipulated here         *
 * Input         : pOrgName (I) - pointer to the actual string.        *
 *               : pNewName (O) - new html file name.                  *
 *                 pAppData (O) - pointer to the application data.     *
 * Output        : None.                                               *
 * Return value  : 0 on manipulation                                   *
 *                 1 if nothing is modified                            *
 ***********************************************************************/

int32_t Httpd_FilterHtmlFile( char  *pOrgName,
                              char  *pNewName,
                              char  *pAppData )
{
#ifdef INTOTO_HTTPD_CBK_SUPPORT
  char  path[64]={'\0'};
  char *handle=NULL, *error=NULL;
  int32_t (* pinitfun)(char* ,char* ,char*);
  
  /**
   * This code is used to show port name in bridge page.
   */
  if (of_strncmp(pOrgName, "portcnf*", 8) == 0)
  {
    of_strcpy(pAppData, pOrgName + 8);
    return OF_SUCCESS;
  }

  /**
   * This code is used to either adding or deleting of bridge names.
   */
  if (of_strncmp(pOrgName, "bridgeconf*", 11) == 0)
  {
    if (of_strncmp(pOrgName + 11, "[ATTACH]", 8) == 0)
    {
      of_strncpy(pAppData, pOrgName + 20,
                (of_strchr(pOrgName + 20, '*') - (pOrgName + 20)));
      of_strcpy(pNewName, "rbriadd.htm");
    }
    if (of_strncmp(pOrgName + 11, "[DETACH]", 8) == 0)
    {
      of_strcpy(pAppData, pOrgName + 20);

      of_strcpy(pNewName, "eptdetah.htm");
    }
    return OF_SUCCESS;
  }

  /**
  *  This code is used to display dhcp service leases.
  */
  if (of_strncmp(pOrgName, "viewdhcp*", 9) == 0)
  {
    of_strcpy(pAppData, pOrgName + 9);
    of_strcpy(pNewName, "rlandhcp.htm");
    return OF_SUCCESS;
  }

    if ( of_strncmp (pOrgName, "qos", 3) == 0)
    {
#define INTOTO_MAX_PATH_LENGTH (64) 
#define IGW_ITM_HTTP_LIB_PATH    "/igateway/lib/httpcbk/libitm_httpcbk.so"

  of_strcpy(path,IGW_ITM_HTTP_LIB_PATH);


  if((of_strlen(path) >= INTOTO_MAX_PATH_LENGTH) )
  {
    printf("%s(): Input exceeds the max limit. Buffer overflow!\n\r",
           __FUNCTION__);
    return OF_FAILURE;
  }

  handle = dlopen(path,RTLD_LAZY);
  if(!handle)
  {
    if ((error = dlerror()) != NULL)
    {
      Trace_2(TM_ID,TRACE_SEVERE,"%s(): dlopen failed for path:%s\n\r",
              __FUNCTION__,path);  
      printf("dlopen failed for path:%s\n",path);
      return OF_FAILURE;
    }
    Trace_2(TM_ID,TRACE_SEVERE,"%s(): dlopen failed for path:%s\n\r",
            __FUNCTION__,path);  
    printf("dlopen failed for path:%s\n",path);
    return OF_FAILURE;
  }

  pinitfun = dlsym(handle,"itm_httpd_FilterHtmlFile");
  if ((error = dlerror()) != NULL)
  {
    fprintf (stderr, "itm_httpd_FilterHtmlFile:%s\n",error);
    Trace_2(TM_ID,TRACE_SEVERE,"%s(): dlsym failed for path:%s for "
            "itm_httpd_FilterHtmlFile\n\r",__FUNCTION__,path);  
    printf("dlsym failed for path:%s\n",path);
    dlclose(handle);
    return OF_FAILURE;
  }
   pinitfun(pOrgName,pNewName,pAppData);
  }


#if defined(INTOTO_UPN_SUPPORT) && defined(INTOTO_IPDB_SUPPORT)

  /**
   * UPN related entries.
   */
  /**
   * Modifying the selected upn entry.
   */
  if (of_strncmp(pOrgName, "upnpcfg*", 8) == 0)
  {
    of_strcpy(pAppData, pOrgName + 8);
    of_strcpy(pNewName, "seupnpm.htm");

    return OF_SUCCESS;
  }

  /**
   * Deleting the selected upn entry.
   */
  if (of_strncmp(pOrgName, "upnpdel*", 8) == 0)
  {
    IGWUPNDbArgs_t  UpnDelArgs;
    int32_t         iResult;

    /**
     * Initialize the UpnDelArgs.
     */
    of_memset(&UpnDelArgs, 0, sizeof(UpnDelArgs));
    of_strcpy(UpnDelArgs.DelOpHostRecName, pOrgName + 8);
    UpnDelArgs.ulFlags |= IGWUPN_HOSTREC_NAME;

    /**
     * Delete the selected entry.
     */
    iResult = IGWUPNUiDelHostRecAPI(&UpnDelArgs);

    if (iResult != OF_SUCCESS)
    {
      of_strcpy(pAppData, "Failed to delete the entry");
      of_strcpy(pNewName, "errscrn.htm");
    }
    else
    {
      of_strcpy(pNewName, "seupnp.htm");
    }
    return OF_SUCCESS;
  }

  /**
   * Adding a service record to the given upn entry.
   */
  if (of_strncmp(pOrgName, "upnpaddsrv*", 11) == 0)
  {
    of_strcpy(pAppData, pOrgName + 11);
    of_strcpy(pNewName, "supnsrva.htm");

    return OF_SUCCESS;
  }

  /**
   * Deleting a service record from the given upn entry.
   */
  if (of_strncmp(pOrgName, "upnpdelsrv*", 11) == 0)
  {
    of_strcpy(pAppData, pOrgName + 11);
    of_strcpy(pNewName, "supnsrvd.htm");

    return OF_SUCCESS;
  }

#endif /* INTOTO_UPN_SUPPORT && INTOTO_IPDB_SUPPORT */

#ifdef L2BRIDGE_SUPPORT
  if (of_strncmp(pOrgName, "brstpcfg*", 9) == 0)
  {
    of_strcpy(pAppData, pOrgName + 9);
    of_strcpy(pNewName, "rspantre.htm");
    return OF_SUCCESS;
  }
  if (of_strncmp(pOrgName, "brprtadd*", 9) == 0)
  {
    of_strcpy(pAppData, pOrgName + 9);
    of_strcpy(pNewName, "rbridge.htm");
    return OF_SUCCESS;
  }

#endif


#if defined (INTOTO_ALGV4_BASIC_SUPPORT) && defined (INTOTO_ALGV4_HTTP_SUPPORT)
  IGWALGV4HtpRegHrefHdlrs(pOrgName, pNewName, pAppData);
#endif /* (INTOTO_ALGV4_BASIC_SUPPORT) && (INTOTO_ALGV4_HTTP_SUPPORT) */

#if defined(INTOTO_8021X_SUPPORT) && defined(INTOTO_8021X_HTTP_SUPPORT)
  if (of_strncmp(pOrgName, "configwlan*", 11) == 0)
  {
    of_strcpy(pAppData, pOrgName + 11);
    of_strcpy(pNewName, "wpacfg.htm");
    return OF_SUCCESS;
  }

  if (of_strncmp(pOrgName, "viewwlan*", 9) == 0)
  {
    of_strcpy(pAppData, pOrgName + 9);
    of_strcpy(pNewName, "wpashow.htm");
    return OF_SUCCESS;
  }
#endif /* INTOTO_8021X_SUPPORT && INTOTO_8021X_HTTP_SUPPORT */


#endif /* INTOTO_HTTPD_CBK_SUPPORT */
  return 1;
}

#endif /* OF_HTTPD_SUPPORT */

