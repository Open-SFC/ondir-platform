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
/*  File        : httpdapi.c                                              */
/*                                                                        */
/*  Description : This file contains the functions defined in the file,   */
/*                httpwrp.c, which are common across all the three        */
/*                platforms, vi, vx, and lx.                              */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      05.20.04     dram     Initial implementation               */
/*    1.1      22.02.06    Harishp   Modified during MultipleInstances	  */	
/*                                   and Multithreading	                  */
/**************************************************************************/

#ifdef OF_HTTPD_SUPPORT

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

/*#include "httpdinc.h"*/
#include "cmincld.h"
#include "cmdefs.h"
#include "cmutil.h"
#include "dmgdef.h"
#include <openssl/evp.h>
#include "cmmd5.h"
#include "httpwrp.h"
#include "htpdglb.h"
#include "htpercod.h"
#include "httpdtmr.h"
#include "httpd.h"
#include "httpgif.h"

#define bool int
extern uint32_t igwHTTPDLogUserMsg(uint32_t uiMsgId_p,uint32_t uiMesFamily,
                                   unsigned char *pUserName,ipaddr UserIpAddr,
                                   uint32_t ulLoginTime, uint32_t ulLogoutTime);
extern int32_t igwHTTPDGetUserloginTimeFromKuki(char *pUserName, uint32_t *pulLoginTime);
extern int32_t igwHTTPDUpdateUserloginTimeInKuki(char *pUserName, uint32_t ulLoginTime);
/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**************************************************************************
 * Function Name : Httpd_MD5Encrypt                                       *
 * Description   : This function is used to encrypt the passed string     *
 *                 using MD5 encryption algorithm.                        *
 * Input         : pInputString  (I) - pointer to the input string to be  *
 *                                     encrypted.                         *
 *                 pOutputString (O) - pointer to the output string, which*
 *                                     holds encrypted string.            *
 * Output        : Encrypted string.                                      *
 * Return value  : Encrypted string on success, otherwise NULL.           *
 **************************************************************************/

unsigned char *Httpd_MD5Encrypt( unsigned char  *pInputString,
                            unsigned char  *pOutputString )
{
  unsigned char  *pTmpString; 

  pTmpString = pOutputString;
  of_strlen((char *)pInputString);
#ifdef CM_PAM_SUPPORT
  cm_md5_encrypt_buf((char*)pInputString,(char*)pTmpString);
#else
      //of_strcpy(aPaswd, pAuth->pPwd);
#endif
 
  return pTmpString;

} /* Httpd_MD5Encrypt() */

/**************************************************************************
 * Function Name : HttpAuthUserLogin                                      *
 * Description   : This function is used to authenticate the user login   *
 *                 information.                                           *
 * Input         : pHttpRequest (I) - pointer to the HTTPRequest structure*
 *                 pUName       (I) - pointer to the user name.           *
 *                 pResp        (I) - pointer to the response.            *
 *                 pChallenge   (I) - pointer to the challenge value.     *
 *                 IPAddress    (I) - IGW IP address structure for		  *
 *									   IPV6 and IPV4 family.              *
 *                 pSNetName    (I) - pointer to the SNet name.           *
 * Output        : Status of the authentication process.                  *
 * Return value  : OF_SUCCESS on successful athentication, otherwise       *
 *                 OF_FAILURE.                                             *
 **************************************************************************/
/*
 * FIXME: Following method needs to be fixed with PAM
 */
int32_t HttpAuthUserLogin( HttpRequest  *pHttpRequest,
                           char      *pUName,
                           char      *pResp,
                           char      *pChallenge,
                           ipaddr       IPAddress,
                           char      *pSNetName )
{
  /* IGWIpAddr_t   LogInIpAddr={}; 
  ipaddr LogInIpAddr; */
  /*UserDbTemp_t  UserInfo; // for UCM
  PGUserInfo_t  PGUser;*/
  /*uint8_t       Privileges      = 0;
  int32_t       InactivityTimer = 0;
  int32_t       iReturnValue    = 0;
  char      aSrcIPAddress[CM_INET6_ADDRSTRLEN];
  char      aMessage[MAX_LOG_MESSAGE_SIZE];
  uint32_t      SnetId = 0;
  IGWSNetOps_t  SNetOpts; // for UCM
  int32_t       iRetVal = 0;*/
  HttpClientReq_t  *pClientReq=NULL;
  HttpGlobals_t  *pHttpg=NULL;
#ifdef MESGCLNT_SUPPORT
  uint32_t ulLogInTime = 0;
#endif /*MESGCLNT_SUPPORT*/

  pClientReq = (HttpClientReq_t *)pHttpRequest->pPvtData;
  pHttpg=(HttpGlobals_t *)pClientReq->pGlobalData;
  if(!pHttpg)
  {
    Httpd_Error("pHttpg is NULL\r\n");
    return OF_FAILURE;
  }
  /**
   * I observed that the IP address is in the network byte order,
   * hence we need to change the byte order.
   */
#ifdef INTOTO_IPV6_SUPPORT
  if(IPAddress.ip_type_ul==IGW_INET_IPV6)
  {
    	/* of_memcpy(LogInIpAddr.IPv6Addr8, IPAddress.IPv6Addr8,IGW_IPV6_ADDR_LEN);*/
         /* of_memcpy(LogInIpAddr, IPAddress,UCM_IPV6_ADDR_LEN);*/
  }
  else
#endif /*INTOTO_IPV6_SUPPORT*/
 {
 	 /* LogInIpAddr.IPv4Addr = ntohl(IPAddress.IPv4Addr);*/ 
         /* ntohl(IPAddress); */
 }


#ifndef OF_CM_SUPPORT
  /**
   * Initialize the snetName.
   */
  of_memset(&SNetOpts, 0, sizeof(IGWSNetOps_t));

  of_strcpy(SNetOpts.aSNetName, pSNetName);

  if (IGWSNetSearchByName(&SNetOpts)==OF_FAILURE)
  {
    Httpd_Debug("not able to search by name\n");
  }
  else
  {
    SnetId = SNetOpts.ulSNetId;
  }
  
  of_memset((void*)&UserInfo, 0, sizeof(UserDbTemp_t));

  if (GetExactUsrDbRec(SnetId, pUName, &UserInfo) != OF_SUCCESS)
  {
    SnetId = 0;
    if (GetExactUsrDbRec(SnetId, pUName, &UserInfo) != OF_SUCCESS)
    {
      if (HttpUserIsConfigType(UserInfo.aUserName) == FALSE)
      {
	        of_memset(aMessage, 0, sizeof(aMessage));
		#ifdef INTOTO_IPV6_SUPPORT
		 if(pHttpRequest->PeerIp.ip_type_ul==IGW_INET_IPV6)
		{
			if(HttpdNumToPresentation(&pHttpRequest->PeerIp,(unsigned char *)aSrcIPAddress)!=OF_SUCCESS)
			{
	   	           HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
    	     		    return OF_FAILURE;
			} 		
		 }
		 else
	 	#endif  /*INTOTO_IPV6_SUPPORT*/
		 {		 
			cm_inet_ntoal(pHttpRequest->PeerIp, aSrcIPAddress);
		 }
		IGWSprintf(aMessage, "Web- Attempt to login with a wrong name %s from Ip Addr: %s",
						 UserInfo.aUserName, aSrcIPAddress);
#ifdef MESGCLNT_SUPPORT
	        IBsendSysLogMsg(NULL, 0, 0, aMessage, 0, ISEC_UN_AUTH_ACCESS,
	                        LOGWARNING,
	                        HTTPD_SYSMSGID_USR_LOGIN_INVALIDNAME_4);
                of_time(&ulLogInTime);
                igwHTTPDUpdateUserloginTimeInKuki(UserInfo.aUserName,ulLogInTime);
                igwHTTPDLogUserMsg(HTTPD_SYSMSGID_USR_LOGIN_INVALIDNAME_4,
                                   IGW_LOG_FAMILY_USERLOGIN,UserInfo.aUserName,
                                   pHttpRequest->PeerIp,ulLogInTime, 0);
#endif /* MESGCLNT_SUPPORT */
      }
      PGUserUpdateStats(PG_USER_HTTP_LOGIN, PG_POLGRP_NOT_FOUND);
      return HTTPD_ERR_NO_USER;
    }
  }

  if ((iRetVal = UserEncryAuthenticate(SnetId, pUName, pChallenge, pResp,
                            &Privileges, &InactivityTimer)) != OK)
  {
    /* Here we need to examine the new return value. ---> sureshd */
    if (iRetVal == UDB_USER_LOCKEDOUT)
    {
      PGUserUpdateStats(PG_USER_HTTP_LOGIN, PG_POLGRP_NOT_FOUND);
      return HTTPD_ERR_USER_LOCKEDOUT;
    }
    PGUserUpdateStats(PG_USER_HTTP_LOGIN, PG_POLGRP_NOT_FOUND);
    return HTTPD_ERR_NO_USER;
  }
  if(pClientReq->ulConnType) 
   { 
	 if (HttpCookie_Add(pHttpg,pClientReq->ucCookieVal, 
						Http_GetCookieName(pHttpg),
						aSrcIPAddress, 
						pChallenge, 
						"/", 
						pClientReq->eVersion, 
						Httpd_GetMinSecs(pHttpg)) != OF_SUCCESS) 
      PGUserUpdateStats(PG_USER_HTTP_LOGIN, PG_POLGRP_NOT_FOUND);
	 return HTTPD_ERR_NO_USER; 
   } 


  /**
   * Add the SNet identification to the cookie.
   */
  if (HttpCookie_AddSNetId(pHttpRequest->pCookieVal,
                         SnetId) == OF_FAILURE)
  {
    Httpd_Error("HttpCookie_AddSNetId: Unable To Add SNet Id\n");
  }

  /**
   * If the user is config admin without policy group, dont call
   * PGPerformUserLogin(). This user has nothing to do with 
   * packet processing module.
   */
  if ((UserInfo.userType == UDB_CONFIG_USER) &&
      (UserInfo.uiPrivileges == ADMIN_PRIV) &&
      (!of_strcmp(UserInfo.aPolGrname_p, "")))
  {
    return OF_SUCCESS;
  }

  of_memset((void*)&PGUser, 0, sizeof(PGUserInfo_t));
  of_strcpy(PGUser.aUserName, pUName);
  PGUser.uloginTimeout =UserInfo.lInactivityTimer;
#ifdef INTOTO_IPV6_SUPPORT
  if( IPAddress.ip_type_ul == IGW_INET_IPV6 && 
      IGWIsIPv4MappedV6Addr(&IPAddress.IP.IPv6Addr) )
  {
    PGUser.DualLoginIPAddr.IPv4Addr = ntohl(IPAddress.IPv6Addr32[3]);
    PGUser.DualLoginIPAddr.ip_type_ul = IGW_INET_IPV4;
  }
  else
#endif
    PGUser.DualLoginIPAddr= IPAddress;

  /*TODO look arp table to get MAC address*/
  if(pClientReq->bISPgUserLoginReq) 
  { 
     iReturnValue = PGPerformUserLogin(SnetId, UserInfo.aPolGrname_p,
                                       &PGUser);
     if (iReturnValue == OF_SUCCESS)
     {
       UDBUpdateLastLoginInfo(SNetOpts.ulSNetId, PGUser.aUserName, TRUE);
       return OF_SUCCESS;
     }
     else
     {
       of_memset(aMessage, 0, sizeof(aMessage));
       #ifdef INTOTO_IPV6_SUPPORT
       if(pHttpRequest->PeerIp.ip_type_ul==IGW_INET_IPV6)
      {
   	if(HttpdNumToPresentation(&pHttpRequest->PeerIp,(unsigned char *)aSrcIPAddress)!=OF_SUCCESS)
    	{
              HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
              return OF_FAILURE;
    	}
     }
     else
     #endif  /*INTOTO_IPV6_SUPPORT*/
     {		 
      	 cm_inet_ntoal(pHttpRequest->PeerIp, aSrcIPAddress);
     }
     IGWSprintf(aMessage, "Web- Attempt to login with a wrong name %s from Ip Addr: %s",
      				 UserInfo.aUserName, aSrcIPAddress);
#ifdef MESGCLNT_SUPPORT
       IBsendSysLogMsg(NULL, 0, 0, aMessage, 0, ISEC_UN_AUTH_ACCESS,
                       LOGWARNING, HTTPD_SYSMSGID_USR_LOGIN_INVALIDNAME_4);
       of_time(&ulLogInTime);
       igwHTTPDUpdateUserloginTimeInKuki(UserInfo.aUserName,ulLogInTime);
       igwHTTPDLogUserMsg(HTTPD_SYSMSGID_USR_LOGIN_INVALIDNAME_4,
                          IGW_LOG_FAMILY_USERLOGIN,UserInfo.aUserName,
                          pHttpRequest->PeerIp, ulLogInTime, 0);
#endif /* MESGCLNT_SUPPORT */
       return OF_FAILURE;
  }
 }
#endif /* OF_CM_SUPPORT */
 return OF_SUCCESS;
} /* HttpAuthUserLogin() */


/**************************************************************************
 * Function Name : HttpUserHasAdminPriv                                   *
 * Description   : This function is used to know whether the user has     *
 *                 admin privilages and the user of type config           *
 * Input         : pUserName - user name                                  *
 * Output        : None                                                   *
 * Return value  : TRUE if the user has admin privilages and is of type   *
 *                 config else FALSE                                      *
 **************************************************************************/

bool HttpUserHasAdminPriv(char *pUserName)
{
  return TRUE; // Need to Check with PAM
#ifndef OF_CM_SUPPORT
  UserDbTemp_t             pUserInfo;
  uint32_t                 ulSnetId ;
  uint32_t                 ulMaxSnets;
  bool                  isMultipleSnetsEnabled;
  
  IGWIsMultipleSnetsEnabled(&isMultipleSnetsEnabled);
  if (isMultipleSnetsEnabled == FALSE)
  {  	
  	ulMaxSnets = 1;
  }
  else
  {
    IGWGetMaxSNetIDs(&ulMaxSnets);
  }

  for (ulSnetId = 0; ulSnetId < ulMaxSnets; ulSnetId++)
  {
    if (GetExactUsrDbRec(ulSnetId, pUserName, &pUserInfo) == OF_SUCCESS)
    {
      if ((pUserInfo.uiPrivileges == ADMIN_PRIV) &&
          (pUserInfo.userType == UDB_CONFIG_USER))
         return TRUE;
      else
         return FALSE;
    }
  }
  return FALSE;
#endif /*OF_CM_SUPPORT*/

} /* end of HttpUserHasAdminPriv() */


/**************************************************************************
 * Function Name : HttpUserIsConfigType                                   *
 * Description   : This function is used to know whether the user is      *
 *                 of type config                                         *
 * Input         : pUserName - user name                                  *
 * Output        : None                                                   *
 * Return value  : TRUE if the user is of type config else FALSE          *
 **************************************************************************/
bool HttpUserIsConfigType(char* pUserName)
{
  return TRUE;
#ifndef OF_CM_SUPPORT
  UserDbTemp_t             pUserInfo;
  uint32_t                 ulSnetId ;
  uint32_t                 ulMaxSnets;
  bool                  isMultipleSnetsEnabled;
  
  IGWIsMultipleSnetsEnabled(&isMultipleSnetsEnabled);
  if (isMultipleSnetsEnabled == FALSE)
  {  	
  	ulMaxSnets = 1;
  }
  else
  {
    IGWGetMaxSNetIDs(&ulMaxSnets);
  }

  for (ulSnetId = 0; ulSnetId < ulMaxSnets; ulSnetId++)
  {
    if (GetExactUsrDbRec(ulSnetId, pUserName, &pUserInfo) == OF_SUCCESS)
    {
      if (pUserInfo.userType == UDB_CONFIG_USER)
         return TRUE;
      else
         return FALSE;
    }
  }
  return FALSE;
#endif /*OF_CM_SUPPORT*/
}

#endif /* OF_HTTPD_SUPPORT */

