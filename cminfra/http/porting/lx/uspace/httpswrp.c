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
/*  File        : httpswrp.c                                              */
/*                                                                        */
/*  Description : This file contains wrapper functions required by http   */
/*                server certificates. These wrapper functions are used   */
/*                for the lx flavor.                                      */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.19.04     DRAM     Modified during the code review.     */
/**************************************************************************/

#if defined(OF_HTTPD_SUPPORT) && defined(OF_HTTPS_SUPPORT)

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "httpdinc.h"
/*#include "iocopt.h"*/
#include "openssl/ssl.h"

/****************************************************************
 * * * *  F u n c t i o n    I m p l e m e n t a t i o n  * * * *
 ****************************************************************/

#ifdef HTTPS_ENABLE
/**************************************************************************
 * Function Name : SaveHttpsCerts                                         *
 * Description   : This function is used to save the certificates and     *
 *                 applicable only for the linux environment.             *
 * Input         : None.                                                  *
 * Output        : Status of the https certificates save option.          *
 * Return value  : OF_SUCCESS if https certificates are successfully saved,*
 *                 otherwise OF_FAILURE.                                   *
 **************************************************************************/

int32_t SaveHttpsCerts( void )
{
  /**
   * Save the https certificates.
   */
  return HttpsSaveCerts();
} /* SaveHttpsCerts() */

#endif /* HTTPS_ENABLE */

#endif /* OF_HTTPD_SUPPORT && OF_HTTPS_SUPPORT */

