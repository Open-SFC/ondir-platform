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
/*  File        : httpwrp.h                                               */
/*                                                                        */
/*  Description : This file contains declarations of wrapper functions    */
/*                required by Http Server.                                */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.14.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#ifndef __HTTPWRP_H
#define __HTTPWRP_H

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "cmincld.h"
#include <stdio.h>
#include <assert.h>
#include "httpgdef.h"

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  Httpd_Print                  printf

/* KMD-FIXME-Move it to normal assert instead of reverse */
#define  HTTPD_ASSERT(condition)      assert(!(condition))


#ifdef HTTPD_DEBUG
#define  Httpd_Heavy_Debug(a)         Httpd_Print((a))
#else
#define  Httpd_Heavy_Debug(a)
#endif

#ifdef HTTPD_DEBUG
#define  Httpd_Debug(a)     Httpd_Print("DEBUG:%s\r\n",(a))
#define  Httpd_Error(a)     Httpd_Print("Error:%s(%d): %s\r\n",__FILE__,__LINE__,(a))
#define  Httpd_SysError(a)  Httpd_Print("SysError:%s(%d):%s\r\n",__FILE__,__LINE__,(a))
#define  HttpDebug(x)       printf("%s:%d: ",__FUNCTION__,__LINE__);printf x
#else
#define  Httpd_Debug(a)
#define  Httpd_Error(a)
#define  Httpd_SysError(a)
#define  HttpDebug(x) 
#endif

#define  Httpd_Warning                Httpd_Print

#define  HTTPD_FUNCTION_ENTRY()  Httpd_Debug("Function:%s(%d):: Entry\n",__FILE__,__LINE__)
#define  HTTPD_FUNCTION_EXIT()   Httpd_Debug("Function:%s(%d):: Exit\n",__FILE__,__LINE__)

#define  Httpprintf                   Httpd_Print

/**
 * FIXME:
 * igwtypes depending on so many header files.
 * Till igwtypes.h becomes portable for all OSs, we use our own defined
 * value when it is ready, just use the following
 *
 * #define HTTPD_IPPROTO_TCP    IGW_IPPROTO_TCP
 *
 */
#define  HTTPD_IPPROTO_TCP            (0x06)

#define  HTTPD_INADDR_ANY             (0)

#define  HTTPD_MAX_USERNAME_LEN       (64)   /* in udbldef.h */
#define  HTTPD_MAX_PASSWORD_LEN       (33)

#define  HTTPD_PRIV_ADMIN             (0x01)
#define  HTTPD_PRIV_USER              (0x02)

#define  HTTPD_ERR_GENERAL            (-1)
#define  HTTPD_ERR_NO_USER            (-2)
#define  HTTPD_ERR_BAD_NAME           (-3)
#define  HTTPD_ERR_NOT_ALLOWED        (-4)
#define  HTTPD_ERR_UDB                (-5)
#define  HTTPD_ERR_USER_LOCKEDOUT     (-6)

#define  Httpd_GetGMT                 T_gmtime

#define SERVER_TYPE_2STR(a)  ((a == hServerNormal) ? "Normal" : "SSL")
#define HTTP_ENABLED_2STR(a) ((a == TRUE) ? "TRUE" : "FALSE")

/**
 * IpAddr Must Be In Network Byte-Order.
 */
#define  HttpNotifyBadLogin(IpAddr,pUserName,pMsg)  HttpNotifyMessage( IpAddr, pUserName, pMsg, "" )
#define  HttpNotifyRandomAccess(IpAddr,pUserName,pMsg,pPage)  HttpNotifyMessage( IpAddr, pUserName, pMsg, pPage)

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 * This function is used to
 */
int32_t HttpServerTask( void );

/**
 * This function is used to
 */
int32_t CreateHttpServer( void );

/**
 * This function is used to
 */
int32_t Httpd_GetTimeStr( char  *pStr );

/**
 * This function is used to
 */

int32_t Httpdget_ticks_per_sec(uint32_t  *pTicksPerSec);

/**
 * This function is used to
 */

char *Httpd_ntoa( ipaddr   IpAddr,
                     char  *pIp );

/**
 * This function is used to
 */
ipaddr  Httpd_aton( char  *pIp );

/**
 * This function is used to
 */
int32_t Httpd_Cookie_SemCreate( void );

/**
 * This function is used to
 */
int32_t Httpd_Cookie_SemGet( void );

/**
 * This function is used to
 */
int32_t Httpd_Cookie_SemRelease( void );


/*bool HttpAllowExtLogin(ipaddr IpAddr);*/

/**
 * This function is used to
 */
void HttpNotifyMessage( ipaddr   IpAddr,
                          char  *pUser,
                          char  *pMsg1,
                          char  *pMsg2 );

/**
 * This function is used to
 */
int32_t HttpAuthUserLogin( HttpRequest  *pHttpReq,
                           char      *pUName,
                           char      *pResp,
                           char      *pChallenge,
                           ipaddr       IpAddr,
                           char      *pSNetName );

#ifndef OF_CM_SUPPORT
int32_t HttpAuthRadiusLogin( HttpRequest  *pHttpReq,
                           char      *pUName,
                           char      *pResp,
                           char      *pChallenge,
                           IGWIpAddr_t       IpAddr,
                           char      *pSNetName );
#endif /*OF_CM_SUPPORT*/

/**
 * This function is used to
 */
int32_t HttpUserLogout( struct cm_ip_addr  IpAddr );

/**
 * This function is used to
 */
unsigned char *Httpd_MD5Encrypt( unsigned char  *pInStr,
                            unsigned char  *pOutStr );

/**
 * This function is used to
 */
int32_t Httpd_FilterHtmlFile( char  *pOrgName,
                              char  *pNewName,
                              char  *pAppData );

/**
 * This function is used to
 */
int32_t Httpd_GetPermission( char  *pFileName,
                             int32_t  *pPermission );

/**
 * This function is used to
 */
int32_t HttpGetPktDirection( struct cm_ip_addr    *PktIp,
			                              void    *pIface,
			                              void    *opaque,
			                              uint32_t  uiType );

#endif /* __HTTPWRP_H */
