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
/*  File        : hcmgrgdf.h                                              */
/*                                                                        */
/*  Description : This file contains global declarations and type         */
/*                definitions related https certificates.                 */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.1      05.14.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#ifndef _HTTPSGDEF_H
#define _HTTPSGDEF_H

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "openssl/asn1_mac.h"
#include "openssl/x509.h"
#include "openssl/pem.h"

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

/**
 * Encoding Types....
 */

#define  HTTPS_DER                    (1)

/*
 * Identity Types..
 */

#define  HTTPS_IPADDR                 (1)

/*
 * Certificate Types..
 */

#define  HTTPS_RSA_CERT               (1)

#define  RSA_MD5                      (1)
#define  RSA_SHA1                     (2)

/*
 * Some general definitions...
 */
#define  HTTPS_ERROR_NO_MATCHING_PRIV_KEY   (0x0204)
#define  HTTPS_X509_SIG                     (2)
#define  HTTPS_CERT_MIN_DEPTH               (8)
#define  HTTPS_TRUSTED_CA_CERT_NOT_FOUND    (9)
#define  HTTPS_I2D_DECODE_FAILED            (10)
#define  HTTPS_FORM_CHAIN_FAILED            (11)
#define  HTTPS_PRIVKEY_NOT_FOUND            (12)
#define  HTTPS_FOPEN_ERROR                  (13)
#define  HTTPS_VALIDATION_FAILED            (14)
#define  HTTPS_READ_FILE_FAILED             (15)
#define  HTTPS_ERROR_VERIFY_CERT_FAILED     (16)
#define  HTTPS_ERROR_VERIFY_KEYPAIR_FAILED  (17)
#define  HTTPS_DUPLICATE_CA_CERT            (18)
#define  HTTPS_MAX_CERT_SIZE_ERROR          (19)

#define  HTTPS_CERT_LIMIT_SIZE              (10240)
#define  HTTPS_MAX_NAME_LEN                 (50)
#define  HTTPS_INVALID_SIG_TYPE             (55)
#define  HTTPS_TRUSTED_MAX_LIMIT            (56)

#define  HTTPS_PRIV_KEY                     (1)
#define  HTTPS_CERT_REQ                     (2)
#define  HTTPS_CERT                         (3)

#define  ulstartadr                         (0x1d0000)
#define  ulsize                             (0x10000)

#define  HTTPS_MAX_BUF_LEN                  (10240)
#define  MAX_CERT_BUF_SIZE                  (4)

/**
 * The following macros are moved from cmgrldef.h.
 */
#define  HTTPS_DUPLICATE_KEY_NAME           (2004)
#define  HTTPS_PRIV_KEY_LIMIT               (2005)

#define  HTTPS_MAX_NUM_OF_KEY_CERT          (3)
#define  HTTPS_MAX_NUM_OF_CA_CERT           (3)

#define  HTTPS_INVALID_CERTIFICATE          (20)
#define  HTTPS_CERTIFICATE_LEN              (1024)
#define  HTTPS_ID_NAME_LEN                  (8)
#define  HTTPS_ID_DATA_LEN                  (256)

/***************************************************************************
 CERT_PARAMS_DEFINTIONS
****************************************************************************/
#define  HTTPS_PARAMS_ID_NAME_LEN           (10)
#define  HTTPS_PARAMS_USER_NAME_LEN         (50+1)
#define  HTTPS_PARAMS_SUB_LEN               (50+1)
#define  HTTPS_PARAMS_ORGANIZATION_LEN      (50+1)
#define  HTTPS_PARAMS_CITY_NAME_LEN         (50+1)
#define  HTTPS_PARAMS_POSTAL_CODE_LEN       (10+1)
#define  HTTPS_PARAMS_DEPT_NAME_LEN         (50+1)
#define  HTTPS_PARAMS_STATE_NAME_LEN        (50+1)
#define  HTTPS_PARAMS_EMAIL_ID_LEN          (50+1)
#define  HTTPS_PARAMS_SELF_EMAIL_ID_LEN     (50+1)
#define  HTTPS_PARAMS_DOMAIN_NAME_LEN       (50+1)
#define  HTTPS_PARAMS_IP_ADDRESS_LEN        (16+1)
#define  HTTPS_PARAMS_COUNTRY_LEN           (2+1)
#define  HTTPS_PARAMS_SERIAL_NUMBER_LEN     (50)
#define  HTTPS_PARAMS_SIGNATURE_ALG_LEN     (50)
#define  HTTPS_PARAMS_ISSUER_NAME_LEN       (256)
#define  HTTPS_PARAMS_EXPIRY_LEN            (256)
#define  HTTPS_PARAMS_DATE_NOT_BEFORE_LEN   (50)
#define  HTTPS_PARAMS_DATE_NOT_AFTER_LEN    (50)
#define  HTTPS_PARAMS_SUB_NAME_LEN          (256)
#define  HTTPS_PARAMS_PUB_KEY_ALG_LEN       (50)
#define  HTTPS_PARAMS_SIGNATURE_LEN         (256)
#define  HTTPS_PARAMS_SUB_ALT_NAME_LEN      (512)
#define  HTTPS_PARAMS_RSA_KEY_PAIRS_LEN     (10)

#define  HTTPS_MAX_CERT_BODY_SIZE           (2048)

/****************************************************
 * * * *  T y p e    D e c l a r a t i o n s  * * * *
 ****************************************************/

typedef struct  x509_st             HX509Cert_t;
typedef struct  x509_store_st       HX509Store_t;
typedef struct  x509_store_ctx_st   HX509StoreContext_t;
typedef struct  X509_name_st        HX509Name_t;
typedef struct  X509_extension_st   HX509Extension_t;

typedef struct  X509_algor_st       HX509Algo_t;
typedef struct  X509_pubkey_st      HX509PubKey_t;
typedef struct  X509_name_entry_st  HX509NameEntry_t;
typedef struct  X509_req_info_st    HX509ReqInfo_t;
typedef struct  X509_req_st         HX509Req_t;
typedef struct  evp_pkey_st         HttpsPrivKey_t;

/****************************************************************
    Certificate buffer structure for the pool
 *****************************************************************/

typedef struct  HttpsCertBody_s
{
  unsigned char                *pData;
  bool                 bHeap;
  struct HttpsCertBody_s  *pNext;
} HttpsCertBody_t;

/**
 * Certificate structure which holds the DER encoded certificate.
 */
typedef struct  HttpsCert
{
  HttpsCertBody_t  *pCert;
  uint16_t         usLength;
} HttpsCertificate_t;

/**
 * Structure which holds the CA name received in a certificate request
 * payload in a DER encoded format.
 */
typedef struct  HttpsCAName
{
  unsigned char     aCA[HTTPS_CERTIFICATE_LEN];
  uint16_t     usLength;
} HttpsCAName_t;

/**
 * Certificate revocation list structure which holds the CRL in DER format.
 */

/**
 * Structure which holds the node's certificate and the corresponding
 * private key.
 */
typedef struct  HttpsMyKeyCert
{
  HX509Cert_t     *pMyCert;
  HttpsPrivKey_t  *pMyPrivKey;
  bool         bUsed;
  char         aIDName[HTTPS_ID_NAME_LEN];
} HttpsMyKeyCert_t;

typedef struct  HttpsTrustedCACert
{
  HX509Cert_t  *pCACert;
  bool      bUsed;
  char      aIDName[HTTPS_ID_NAME_LEN];
} HttpsTrustedCACert_t;

typedef struct  HttpsCertinfo
{
  unsigned char  pSerialNum[32];
  uint8_t   usSerialNumLen;
  unsigned char  pIssuerName[1024];
  uint16_t  name_length;
} HttpsCertInfo_t;

typedef struct  HttpsCertReqInfo
{
  uint8_t  usCertType;
  union
  {
    struct
    {
      unsigned char  aSubjectName[HTTPS_MAX_NAME_LEN];
      uint16_t  usNumBits;
    } rsa_params;
  } cr_params;
} HttpsCertReqInfo_t;

typedef struct  HttpsCertIdInfo
{
  uint16_t  usIdType;
  uint16_t  usIdLength;
  char   aIdData[HTTPS_ID_DATA_LEN];
} HttpsCertIdInfo_t;

typedef struct  HttpsGetCert_s
{
  char  Name[9];
  char  Issuer[HTTPS_PARAMS_ISSUER_NAME_LEN];
  char  Sub[HTTPS_PARAMS_SUB_NAME_LEN];
  char  SerialNum[HTTPS_PARAMS_SERIAL_NUMBER_LEN];
  char  Expiry[HTTPS_PARAMS_EXPIRY_LEN];
} HttpsGetCert_t;

typedef struct  HttpsValcert_s
{
  unsigned char ucrectype;
  unsigned char ucenctype;
  unsigned char ucdefcert;
  unsigned char uctype;
  unsigned char ucsigntype;
} HttpsValCert_t;

typedef struct  Httpskeycertpair
{
  HTTPS_mblk_t  *pkey;      /* Points to  mblk for key.    */
  HTTPS_mblk_t  *pcert;     /* Points to  mblk for cert.   */
  HTTPS_mblk_t  *preq;      /* Points to  mblk for request */
  uint32_t      uikeycnt;   /* Count of bytes in key mblk  */
  uint32_t      uicertcnt;  /* Count of bytes in cert mblk */
  uint32_t      uireqcnt;   /* Count of bytes in req mblk  */
  bool       bValid;
  unsigned char      IdName[HTTPS_ID_NAME_LEN];
  HttpsValCert_t val;
} HttpsKeyCertPair_t;

typedef struct  HttpsCACert
{
  HTTPS_mblk_t    *phead;    /* Points to  mblk for key.    */
  uint32_t        uicertcnt; /* Count of bytes in cert mblk */
  bool         bValid;
  unsigned char        IdName[HTTPS_ID_NAME_LEN];
  HttpsValCert_t  val;
} HttpsCACertInfo_t;

typedef struct  HttpsCertReqParams_s
{
  unsigned char  aIdName[HTTPS_PARAMS_ID_NAME_LEN];  /* KeyId now*/
  unsigned char  aUserName[HTTPS_PARAMS_USER_NAME_LEN];
  unsigned char  ucSub[HTTPS_PARAMS_SUB_LEN];
  unsigned char  aOrg[HTTPS_PARAMS_ORGANIZATION_LEN];
  unsigned char  aDept[HTTPS_PARAMS_DEPT_NAME_LEN];
  unsigned char  aCity[HTTPS_PARAMS_CITY_NAME_LEN];
  unsigned char  aPostalCode[HTTPS_PARAMS_POSTAL_CODE_LEN];
  unsigned char  aState[HTTPS_PARAMS_STATE_NAME_LEN];
  unsigned char  aEmailId[HTTPS_PARAMS_EMAIL_ID_LEN];
  unsigned char  aDomain[HTTPS_PARAMS_DOMAIN_NAME_LEN];
  unsigned char  aIpAddr[HTTPS_PARAMS_IP_ADDRESS_LEN];
  unsigned char  ucCn[3];
  uint8_t   uiCert;
  uint16_t  uiNoBits;
} HttpsCertReqParams_t;

typedef struct  HttpsSelfSignedCertParams_st
{
  unsigned char  aSubName[HTTPS_PARAMS_SUB_LEN];
  unsigned char  aCountryCode[HTTPS_PARAMS_COUNTRY_LEN];
  unsigned char  aState[HTTPS_PARAMS_STATE_NAME_LEN];
  unsigned char  aOrgName[HTTPS_PARAMS_ORGANIZATION_LEN];
  unsigned char  aOrgUnitName[HTTPS_PARAMS_ORGANIZATION_LEN];
  unsigned char  aIpAddr[HTTPS_PARAMS_IP_ADDRESS_LEN];
  unsigned char  aDnsName[HTTPS_PARAMS_DOMAIN_NAME_LEN];
  unsigned char  aEmailId[HTTPS_PARAMS_SELF_EMAIL_ID_LEN];
  unsigned char  ucSigAlg;
  uint32_t  ulKeyLen;
} HttpsSelfSignedCertParams_t;

/**
 * This structure is used to send SelfSignedCertificate data to other
 * modules
 */
typedef struct  HttpsSelfSignedCert_st
{
  unsigned char  aPemSelfSignedCert[HTTPS_MAX_CERT_BODY_SIZE];
  unsigned char  aPemPrivKey[HTTPS_MAX_CERT_BODY_SIZE];
  bool   bResult;
} HttpsSelfSignedCert_t;

typedef struct  HttpsSelfSignDetails_st
{
  int32_t   lVersion;
  unsigned char  aSerialNumber[HTTPS_PARAMS_SERIAL_NUMBER_LEN];
  unsigned char  aSignatureAlgor[HTTPS_PARAMS_SIGNATURE_ALG_LEN];
  unsigned char  aIssuerName[HTTPS_PARAMS_ISSUER_NAME_LEN];
  unsigned char  aNotBefore[HTTPS_PARAMS_DATE_NOT_BEFORE_LEN];
  unsigned char  aNotAfter[HTTPS_PARAMS_DATE_NOT_AFTER_LEN];
  unsigned char  aSubName[HTTPS_PARAMS_SUB_NAME_LEN];
  unsigned char  aPubKeyAlgor[HTTPS_PARAMS_PUB_KEY_ALG_LEN];
  unsigned char  aSignature[HTTPS_PARAMS_SIGNATURE_LEN];
  int32_t   lSigLen;
  unsigned char  aSubAltNameExtns[HTTPS_PARAMS_SUB_ALT_NAME_LEN];

} HttpsSelfSignDetails_t;

#endif /* HTTPSGDEF_H */

