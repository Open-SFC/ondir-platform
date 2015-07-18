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

/*****************************************************************************
 *  File         : ucmhtses.c                                                *
 *                                                                           *
 *  Description  : This file contains APIs related to create session,        * 
                   update session details and delete session.
 *                                                                           *
 *  Version      Date        Author         Change Description               *
 *  -------    --------      ------        ----------------------            *
 *   1.0       11/06/09                     Initial implementation           *
 ****************************************************************************/
#if defined(OF_HTTPD_SUPPORT)
#ifdef OF_CM_SUPPORT

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/
#include "httpdinc.h"
#include "cmtransgdf.h"
#include "dmgdef.h"
#include "jegdef.h"
#include "uchtldef.h"
#include "uchtlgif.h"

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

/********************************************************
 * * * * * * G l o b a l V a r i a b l e s  * * * * * * *
 ********************************************************/
UCMDllQ_t ucmHTTPSessionHead_g;

/************************************************************************
 * Function Name : HttpdUCMSessionInit                                  *
 * Description   : This function is used to initilize UCM HTTP session. *
 * Output        : none                                                 *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.        *
 ************************************************************************/
void HttpdUCMSessionInit(void)
{
  CM_DLLQ_INIT_LIST(&ucmHTTPSessionHead_g);
  UCMHTTPDEBUG("Initialized UCM HTTP Session.\n");
  return;
} 
/************************************************************************
 * Function Name : HttpdUCMCreateLoginSession                           *
 * Description   : This function is used to create UCM HTTP session.    *
 * Input         : pRoleInfo - pointer to struct cm_role_info structure.      *
                   pUserName - user name
                   pPassword - Password
                   uiIpAddr - IP Address
 * Output        : pUCMHttpSession - pointer to UCMHttpSession_t        *
 *                 structure.                                           *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.        *
 ************************************************************************/
int32_t HttpdUCMCreateLoginSession(unsigned char *pUserName, unsigned char *pPassword,
                                   struct cm_role_info *pRoleInfo, ipaddr uiIpAddr)
{
  int32_t iLen = 0;
  UCMHttpSession_t *pUCMHttpSession = NULL;

  if(!pUserName)
  {
    UCMHTTPDEBUG("Invalide input.\n");
    return OF_FAILURE;
  }
  /* Find if the session already exists */
  
  if(ucmHTTPFindSessionEntry((char *) pUserName, uiIpAddr, &pUCMHttpSession) == OF_SUCCESS)
  {
    UCMHTTPDEBUG("UCM HTTP Session for %s already exists.\n", pUserName);
    return UCMHTTP_SESSION_EXISTS;
  }

  /* Allocate a free session */
  pUCMHttpSession = (UCMHttpSession_t*)of_calloc(1, sizeof(UCMHttpSession_t));
  if(!pUCMHttpSession)
  {
    UCMHTTPDEBUG("Unable to allocate UCM Http session\n");
    return OF_FAILURE;
  }
  pUCMHttpSession->uiIpAddr = uiIpAddr;

  iLen = of_strlen((char *) pUserName);
  pUCMHttpSession->pUserName = (char*)of_calloc(1, iLen + 1);
  if(!pUCMHttpSession->pUserName)
  {
    UCMHTTPDEBUG("Unable to allocate UCM Http session\n");
    of_free(pUCMHttpSession);
    pUCMHttpSession = NULL;
    return OF_FAILURE;
  }
  of_memcpy(pUCMHttpSession->pUserName, pUserName, iLen +1);
  /* 
   * After initialising the HTTP session start the Transport connection with JE
   */
  if((pUCMHttpSession->pTransChannel = 
     (struct cm_tnsprt_channel*)ucmHttpEstablishTransportChannel(pUCMHttpSession->uiIpAddr)) == NULL)
  {
    UCMHTTPDEBUG("Could not create UCM client socket\n\r");
    of_free(pUCMHttpSession->pUserName);
    of_free(pUCMHttpSession);
    pUCMHttpSession = NULL;
    return OF_FAILURE;
  }
#ifdef CM_ROLES_PERM_SUPPORT
  //pUCMHttpSession->RoleInfo = *pRoleInfo;
  of_memcpy(&pUCMHttpSession->RoleInfo, pRoleInfo, sizeof(struct cm_role_info));
#endif
  /* 
   * Add the pUCMHttpSession to global linked list. 
   */
  CM_DLLQ_APPEND_NODE (&ucmHTTPSessionHead_g, (UCMDllQNode_t *) pUCMHttpSession);
  UCMHTTPDEBUG("Successfully created the UCM HTTP session for: %s \n", pUserName);

  return OF_SUCCESS;
}

/******************************************************************************
 ** Function Name : ucmHTTPFindSessionEntry 
 ** Description   : Function to find out existing session entry.
 **
 ** Input params  : pUserName - User name
                    uiIpAddr - IP Address
 ** Output params : pHttpSession - Session entry pointer 
 ** Return value  :	This method searches session list for particular session 
 **                 and if the session found it will return pointer to that node
 **                 or else return null.
 *******************************************************************************/
int32_t ucmHTTPFindSessionEntry(char *pUsrName, ipaddr uiIpAddr,
                                          UCMHttpSession_t **pHttpSession)
{

  UCMHttpSession_t *pHttpSess = NULL;
  
  CM_DLLQ_SCAN (&ucmHTTPSessionHead_g, pHttpSess, UCMHttpSession_t *)
  {
    if(pHttpSess)
    {
      if(of_strcmp(pHttpSess->pUserName, pUsrName) ==0 
                 && pHttpSess->uiIpAddr == uiIpAddr)
      {
        *pHttpSession = pHttpSess;
        return OF_SUCCESS;
      }
    }
  }
  return OF_FAILURE;
}
/******************************************************************************
 ** Function Name : ucmHTTPDeleteSessionEntryFromList 
 ** Description   : This API Deletes UCM HTTP Session from the Global list.
 ** Input params  : pUserName- pointer to char
 ** Output params : NONE
 ** Return value  : OF_SUCCESS in Success
 **                 OF_FAILURE in Failure case
 *******************************************************************************/
int32_t ucmHTTPDeleteSessionEntryFromList(char*  pUserName)
{
  UCMDllQNode_t *pDllQNode = NULL, *pTmpNode = NULL;
  UCMHttpSession_t *pHttpSession = NULL;

  if (pUserName == NULL)
  {
    UCMHTTPDEBUG("User Name is NULL");
    return OF_FAILURE;
  }

  CM_DLLQ_DYN_SCAN(&ucmHTTPSessionHead_g, pDllQNode, pTmpNode, UCMDllQNode_t *)
  {
    pHttpSession =
      CM_DLLQ_LIST_MEMBER (pDllQNode, UCMHttpSession_t*, ListNode);
    if ((pHttpSession) &&
           (of_strcmp(pHttpSession->pUserName, pUserName) == 0))
    {
      CM_DLLQ_DELETE_NODE (&ucmHTTPSessionHead_g, pDllQNode);
      if((pHttpSession) && (pHttpSession->pTransChannel))
      {
        ucmHttpTerminateTransportChannel(pHttpSession->pTransChannel);
        of_free(pHttpSession->pUserName);
        of_free(pHttpSession);
        pHttpSession = NULL;
      }
    }
  }

  return OF_SUCCESS;
}
#endif /*OF_CM_SUPPORT */
#endif /*OF_HTTPD_SUPPORT */
