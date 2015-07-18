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
/*  File        : httpsapi.c                                              */
/*                                                                        */
/*  Description : This file contains https API implementation.            */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.1      05.11.04    DRAM      Modified during the code review.     */
/*    1.1      05.31.04    DRAM      Added functions, which are common    */
/*                                   across vi, vx, and lx of httpswrp.c  */
/*                                   file are added to this file.         */
/**************************************************************************/

#if defined(OF_HTTPD_SUPPORT) && defined(OF_HTTPS_SUPPORT)

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "httpdinc.h"
#include "cmutil.h"
#ifdef OF_HTTPS_SUPPORT
#include "httpswrp.h"
#endif /*OF_HTTPS_SUPPORT*/
#include <openssl/x509_vfy.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include "hcmgrmbl.h"
#include "hcmgrgdf.h"
#include "hcmgrgif.h"
#include "hcmgrldf.h"
#include "hcmgrlif.h"
#include <openssl/ssl.h>

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

static const char  *mon[12] =
{
  "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct",
  "Nov", "Dec"
};

enum AltNameTypes
{
  HTTPS_SAN_OTHERNAME = 0,
  HTTPS_SAN_RFC822NAME,
  HTTPS_SAN_DNSNAME,
  HTTPS_SAN_X400NAME,
  HTTPS_SAN_DIRECTORYNAME,
  HTTPS_SAN_EDIPARTYNAME,
  HTTPS_SAN_URI,
  HTTPS_SAN_IPADDR,
  HTTPS_SAN_REGISTEREDID
};

/************************************************************
 * * * *  V a r i a b l e    D e c l a r a t i o n s  * * * *
 ************************************************************/

uint32_t  HttpsMaxCAs = 0;

/**
 * External variable declarations
 */
extern uint32_t            HttpsPoolId;
extern HttpsKeyCertPair_t  HttpsKeyCertPair[HTTPS_MAX_NUM_OF_KEY_CERT];
extern HttpsCACertInfo_t   HttpsCACert[HTTPS_MAX_NUM_OF_CA_CERT];
extern HttpsMyKeyCert_t    HttpsRSAKeyPairs[HTTPS_PARAMS_RSA_KEY_PAIRS_LEN];
extern STACK_OF(X509)*     HttpsRSATrustedCACerts;

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 *
 */
 int32_t Https_generalized_time_print( char       *pExpiryDate,
                                      ASN1_UTCTIME  *tm );

/**
 *
 */

int32_t Https_utctime_print( char       *pExpiryDate,
                             ASN1_UTCTIME  *tm );

/**
 *
 */

int32_t HttpsGetSubAltType( char         *pSubName,
                            X509_EXTENSION  *pExt );

/**
 *
 */

int32_t HttpsVerifyKeyPair( HX509Cert_t  *pX509Cert,
                            EVP_PKEY     *pPrivKey,
                            uint32_t     ulKeyType );

/**
 *
 */
int32_t HttpsCheckCAName( unsigned char  *name_p,
                          uint16_t  *pStatus );

/**
 *
 */

int32_t HttpsPrintASN1Integer( char       *pInBuf,
                               ASN1_INTEGER  *a );
/**
 *
 */

int32_t HttpsUploadCert( unsigned char  uctype,
                         unsigned char  ucrectype,
                         unsigned char  ucenctype,
                         unsigned char  ucdefcert,
                         unsigned char  ucsigntype,
                         unsigned char  *pSelfbuf,
                         int32_t   length,
                         unsigned char  *name_p );
/**
 *
 */

int32_t HttpsUploadTrustedCert( unsigned char  uctype,
                                unsigned char  ucrectype,
                                unsigned char  ucenctype,
                                unsigned char  ucdefcert,
                                unsigned char  ucsigntype,
                                unsigned char  *pTrustbuf,
                                int32_t   length,
                                uint8_t   *pHandle,
                                uint32_t  ulHandleLength );
/*************************************************************************
 * Function Name : HttpsUploadCert                                       *
 * Description   : This function will actually write the                 *
 *                 self-certificate into the Mblocks.                    *
 * Input         : pSelfBuf is buffer                                    *
 * Output        :                                                       *
 * Return value  : OF_SUCCESS or OF_FAILURE.                               *
 *************************************************************************/

int32_t HttpsUploadCert( unsigned char  uctype,
                         unsigned char  ucrectype,
                         unsigned char  ucenctype,
                         unsigned char  ucdefcert,
                         unsigned char  ucsigntype,
                         unsigned char  *pSelfbuf,
                         int32_t   length,
                         unsigned char  *name_p )
{
  int32_t       theError = 0;
  uint32_t      ii, jj, ulKeyType;
  uint32_t      ulKeyLen, uiNewLen;
  unsigned char      *pCertOrKey = NULL, *pbuf = NULL;
  HTTPS_mblk_t  *pmblk;
  EVP_PKEY      *pPrivKey   = NULL;
  HX509Cert_t   *pX509Cert  = NULL;

  theError = HttpsFlashWrite(uctype, ucrectype, ucenctype, ucdefcert,
                             ucsigntype, pSelfbuf, length, name_p);

  if (theError != OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsUploadCert: Error in storing the cert in memory \n");
#endif /*HTTPD_DEBUG*/
    return theError;
  }

  for (ii = 0; ii < HTTPS_MAX_NUM_OF_KEY_CERT; ii++)
  {
    if (strcmp((char *)HttpsKeyCertPair[ii].IdName, (char *)name_p) == 0)
    {
      break;
    }
  }

  if (ii == HTTPS_MAX_NUM_OF_KEY_CERT)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
      "HttpsUploadCert: private key for this certificate not available\n");
#endif /*HTTPD_DEBUG*/
    return HTTPS_ERROR_NO_MATCHING_PRIV_KEY;
  }

  jj = ii;

  ulKeyLen =  HttpsKeyCertPair[jj].uikeycnt;
  pbuf = pCertOrKey = (unsigned char *)of_calloc(1, ulKeyLen + 2);

  if (pCertOrKey == NULL)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE, "HttpsUploadCert: Error in allocating memory\n");
#endif /*HTTPD_DEBUG*/
    HttpsFreeMsg(HttpsKeyCertPair[jj].pcert);
    HttpsKeyCertPair[jj].pcert = NULL;
    return OF_FAILURE;
  }

  pmblk    = HttpsKeyCertPair[jj].pkey;
  uiNewLen = MIN(ulKeyLen, 1024);

  of_memcpy(pbuf, pmblk->pRptr, uiNewLen);

  ulKeyLen -= uiNewLen;
  pbuf     += uiNewLen;

  while (ulKeyLen > 0)
  {
    pmblk    = pmblk->pCont;
    uiNewLen = MIN(ulKeyLen, 1024);
    of_memcpy(pbuf, pmblk->pRptr, uiNewLen);

    ulKeyLen -= uiNewLen;
    pbuf     += uiNewLen;
  }

  theError = HttpsReadFromFile(pCertOrKey, HTTPS_PRIV_KEY,
                               (void**)&pPrivKey,
                               HttpsKeyCertPair[jj].val.ucsigntype);

  if (theError != OF_SUCCESS)
  {
     HttpsFreeMsg(HttpsKeyCertPair[jj].pcert);
     HttpsKeyCertPair[jj].pcert = NULL;
     of_free(pCertOrKey);

     return theError;
  }

  if ((HttpsKeyCertPair[jj].val.ucsigntype == 'r')  ||
      (HttpsKeyCertPair[jj].val.ucsigntype == 'R'))
  {
    for (ii = 0; ii < 10; ii++)
    {
      if (HttpsRSAKeyPairs[ii].bUsed == FALSE)
      {
        break;
      }
    }

    if (ii != 10)
    {
      of_strcpy((char *)HttpsRSAKeyPairs[ii].aIDName, (char *)name_p);
      HttpsRSAKeyPairs[ii].pMyPrivKey = pPrivKey;
      HttpsRSAKeyPairs[ii].bUsed      = TRUE;
    }
  }

  theError = HttpsReadFromFile(pSelfbuf, HTTPS_CERT, (void**)&pX509Cert,
                               HttpsKeyCertPair[jj].val.ucsigntype);
  if (theError != OF_SUCCESS)
  {
    if ((HttpsKeyCertPair[jj].val.ucsigntype == 'r')  ||
        (HttpsKeyCertPair[jj].val.ucsigntype == 'R'))
    {
      EVP_PKEY_free(HttpsRSAKeyPairs[ii].pMyPrivKey);
      HttpsRSAKeyPairs[ii].bUsed = FALSE;
      HttpsRSAKeyPairs[ii].pMyPrivKey = NULL;
    }

    HttpsFreeMsg(HttpsKeyCertPair[jj].pcert);
    HttpsKeyCertPair[jj].pcert = NULL;
    X509_free(pX509Cert);
    of_free(pCertOrKey);

    return theError;
  }

  theError = HttpsVerifySelfCert(pX509Cert);

  if (theError != OF_SUCCESS)
  {
    if ((HttpsKeyCertPair[jj].val.ucsigntype == 'r')  ||
        (HttpsKeyCertPair[jj].val.ucsigntype == 'R'))
    {
      EVP_PKEY_free(HttpsRSAKeyPairs[ii].pMyPrivKey);
      HttpsRSAKeyPairs[ii].bUsed = FALSE;
      HttpsRSAKeyPairs[ii].pMyPrivKey = NULL;
    }

    HttpsFreeMsg(HttpsKeyCertPair[jj].pcert);
    HttpsKeyCertPair[jj].pcert = NULL;
    X509_free(pX509Cert);
    of_free(pCertOrKey);
    return theError;
  }

  ulKeyType = OBJ_obj2nid(pX509Cert->cert_info->key->algor->algorithm);

  theError = HttpsVerifyKeyPair(pX509Cert, pPrivKey, ulKeyType);

  if (theError != OF_SUCCESS)
  {
    if ((HttpsKeyCertPair[jj].val.ucsigntype == 'r')  ||
        (HttpsKeyCertPair[jj].val.ucsigntype == 'R'))
    {
      EVP_PKEY_free(HttpsRSAKeyPairs[ii].pMyPrivKey);
      HttpsRSAKeyPairs[ii].bUsed = FALSE;
      HttpsRSAKeyPairs[ii].pMyPrivKey = NULL;
    }

    HttpsFreeMsg(HttpsKeyCertPair[jj].pcert);
    HttpsKeyCertPair[jj].pcert = NULL;
    X509_free(pX509Cert);
    of_free(pCertOrKey);

    return theError;
  }

  if ((ulKeyType == EVP_PKEY_RSA) ||
      (ulKeyType == EVP_PKEY_RSA2))
  {
    HttpsRSAKeyPairs[ii].pMyCert = pX509Cert;
  }
  of_free(pCertOrKey);

  return OF_SUCCESS;
} /* HttpsUploadCert() */

/******************************************************************
 *  Function Name    : HttpsUploadTrustedCert
 *  Description      : This function will actually write the
 *                     Trusted certificate into the M-blocks.
 *  Input            : pTurstbuf is a buffer
 *  Output           :
 *  Return value     : OF_SUCCESS or OF_FAILURE.
 ******************************************************************/

 /* avs added pHandle, ulHandleLength, to get back the handle for the
     trusted Certificate  02/25 */

int32_t HttpsUploadTrustedCert( unsigned char  uctype,
                                unsigned char  ucrectype,
                                unsigned char  ucenctype,
                                unsigned char  ucdefcert,
                                unsigned char  ucsigntype,
                                unsigned char  *pTrustbuf,
                                int32_t   length,
                                unsigned char  *pHandle,
                                uint32_t  ulHandleLength )
{
  int32_t      theError = 0;
  uint32_t     ulKeyType, NumCAs = 0, Index=0;
  HX509Cert_t  *pTmpCert  = NULL;
  HX509Cert_t  *pX509Cert = NULL;
  unsigned char     aCaName[HTTPS_ID_NAME_LEN];
  bool      bDupCert = FALSE;

  STACK_OF(X509) *pTrustedCACerts = NULL;

  if (HttpsMaxCAs >= HTTPS_MAX_NUM_OF_CA_CERT)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpTrustedVal: Limit reached for Trusted Certs\n");
#endif /*HTTPD_DEBUG*/
    return HTTPS_TRUSTED_MAX_LIMIT;
  }

  of_memset(aCaName, 0, 8);

  theError = HttpsFlashWriteTrust(uctype, ucrectype, ucenctype, ucdefcert,
                                  ucsigntype, pTrustbuf, length, aCaName);

  if (theError != OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpTrustedVal: Error in storing the cert in memory\n");
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }

  theError = HttpsReadFromFile(pTrustbuf, HTTPS_CERT, (void**)&pX509Cert,
                               ucsigntype);

  if (theError != OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpTrustedVal: Error in extracting X509cert\n");
#endif /*HTTPD_DEBUG*/
    HttpsDeleteCerts(aCaName, 't');
    return OF_FAILURE;
  }

  ulKeyType = OBJ_obj2nid(pX509Cert->cert_info->key->algor->algorithm);

  if ((((ulKeyType == EVP_PKEY_RSA) ||
        (ulKeyType == EVP_PKEY_RSA2)) &&
        (ucsigntype == 'd'))   ||
      (((ulKeyType == EVP_PKEY_DSA)  ||
        (ulKeyType == EVP_PKEY_DSA2) ||
        (ulKeyType == EVP_PKEY_DSA3)) &&
        (ucsigntype == 'r')))
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE, "Invalid Signature Type  \n");
#endif /*HTTPD_DEBUG*/
    HttpsDeleteCerts(aCaName, 't');
    X509_free(pX509Cert);
    return HTTPS_INVALID_SIG_TYPE;
  }

  if (HttpsVerifyTrustedCert(pX509Cert) != OF_SUCCESS)
  {
    HttpsDeleteCerts(aCaName, 't');
    X509_free(pX509Cert);
    return OF_FAILURE;
  }

  pTrustedCACerts = HttpsRSATrustedCACerts;

  if (pTrustedCACerts != NULL)
  {
    NumCAs = sk_X509_num(pTrustedCACerts);
  }

  for (Index = 0; Index < NumCAs; Index++)
  {
    pTmpCert = (HX509Cert_t *) sk_X509_value(pTrustedCACerts, Index);

    if (X509_cmp(pTmpCert, pX509Cert) == 0)
    {
      bDupCert = TRUE;
      break;
    }
  }
  if (bDupCert == TRUE)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE, " Duplicate Trusted Cert Found \n");
#endif /*HTTPD_DEBUG*/
    HttpsDeleteCerts(aCaName, 't');
    X509_free(pX509Cert);
    return HTTPS_DUPLICATE_CA_CERT;
  }

  if ((ulKeyType == EVP_PKEY_RSA) ||
      (ulKeyType == EVP_PKEY_RSA2))
  {
    sk_push(/*(STACK *)*/(_STACK *)HttpsRSATrustedCACerts, (char *)pX509Cert);
#if 0
    #ifdef IGW_UCM_DP_CHANNEL_SUPPORT
    sk_push((STACK *)HttpsRSATrustedCACerts, (char *)pX509Cert);
    #else
    sk_push((_STACK *)HttpsRSATrustedCACerts, (char *)pX509Cert);
    #endif
    #endif
  }
  else
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE, "HttpTrustedVal: Cert Type invalid \n");
#endif /*HTTPD_DEBUG*/
    HttpsDeleteCerts(aCaName, 't');
    X509_free(pX509Cert);
    return OF_FAILURE;
  }

  HttpsMaxCAs++;
  of_strncpy((char *)pHandle, (char *)aCaName, ulHandleLength);

  return OF_SUCCESS;
} /* HttpsUploadTrustedCert() */

/******************************************************************
 *  Function Name    : HttpsDeleteCerts
 *  Description      : This is an API provided by the CertMgr
 *                     module.  It deletes the self or trusted
 *                     certificates stored in the M-blocks based
 *                     on the Certificate type and Id Name
 *  Input            : unsigned char * - The Id name of the certificate
 *                     unsigned char   - The certificate type.
 *  Output           : NONE
 *  Return value     : OF_SUCCESS or OF_FAILURE.
 ******************************************************************/

int32_t HttpsDeleteCerts( unsigned char  *pIdName,
                          unsigned char  ucCertType )
{
  int32_t   uiIndex;

  switch (ucCertType)
  {
  case 's':
    for (uiIndex = 0; uiIndex < HTTPS_MAX_NUM_OF_KEY_CERT; uiIndex++)
    {
      if (strcmp((char *)HttpsKeyCertPair[uiIndex].IdName, (char *)pIdName) == 0)
      {
        break;
      }
    }

    if (uiIndex == HTTPS_MAX_NUM_OF_KEY_CERT)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsDeleteCerts: No matching private key found\n");
#endif /*HTTPD_DEBUG*/
      return HTTPS_ERROR_NO_MATCHING_PRIV_KEY;
    }
    HttpsKeyCertPair[uiIndex].bValid = 0;
    HttpsFreeMsg(HttpsKeyCertPair[uiIndex].pkey);
    HttpsKeyCertPair[uiIndex].pkey = NULL;
    HttpsFreeMsg(HttpsKeyCertPair[uiIndex].pcert);
    HttpsKeyCertPair[uiIndex].pcert = NULL;
    HttpsFreeMsg(HttpsKeyCertPair[uiIndex].preq);
    HttpsKeyCertPair[uiIndex].preq = NULL;

    break;
  case 't':
    for (uiIndex = 0; uiIndex < HTTPS_MAX_NUM_OF_CA_CERT; uiIndex++)
    {
      if (strcmp((char *)HttpsCACert[uiIndex].IdName, (char *)pIdName)== 0)
      {
        break;
      }
    }
    if (uiIndex == HTTPS_MAX_NUM_OF_CA_CERT)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsDeleteCerts: No matching trusted certificate for deletion\n");
#endif /*HTTPD_DEBUG*/
      return HTTPS_TRUSTED_CA_CERT_NOT_FOUND;
    }
    HttpsCACert[uiIndex].bValid = 0;
    HttpsFreeMsg(HttpsCACert[uiIndex].phead);
    HttpsCACert[uiIndex].phead = NULL;
    of_memset(HttpsCACert[uiIndex].IdName, 0, 8);
    break;
  }

  return OF_SUCCESS;
} /* HttpsDeleteCerts() */

/******************************************************************
 *  Function Name    : HttpsDelMyCert
 *  Description      : This is an API provided by the CertMgr
 *                     module.  This will delete the selected
 *                     Self-Certificate based on the Id Name
 *  Input            : char * - The Self-certificate unique name
 *  Output           : NONE
 *  Return value     : OF_SUCCESS or OF_FAILURE.
 ******************************************************************/

int32_t HttpsDelMyCert( char  *pIDName )
{
  uint16_t          index;
  bool           bCertFound = FALSE;
  HttpsMyKeyCert_t  *aMyKeyPairs;
  EVP_PKEY          *pPrivKey   = NULL;
  HX509Cert_t       *pX509Cert  = NULL;

#ifdef HTTPD_DEBUG
  Trace(HTTPS_ID, TRACE_INFO, "HttpsDelMyCert : Entry \r\n");
#endif /*HTTPD_DEBUG*/

  aMyKeyPairs = HttpsRSAKeyPairs;

#ifdef HTTPD_DEBUG
  Trace(HTTPS_ID, TRACE_INFO, "HttpsDelMyCert:Starting RSA Pair\r\n");
#endif /*HTTPD_DEBUG*/

  for ((index) = 0;
       (index)<HTTPS_MAX_SELF_CERTS &&
       aMyKeyPairs[index].pMyCert != NULL &&
       aMyKeyPairs[index].pMyPrivKey != NULL;
       (index)++)
  {
    if (strcmp(pIDName, aMyKeyPairs[index].aIDName) == 0)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_INFO,
            "HttpsDelMyCert : Got Matching Pair\r\n");
#endif /*HTTPD_DEBUG*/
      bCertFound = TRUE;
      break;
    }
  } /* End of FOR */

  if (bCertFound == TRUE)
  {
    pX509Cert = aMyKeyPairs[index].pMyCert;
    pPrivKey = aMyKeyPairs[index].pMyPrivKey;

    for (;
          (index)<=(HTTPS_MAX_SELF_CERTS-2) &&
          aMyKeyPairs[index].pMyCert != NULL &&
          aMyKeyPairs[index].pMyPrivKey != NULL;
          (index)++)
    {
      aMyKeyPairs[index].pMyCert = aMyKeyPairs[index+1].pMyCert;
      aMyKeyPairs[index].pMyPrivKey = aMyKeyPairs[index+1].pMyPrivKey;
      aMyKeyPairs[index].bUsed = aMyKeyPairs[index+1].bUsed;
      of_memcpy(aMyKeyPairs[index].aIDName, aMyKeyPairs[index+1].aIDName,
               8);
    }
    aMyKeyPairs[index].pMyCert = NULL;
    aMyKeyPairs[index].pMyPrivKey = NULL;
    aMyKeyPairs[index].bUsed = FALSE;
    of_strcpy(aMyKeyPairs[index].aIDName, "");

    X509_free(pX509Cert);
    EVP_PKEY_free(pPrivKey);
    bCertFound = FALSE;
  }

#ifdef HTTPD_DEBUG
  Trace(HTTPS_ID, TRACE_INFO, "HttpsDelMyCert : Exit \r\n");
#endif /*HTTPD_DEBUG*/
  return OF_SUCCESS;
} /* HttpsDelMyCert() */

/******************************************************************
 *  Function Name    : HttpsDelTrustedCert
 *  Description      : This function is
 *  Input            :
 *  Output           : NONE
 *  Return value     : OF_SUCCESS or OF_FAILURE.
 ******************************************************************/

int32_t HttpsDelTrustedCert( char  *pIDName )
{
  uint16_t        NumCAs = 0;
  int32_t         uiRetVal = OF_SUCCESS, ii = 0;
  uint16_t        CAIndex;
  uint32_t        ulNewLen;
  bool         bFoundTrustedCert = FALSE;
  HX509Cert_t     *pCert            = NULL;
  STACK_OF(X509)  *pTrustedCACerts  = NULL;
  unsigned char        *pBuff            = NULL;
  HX509Cert_t     *pX509Cert        = NULL;
  HTTPS_mblk_t    *pTmp             = NULL;

#ifdef HTTPD_DEBUG
  Trace(HTTPS_ID, TRACE_INFO, "HttpsDelTrustedCert : Entry \r\n");
#endif /*HTTPD_DEBUG*/

  pTrustedCACerts = HttpsRSATrustedCACerts;

  if (pTrustedCACerts != NULL)
  {
    NumCAs = sk_X509_num(pTrustedCACerts);
  }

#ifdef HTTPD_DEBUG
  Trace(HTTPS_ID, TRACE_INFO,
        "HttpsDelTrustedCert:Started for RSA Trusted\r\n");
#endif /*HTTPD_DEBUG*/

  for (CAIndex = 0; (CAIndex < HTTPS_MAX_NUM_OF_CA_CERT); CAIndex++)
  {
    if (strcmp((char *)pIDName, (char *)HttpsCACert[CAIndex].IdName) == 0)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_INFO,
            "HttpsDelTrustedCert:Got Matching Trusted Cert\r\n");
#endif /*HTTPD_DEBUG*/
      HttpsCACert[CAIndex].bValid = FALSE;
      break;
    }
  }

  if (CAIndex == HTTPS_MAX_NUM_OF_CA_CERT)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsDelTrustedCert: No matching trusted certificate for deletion\n");
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }

  pBuff = (unsigned char *) of_calloc(1, HttpsCACert[CAIndex].uicertcnt + 2);

  if (pBuff == NULL)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          " HttpsDelTrustedCert: Error in allocation memory\n");
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }

  of_memcpy(pBuff, HttpsCACert[CAIndex].phead->pRptr,
           MIN(HttpsCACert[CAIndex].uicertcnt, 1024));

  ii = 1024;

  pTmp = HttpsCACert[CAIndex].phead->pCont;

  while ((ii < HttpsCACert[CAIndex].uicertcnt) &&
         (pTmp != NULL))
  {
    ulNewLen = MIN((HttpsCACert[CAIndex].uicertcnt- ii), 1024);
    of_memcpy(pBuff + ii, pTmp->pRptr, ulNewLen);
    pTmp = pTmp->pCont;
    ii += ulNewLen;
  }

  uiRetVal = HttpsReadFromFile(pBuff, HTTPS_CERT,
                               (void**)&pX509Cert, 'r');

  if (uiRetVal != OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsDelTrustedCert : Error in extracting X509cert\n");
#endif /*HTTPD_DEBUG*/
    of_free(pBuff);
    return OF_FAILURE;
  }
  for (CAIndex = 0; CAIndex < NumCAs; CAIndex++)
  {
    pCert = (HX509Cert_t *) sk_X509_value(pTrustedCACerts, CAIndex);

    if (X509_issuer_and_serial_cmp(pCert, pX509Cert) == 0)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_INFO,
            "HttpsDelTrustedCert: Get the matching CA Cert\r\n");
#endif /*HTTPD_DEBUG*/
      bFoundTrustedCert = TRUE;
      break;
    }
  }

  if (bFoundTrustedCert == TRUE)
  {
    X509_free((X509*)sk_delete_ptr(/*(STACK *)*/(_STACK *)pTrustedCACerts,
                                   (char *)pCert));
#if 0
    #ifdef IGW_UCM_DP_CHANNEL_SUPPORT
    X509_free((X509*)sk_delete_ptr((STACK *)pTrustedCACerts,
                                   (char *)pCert));
    #else
     X509_free((X509*)sk_delete_ptr((_STACK *)pTrustedCACerts,
                                   (char *)pCert));
    #endif
#endif
    bFoundTrustedCert = FALSE;
  }
  HttpsMaxCAs--;
  X509_free(pX509Cert);
  of_free(pBuff);
#ifdef HTTPD_DEBUG
  Trace(HTTPS_ID, TRACE_INFO, "HttpsDelTrustedCert: Exit\r\n");
#endif /*HTTPD_DEBUG*/
  return OF_SUCCESS;
} /* HttpsDelTrustedCert() */

/******************************************************************
 *  Function Name    : HttpsDelPrivateKey
 *  Description      :                                            *
 *  Input            :
 *  Output           : NONE                                       *
 *  Return value     : OF_SUCCESS or OF_FAILURE.                    *
 ******************************************************************/

int32_t HttpsDelPrivateKey( char  *pIDName )
{
  uint16_t          index;
  bool           bCertFound = FALSE;
  HttpsMyKeyCert_t  *aMyKeyPairs;
  EVP_PKEY          *pPrivKey   = NULL;

#ifdef HTTPD_DEBUG
  Trace(HTTPS_ID, TRACE_INFO, "HttpsDelPrivateKey: Entry\r\n");
#endif /*HTTPD_DEBUG*/

  aMyKeyPairs = HttpsRSAKeyPairs;
#ifdef HTTPD_DEBUG
  Trace(HTTPS_ID, TRACE_INFO,
        "HttpsDelPrivateKey:Starting RSA Private Key\r\n");
#endif /*HTTPD_DEBUG*/

  for ((index) = 0; (index)<HTTPS_MAX_SELF_CERTS; (index)++)
  {
    if (strcmp(pIDName, aMyKeyPairs[index].aIDName) == 0)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_INFO,
            "HttpsDelPrivateKey:Got Matching Private key\r\n");
#endif /*HTTPD_DEBUG*/
      bCertFound = TRUE;
      break;
    }
  }

  if (bCertFound == TRUE)
  {
    pPrivKey = aMyKeyPairs[index].pMyPrivKey;

    for (; (index)<=(HTTPS_MAX_SELF_CERTS-2); (index)++)
    {
      aMyKeyPairs[index].pMyCert = aMyKeyPairs[index+1].pMyCert;
      aMyKeyPairs[index].pMyPrivKey = aMyKeyPairs[index+1].pMyPrivKey;
      of_memcpy(aMyKeyPairs[index].aIDName, aMyKeyPairs[index+1].aIDName,
               8);
    }
    aMyKeyPairs[index].pMyPrivKey = NULL;
    of_strcpy(aMyKeyPairs[index].aIDName, "");
    EVP_PKEY_free(pPrivKey);
    bCertFound = FALSE;
  }

#ifdef HTTPD_DEBUG
  Trace(HTTPS_ID, TRACE_INFO, "HttpsDelPrivateKey : Exit \r\n");
#endif /*HTTPD_DEBUG*/
  return OF_SUCCESS;
} /* HttpsDelPrivateKey() */

/***************************************************************************
 * Function Name : HttpsGetPrivCert
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 ***************************************************************************/

int32_t HttpsGetPrivCert( HttpsKeyCertPair_t  *pMyKeyPairs )
{
  int32_t  index;

  for ((index) = 0; (index) < HTTPS_MAX_NUM_OF_KEY_CERT; (index)++)
  {
    pMyKeyPairs[index] = HttpsKeyCertPair[index];
  }
  return OF_SUCCESS;
} /* HttpsGetPrivCert() */

/***************************************************************************
 * Function Name : HttpsVerifySelfCert
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 ***************************************************************************/

int32_t HttpsVerifySelfCert( HX509Cert_t  *pCert )
{
  HX509Store_t         *pStore = NULL;
  HX509StoreContext_t  *StoreContext;
  uint32_t             ulKeyType = 0;
  uint32_t             lNumOfCerts = 0, ii = 0;
  STACK_OF(X509)       *pTrustedCerts = NULL;

#ifdef HTTPD_DEBUG
  Trace(HTTPS_ID, TRACE_SEVERE, "HttpsVerifySelfCert:: Entry\n\r");
#endif /*HTTPD_DEBUG*/

  StoreContext = X509_STORE_CTX_new();

  if (StoreContext == NULL)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsVerifyCert : Creation of StoreContext failed\n\r");
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }

  if ((pStore = (HX509Store_t *) X509_STORE_new()) == NULL)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsVerifyCert : Creation of X509Store failed\n\r");
#endif /*HTTPD_DEBUG*/
    X509_STORE_CTX_free(StoreContext);
    return OF_FAILURE;
  }

  ulKeyType = OBJ_obj2nid(pCert->cert_info->key->algor->algorithm);

  if ((ulKeyType == EVP_PKEY_RSA) ||
      (ulKeyType == EVP_PKEY_RSA2))
  {
    pTrustedCerts = HttpsRSATrustedCACerts;
  }
  else
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsVerifySelfCert::Invalid SIGNATURE Type\n\r");
#endif /*HTTPD_DEBUG*/
  }

  X509_STORE_CTX_init(StoreContext, pStore, pCert, NULL);

  if (pTrustedCerts != NULL)
  {
    lNumOfCerts = sk_X509_num(pTrustedCerts);

    for (ii = 0; ii < lNumOfCerts; ii++)
    {
      if (X509_STORE_add_cert(pStore,
                              sk_X509_value(pTrustedCerts, ii)) == 0)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HTTPSVerifySelfCert :: X509_STORE_add_cert failed\n\r");
#endif /*HTTPD_DEBUG*/
        X509_STORE_free(pStore);
        X509_STORE_CTX_free(StoreContext);
        return OF_FAILURE;
      }
    }
  }
  else
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HTTPSVerifySelfCert: pTrustedCerts is NULL\n\r");
#endif /*HTTPD_DEBUG*/
    X509_STORE_free(pStore);
    X509_STORE_CTX_free(StoreContext);
    return OF_FAILURE;
  }
  if (X509_verify_cert(StoreContext) != 1)
  {
    X509_STORE_free(pStore);
    X509_STORE_CTX_free(StoreContext);
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsVerifySelfCert: X509_verify_cert failed \n\r");
#endif /*HTTPD_DEBUG*/
    return HTTPS_ERROR_VERIFY_CERT_FAILED;
  }
  else
  {  ;  }

  X509_STORE_free(pStore);
  X509_STORE_CTX_free(StoreContext);
#ifdef HTTPD_DEBUG
  Trace(HTTPS_ID, TRACE_SEVERE,
        "HttpsVerifySelfCert: return SUCCESS :: Exit\n\r");
#endif /*HTTPD_DEBUG*/
  return OF_SUCCESS;
} /* HttpsVerifySelfCert() */

/***************************************************************************
 * Function Name : HttpsVerifyKeyPair()
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  :
 ***************************************************************************/

int32_t HttpsVerifyKeyPair( HX509Cert_t  *pX509Cert,
                            EVP_PKEY     *pPrivKey,
                            uint32_t     ulKeyType )
{
  int16_t   len;
  char   ct[]     = "This the clear text";
  char   *buf     = NULL;
  char   *buf2    = NULL;
  EVP_PKEY  *pPubKey = NULL;

  if ((ulKeyType == EVP_PKEY_RSA) ||
      (ulKeyType == EVP_PKEY_RSA2))
  {
    pPubKey = X509_extract_key(pX509Cert);

    if (pPubKey == NULL)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsVerifyKeyPair:RSA::X509_extract_key failed\n\r");
#endif /*HTTPD_DEBUG*/
      return OF_FAILURE;
    }

    buf = of_malloc(EVP_PKEY_size(pPubKey));

    if (buf)
    {
      of_memset(buf, 0, EVP_PKEY_size(pPubKey));
    }

    buf2 = of_malloc(EVP_PKEY_size(pPubKey));

    if (buf2)
    {
      of_memset(buf2, 0, EVP_PKEY_size(pPubKey));
    }

    if ((buf == NULL) ||
        (buf2 == NULL))
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsVerifyKeyPair:RSA::malloc failed\n\r");
#endif /*HTTPD_DEBUG*/

      if (pPubKey)
      {
        EVP_PKEY_free(pPubKey);
      }

      if (buf)
      {
        of_free(buf);
      }

      if (buf2)
      {
        of_free(buf2);
      }

      return OF_FAILURE;
    }

    len = RSA_public_encrypt(strlen(ct)+1, (unsigned char*) ct, (unsigned char*) buf, pPubKey->pkey.rsa,
                             RSA_PKCS1_PADDING);

    if (len != EVP_PKEY_size(pPubKey))
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsVerifyKeyPair:RSA:: EVP_PKEY_size fail \n\r");
#endif /*HTTPD_DEBUG*/

      if (pPubKey)
      {
        EVP_PKEY_free(pPubKey);
      }

      of_free(buf);
      of_free(buf2);
      return OF_FAILURE;
    }

    RSA_private_decrypt(len, (unsigned char *)buf, (unsigned char *)buf2, pPrivKey->pkey.rsa,
                        RSA_PKCS1_PADDING);

    if (strcmp(ct, buf2) == 0)
    {
      if (pPubKey)
      {
        EVP_PKEY_free(pPubKey);
      }

      of_free(buf);
      of_free(buf2);
      return OF_SUCCESS;
    }
    else
    {
      if (pPubKey)
      {
        EVP_PKEY_free(pPubKey);
      }

      of_free(buf);
      of_free(buf2);
      return OF_FAILURE;
    }
  }
  return OF_FAILURE;
} /* HttpsVerifyKeyPair() */

/***************************************************************************
 * Function Name : HttpsVerifyTrustedCert
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 ***************************************************************************/

int32_t HttpsVerifyTrustedCert( HX509Cert_t  *pCert )
{
  int32_t i;

  ASN1_UTCTIME *tmpTime;

  if (pCert == NULL)
  {
    return OF_FAILURE;
  }

  tmpTime = X509_get_notBefore(pCert);

  i = X509_cmp_current_time(tmpTime);

  if (i == 0)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "VerifyTrustedCert:: error in certificate's notBefore field\n");
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }

  if (i > 0)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "VerifyTrustedCert:: certificate is not yet valid\n");
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }

  i=X509_cmp_current_time(X509_get_notAfter(pCert));

  if (i == 0)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "VerifyTrustedCert:: error in certificate's notAfter field\n");
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }

  if (i < 0)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "VerifyTrustedCert:: certificate has expired\n");
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }
  return OF_SUCCESS;
} /* HttpsVerifyTrustedCert() */

/******************************************************************
 *  Function Name    :  HttpsFlashWrite
 *  Description      :
 *  Input            :
 *  Output           :
 *  Return value     : OF_SUCCESS or OF_FAILURE.
 ******************************************************************/

int32_t HttpsFlashWrite( unsigned char  uctype,
                         unsigned char  ucrectype,
                         unsigned char  ucenctype,
                         unsigned char  ucdefcert,
                         unsigned char  ucsigntype,
                         unsigned char  *pbuf,
                         int32_t   length,
                         unsigned char  *pCaName )
{
  uint32_t      uiNewLen;
  int32_t       uiIndex = 0;
  HTTPS_mblk_t  *pmblk = NULL;

  switch (ucrectype)
  {
  case 'p':
    {
      for (uiIndex = 0; uiIndex < HTTPS_MAX_NUM_OF_KEY_CERT; uiIndex++)
      {
        if (!HttpsKeyCertPair[uiIndex].bValid)
        {
          break;
        }
      }

      if (uiIndex == HTTPS_MAX_NUM_OF_KEY_CERT)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
        "Flashwrite: key-cert pair allowed is only HTTPS_MAX_NUM_OF_KEY_CERT\n");
#endif /*HTTPD_DEBUG*/
        return HTTPS_ERROR_MAX_SELF_CERTS;
      }

      HttpsKeyCertPair[uiIndex].pkey = pmblk = HttpsAllocbFromPool(HttpsPoolId,
                                                                   1024);

      if (pmblk == NULL)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "Flashwrite: Allocation of memory block for private key failed\n");
#endif /*HTTPD_DEBUG*/
        return HTTPS_MEM_ALLOC_FAILED;
      }

      HttpsKeyCertPair[uiIndex].bValid         = 1;
      HttpsKeyCertPair[uiIndex].val.ucrectype  = ucrectype;
      HttpsKeyCertPair[uiIndex].val.ucenctype  = ucenctype;
      HttpsKeyCertPair[uiIndex].val.ucdefcert  = ucdefcert;
      HttpsKeyCertPair[uiIndex].val.uctype     = uctype;
      HttpsKeyCertPair[uiIndex].val.ucsigntype = ucsigntype;
      HttpsKeyCertPair[uiIndex].uikeycnt       = length;
      strncpy((char *)HttpsKeyCertPair[uiIndex].IdName, (char *)pCaName, HTTPS_ID_NAME_LEN-1);
      HttpsKeyCertPair[uiIndex].IdName[HTTPS_ID_NAME_LEN-1] = '\0';
      break;
    }

  case 'r':
    {
      for (uiIndex = 0; uiIndex < HTTPS_MAX_NUM_OF_KEY_CERT; uiIndex++)
      {
        if (strcmp((char *)HttpsKeyCertPair[uiIndex].IdName, (char *)pCaName) == 0)
        {
          break;
        }
      }

      if (uiIndex == HTTPS_MAX_NUM_OF_KEY_CERT)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
        "Flashwrite: key-cert pair allowed is only HTTPS_MAX_NUM_OF_KEY_CERT\n");
#endif /*HTTPD_DEBUG*/
        return HTTPS_ERROR_MAX_SELF_CERTS;
      }

      if (HttpsKeyCertPair[uiIndex].preq != NULL)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
        "Flashwrite:Certificate request already exists for the private key %s\n",
              pCaName);
#endif /*HTTPD_DEBUG*/
        return OF_FAILURE;
      }

      HttpsKeyCertPair[uiIndex].preq = pmblk = HttpsAllocbFromPool(HttpsPoolId,
                                                                   1024);

      if (pmblk == NULL)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
        "Flashwrite: Allocation of memory block for private key failed\n");
#endif /*HTTPD_DEBUG*/
        return HTTPS_MEM_ALLOC_FAILED;
      }

      HttpsKeyCertPair[uiIndex].uireqcnt = length;

      break;
    }

  case 'c':
    {
      for (uiIndex = 0; uiIndex < HTTPS_MAX_NUM_OF_KEY_CERT; uiIndex++)
      {
        if (strcmp((char *)HttpsKeyCertPair[uiIndex].IdName, (char *)pCaName) == 0)
        {
          break;
        }
      }

      if (uiIndex == HTTPS_MAX_NUM_OF_KEY_CERT)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
          "Flashwrite: private key for this certificate not available\n");
#endif /*HTTPD_DEBUG*/
        return HTTPS_ERROR_NO_MATCHING_PRIV_KEY;
      }

      if (HttpsKeyCertPair[uiIndex].pcert != NULL)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
        "Flashwrite:Public Key is already exists for the private key %s\n",
              pCaName);
#endif /*HTTPD_DEBUG*/
        return OF_FAILURE;
      }

      HttpsKeyCertPair[uiIndex].uicertcnt = length;
      HttpsKeyCertPair[uiIndex].pcert= pmblk = HttpsAllocbFromPool(HttpsPoolId,
                                                                   1024);
      if (pmblk == NULL)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
        "Flashwrite: Allocation of memory block for certificate falied\n");
#endif /*HTTPD_DEBUG*/
        return HTTPS_MEM_ALLOC_FAILED;
      }
      break;
    }
  }

  uiNewLen = MIN(length, 1024);
  if(pmblk == NULL)
  {
#ifdef HTTPD_DEBUG
     Trace(HTTPS_ID, TRACE_SEVERE,"Flashwrite:pmblk is NULL %s\n");
#endif /*HTTPD_DEBUG*/
     return OF_FAILURE;
  }
  of_memcpy(pmblk->pWptr, pbuf, uiNewLen);

  pmblk->pWptr += uiNewLen;
  length       -= uiNewLen;
  pbuf         += uiNewLen;

  while (length > 0)
  {
    pmblk->pCont = HttpsAllocbFromPool(HttpsPoolId, 1024);

    if (pmblk->pCont == NULL)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "Flashwrite: Allocation of memory block  falied\n");
#endif /*HTTPD_DEBUG*/
      switch (ucrectype)
      {
      case 'p':
        HttpsKeyCertPair[uiIndex].bValid = 0;
        pmblk = HttpsKeyCertPair[uiIndex].pkey;
        HttpsKeyCertPair[uiIndex].pkey = NULL;
        break;
      case 'r':
        pmblk = HttpsKeyCertPair[uiIndex].preq;
        HttpsKeyCertPair[uiIndex].preq = NULL;
        break;
      case 'c':
        pmblk = HttpsKeyCertPair[uiIndex].pcert;
        HttpsKeyCertPair[uiIndex].pcert = NULL;
        break;
      }
      HttpsFreeMsg(pmblk);
      return HTTPS_MEM_ALLOC_FAILED;
    }
    pmblk    = pmblk->pCont;
    uiNewLen = MIN(length, 1024);
    of_memcpy(pmblk->pWptr, pbuf, uiNewLen);

    pmblk->pWptr += uiNewLen;
    length       -= uiNewLen;
    pbuf         += uiNewLen;
  }
  return OF_SUCCESS;
} /* HttpsFlashWrite() */

/******************************************************************
 *  Function Name    :  HttpsFlashWriteTrust
 *  Description      :
 *  Input            :
 *  Output           :
 *  Return value     : OF_SUCCESS or OF_FAILURE.
 ******************************************************************/

int32_t HttpsFlashWriteTrust( unsigned char  uctype,
                              unsigned char  ucrectype,
                              unsigned char  ucenctype,
                              unsigned char  ucdefcert,
                              unsigned char  ucsigntype,
                              unsigned char  *pbuf,
                              int32_t   length,
                              unsigned char  *pCaName )
{
  uint32_t      uiNewLen;
  uint32_t      uiIndex;
  HTTPS_mblk_t  *pmblk;

  for (uiIndex = 0; uiIndex < HTTPS_MAX_NUM_OF_CA_CERT; uiIndex++)
  {
    if (!HttpsCACert[uiIndex].bValid)
    {
      break;
    }
  }

  if (uiIndex == HTTPS_MAX_NUM_OF_CA_CERT)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
    "FlashwriteTrust:  Maximum CA cert allowed is only HTTPS_MAX_NUM_OF_CA_CERT\n");
#endif /*HTTPD_DEBUG*/
    return HTTPS_TRUSTED_MAX_LIMIT;
  }

  HttpsCACert[uiIndex].phead= pmblk = HttpsAllocbFromPool(HttpsPoolId,
                                                          1024);

  if (pmblk == NULL)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "FlashwriteTrust: Allocation of memory block  falied\n");
#endif /*HTTPD_DEBUG*/
    return HTTPS_MEM_ALLOC_FAILED;
  }

  HttpsCACert[uiIndex].bValid         = 1;
  HttpsCACert[uiIndex].val.ucrectype  = ucrectype;
  HttpsCACert[uiIndex].val.ucenctype  = ucenctype;
  HttpsCACert[uiIndex].val.ucdefcert  = ucdefcert;
  HttpsCACert[uiIndex].val.uctype     = uctype;
  HttpsCACert[uiIndex].val.ucsigntype = ucsigntype;
  HttpsCACert[uiIndex].uicertcnt      = length;

  sprintf((char *)HttpsCACert[uiIndex].IdName, "CA-%ld", (long int) uiIndex);
  strcpy((char *)pCaName, (char *)HttpsCACert[uiIndex].IdName);
  uiNewLen = MIN(length, 1024);
  memset(pmblk->pData->pDbBase, 0, 1024);
  of_memcpy(pmblk->pWptr, pbuf, uiNewLen);
  pmblk->pWptr += uiNewLen;
  length       -= uiNewLen;
  pbuf         += uiNewLen;

  while (length > 0)
  {
    pmblk->pCont = HttpsAllocbFromPool(HttpsPoolId, 1024);

    if (pmblk->pCont == NULL)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "FlashwriteTrust: Allocation of memory block failed\n");
#endif /*HTTPD_DEBUG*/
      HttpsFreeMsg(HttpsCACert[uiIndex].phead);
      HttpsCACert[uiIndex].phead = NULL;
      HttpsCACert[uiIndex].bValid = 0;

      return HTTPS_MEM_ALLOC_FAILED;
    }
    pmblk    = pmblk->pCont;
    uiNewLen = MIN(length, 1024);
    memset(pmblk->pData->pDbBase, 0, 1024);
    of_memcpy(pmblk->pWptr, pbuf, uiNewLen);

    pmblk->pWptr += uiNewLen;
    length       -= uiNewLen;
    pbuf         += uiNewLen;
  }
  return OF_SUCCESS;
} /* HttpsFlashWriteTrust() */

/*********************************************************************
 * Function Name : HttpsDoGenCertReq
 * Description   : This is an API provided by the CertMgr for the
 *                 generation of the Certificate Request.
 * Input         : HttpsCertReqParams_t * - The certificate request parameters
 * Output        : None
 * Return value  : 0 on success and -1 on failure
**********************************************************************/

int32_t HttpsDoGenCertReq( HttpsCertReqParams_t  *pParams )
{
  HttpsCertReqInfo_t  CertReqInfo;
  unsigned char            *pPrivbuf = NULL;
  unsigned char            *pCertbuf = NULL;

  /* static unsigned char seed[20] = {0xd5, 0x01, 0x4e, 0x4b, 0x60,
                                   0xef, 0x2b, 0xa8, 0xb6, 0x21,
                                   0x1b, 0x40, 0x62, 0xba, 0x32,
                                   0x24, 0xe0, 0x42, 0x7d, 0xd3}; */

  of_memset(&CertReqInfo, 0, sizeof(HttpsCertReqInfo_t));

  CertReqInfo.usCertType = pParams->uiCert;

  /**
   * Bits used in generating Public key..
   */
  if ((CertReqInfo.usCertType == RSA_MD5) ||
      (CertReqInfo.usCertType == RSA_SHA1))
  {
    CertReqInfo.cr_params.rsa_params.usNumBits = (uint16_t)
                                                      pParams->uiNoBits;

    if (CertReqInfo.cr_params.rsa_params.usNumBits == 0)
    {
      CertReqInfo.cr_params.rsa_params.usNumBits = 1024;
    }

    if ((CertReqInfo.cr_params.rsa_params.usNumBits != 2048) &&
        (CertReqInfo.cr_params.rsa_params.usNumBits != 1024) &&
        (CertReqInfo.cr_params.rsa_params.usNumBits != 512))
    {
      return OF_FAILURE;
    }
  }

  if ((pPrivbuf = (unsigned char *)of_malloc(4096)) == NULL)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE, "HttpsDoGenCertReq: of_malloc failed\n");
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }

  if ((pCertbuf = (unsigned char *)of_malloc(4096)) == NULL)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsDoGenCertReq : of_malloc failed\n");
#endif /*HTTPD_DEBUG*/
    of_free(pPrivbuf);
    return OF_FAILURE;
  }

  of_memset(pPrivbuf, '\0', 4096);
  of_memset(pCertbuf, '\0', 4096);

  if (HttpsGenCertReq(&CertReqInfo, pPrivbuf, pCertbuf,
                      pParams) !=OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsDoGenCertReq : HttpsGenCertReq failed\n");
#endif /*HTTPD_DEBUG*/
    of_free(pPrivbuf);
    of_free(pCertbuf);
    return OF_FAILURE;
  }

  of_free(pPrivbuf);
  of_free(pCertbuf);

  return OF_SUCCESS;
} /* HttpsDoGenCertReq() */

/********************************************************************
 * Function Name : HttpsPrintASN1Integer
 * Description   :
     *  This function is used to convert the given ASN1 Integer into
     *  the char format.
 * Input         : ASN1_INTEGER* - Pointer to ASN1 integer
 * Output        : char*      - Integer in char format
 * Return value  : Size of the ASN1 integer in char format
********************************************************************/


int32_t HttpsPrintASN1Integer( char       *pInBuf,
                               ASN1_INTEGER  *a )
{
  int32_t               i, j, n = 0;
  static const char  h[] = "0123456789ABCDEF";

  if (a == NULL)
  {
    return(0);
  }

  if (a->length == 0)
  {
    n = 0;
  }
  else
  {
    for (j = 0, i = 0; i < a->length; j += 2, i++)
    {
      pInBuf[j]   = h[((unsigned char)a->data[i]>>4)&0x0f];
      pInBuf[j+1] = h[((unsigned char)a->data[i]   )&0x0f];
      n += 2;
    }
  }
  pInBuf[n] = '\0';
  return(n);
} /* HttpsPrintASN1Integer() */

/******************************************************************
 *  Function Name    : HttpsGetSelfCerts
 *  Description      :
 *  Input            :
 *  Output           :
 *  Return value     : OF_SUCCESS or OF_FAILURE.
 ******************************************************************/

int32_t HttpsGetSelfCerts( HttpsGetCert_t  *pGetCerts )
{
  HttpsMyKeyCert_t  *aMyKeyPairs;
  int16_t           iItem = 0, count = 0;
  HX509Cert_t       *pTmpCert        = NULL;
  ASN1_TIME         *tm;
  HX509Extension_t  *pExt = NULL;
  uint16_t          index;
  unsigned char          aSubName[HTTPS_PARAMS_SUB_NAME_LEN];

  memset(aSubName, 0, HTTPS_PARAMS_SUB_NAME_LEN);
  aMyKeyPairs = HttpsRSAKeyPairs;

  for (index = 0;
         ((index < HTTPS_MAX_SELF_CERTS) &&
         (aMyKeyPairs[index].pMyCert != NULL) &&
         (aMyKeyPairs[index].pMyPrivKey != NULL) &&
         (count < (MAX_CERT_BUF_SIZE - 1)));
         index++)
  {
    pTmpCert = aMyKeyPairs[index].pMyCert;
    strcpy((pGetCerts+count)->Name, aMyKeyPairs[index].aIDName);

    /**
     * Extracting the serial number also, so that revoking
     * becomes easier.
     *                  Suram (July 18, 01)
     */
    HttpsPrintASN1Integer((pGetCerts+count)->SerialNum,
                          pTmpCert->cert_info->serialNumber);

    iItem = X509v3_get_ext_by_NID
                (aMyKeyPairs[index].pMyCert->cert_info->extensions,
                 NID_subject_alt_name, -1);

    if (iItem < 0)
    {
      X509_NAME_oneline(X509_get_subject_name(pTmpCert), (char *) aSubName, 256);
      strcpy((pGetCerts+count)->Sub, (char *) aSubName);
    }
    else
    {
      pExt = (X509_EXTENSION *) sk_X509_EXTENSION_value
                        (aMyKeyPairs[index].pMyCert->cert_info->extensions,
                         iItem);
      HttpsGetSubAltType((pGetCerts+count)->Sub, pExt);
    }
    tm = X509_get_notAfter(pTmpCert);

    if (tm->type == V_ASN1_UTCTIME)
    {
      Https_utctime_print((pGetCerts+count)->Expiry, tm);
    }
    else if (tm->type == V_ASN1_GENERALIZEDTIME)
    {
      Https_generalized_time_print((pGetCerts+count)->Expiry, tm);
    }
    else
    {
      strcpy((pGetCerts+count)->Expiry, "Bad time value");
    }

    X509_NAME_oneline(X509_get_issuer_name(aMyKeyPairs[index].pMyCert),
                      (pGetCerts+count)->Issuer, 256);
    count++;
  }

  strcpy((pGetCerts+count)->Name, "end");
  return OF_SUCCESS;
} /* HttpsGetSelfCerts() */

/******************************************************************
 *  Function Name    : HttpsGetTrustedCerts
 *  Description      :
 *  Input            :
 *  Output           :
 *  Return value     : OF_SUCCESS or OF_FAILURE.
 ******************************************************************/

int32_t HttpsGetTrustedCerts( HttpsGetCert_t  *pGetCerts )
{
  uint16_t      Index = 0;
  uint32_t      ulNewLen  = 0;
  HX509Cert_t   *pTmpCert = NULL;
  ASN1_TIME     *tm = NULL;
  unsigned char      *pBuff = NULL;
  int32_t       ii = 0, iRetVal;
  HTTPS_mblk_t  *pTmp = NULL;
  int8_t        ucCnt = 0;

  for (Index = 0; Index < HTTPS_MAX_NUM_OF_CA_CERT; Index++)
  {
    /**
     * Copy the certificate data from mblk into a buffer.
     */
    if (HttpsCACert[Index].bValid == TRUE)
    {
      of_memset((pGetCerts+ucCnt), 0, sizeof(HttpsGetCert_t));

      pBuff = (unsigned char *)of_calloc(1, HttpsCACert[Index].uicertcnt+2);

      if (pBuff == NULL)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsGetTrustedCert : Error in allocation memory \n");
#endif /*HTTPD_DEBUG*/
        return OF_FAILURE;
      }

      of_memcpy(pBuff, HttpsCACert[Index].phead->pRptr,
               MIN(HttpsCACert[Index].uicertcnt, 1024));

      ii = 1024;

      pTmp = HttpsCACert[Index].phead->pCont;

      while ((ii < HttpsCACert[Index].uicertcnt) &&
             (pTmp != NULL))
      {
        ulNewLen = MIN((HttpsCACert[Index].uicertcnt- ii), 1024);
        of_memcpy(pBuff + ii, pTmp->pRptr, ulNewLen);
        pTmp = pTmp->pCont;
        ii += ulNewLen;
      }

      /**
       * Convert the certificate from buffer into internal form.
       */
      iRetVal = HttpsReadFromFile(pBuff, HTTPS_CERT, (void**)&pTmpCert,
                                  'r');
      if (iRetVal != OF_SUCCESS)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsGetTrustedCert : Error in extracting X509cert\n");
#endif /*HTTPD_DEBUG*/
        of_free(pBuff);
        return OF_FAILURE;
      }
      of_free(pBuff);
      pBuff = NULL;

      /**
       * Id Name.
       */
      of_strcpy((pGetCerts+ucCnt)->Name, (char *) HttpsCACert[Index].IdName);

      /**
       * Not after.
       */
      tm = X509_get_notAfter(pTmpCert);

      if (tm->type == V_ASN1_UTCTIME)
      {
        Https_utctime_print((pGetCerts+ucCnt)->Expiry, tm);
      }
      else if (tm->type == V_ASN1_GENERALIZEDTIME)
      {
        Https_generalized_time_print((pGetCerts+ucCnt)->Expiry, tm);
      }
      else
      {
        of_strcpy((pGetCerts+ucCnt)->Expiry, "Bad time value");
      }

      /**
       * Subject Name.
       */
      X509_NAME_oneline(X509_get_subject_name(pTmpCert),
                        (pGetCerts+ucCnt)->Sub, 256);

      /**
       * Issuer Name.
       */
      X509_NAME_oneline(X509_get_issuer_name(pTmpCert),
                        (pGetCerts+ucCnt)->Issuer, 256);
      ucCnt ++;
      X509_free(pTmpCert);
      pTmpCert = NULL;
    }
  }

  of_strcpy((pGetCerts+ucCnt)->Name, "end");
  return OF_SUCCESS;
} /* HttpsGetTrustedCerts() */

/******************************************************************
 *  Function Name    : Https_utctime_print
 *  Description      :                                            *
 *  Input            :
 *  Output           : NONE                                       *
 *  Return value     : OF_SUCCESS or OF_FAILURE.                    *
 ******************************************************************/

int32_t Https_utctime_print( char       *pExpiryDate,
                             ASN1_UTCTIME  *tm )
{
  char  *v;
  int32_t  gmt = 0;
  int32_t  i;
  int32_t  y = 0, M = 0, d = 0, h = 0, m = 0, s = 0;

  i = tm->length;
  v = (char *)tm->data;

  if (i < 10)
  {
    return OF_FAILURE;
  }

  if (v[i-1] == 'Z')
  {
    gmt = 1;
  }

  for (i = 0; i < 10; i++)
  {
    if ((v[i] > '9') ||
        (v[i] < '0'))
    {
      return OF_FAILURE;
    }
  }

  y = (v[0]-'0')*10+(v[1]-'0');

  if (y < 50)
  {
    y += 100;
  }

  M = (v[2]-'0')*10+(v[3]-'0');

  if ((M > 12) ||
      (M < 1))
  {
    return OF_FAILURE;
  }

  d = (v[4]-'0')*10+(v[5]-'0');
  h = (v[6]-'0')*10+(v[7]-'0');
  m = (v[8]-'0')*10+(v[9]-'0');

  if ((v[10] >= '0') &&
      (v[10] <= '9') &&
      (v[11] >= '0') &&
      (v[11] <= '9'))
  {
    s = (v[10]-'0')*10+(v[11]-'0');
  }

  sprintf(pExpiryDate, "%s %2d %02d:%02d:%02d %d%s",
          mon[M-1], d, h, m, s, y+1900, (gmt)?" GMT":"");
  return OF_SUCCESS;
} /* Https_utctime_print() */

/******************************************************************
 *  Function Name    : Https_generalized_time_print
 *  Description      :                                            *
 *  Input            :
 *  Output           : NONE                                       *
 *  Return value     : OF_SUCCESS or OF_FAILURE.                    *
 ******************************************************************/

 int32_t Https_generalized_time_print( char       *pExpiryDate,
                                      ASN1_UTCTIME  *tm )
{
  char  *v;
  int32_t  gmt = 0;
  int32_t  i;
  int32_t  y = 0, M = 0, d = 0, h = 0, m = 0, s = 0;

  i = tm->length;
  v = (char *)tm->data;

  if (i < 12)
  {
    return OF_FAILURE;
  }

  if (v[i-1] == 'Z')
  {
    gmt = 1;
  }

  for (i = 0; i < 12; i++)
  {
    if ((v[i] > '9') ||
        (v[i] < '0'))
    {
      return OF_FAILURE;
    }
  }

  y = (v[0]-'0')*1000+(v[1]-'0')*100 + (v[2]-'0')*10+(v[3]-'0');
  M = (v[4]-'0')*10+(v[5]-'0');

  if ((M > 12) ||
      (M < 1))
  {
    return OF_FAILURE;
  }

  d = (v[6]-'0')*10+(v[7]-'0');
  h = (v[8]-'0')*10+(v[9]-'0');
  m =  (v[10]-'0')*10+(v[11]-'0');

  if ((v[12] >= '0') &&
      (v[12] <= '9') &&
      (v[13] >= '0') &&
      (v[13] <= '9'))
  {
    s = (v[12]-'0')*10+(v[13]-'0');
  }

  sprintf(pExpiryDate, "%s %2d %02d:%02d:%02d %d%s",
          mon[M-1], d, h, m, s, y, (gmt)?" GMT":"");

  return OF_SUCCESS;
} /* Https_generalized_time_print() */

/******************************************************************
 *  Function Name    : HttpsGetSubAltType
 *  Description      :                                            *
 *  Input            :
 *  Output           : NONE                                       *
 *  Return value     : OF_SUCCESS or OF_FAILURE.                    *
 ******************************************************************/

int32_t HttpsGetSubAltType( char         *pSubName,
                            X509_EXTENSION  *pExt )
{
  int32_t                 ii, iNum;
  STACK_OF(GENERAL_NAME)  *pSkSubAlt = NULL;
  GENERAL_NAME            *pSubAlt = NULL;
  unsigned char                *pTmp = NULL;
  ipaddr                  address;
  unsigned char                aIdData[16];

  of_memset(aIdData, 0, 16);

  pSkSubAlt = X509V3_EXT_d2i(pExt);

  if (pSkSubAlt == NULL)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGetSubAltType:: X509_EXT_d2i() failed\r\n");
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }

  iNum = sk_GENERAL_NAME_num(pSkSubAlt);

  if (iNum < 0)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGetSubAltType:: sk_GENERAL_NAME_num() failed\r\n");
#endif /*HTTPD_DEBUG*/
    sk_GENERAL_NAME_pop_free(pSkSubAlt, GENERAL_NAME_free);
    return OF_FAILURE;
  }

  pTmp = (unsigned char *) pSubName;

  for (ii=0; ii<iNum; ii++)
  {
    pSubAlt = sk_GENERAL_NAME_value(pSkSubAlt, ii);

    switch (pSubAlt->type)
    {
    case GEN_IPADD:
      of_strcat((char *)pTmp, "IPV4_ADDR: ");
      pTmp += of_strlen("IPV4_ADDR: ");

      of_memcpy(aIdData, pSubAlt->d.ip->data, pSubAlt->d.ip->length);

      address = of_mbuf_get_32((char *)aIdData);
      cm_inet_ntoal(address, (char *)aIdData);
      of_strcat((char *)pTmp, (char *) aIdData);
      pTmp += of_strlen((char *)aIdData);
      *pTmp = '\n';
      pTmp++;

      break;
    case GEN_DNS:
      of_strcat((char *)pTmp, "FQDN: ");
      pTmp += of_strlen("FQDN: ");
      of_memcpy(pTmp, pSubAlt->d.ia5->data, pSubAlt->d.ia5->length);
      pTmp += pSubAlt->d.ia5->length;
      *pTmp = '\n';
      pTmp++;

      break;
    case GEN_EMAIL:
      of_strcat((char *)pTmp, "USER_FQDN: ");
      pTmp += of_strlen("USER_FQDN: ");
      of_memcpy(pTmp, pSubAlt->d.ia5->data, pSubAlt->d.ia5->length);
      pTmp += pSubAlt->d.ia5->length;
      *pTmp = '\n';
      pTmp++;

      break;
    default:
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsGetSubAltType:: Unknown Sub-Alt-Type\n");
#endif /*HTTPD_DEBUG*/
      sk_GENERAL_NAME_pop_free(pSkSubAlt, GENERAL_NAME_free);
      return OF_FAILURE;
    } /* switch */
  } /* for */

  sk_GENERAL_NAME_pop_free(pSkSubAlt, GENERAL_NAME_free);
  return OF_SUCCESS;
} /* HttpsGetSubAltType() */

/******************************************************************
 *  Function Name    : HttpsCheckPrivKey
 *  Description      :                                            *
 *  Input            :
 *  Output           : NONE                                       *
 *  Return value     : OF_SUCCESS or OF_FAILURE.                    *
 ******************************************************************/

int32_t HttpsCheckPrivKey( unsigned char  *name_p,
                           uint16_t  *pStatus )
{
  int32_t  ii;

  for (ii = 0; ((ii < HTTPS_MAX_NUM_OF_KEY_CERT) &&
                (HttpsKeyCertPair[ii].pkey != NULL)); ii++)
  {
    if (strcmp((char *)HttpsKeyCertPair[ii].IdName, (char *)name_p) == 0)
    {
      *pStatus = HTTPS_DUPLICATE_KEY_NAME;
      return OF_SUCCESS;
    }
  }

  if (ii == HTTPS_MAX_NUM_OF_KEY_CERT)
  {
    *pStatus = HTTPS_PRIV_KEY_LIMIT;
  }
  else
  {
    *pStatus = 0;
  }
  return OF_SUCCESS;
} /* HttpsCheckPrivKey() */

/**********************************************************************
 * Function Name : HttpsStoreTrustedCert
 * Description   : This is an API provided by the CertMgr module to
 *                 store/upload Trusted Certificate into the box.  The
 *                 certificate buffer MAY be either in PEM format or
 *                 DER/BIN format ONLY.
 * Input         : unsigned char * - The certificate buffer.
 *                 uint32_t   - Length of the certificate buffer
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 **********************************************************************/

int32_t HttpsStoreTrustedCert( unsigned char  *pBuf,
                               uint32_t  ulLen,
                               uint8_t   *pHandle,
                               uint32_t  ulHandleLength )
{
  X509      *pCert = NULL;
  unsigned char  ucrtype, ucetype;
  unsigned char  ucdcert, ucty, ucsigntype = '\0';
  int32_t   theError;
  BIO       *pBio = NULL;
  unsigned char  *pTmpCert = NULL;
  int32_t   lCertLen;

  if (of_strncmp((char *)pBuf, "-----BEGIN", 10) != 0)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_INFO,
      "HttpsStoreTrustedCert:: Not a PEM certificate. Trying for DER format\n\r");
#endif /*HTTPD_DEBUG*/
    pCert = d2i_X509(&pCert, (const unsigned char **) &pBuf, ulLen);

    if (pCert == NULL)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsStoreTrustedCert:: Failed for DER also\n\r");
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsStoreTrustedCert: Certificate is not in P E M or D E R format\n");
#endif /*HTTPD_DEBUG*/
      return OF_FAILURE;
    }
  }
  else
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_INFO,
          "HttpsStoreTrustedCert:: Trying for PEM\r\n");
#endif /*HTTPD_DEBUG*/

    pBio = BIO_new_mem_buf(pBuf, ulLen);

    if (pBio == NULL)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsStoreTrustedCert: BIO_new_mem_buf() failed\n");
#endif /*HTTPD_DEBUG*/
      return OF_FAILURE;
    }

    pCert = PEM_read_bio_X509(pBio, NULL, NULL, NULL);

    if (pCert == NULL)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsStoreTrustedCert: PEM_read_bio_X509 failed\n");
#endif /*HTTPD_DEBUG*/
      BIO_free(pBio);
      return OF_FAILURE;
    }
    BIO_free(pBio);
    pBio = NULL;
  }

  /**
   * Convert the X509 format into PEM format.
   */
  pBio = BIO_new(BIO_s_mem());

  if (pBio == NULL)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsStoreTrustedCert:: BIO_new() is failed\n\r");
#endif /*HTTPD_DEBUG*/
    X509_free(pCert);
    return OF_FAILURE;
  }

  if (!PEM_write_bio_X509(pBio, pCert))
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsStoreTrustedCert:: PEM_write_bio_X509 failed\n\r");
#endif /*HTTPD_DEBUG*/
    X509_free(pCert);
    BIO_free(pBio);
  }

  lCertLen = BIO_get_mem_data(pBio, &pTmpCert);

  /**
   * Check for the Maximum size of the trusted certificate.
   */
  if (lCertLen > HTTPS_CERT_LIMIT_SIZE)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "CA Certificate is too big. Supported up to 10K\r\n");
#endif /*HTTPD_DEBUG*/
    X509_free(pCert);
    BIO_free(pBio);
    return HTTPS_MAX_CERT_SIZE_ERROR;
  }

  /**
   * 1. Certificate type
   * 2. Request type
   * 3. Encryption type
   * 4. ucdcert (I don't know)
   * 5. Signature type
   * 6. Private Key name
   * 7. Certificate Buffer
   * 8. Certificate Buffer length
   */
  if ((OBJ_obj2nid((pCert)->sig_alg->algorithm)
        == NID_md5WithRSAEncryption) ||
     (OBJ_obj2nid((pCert)->sig_alg->algorithm)
        == NID_sha1WithRSAEncryption) ||
     (OBJ_obj2nid((pCert)->sig_alg->algorithm)
        == NID_shaWithRSAEncryption))
  {
    ucsigntype = 'r';
  }
  else if ((OBJ_obj2nid((pCert)->sig_alg->algorithm)
           == NID_dsaWithSHA) ||
          (OBJ_obj2nid((pCert)->sig_alg->algorithm)
           == NID_dsaWithSHA1))
  {
    ucsigntype = 'd';
  }
  else
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsStoreTrustedCert: Unknown signature type\n");
#endif /*HTTPD_DEBUG*/
    if (pCert)
    {
      X509_free(pCert);
      BIO_free(pBio);
      pCert = NULL;
    }
    return HTTPS_INVALID_SIG_TYPE;
  }

  ucrtype = 'c';
  ucetype = 'p';
  ucdcert = 'n';
  ucty    = 't';

  /**
   * AVS added pHandle, ulHandleLength, to get back the handle for the
   * trusted Certificate  02/25.
   */
  theError = HttpsUploadTrustedCert(ucty, ucrtype, ucetype, ucdcert,
                                    ucsigntype, pTmpCert, lCertLen,
                                    pHandle, ulHandleLength);

  if (theError != OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpTrustedVal: Error in uploading trusted certificate\n");
#endif /*HTTPD_DEBUG*/
    X509_free(pCert);
    BIO_free(pBio);
    return theError;
  }

  X509_free(pCert);
  BIO_free(pBio);
  return OF_SUCCESS;
} /* HttpsStoreTrustedCert() */

/**********************************************************************
 * Function Name : HttpsStoreSelfCert
 * Description   : This is an API provided by the Certmgr module for
 *                 storing/uploading the self-certificates into the box.
 *                 The certificate date MAY be either in PEM format or
 *                 in DER/BIN format ONLY.
 * Input         : unsigned char * - The certificate buffer
 *                 uint32_t   - The certificate buffer length
 *                 unsigned char * - The private key name
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 **********************************************************************/

int32_t HttpsStoreSelfCert( unsigned char  *pBuf,
                            uint32_t  ulLen,
                            unsigned char  *name_p )
{
  HX509Cert_t  *pCert = NULL;
  unsigned char     ucrectype, ucenctype = 0;
  unsigned char     ucdefcert = 0, uctype = 0;
  unsigned char     ucsigntype = 0;
  int32_t      theError;
  BIO          *pBio = NULL;
  unsigned char     *pTmpCert = NULL;
  int32_t      lCertLen;

  if (of_strncmp((char *)pBuf, "-----BEGIN", 10) != 0)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_INFO,
          "HttpsStoreSelfCert:: Not a PEM certificate.. Trying for DER format\n\r");
#endif /*HTTPD_DEBUG*/
    pCert = d2i_X509(&pCert, (const unsigned char **) &pBuf, ulLen);

    if (pCert == NULL)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsStoreSelfCert:: Failed for DER also\n\r");
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsStoreSelfCert: Certificate is not in P E M or D E R format\n");
#endif /*HTTPD_DEBUG*/
      return OF_FAILURE;
    }
  }
  else
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_INFO,
          "HttpsStoreSelfCert:: Trying for PEM format\n\r");
#endif /*HTTPD_DEBUG*/
    pBio = BIO_new_mem_buf(pBuf, ulLen);

    if (pBio == NULL)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsStoreSelfCert: BIO_new_mem_buf() failed\n");
#endif /*HTTPD_DEBUG*/
      return OF_FAILURE;
    }

    pCert = PEM_read_bio_X509(pBio, NULL, NULL, NULL);

    if (pCert == NULL)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsStoreSelfCert: PEM_read_bio_X509 failed\n");
#endif /*HTTPD_DEBUG*/
      BIO_free(pBio);
      return OF_FAILURE;
    }

    BIO_free(pBio);
    pBio = NULL;
  }

  /**
   * Convert the X509 format into PEM format.
   */
  pBio = BIO_new(BIO_s_mem());

  if (pBio == NULL)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsStoreSelfCert:: BIO_new() is failed\n\r");
#endif /*HTTPD_DEBUG*/
    X509_free(pCert);
    return OF_FAILURE;
  }

  if (!PEM_write_bio_X509(pBio, pCert))
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsStoreSelfCert:: PEM_write_bio_X509 failed\n\r");
#endif /*HTTPD_DEBUG*/
    X509_free(pCert);
    BIO_free(pBio);
    return OF_FAILURE;
  }

  lCertLen = BIO_get_mem_data(pBio, &pTmpCert);

  /**
   * Check for the Maximum size of the Self-certificate.
   */
  if (lCertLen > HTTPS_CERT_LIMIT_SIZE)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "Self Certifiate is too big. Supported up to 10K");
#endif /*HTTPD_DEBUG*/
    X509_free(pCert);
    BIO_free(pBio);
    return HTTPS_MAX_CERT_SIZE_ERROR;
  }

  /**
   * 1. Certificate type
   * 2. Request type
   * 3. Encryption type
   * 4. ucdefcert
   * 5. Signature type
   * 6. Certificate Buffer
   * 7. Certificate Buffer length
   * 8. Private Key name
   */

  ucrectype = 'c';

  if ((OBJ_obj2nid((pCert)->sig_alg->algorithm)
        == NID_md5WithRSAEncryption) ||
     (OBJ_obj2nid((pCert)->sig_alg->algorithm)
        == NID_sha1WithRSAEncryption) ||
     (OBJ_obj2nid((pCert)->sig_alg->algorithm)
        == NID_shaWithRSAEncryption))
  {
    ucsigntype = 'r';
  }
  else if ((OBJ_obj2nid((pCert)->sig_alg->algorithm)
            == NID_dsaWithSHA) ||
          (OBJ_obj2nid((pCert)->sig_alg->algorithm)
            == NID_dsaWithSHA1))
  {
    ucsigntype = 'd';
  }
  else
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsStoreSelfCert: Invalid Signature algorithm in Certificate\n");
#endif /*HTTPD_DEBUG*/
    BIO_free(pBio);
    X509_free(pCert);
    pCert = NULL;

    return OF_FAILURE;
  }

  theError = HttpsUploadCert(uctype, ucrectype, ucenctype, ucdefcert,
                             ucsigntype, pTmpCert, lCertLen, name_p);

  if (theError != OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
     Trace(HTTPS_ID, TRACE_SEVERE,
       "HttpsStoreSelfCert: Error in uploading the self certificate\n");
#endif /*HTTPD_DEBUG*/
     BIO_free(pBio);
     X509_free(pCert);
     return theError;
  }

  X509_free(pCert);
  BIO_free(pBio);
  return OF_SUCCESS;
} /* HttpsStoreSelfCert() */

/***************************************************************************
 * Function Name : HttpsGetCertReq
 * Description   :
 * Input         :
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 ***************************************************************************/

int32_t HttpsGetCertReq( unsigned char  *name_pval,
                         unsigned char  *pCertifybuf,
                         uint32_t  *pReqLen )
{
  int32_t       uiIndex = 0, ii = 0;
  HTTPS_mblk_t  *pTmp = NULL;
  uint32_t      ulNewLen = 0;

  if(name_pval ==  NULL)
  {
#ifdef HTTPD_DEBUG
   Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGetCertReq:: name_pval is NULL\n");
#endif /*HTTPD_DEBUG*/
   return OF_FAILURE;
  }
  for (uiIndex = 0; uiIndex < HTTPS_MAX_NUM_OF_KEY_CERT; uiIndex++)
  {
    if (of_strcmp((char *)HttpsKeyCertPair[uiIndex].IdName, (char *)name_pval) == 0)
    {
      break;
    }
  }

  if (uiIndex == HTTPS_MAX_NUM_OF_KEY_CERT)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGetCertReq:: key-cert pair allowed is only HTTPS_MAX_NUM_OF_KEY_CERT\n");
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }

  if (HttpsKeyCertPair[uiIndex].uireqcnt > HTTPS_MAX_BUF_LEN)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGetCertReq:: Request length greater than HTTPS_MAX_BUF_LEN\n");
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }

  if (HttpsKeyCertPair[uiIndex].preq == NULL)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGetCertReq:: Request field is NULL\n");
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }

  /**
   * If no memory is allocated for this variable, just return the length.
   */
  if (pCertifybuf == NULL)
  {
    *pReqLen = HttpsKeyCertPair[uiIndex].uireqcnt;
    return OF_SUCCESS;
  }

  of_memcpy(pCertifybuf, HttpsKeyCertPair[uiIndex].preq->pRptr,
           MIN(HttpsKeyCertPair[uiIndex].uireqcnt, 1024));

  ii = 1024;
  pTmp = HttpsKeyCertPair[uiIndex].preq->pCont;
  while ((ii < HttpsKeyCertPair[uiIndex].uireqcnt) &&
         (pTmp != NULL))
  {
    ulNewLen = MIN((HttpsKeyCertPair[uiIndex].uireqcnt- ii), 1024);
    of_memcpy((pCertifybuf+ ii), pTmp->pRptr, ulNewLen);
    pTmp = pTmp->pCont;
    ii += ulNewLen;
  }
  *pReqLen = HttpsKeyCertPair[uiIndex].uireqcnt;

  return OF_SUCCESS;
} /* HttpsGetCertReq() */

#ifdef HTTPS_ENABLE

int32_t HttpsGetValidCert( HX509Cert_t  **pX509Cert,
                           EVP_PKEY     **pPrivKey )
{
  int32_t  ii = 0;

  if ((pX509Cert == NULL) ||
      (pPrivKey == NULL))
  {
    return OF_FAILURE;
  }

  while (ii < 10)
  {
    if ((HttpsRSAKeyPairs[ii].pMyCert) &&
        (HttpsRSAKeyPairs[ii].pMyPrivKey))
    {
      if (HttpsVerifySelfCert(HttpsRSAKeyPairs[ii].pMyCert) == OF_SUCCESS)
      {
        break;
      }
    }
    ii++;
  }

  if (ii == 10)
  {
    return OF_FAILURE;
  }

  *pX509Cert = HttpsRSAKeyPairs[ii].pMyCert;
  *pPrivKey  = HttpsRSAKeyPairs[ii].pMyPrivKey;
  return OF_SUCCESS;
} /* HttpsGetValidCert() */

#else /* HTTPS_ENABLE */

int32_t HttpsGetValidCert( HX509Cert_t  **pX509Cert,
                           EVP_PKEY     **pPrivKey )
{
  return OF_FAILURE;
} /* HttpsGetValidCert() */

#endif /* HTTPS_ENABLE */

/**************************************************************************
 * Function Name : IGWSSL_load_error_strings                              *
 * Description   : This function is used load the error strings.          *
 * Input         : None.                                                  *
 * Output        : Loads error strings.                                   *
 * Return value  : None.                                                  *
 **************************************************************************/

void IGWSSL_load_error_strings( void )
{
  /**
   * Load the error strings.
   */
  SSL_load_error_strings();
} /* IGWSSL_load_error_strings() */

/**************************************************************************
 * Function Name : IGWSSLeay_add_ssl_algorithms                           *
 * Description   : This function is used to add ssl algorithms.           *
 * Input         : None.                                                  *
 * Output        : Adds the ssl algorithms.                               *
 * Return value  : None.                                                  *
 **************************************************************************/

void IGWSSLeay_add_ssl_algorithms( void )
{
  /**
   * Add the ssl algorithms.
   */
  SSLeay_add_ssl_algorithms();
} /* IGWSSLeay_add_ssl_algorithms() */

/**************************************************************************
 * Function Name : IGWSSLv23_server_method                                *
 * Description   : This function is used to get the server method.        *
 * Input         : None.                                                  *
 * Output        : Gets the server method used.                           *
 * Return value  : Returns the configured ssl methods configured or NULL. *
 **************************************************************************/

SSL_METHOD *IGWSSLv23_server_method( void )
{
  /**
   * Create the ssl method.
   */
  return (SSL_METHOD *)SSLv23_server_method();
} /* IGWSSLv23_server_method() */

/**************************************************************************
 * Function Name : IGWSSL_CTX_new                                         *
 * Description   : This function is used to get the SSL_CTX for the given *
 *                 method.                                                *
 * Input         : sslMethod (I) - New ssl method.                        *
 * Output        : SSL_CTX for the given method.                          *
 * Return value  : Newly created ssl ctx.                                 *
 **************************************************************************/

SSL_CTX *IGWSSL_CTX_new( SSL_METHOD  *sslMethod )
{
  /**
   * Create the ssl ctx with the given ssl method.
   */
  //return SSL_CTX_new(sslMethod);
  SSL_CTX * ctx;
  ctx = SSL_CTX_new(sslMethod);
  if(!ctx) {
    ERR_print_errors_fp(stderr);
  }
  return ctx;
    
} /* IGWSSL_CTX_new() */

/**************************************************************************
 * Function Name : IGWSSL_CTX_use_certificate                             *
 * Description   : This function is used to initialize the http server   *
 *                 and creates http server task.                          *
 * Input         : ctx   (I) - pointer to the ctx.                        *
 *                 pCert (I) - pointer to the certificate.                *
 * Output        : None.                                                  *
 * Return value  : Status of the use certificate operation for the given  *
 *                 ssl ctx.                                               *
 **************************************************************************/

int32_t IGWSSL_CTX_use_certificate( SSL_CTX  *ctx,
                                    X509     *pCert )
{
  /**
   * Use the given certificate for the given ssl ctx.
   */
  return SSL_CTX_use_certificate(ctx, pCert);
} /* IGWSSL_CTX_use_certificate() */

/**************************************************************************
 * Function Name : IGWSSL_CTX_use_PrivateKey                              *
 * Description   : This function is used to use the private key for the   *
 *                 given ssl ctx.                                         *
 * Input         : ctx  (I) - pointer to the ctx.                         *
 *                 pKey (I) - pointer to the private key.                 *
 * Output        : None.                                                  *
 * Return value  : Status of the use private key operation for the given  *
 *                 ssl ctx.                                               *
 **************************************************************************/

int32_t IGWSSL_CTX_use_PrivateKey( SSL_CTX   *ctx,
                                   EVP_PKEY  *pKey )
{
  /**
   * Use the given private key for the given ssl ctx.
   */
  return SSL_CTX_use_PrivateKey(ctx, pKey);
} /* IGWSSL_CTX_use_PrivateKey() */

/**************************************************************************
 * Function Name : IGWSSL_CTX_check_private_key                           *
 * Description   : This function is used to check the private key from the*
 *                 given ssl ctx.                                         *
 * Input         : ctx (I) - pointer to the ctx.                          *
 * Output        : None.                                                  *
 * Return value  : Status of the private key from the given ssl ctx.      *
 **************************************************************************/

int32_t IGWSSL_CTX_check_private_key( SSL_CTX  *ctx )
{
  /**
   * Check the private key from the given ssl ctx.
   */
  return SSL_CTX_check_private_key(ctx);
} /* IGWSSL_CTX_check_private_key() */

/**************************************************************************
 * Function Name : IGWSSL_use_certificate                                 *
 * Description   : This function is used to use the certificate passed.   *
 * Input         : ssl   (I) - pointer to the ssl.                        *
 *                 pCert (I) - pointer to the certificate.                *
 * Output        : None.                                                  *
 * Return value  : Status of the use certificate operation.               *
 **************************************************************************/

int32_t IGWSSL_use_certificate( SSL   *ssl,
                                X509  *pCert )
{
  /**
   * Use the given certificate.
   */
  return SSL_use_certificate(ssl, pCert);
} /* IGWSSL_use_certificate() */

/**************************************************************************
 * Function Name : IGWSSL_use_PrivateKey                                  *
 * Description   : This function is used to use the given private key.    *
 * Input         : ssl  (I) - pointer to the ssl.                         *
 * Input         : pKey (I) - pointer to the private key.                 *
 * Output        : None.                                                  *
 * Return value  : Status of the use private key operation.               *
 **************************************************************************/

int32_t IGWSSL_use_PrivateKey( SSL       *ssl,
                               EVP_PKEY  *pKey )
{
  /**
   * Use the given private key.
   */
  return SSL_use_PrivateKey(ssl, pKey);
} /* IGWSSL_use_PrivateKey() */

/**************************************************************************
 * Function Name : IGWSSL_check_private_key                               *
 * Description   : This function is used to check the given private key.  *
 * Input         : ssl (I) - pointer to the ssl.                          *
 * Output        : None.                                                  *
 * Return value  : Status of the check private key operation.             *
 **************************************************************************/

int32_t IGWSSL_check_private_key( SSL  *ssl )
{
  /**
   * Check the given private key.
   */
  return SSL_check_private_key(ssl);
} /* IGWSSL_check_private_key() */

/**************************************************************************
 * Function Name : IGWSSL_write                                           *
 * Description   : This function is used to write the ssl using the given *
 *                 parameters.                                            *
 * Input         : ssl  (I) - pointer to the ssl.                         *
 *                 lbuf (I) - pointer to the buffer pointer.              *
 *                 len  (I) - length of the buffer.                       *
 * Output        : None.                                                  *
 * Return value  : Status of the write operation.                         *
 **************************************************************************/

int32_t IGWSSL_write( SSL      *ssl,
                      void   *buf,
                      int32_t  len )
{
  /**
   * Write the ssl.
   */
  return SSL_write(ssl, buf, len);
} /* IGWSSL_write() */

/**************************************************************************
 * Function Name : IGWSSL_read                                            *
 * Description   : This function is used to read the ssl using the given  *
 *                 parameters.                                            *
 * Input         : ssl  (I) - pointer to the ssl.                         *
 *                 lbuf (I) - pointer to the buffer pointer.              *
 *                 len  (I) - length of the buffer.                       *
 * Output        : None.                                                  *
 * Return value  : Status of the read operation.                          *
 **************************************************************************/

int32_t IGWSSL_read( SSL      *ssl,
                     void   *lbuf,
                     int32_t  len,
                     uint32_t *pRdStatus)
{
  int32_t iBytesRead = 0;
  int32_t iTotalBytesRead = 0;
  unsigned char *pBuffer = (unsigned char *) lbuf;

  /**
   * Read the ssl.
   */
   do
   {
     if(len <= 0)
     {
       *pRdStatus = 1;   /* set 1 to send SSL_READ_PENDING event to HTTP Server */
       return iTotalBytesRead;
     }
     iBytesRead = SSL_read(ssl, pBuffer, len);
     if(iBytesRead > 0)
     {
       iTotalBytesRead += iBytesRead;
       pBuffer = pBuffer + iBytesRead;
       len -= iBytesRead;
     }
     else 
     {
       if(!iBytesRead)
       {
          return (-1 * SSL_ERROR_ZERO_RETURN);
       }
       else
         break;
     }
   }while(SSL_pending(ssl));
   
   if(iTotalBytesRead > 0)
   {
     return iTotalBytesRead;
   }
   else
   {
     return -1;
   }
} /* IGWSSL_read() */

/**************************************************************************
 * Function Name : IGWSSL_new                                             *
 * Description   : This function is used to create the new ctx for the    *
 *                 given parameter.                                       *
 * Input         : ctx (I) - pointer to the ctx.                          *
 * Output        : None.                                                  *
 * Return value  : Newly created ctx.                                     *
 **************************************************************************/

SSL *IGWSSL_new( SSL_CTX  *ctx )
{
  /**
   * Create the new ctx.
   */
  return SSL_new(ctx);
} /* IGWSSL_new() */

/**************************************************************************
 * Function Name : IGWSSL_accept                                          *
 * Description   : This function is used to accept the given ssl.         *
 * Input         : ssl (I) - pointer to the ssl.                          *
 * Output        : None.                                                  *
 * Return value  : Status of the accept operation.                        *
 **************************************************************************/

int32_t IGWSSL_accept( SSL  *ssl )
{
  /**
   * Call the ssl accept function for the given ssl.
   */
  return SSL_accept(ssl);
} /* IGWSSL_accept() */

/**************************************************************************
 * Function Name : IGWSSL_set_rfd                                         *
 * Description   : This function is used to set the read file descriptor  *
 *                 for the given ssl.                                     *
 * Input         : ssl    (I) - pointer to the ssl.                       *
 *                 handle (I) - file descriptor handle.                   *
 * Output        : None.                                                  *
 * Return value  : 1 if the set operation is successful, otherwise 0.     *
 **************************************************************************/

int32_t IGWSSL_set_rfd( SSL      *ssl,
                        int32_t  handle )
{
  /**
   * Set the read fd for the given ssl and return the status of the
   * operation.
   */
  return SSL_set_rfd(ssl, handle);
} /* IGWSSL_set_rfd() */

/**************************************************************************
 * Function Name : IGWSSL_set_fd                                          *
 * Description   : This function is used to set the fd for the given ssl. *
 * Input         : ssl    (I) - pointer to the ssl.                       *
 *                 handle (I) - file descriptor handle.                   *
 * Output        : None.                                                  *
 * Return value  : 1 if the set operation is successful otherwise 0.      *
 **************************************************************************/

int32_t IGWSSL_set_fd( SSL      *ssl,
                       int32_t  handle )
{
  /**
   * Set the fd for the given ssl and return the status of the operation.
   */
  return SSL_set_fd(ssl, handle);
} /* IGWSSL_set_fd() */

/**************************************************************************
 * Function Name : IGWSSL_is_init_finished                                *
 * Description   : This function is used to check if the status of the    *
 *                 passed ssl is finished or not.                         *
 * Input         : ssl (I) - pointer to the ssl.                          *
 * Output        : Status of the ssl passed.                              *
 * Return value  : OF_SUCCESS on success, otherwise OF_FAILURE.             *
 **************************************************************************/

int32_t IGWSSL_is_init_finished( SSL  *ssl )
{
  /**
   * Get the status of the passed ssl and return it.
   */
  return SSL_is_init_finished(ssl);
} /* IGWSSL_is_init_finished() */

/**************************************************************************
 * Function Name : IGWSSL_get_error                                       *
 * Description   : This function is used to get the actual error id based *
 *                 on the parameters passed.                              *
 * Input         : ssl  (I) - pointer to the ssl.                         *
 *                 iErr (I) - error number.                               *
 * Output        : Actual error identification.                           *
 * Return value  : Relevant error identification.                         *
 **************************************************************************/

int32_t IGWSSL_get_error( SSL      *ssl,
                          int32_t  iErr )
{
  /**
   * Get the actual error code using the passed parameters and return it.
   */
  return SSL_get_error(ssl, iErr);
} /* IGWSSL_get_error() */

/**************************************************************************
 * Function Name : IGWSSL_free                                            *
 * Description   : This function is used to free the memory of the passed *
 *                 ssl parameter.                                         *
 * Input         : ssl - pointer to the ssl.                              *
 * Output        : None.                                                  *
 * Return value  : None.                                                  *
 **************************************************************************/

void IGWSSL_free( SSL  *ssl )
{
  /**
   * Free the memory associated with the passed ssl parameter.
   */
  SSL_free(ssl);
} /* IGWSSL_free() */

#endif /* OF_HTTPD_SUPPORT && OF_HTTPS_SUPPORT */


