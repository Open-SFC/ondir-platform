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
/*  File        : hcmgrgif.h                                              */
/*                                                                        */
/*  Description : This file contains function prototypes of related to    */
/*                the https certificate manager.                          */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.1      05.14.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#ifndef HTTPSGIF_H
#define HTTPSGIF_H

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 *
 */
int32_t HTTPSCertsInit( void );

/**
 *
 */
int32_t HttpsCliInit( void );

/**
 *
 */
int32_t HttpsSaveCerts( void );

/**
 *
 */
/**
 *
 */
int32_t HttpsGetPrivateKey( HttpsCertInfo_t  *pCertInfo,
                              void         **pPrivKeyInfo );

/**
 *
 */
int32_t HttpsValidateCert( HttpsCertIdInfo_t   *pCertIdInfo,
                           uint8_t             usInNumCerts,
                           uint8_t             ucCertEncoding,
                           HttpsCertificate_t  *pCerts,
                           uint8_t             usInNumCRLs,
                           uint8_t             ucCRLEncoding,
                           void              (**pFreeFn)(EVP_PKEY *),
                           void              **pFreeFnArg,
                           void              **pPeerPubKey );

/**
 *
 */
int32_t HttpsGenCertReq( HttpsCertReqInfo_t    *pCertReqInfo,
                         unsigned char              *pPrivbuf,
                         unsigned char              *pCertbuf,
                         HttpsCertReqParams_t  *pParams );

/**
 *
 */
int32_t HttpsDelMyCert( char  *pIDName );

/**
 *
 */
int32_t HttpsDelTrustedCert( char  *pIDName );

/**
 *
 */
int32_t HttpsVerifySelfCert( HX509Cert_t  *pCert );

/**
 *
 */
int32_t HttpsVerifyTrustedCert( HX509Cert_t  *pCert );

/**
 *
 */
int32_t HttpsDelPrivateKey( char  *pIDName );

/**
 *
 */
int32_t HttpsGetPrivCert( HttpsKeyCertPair_t  *pMyKeyPairs );

/**
 *
 */
int32_t HttpsGetSelfCerts( HttpsGetCert_t  *pGetCerts );

/**
 *
 */
int32_t HttpsGetTrustedCerts( HttpsGetCert_t  *pGetCerts );

/**
 *
 */
int32_t HttpsDeleteCerts( unsigned char   *pIdName,
                          unsigned char   ucCertType );

/**
 *
 */
int32_t HttpsCheckPrivKey( unsigned char  *name_p,
                           uint16_t  *pStatus );
/**
 *
 */
int32_t HttpsDoGenCertReq( HttpsCertReqParams_t  *pParams );

/**
 *
 */
int32_t HttpsLdSvInit( void );

int32_t HttpsVerifyDebug( unsigned char  *pMsgDigest,
                          uint16_t  usMsgDigLen,
                          unsigned char  *pSignedData,
                          uint16_t  usSigLen );

/**
 *
 */
int32_t HttpsGetCertReq( unsigned char  *name_pval,
                         unsigned char  *pCertifybuf,
                         uint32_t  *pReqLen );

/**
 *
 */
int32_t HttpsStoreTrustedCert( unsigned char  *pBuf,
                               uint32_t  ulLen,
                               uint8_t   *pHandle,
                               uint32_t  ulHandleLen );

/**
 *
 */
int32_t HttpsStoreSelfCert( unsigned char  *pBuf,
                            uint32_t  ulLen,
                            unsigned char  *name_p );

/**
 *
 */
int32_t HttpsFindTimeDiff( HttpsCertIdInfo_t  *pIdInfo,
                           uint16_t           usCertType,
                           int32_t            ulLifeTime,
                           int32_t            *iTimeDiff );

/**
 *
 */
HX509Cert_t *HTTPSGetCACert( unsigned char     *pCAReq,
                             uint16_t     usLength,
                             HX509Cert_t  *pPeerCert );

/**
 *
 */
int32_t HttpsDelSelfSignedCert( void );

/**
 *
 */
int32_t HttpsPrintSelfCerts( void );

/**
 *
 */
int32_t HttpsPrintTrustedCerts( void );

/**
 *
 */
int32_t HttpsGetValidCert( HX509Cert_t  **pX509Cert,
                           EVP_PKEY     **pPrivKey );

#endif /* HTTPSGIF_H */

