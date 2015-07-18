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

/* Reviewed :: NO
   Coverity :: NO
   Ready For review:: NO
   Doxyzen :: NO */

/*
 * $Source: /cvsroot/fslrepo/sdn-of/cminfra/http/include/uchtldef.h,v $
 * $Revision: 1.1.2.1.2.1 $
 * $Date: 2014/06/25 11:53:48 $
 */
/**************************************************************************/
/*  File        : uchtldef.h                                              */
/*                                                                        */
/*  Description : This file contains lxpc system specific functions       */
/*                related to the http server.                             */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00              Initial implementation               */
/**************************************************************************/

#ifdef OF_HTTPD_SUPPORT

#ifndef UCM_HTTP_DEBUG
#define UCM_HTTP_DEBUG
#endif

/* Debug macro */
#ifdef UCM_HTTP_DEBUG
#define UCMHTTPDEBUG  printf
#else
#define UCMHTTPDEBUG(format,args...)
#endif

#define UCMHTTP_SESSION_EXISTS  (-2)
#define UCMHTTP_USER_AUTH_FAILED  (-3)
#define UCM_HTTP_MAX_RECORDS_PER_PAGE 1

typedef uint32_t ipaddr;

/* UCM HTTP Session data structure */

typedef struct UCMHttpSession_s
{
  UCMDllQNode_t  ListNode; /* double linked list node. 
                             contains prev, next pointers */
  char      *pUserName;
  ipaddr uiIpAddr;
//#ifdef CM_ROLES_PERM_SUPPORT 
  struct cm_role_info RoleInfo;
//#endif
  struct cm_je_config_session *pConfigSession;
  struct cm_tnsprt_channel *pTransChannel;
} UCMHttpSession_t;

int32_t Httpd_PostString_Convert( unsigned char  *cInString );
#endif /* OF_HTTPD_SUPPORT */
