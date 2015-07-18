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
 * $Source: /cvsroot/fslrepo/sdn-of/cminfra/http/include/ucmglue.h,v $
 * $Revision: 1.1.2.1 $
 * $Date: 2014/04/07 12:12:27 $
 */

/*****************************************************************************
 *  File         : ucmglue.h                                                 *
 *                                                                           *
 *  Description  : UCM handler for iGateway handler.                         *
 *                                                                           *
 *  Version      Date        Author         Change Description               *
 *  -------    --------      ------        ----------------------            *
 *   1.0       05/05/09                     Initial implementation           *
 ****************************************************************************/
#if defined(OF_HTTPD_SUPPORT)
#ifdef OF_CM_SUPPORT

/***********************************************************************
 * Function Name : HttpdUCMConfigGetHandler                           *
 * Description   : This function is used for handling POST requests.   *
 * Input         : HTTP request structure.                             *
 * Output        : Html file to send to the browser with or without    *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.       *
 ***********************************************************************/
int32_t HttpdUCMConfigGetHandler(HttpRequest *pHttpReq);

#endif /*OF_CM_SUPPORT */
#endif /*OF_HTTPD_SUPPORT */
