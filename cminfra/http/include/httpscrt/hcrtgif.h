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
/*  File        : hcrtgif.h                                               */
/*                                                                        */
/*  Description : This file contains function prototypes related to the   */
/*                https certificates.                                     */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.1      05.14.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#ifndef _HCERTGIF_H
#define _HCERTGIF_H

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

/*#include "igwdummy.h"*/

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 * This function is used to
 */
int32_t HttpCertReq( HttpRequest  *pHttpReq,
                     unsigned char     *pPostString );

/**
 * This function is used to
 */
int32_t HttpTrustedVal( HttpRequest  *pHttpReq,
                        unsigned char     *pPostString );

/**
 * This function is used to
 */
int32_t HttpSelfCert( HttpRequest  *pHttpReq,
                      unsigned char     *pPostString );

/**
 * This function is used to
 */
int32_t CMgrSelfSendTabl( HttpRequest  *pHttpReq,
                          unsigned char     *pLineBuf,
                          uint32_t     uiLength );

/**
 * This function is used to
 */
int32_t CMgrTrustedSendTabl( HttpRequest  *pHttpReq,
                             unsigned char     *pLineBuf,
                             uint32_t     uiLength );

/**
 * This function is used to
 */
int32_t SelfCertTabSelect( HttpRequest  *pHttpReq,
                           unsigned char     *pPostString );

/**
 * This function is used to
 */
int32_t TrustedCertTabSelect( HttpRequest  *pHttpReq,
                              unsigned char     *pPostString );

/**
 * This function is used to
 */
int32_t CMgrPrivateSendTabl( HttpRequest  *pHttpReq,
                             unsigned char     *pLineBuf,
                             uint32_t     uiLength );

/**
 * This function is used to
 */
int32_t PrivateTabSelect( HttpRequest  *pHttpReq,
                          unsigned char     *pPostString );

/*******************************************************************
 * * * *  H t t p    F u n c t i o n    P r o t o t y p e s  * * * *
 *******************************************************************/

/**
 * This function is used to
 */
int32_t HttpsHttp_RegLocalTags( void );
/**
 * Self signed certificates.
 */

/**
 * This function is used to
 */
int32_t HttpsActHdlr_selfsign( HttpRequest    *pHttpReq,
                               Httpd_Value_t  *ValArray,
                               uint32_t       uiValCount );

/**
 * This function is used to
 */
int32_t HttpsActHdlr_selfsigntab( HttpRequest    *pHttpReq,
                                  Httpd_Value_t  *ValArray,
                                  uint32_t       nCount );

/**
 * This function is used to
 */
int32_t HttpsGenHdlr_SELFSIGN( HttpRequest    *pHttpReq,
                               Httpd_Value_t  *ValArray,
                               uint32_t       uiValCount,
                               void         *pAppData );

#endif /* _HCERTGIF_H */

