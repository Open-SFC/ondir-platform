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
 * $Source: /cvsroot/fslrepo/sdn-of/cminfra/http/include/uchtlgif.h,v $
 * $Revision: 1.1.2.1.2.1 $
 * $Date: 2014/06/25 11:53:48 $
 */
/**************************************************************************/
/*  File        : uchtlgif.h                                              */
/*                                                                        */
/*  Description : This file contains include entries used by the http     */
/*                server for the lx flavor.                               */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0                            Intital version of the code.         */
/**************************************************************************/

#ifndef _HTTPD_UCMLCL_INC_H_
#define _HTTPDINC_UCMLCL__H_

#include "cmincld.h"

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

int32_t ucmHttpConvertNvPairToXML(struct cm_array_of_nv_pair *pnv_pair_array, 
                                  uint32_t  count_ui,
                                  char  **pXMLOutBuff,
                                  int32_t  bFinal);


void ucmHttpConvertoNameValuePair( char *pInBuf,
                                     struct cm_array_of_nv_pair *pnv_pair_array,
                                     unsigned char *delims);

void ucmHttpConvertoUCMCommandsPair(char *pInBuf, int32_t iCmd, 
                                      int32_t *pCount, unsigned char *delims, 
                                      unsigned char *subdelim, struct cm_command **pCmds);

int32_t ucmHttpSaveConfig(UCMHttpSession_t *pSess, char aDMPath[], 
                                                   char **pResStr);

int32_t ucmHttpRebootDevice(UCMHttpSession_t *pSess, char aDMPath[], 
                                                   char **pResStr);

int32_t ucmHttpSetFactoryDefaults(UCMHttpSession_t *pSess, char aDMPath[],
                                                          char **pResStr);

int32_t ucmHttpFrameAndSendMultCmdsToJE( UCMHttpSession_t *pSess,
                                          int32_t iCount, 
                                          struct cm_command *pCmds,
                                          char **pResStr);
int32_t ucmHttpFrameAndSendAddParamsToJE( UCMHttpSession_t *pSess,
                                          int32_t iCount, 
                                          struct cm_command *pCmds,
                                          char **pResStr);

int32_t ucmHttpFrameAndSendSetParamsToJE( UCMHttpSession_t *pSess,
                                          int32_t iCount, 
                                          struct cm_command *pCmds,
                                          char **pResStr);

int32_t ucmHttpSendDeletParamsToJE( UCMHttpSession_t *pSess,
                                          int32_t iCount, 
                                          struct cm_command *pCmds,
                                          char **pResStr);

void *ucmHttpEstablishTransportChannel(ipaddr uiAddr);

int32_t ucmHttpSetTransportChannelAttribs(void *pTransChannel,
                                         uint16_t attrib_id_ui,
                                         void *pattrib_data,
                                         struct cm_role_info *pRoleInfo);

int32_t ucmHttpCommitConfigSession(UCMHttpSession_t *pSess, char **pResStr);
int32_t ucmHttpRevokeConfigSession(UCMHttpSession_t *pSess);

void ucmHttpTerminateTransportChannel(struct cm_tnsprt_channel *pTransChannel);

void UCMHttpDisplayResult(struct cm_result *pResult, char **pResStr);

int32_t ucmHttpDisplayTableNextNRecords(UCMHttpSession_t *pSess,
                                   struct cm_array_of_nv_pair *pKeysArray,
                                   struct cm_array_of_nv_pair *pPrmKeysArray,
                                   char *dm_path_p, int32_t recCount,
                                   char **xmlBuf);

int32_t ucmHttpDisplayTableRecords(UCMHttpSession_t *pSess,
                                   struct cm_array_of_nv_pair *pKeysArray,
                                   struct cm_array_of_nv_pair *pDMKeysArray,
                                   char *dm_path_p, int32_t bExact,
                                   char **xmlBuf);

void HttpdUCMSessionInit(void);
int32_t HttpdUCMCreateLoginSession(unsigned char *pUserName, unsigned char *pPassword,
                                     struct cm_role_info *pRoleInfo, ipaddr uiIpAddr);

int32_t ucmHTTPDeleteSessionEntryFromList(char  *pUserName);

int32_t ucmHTTPFindSessionEntry(char *pUsrName, ipaddr uiIpAddr, 
                                UCMHttpSession_t **pHttpSess);

int32_t ucmHttpDoneCmd (UCMHttpSession_t *pSess, char aDMPath[],
                                                   char **pResStr);
int32_t ucmHttpCancelCmd (UCMHttpSession_t *pSess, char aDMPath[],
                                                   char **pResStr);

int32_t ucmHttpCheckParams(struct cm_array_of_nv_pair *pKeysArray);

#ifdef UCM_STATS_COLLECTOR_SUPPORT
int32_t ucmHttpDisplayStats (UCMHttpSession_t * pSess,
                             char *dm_path_p, struct cm_array_of_nv_pair *pKeyArrNvPair,
                             uint32_t uiSubCmd,
                             char **xmlBuf);
#endif
#ifdef UCM_ROLES_PERM_SUPPORT
int32_t ucmHttpSetRolePermissions(UCMHttpSession_t *pSess,
                                  char aDMPath[],
                                  struct cm_array_of_nv_pair *pnv_pair_array,char **pResStr);

int32_t ucmHttpSetInstanceRolePermissions(UCMHttpSession_t *pSess,
                                          char aDMPath[],
                                          struct cm_array_of_nv_pair *pnv_pair_array);

int32_t ucmHttpDelRolePermissions(UCMHttpSession_t *pSess, 
                                  char aDMPath[],
                                  struct cm_array_of_nv_pair *pnv_pair_array);

int32_t ucmHttpDelInstanceRolePermissions(UCMHttpSession_t *pSess, 
                                  char aDMPath[],
                                  struct cm_array_of_nv_pair *pnv_pair_array);

int32_t ucmHttpDisplayRolesettings(UCMHttpSession_t *pSess,
                                   char *dm_path_p,
                                   char **xmlBuf);

int32_t ucmHttpDisplayInstanceRolesettings(UCMHttpSession_t *pSess,
                                   char *dm_path_p,
                                   char **xmlBuf);

int32_t ucmHttpDisplayRolesettingsByRole(UCMHttpSession_t *pSess,
                                   char *dm_path_p,
                                   struct cm_array_of_nv_pair *pnv_pair_array,
                                   char **xmlBuf);

int32_t ucmHttpDisplayInstRolesettingsByRole(UCMHttpSession_t *pSess,
                                   char *dm_path_p,
                                   struct cm_array_of_nv_pair *pnv_pair_array,
                                   char **xmlBuf);

#endif /*UCM_ROLES_PERM_SUPPORT*/

#endif /* _HTTPDINC_UCMLCL__H_ */
