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

/* Reviewed :: NO    Coverity :: NO   Ready For review:: NO   Doxyzen :: YES / Not Required / NO */
/*
 * $Source: /cvsroot/fslrepo/sdn-of/cminfra/http/include/uctrngif.h,v $
 * $Revision: 1.1.2.1 $
 * $Date: 2014/04/07 12:12:27 $
 */

/* @(#) UCM-TRANSPORT_CHANNEL   06/02/2009  */
/****************************************************************************
 *  File                : transgif.h
 *  Description         : This file contains API declarations provided by
 *                        UCM transport channel layer.
 *  Revision History    :
 *    Version    Date          Author           Change Description
****************************************************************************/
#ifndef UCM_TRANS_GIF_H
#define UCM_TRANS_GIF_H

void* cm_tnsprt_create_channel(uint32_t uiTransProtocol,
                                  uint16_t usSrcPort,
                                  uint32_t uiAddr,
                                  uint16_t usDestPort);

void cm_tnsprt_close_channel(struct cm_tnsprt_channel *pTransChannel);
int32_t cm_tnsprt_set_channel_attribs(void *pTransChannel,
                                      struct cm_tnsprt_channel_attribs *pAttrib,
                                      struct cm_role_info *pRoleInfo);
int32_t UCMTransportChannelSendMsg(void *pTransChannel,
                                   UCMMsgGenericHeader_t *pGenHdr,
                                   char *pInputBuff,
                                   uint32_t uiInputBuffLen);
int32_t UCMTransportChannelReceiveMsg(cm_sock_handle iSockFd, 
                                      UCMMsgGenericHeader_t *pGenHdr,
                                      unsigned char **pMsg, uint32_t *pulMsgLen);


#endif /* UCM_TRANS_GIF_H */


