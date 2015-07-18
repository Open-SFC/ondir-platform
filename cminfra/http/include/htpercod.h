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
/*  File        : htpercod.h                                              */
/*                                                                        */
/*  Description : This file contains the Http error codes                 */
/*                                                                        */
/*  Version      Date          Author      Change Description             */
/*  -------    --------        ------     ---------------------           */
/*   1.0      23-07-2004     K.RamaRao    Initial Implimentation          */
/**************************************************************************/

#ifndef __HTPERCOD_H
#define __HTPERCOD_H


typedef enum HttpErrcodes
{
  HTTP_ERR_START = 10000,
/* Error codes start here*/
  HTTP_ERR_NO_MEMORY,
  HTTP_ERR_PORT_IN_USE,
  HTTP_ERR_MAX_SERVERS_REACHED,
  HTTP_ERR_INVALID_COMMAND,
  HTTP_ERR_INVALID_APPLICATION_TYPE,
  HTTP_ERR_INVALID_ARGUMENT,
  HTTP_ERR_PORT_ALREADY_ENABLED,
  HTTP_ERR_PORT_ALREADY_DISABLED,
  HTTP_ERR_PORT_NOT_FOUND,
  HTTP_ERR_PORT_NOT_CONFIGURED,
  HTTP_ERR_LAST_PORT,
  HTTP_ERR_SOCKET_CREATE_FAIL,
  HTTP_ERR_BIND_FAIL,
  HTTP_ERR_LISTEN_FAIL,
  HTTP_ERR_LPBK_DATA_SEND_FAIL,
  HTTP_ERR_LPBK_RECV_FAIL,
  HTTP_ERR_INVALID_SERVER_TYPE,
  HTTP_ERR_INVALID_PORT_NUM,
  HTTP_PSEUDO_DRIVER_OPEN_FAIL,
  HTTP_ERR_TYPE_CHANGE_NOT_ALLLOWED,
  HTTP_ERR_PORT_CHANGE_NOT_ALLOWED,
  HTTP_ERR_NOTHING_TO_MODIFY,
  HTTP_ERR_ONE_SERVER_REQUIRED,
/* Error codes end here */
  HTTP_ERR_MAX
}HttpErrorCodes_e;

#endif
