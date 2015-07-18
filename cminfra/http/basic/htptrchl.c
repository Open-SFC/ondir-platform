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
 * $Source: /cvsroot/fslrepo/sdn-of/cminfra/http/basic/htptrchl.c,v $
 * $Revision: 1.1.2.1 $
 * $Date: 2014/04/07 12:07:08 $
 */

/* @(#) UCM-HTTP   */
/****************************************************************************
 *  File                : htptrchl.c
 *  Description         : This file is used to create, update and close trasport channel 
                          with JE.
 *  Revision History    :
 *  Version    Date          Author            Change Description
 ****************************************************************************/

#ifdef OF_CM_SUPPORT
#include "cmincld.h"
#include "cmdefs.h"
#include "cmsock.h"
#include "cmdll.h"
#include "dmgdef.h"
#include "cmtransgdf.h"
#include "jegdef.h"
#include "jegif.h"
#include "transgif.h"
#include "uchtldef.h"
#include "uchtlgif.h"

/******************************************************************************
 * Function Name : ucmHttpEstablishTransportChannel
 * Description   : This API is used to establish a transport channel with the UCM
 *                 JE.
 * Input params  : uiAddr - IP Address 
 * Output params : NONE
 * Return value  : Pointer to Transport channel on successful creation of Transport
 *                 channel.
 *                 NULL in any failure.
 *****************************************************************************/
void *ucmHttpEstablishTransportChannel(ipaddr uiAddr)
{
  struct cm_tnsprt_channel *pTransChannel;

  pTransChannel = 
    (struct cm_tnsprt_channel*) cm_tnsprt_create_channel(CM_IPPROTO_TCP, uiAddr,
                                            0, UCMJE_CONFIGREQUEST_PORT);
  if(!pTransChannel)
  {
    UCMHTTPDEBUG("%s :: cm_tnsprt_create_channel failed\n\r", __FUNCTION__);
    return NULL;
  }
  return ((void *)pTransChannel);
}

/******************************************************************************
 * Function Name : ucmHttpSetTransportChannelAttribs
 * Description   : This API is used to Set the transport channel attributes. The
 *                 Attributes can be Inactivity timeout, etc.
 * Input params  : pTransChannel - pointer to transport channel
 *                 attrib_id_ui - Attribute ID
 *                 pattrib_data - Attribute data
 * Output params : TransAttribs - Transport channel attributes
 * Return value  : OF_SUCCESS on success
 *                 OF_FAILURE on any failure
 *****************************************************************************/
int32_t ucmHttpSetTransportChannelAttribs(void *pTransChannel,
                                         uint16_t attrib_id_ui,
                                         void *pattrib_data,
                                         struct cm_role_info *pRoleInfo)
{
  struct cm_tnsprt_channel_attribs TransAttribs;
  
  TransAttribs.attrib_id_ui = attrib_id_ui;
  TransAttribs.attrib_data.inact_time_out = *(uint16_t*)pattrib_data;     

  return (cm_tnsprt_set_channel_attribs(pTransChannel,
                                       &TransAttribs));
}

/******************************************************************************
 * Function Name : ucmHttpTerminateTransportChannel
 * Description   : This API is used to close a Transport channel.
 * Input params  : pTransChannel - pointer to transport channel node.
 * Output params : NONE
 * Return value  : NONE
 *****************************************************************************/
void ucmHttpTerminateTransportChannel(struct cm_tnsprt_channel *pTransChannel)
{
  if(pTransChannel)
  {
    cm_tnsprt_close_channel(pTransChannel);
  }
}

#endif /* OF_CM_SUPPORT */


