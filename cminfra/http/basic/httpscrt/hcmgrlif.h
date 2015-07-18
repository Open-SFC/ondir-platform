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
/*  File        : hcmgrlif.h                                              */
/*                                                                        */
/*  Description : This file contains local function prototypes that are   */
/*                implemented as part of https certificates processing.   */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.1      03.31.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#ifndef HTTPSLIF_H
#define HTTPSLIF_H

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 * This function is used to
 */
int32_t HttpsD2iX509( HX509Cert_t  **ppX509Cert,
                      char      **ppCertdata,
                      uint16_t     uiCertLen );

/**
 * This function is used to
 */
int32_t HttpsReadDir( uint8_t  dirType,
                      uint8_t  *pNumCerts );

/**
 * This function is used to
 */
int32_t HttpsFindSubAltNameInCert( HttpsCertIdInfo_t  *pCertIdInfo,
                                   X509               *pX509 );

/**
 * This function is used to
 */
uint16_t HttpsGetSubAltNameIdType( uint16_t  usIdType );

/**
 * This function is used to
 */
int32_t HttpsFlashWrite( unsigned char  uctype,
                         unsigned char  ucrectype,
                         unsigned char  ucenctype,
                         unsigned char  ucdefcert,
                         unsigned char  ucsigntype,
                         unsigned char  *pbuf,
                         int32_t   length,
                         unsigned char  *pCaName );

/**
 * This function is used to
 */
int32_t HttpsFlashWriteTrust( unsigned char  uctype,
                              unsigned char  ucrectype,
                              unsigned char  ucenctype,
                              unsigned char  ucdefcert,
                              unsigned char  ucsigntype,
                              unsigned char  *pbuf,
                              int32_t   length,
                              unsigned char  *pCaName );

/**
 * This function is used to
 */
/*int32_t  HttpsGetPrivCert(HttpsKeyCertPair_t* pMyKeyPairs);*/

/**
 * This function is used to
 */
int32_t HttpsReadFromFile( unsigned char  *preq,
                           uint16_t  usSrcType,
                           void    ** pSrc,
                           unsigned char  ucCt );

/**
 * This function is used to
 */
int32_t HttpsWritetoFile( unsigned char  *pbuf,
                          uint16_t  usSrcType,
                          void    *pSrc,
                          unsigned char  ucCT,
                          unsigned char  *pCaName );

/**
 * This function is used to
 */
int32_t HttpsLoadSelfCert( HttpsMyKeyCert_t  SelfCert );

/**
 * This function is used to
 */
int32_t HttpsGetSelfSignedCert( unsigned char  *pCert,
                                unsigned char  *pPrivKey );

#endif /* HTTPSLIF_H */
