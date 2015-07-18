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
/*  File        : httpmcfg.c                                              */
/*                                                                        */
/*  Description : This file contains global configuration for http server */
/*                related to lxpc system.                                 */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kramarao  Initial implementation               */
/**************************************************************************/

#include "cmincld.h"
#include "httpgdef.h"
#include "httpgif.h"
#include "httpcfg.h"
#include "basic_tunable.h"
#include "httpdadv.h"

/***************************************************************************
 * Function Name  : HttpGetMaxServers
 * Description    : This API will get the maximum number of server that can
 *                  be configured for a given type of server. This API
 *                  returns the maximum number of servers that can be
 *                  configured for the given type. If -1 is send as input
 *                  it will return the max. number of servers that can be
 *                  configured for both the types.
 * Input          : eServerType - type of the server
 * Output         : None
 * Return value   : No. of servers for the given type.
 ***************************************************************************/
int32_t HttpGetMaxServers(hServerType_e eServerType)
{
  if(eServerType == hServerNormal)
  	return HTTP_MAX_SERVERS_NORMAL;
  
#ifdef HTTPS_ENABLE  
  else if(eServerType == hServerSSL)
  	return HTTP_MAX_SERVERS_SSL;
#endif

  else if(eServerType == -1)
  	return HTTP_MAX_SERVERS;
  else 
  	return OF_FAILURE;
}

