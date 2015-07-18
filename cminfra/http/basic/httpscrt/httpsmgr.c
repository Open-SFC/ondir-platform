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
/*  File        : httpsmgr.c                                              */
/*                                                                        */
/*  Description : This file contains https certificate manager functions. */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.1      05.12.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#if defined(OF_HTTPD_SUPPORT) && defined(OF_HTTPS_SUPPORT)

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "httpdinc.h"
#include "htpdglb.h"
#include "hcmgrmbl.h"
#include "hcmgrgdf.h"
#include "hcmgrgif.h"
#include "hcmgrldf.h"
#include "hcmgrlif.h"
#ifdef INTOTO_CMS_SUPPORT
#include "openssl/x509.h"
#endif  /* INTOTO_CMS_SUPPORT */

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  HTTPS_MAX_FILE_NAME_LEN      (32)
#define  HTTPS_MAX_PATH_LEN           (64)
#define  HTTPS_MAX_NUM_OF_FILES       (20)
#define  HTTPS_FILE_EXT_LEN           (3)

/************************************************************
 * * * *  V a r i a b l e    D e c l a r a t i o n s  * * * *
 ************************************************************/

/**
 * for storing the PEM encoded certificate & key in memory using mblk.
 */
uint32_t            HttpsPoolId;
HttpsKeyCertPair_t  HttpsKeyCertPair[HTTPS_MAX_NUM_OF_KEY_CERT];
HttpsCACertInfo_t   HttpsCACert[HTTPS_MAX_NUM_OF_CA_CERT];

char  HttpsTrustedCerts[HTTPS_MAX_NUM_OF_FILES][HTTPS_MAX_FILE_NAME_LEN];
char  HttpsSelfCerts[HTTPS_MAX_NUM_OF_FILES][HTTPS_MAX_FILE_NAME_LEN];

HttpsMyKeyCert_t  HttpsRSAKeyPairs[HTTPS_PARAMS_RSA_KEY_PAIRS_LEN];

STACK_OF(X509)  *HttpsRSATrustedCACerts;

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 *
 */
int32_t HttpsTestCert( void );

/**
 *
 */
int32_t HttpsTimeOut( void );

/**
 *
 */

int32_t HttpsFindPrivKeyIndex( HttpsMyKeyCert_t  *aMyKeyPairs,
                               ASN1_INTEGER      *inASN1Serial,
                               HX509Name_t       *inX509IssuerName,
                               uint16_t          *pIndex );

/**
 *
 */
int32_t HttpsGetExtValue( unsigned char  *pBuffer,
                          unsigned char  *value_p );

/**
 * This function is used to
 */

void HttpsFreeX509Stack( STACK_OF(X509)  *skCert );

/**
 * This function is used to
 */

HX509Cert_t *HttpsFindInStackBySubject( STACK_OF(X509)  *pStack,
                                        HX509Name_t     *pInSubName );

/**
 *
 */
/**/
int32_t HttpsGetTrustedCAs( uint16_t       ucEncType,
                            uint16_t       usCertType,
                            uint16_t       *pNumCAs,
                            HttpsCAName_t  *pCAName );

/**
 *
 */

int32_t HttpsGetPeerCertByID( HttpsCertificate_t  *pInCerts,
                              uint8_t             usInNumCerts,
                              HttpsCertIdInfo_t   *pCertIdInfo,
                              STACK_OF(X509)      **certSK,
                              HX509Cert_t         **pOutCert );
/**
 *
 */
/**/
int32_t HttpsCmpLifeTimeWithCert( unsigned char  *pData,
                                  uint16_t  usLength,
                                  int32_t   iPolicyLifeTime,
                                  uint32_t  *pNotAfter );

#ifdef INTOTO_CMS_SUPPORT

void HttpsFreeX509Stack( STACK_OF(X509)  *skCert );

HX509Cert_t *HttpsFindInStackBySubject( STACK_OF(X509)  *pStack,
                                        HX509Name_t     *pInSubName );

int32_t HttpsGetExtValue( unsigned char  *pBuffer,
                          unsigned char  *value_p );

unsigned char CMSTrustSignedCert[] =
"-----BEGIN CERTIFICATE-----\n\
MIIDKzCCAtWgAwIBAgIQL86LzhMtcJZIYju6xfX6FzANBgkqhkiG9w0BAQUFADCB\n\
nTEmMCQGCSqGSIb3DQEJARYXc3lzLWFkbWluQGludG90b2luZC5jb20xCzAJBgNV\n\
BAYTAklOMQswCQYDVQQIEwJBUDESMBAGA1UEBxMJSHlkZXJhYmFkMRgwFgYDVQQK\n\
Ew9JbnRvdG8gU29mdHdhcmUxEjAQBgNVBAsTCVN5cy1BZG1pbjEXMBUGA1UEAxMO\n\
SW50b3RvIFRlc3QgQ0EwHhcNMDcwMTI1MDczODM1WhcNMzIwMTI1MDc0ODA4WjCB\n\
nTEmMCQGCSqGSIb3DQEJARYXc3lzLWFkbWluQGludG90b2luZC5jb20xCzAJBgNV\n\
BAYTAklOMQswCQYDVQQIEwJBUDESMBAGA1UEBxMJSHlkZXJhYmFkMRgwFgYDVQQK\n\
Ew9JbnRvdG8gU29mdHdhcmUxEjAQBgNVBAsTCVN5cy1BZG1pbjEXMBUGA1UEAxMO\n\
SW50b3RvIFRlc3QgQ0EwXDANBgkqhkiG9w0BAQEFAANLADBIAkEAuaqGFhUhkIP6\n\
bWb6XtjZ5M17k2eLpzd58YfCj1XmZjWgB1Ei57y1H+Fy0XTD5apHQj8BeBlOrtp4\n\
Zl1mP8yGdQIDAQABo4HuMIHrMAsGA1UdDwQEAwIBxjAPBgNVHRMBAf8EBTADAQH/\n\
MB0GA1UdDgQWBBQ3160stzrOmUXaOLh7vdsw/J17bjCBmQYDVR0fBIGRMIGOMESg\n\
QqBAhj5odHRwOi8vYmhhcmF0aS5pbnRvdG9pbmQuY29tL0NlcnRFbnJvbGwvSW50\n\
b3RvJTIwVGVzdCUyMENBLmNybDBGoESgQoZAZmlsZTovL1xcYmhhcmF0aS5pbnRv\n\
dG9pbmQuY29tXENlcnRFbnJvbGxcSW50b3RvJTIwVGVzdCUyMENBLmNybDAQBgkr\n\
BgEEAYI3FQEEAwIBADANBgkqhkiG9w0BAQUFAANBAIL11fJrDwqxBRkBHmvXDzP3\n\
D+CyDrDY9yF2zQslsJUyz4FJXStBNy5LR9V8uNzHL+PEhABl+gDHYt8NQjX8uJk=\n\
-----END CERTIFICATE-----";

unsigned char CMSPKey[] =
"-----BEGIN RSA PRIVATE KEY-----\n\
MIIBOgIBAAJBALxVAZHFzIWDG8Us4oNQ2aNjUCG+16yDPotvsnEV+iSM7Klis4iK\n\
CnigOXBpktfhDOaiWvlngVEgycUaSoithTUCAwEAAQJAOd4AunLFceykFsTA4Mpx\n\
5QM59vR1vnshN6TddmPjVWE0s9Hz1MPXjMxoiJhuUYKS6qfIEauYjTzQrqK4MADO\n\
7QIhAOUeX2HpZ+Xtb2BcA4z1B2OCWV5wnukOHvGr9gwt/HNzAiEA0m2Zg397fDDt\n\
/Ix1svkmarbSOdSLSk7qH3M4XtBZircCID0kUs3dLJXGO++Z/nSSOuuKMHEsWqvU\n\
0dsqSZnMd88TAiBjdw7zS1URVQeJMtOHr6FrG9OvJjpY+4hLKWlh19YbbQIhANsZ\n\
dAdOCKiRnNNKzcp2k7irH3F8b1mI5Taiy7UQ8Hne\n\
-----END RSA PRIVATE KEY-----";

unsigned char CMSSelfSignedCert[] = 
"-----BEGIN CERTIFICATE-----\n\
MIIEPDCCA+agAwIBAgIKNBz7WgAAAAAHXzANBgkqhkiG9w0BAQUFADCBnTEmMCQG\n\
CSqGSIb3DQEJARYXc3lzLWFkbWluQGludG90b2luZC5jb20xCzAJBgNVBAYTAklO\n\
MQswCQYDVQQIEwJBUDESMBAGA1UEBxMJSHlkZXJhYmFkMRgwFgYDVQQKEw9JbnRv\n\
dG8gU29mdHdhcmUxEjAQBgNVBAsTCVN5cy1BZG1pbjEXMBUGA1UEAxMOSW50b3Rv\n\
IFRlc3QgQ0EwHhcNMDcxMTIxMDY0NDAwWhcNMDgxMTIxMDY1NDAwWjAUMRIwEAYD\n\
VQQDEwlpbnRvdG9oeWQwXDANBgkqhkiG9w0BAQEFAANLADBIAkEAvFUBkcXMhYMb\n\
xSzig1DZo2NQIb7XrIM+i2+ycRX6JIzsqWKziIoKeKA5cGmS1+EM5qJa+WeBUSDJ\n\
xRpKiK2FNQIDAQABo4ICjjCCAoowGAYDVR0RBBEwD4INaW50b3RvaHlkLmNvbTAd\n\
BgNVHQ4EFgQUQnBYgnLBCiF4ljOecrn2D8QsM5QwgdkGA1UdIwSB0TCBzoAUN9et\n\
LLc6zplF2ji4e73bMPyde26hgaOkgaAwgZ0xJjAkBgkqhkiG9w0BCQEWF3N5cy1h\n\
ZG1pbkBpbnRvdG9pbmQuY29tMQswCQYDVQQGEwJJTjELMAkGA1UECBMCQVAxEjAQ\n\
BgNVBAcTCUh5ZGVyYWJhZDEYMBYGA1UEChMPSW50b3RvIFNvZnR3YXJlMRIwEAYD\n\
VQQLEwlTeXMtQWRtaW4xFzAVBgNVBAMTDkludG90byBUZXN0IENBghAvzovOEy1w\n\
lkhiO7rF9foXMIGZBgNVHR8EgZEwgY4wRKBCoECGPmh0dHA6Ly9iaGFyYXRpLmlu\n\
dG90b2luZC5jb20vQ2VydEVucm9sbC9JbnRvdG8lMjBUZXN0JTIwQ0EuY3JsMEag\n\
RKBChkBmaWxlOi8vXFxiaGFyYXRpLmludG90b2luZC5jb21cQ2VydEVucm9sbFxJ\n\
bnRvdG8lMjBUZXN0JTIwQ0EuY3JsMIHWBggrBgEFBQcBAQSByTCBxjBgBggrBgEF\n\
BQcwAoZUaHR0cDovL2JoYXJhdGkuaW50b3RvaW5kLmNvbS9DZXJ0RW5yb2xsL2Jo\n\
YXJhdGkuaW50b3RvaW5kLmNvbV9JbnRvdG8lMjBUZXN0JTIwQ0EuY3J0MGIGCCsG\n\
AQUFBzAChlZmaWxlOi8vXFxiaGFyYXRpLmludG90b2luZC5jb21cQ2VydEVucm9s\n\
bFxiaGFyYXRpLmludG90b2luZC5jb21fSW50b3RvJTIwVGVzdCUyMENBLmNydDAN\n\
BgkqhkiG9w0BAQUFAANBAJZ6N4FN49QRdLZXEwqVFnphQGQlB/YQNnl4ac954nUm\n\
aSf7gz/pdVXNQciD5+GInUCb/aRc4Q9VYIPuIaFifEM=\n\
-----END CERTIFICATE-----";

#define HTTPS_PKI_PRIVATE_KEY  1
#define HTTPS_PKI_PUBLIC_KEY   2
EVP_PKEY  *pCMSPKey;
X509      *pCMSSelfSignedCert;
X509      *pCMSTrustSignedCert;


 int32_t HTTPSConvertCertToInternalFormat(unsigned char  *pBuf,
                                       int32_t    lBufLen,
                                       X509     **pX509Cert,
                                       PKCS7    **pPKCS7Cert);
 int32_t HttpsMgrConvertKeyToInternalFormat(
                                      unsigned char    ucKeyType,
                                      unsigned char   *pBuf,
                                      int32_t     lBufLen,
                                      EVP_PKEY  **pKey);

 int32_t HttpsMgrConvertKeyToInternalFormat(
                                      unsigned char    ucKeyType,
                                      unsigned char   *pBuf,
                                      int32_t     lBufLen,
                                      EVP_PKEY  **pKey)
{
  BIO       *pBio = NULL;

  /*
   * Check for the Private Key format type.  Private Key may be in
   * P E M or in D E R  formats.  What ever may be the format
   * convert it into EVP_PKEY format.
   */
  pBio = BIO_new_mem_buf(pBuf, lBufLen);
  if(pBio == NULL)
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
     "HttpsMgrConvertKeyToInternalFormat :: BIO_new_mem_buf returned error\r\n");
    return OF_FAILURE;
  }

  if(of_strncmp(pBuf, "-----BEGIN", 10) != 0)
  {
    Trace(HTTPS_ID, TRACE_INFO,
      "HttpsMgrConvertKeyToInternalFormat:: Trying for DER format\n\r");
    if(ucKeyType == HTTPS_PKI_PRIVATE_KEY)
      *pKey = d2i_PrivateKey_bio(pBio, NULL);
    else if(ucKeyType == HTTPS_PKI_PUBLIC_KEY)
      *pKey = d2i_PUBKEY_bio(pBio, NULL);
    else
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
        "HttpsMgrConvertKeyToInternalFormat :: Invalid Key type passed\r\n");
    return OF_FAILURE;
  }

    if(*pKey == NULL)
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
       "HttpsMgrConvertKeyToInternalFormat :: \
          Passed Key is not in D E R format\r\n");
      BIO_free(pBio);
      return OF_FAILURE;
    }
  }
  else
  {
    Trace(HTTPS_ID, TRACE_INFO,
      "HttpsMgrConvertKeyToInternalFormat :: Trying for PEM format\r\n");

    *pKey = PEM_read_bio_PrivateKey(pBio, NULL, NULL, NULL);

    if(*pKey == NULL)
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
       "HttpsMgrConvertKeyToInternalFormat :: \
          Passed Key is not in P E M format\r\n");
      BIO_free(pBio);
      return OF_FAILURE;
    }
  }
  BIO_free(pBio);

  return OF_SUCCESS;
}

 int32_t HTTPSConvertCertToInternalFormat(unsigned char  *pBuf,
                                       int32_t    lBufLen,
                                       X509     **pX509Cert,
                                       PKCS7    **pPKCS7Cert)
{
  BIO       *pBio = NULL;

  /*
   * Check for the certificate type.  Certificate may be in
   * P E M or in D E R  formats.  What ever may be the format
   * convert it into Internal format.
   */
  if((pX509Cert == NULL) && (pPKCS7Cert == NULL))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
      "HTTPSConvertCertToInternalFormat :: \
            \r\nInvalid arguments to the fuction\r\n");
    return OF_FAILURE;
  }

  if(of_strncmp(pBuf, "-----BEGIN", 10) != 0)
  {
    Trace(HTTPS_ID, TRACE_INFO,
      "HTTPSConvertCertToInternalFormat :: Trying for DER format\n\r");
    /* Try for X509 format if fails try for PKCS7 format */
    if(pX509Cert != NULL)
    {
      *pX509Cert = d2i_X509(NULL, &pBuf, lBufLen);
      if(*pX509Cert != NULL)
      {
        return OF_SUCCESS;
      }
      Trace(HTTPS_ID, TRACE_INFO,
        "HTTPSConvertCertToInternalFormat :: d2i_X509 failed\n\r");
    }
    if(pPKCS7Cert != NULL)
    {
      *pPKCS7Cert = d2i_PKCS7(NULL, &pBuf, lBufLen);
      if(*pPKCS7Cert != NULL)
      {
        return OF_SUCCESS;
      }
      Trace(HTTPS_ID, TRACE_INFO,
        "HTTPSConvertCertToInternalFormat :: d2i_PKCS7 failed\n\r");
    }
    Trace(HTTPS_ID, TRACE_SEVERE,
     "PKIConvertToInternalFormat :: Certificate is not in D E R format\r\n");
    return OF_FAILURE;
  }
  else
  {
    Trace(HTTPS_ID, TRACE_INFO,
      "PKIConvertToInternalFormat :: Trying for PEM format\r\n");

    /* Try for X509 format if fails try for PKCS7 format */
    if(pX509Cert != NULL)
    {
      pBio = BIO_new_mem_buf(pBuf, lBufLen);
      if(pBio == NULL)
      {
        Trace(HTTPS_ID, TRACE_SEVERE,
          "PKIConvertToInternalFormat :: BIO_new_mem_buf returned error\r\n");
        return OF_FAILURE;
      }

      *pX509Cert = PEM_read_bio_X509(pBio, NULL, NULL, NULL);
      if(*pX509Cert != NULL)
      {
        BIO_free(pBio);
        return OF_SUCCESS;
      }
      BIO_free(pBio);
      pBio = NULL;
      Trace(HTTPS_ID, TRACE_INFO,
        "HTTPSConvertCertToInternalFormat :: PEM_read_bio_X509 failed\n\r");
    }
    if(pPKCS7Cert != NULL)
    {
      pBio = BIO_new_mem_buf(pBuf, lBufLen);
      if(pBio == NULL)
      {
        Trace(HTTPS_ID, TRACE_SEVERE,
          "PKIConvertToInternalFormat :: BIO_new_mem_buf returned error\r\n");
        return OF_FAILURE;
      }

      *pPKCS7Cert = PEM_read_bio_PKCS7(pBio, NULL, NULL, NULL);
      if(*pPKCS7Cert != NULL)
      {
        BIO_free(pBio);
        return OF_SUCCESS;
      }
      BIO_free(pBio);
      pBio = NULL;
      Trace(HTTPS_ID, TRACE_INFO,
        "HTTPSConvertCertToInternalFormat :: PEM_read_bio_PKICS7 failed\n\r");
    }
    Trace(HTTPS_ID, TRACE_SEVERE,
       "HTTPSConvertToInternalFormat :: Certificate is not in P E M format\r\n");
    return OF_FAILURE;
  }

  return OF_SUCCESS;
}
#endif  /* INTOTO_CMS_SUPPORT */

/***************************************************************************
 * Function Name : HTTPSCertInit
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  :
 ***************************************************************************/

int32_t HTTPSCertsInit( void )
{
   int32_t  ii;

  /**
   * Clearing the certificate buffers.
   */
  memset(HttpsKeyCertPair, '\0',
         (HTTPS_MAX_NUM_OF_KEY_CERT * sizeof(HttpsKeyCertPair_t)));

  memset(HttpsCACert, '\0',
         (HTTPS_MAX_NUM_OF_CA_CERT * sizeof(HttpsCACertInfo_t)));

  for (ii = 0; ii < HTTPS_MAX_NUM_OF_CA_CERT; ii++)
  {
    HttpsCACert[ii].bValid = FALSE;
  }

  /**
   * ptrustedCACerts is a stack which hold the trusted CA certificates in
   * internal form i.e. X509 form.
   */

  for (ii = 0; ii < 10; ii++)
  {
    HttpsRSAKeyPairs[ii].bUsed = FALSE;
    memset(HttpsRSAKeyPairs[ii].aIDName, 0, HTTPS_ID_NAME_LEN);
    HttpsRSAKeyPairs[ii].pMyCert    = NULL;
    HttpsRSAKeyPairs[ii].pMyPrivKey = NULL;
  }
  if ((HttpsRSATrustedCACerts = sk_X509_new(NULL)) == NULL)
  {

    Trace(HTTPS_ID, TRACE_SEVERE,
          "HTTPSCertsInit: Creation of Stack of Trusted CA certs failed\n");
    return OF_FAILURE;
  }

  HttpsInitMblks();

#ifdef INTOTO_CMS_SUPPORT

  /* Initializing the Key-Cert Pair structure with the Self-signed certifcate and its Key */
  if(HttpsMgrConvertKeyToInternalFormat(HTTPS_PKI_PRIVATE_KEY,
                                        CMSPKey, of_strlen(CMSPKey),
                                        &pCMSPKey) != OF_SUCCESS)
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HTTPSCertsInit: ZYXEL PKEY INITIALIZATION FAILED\n");
    return OF_FAILURE;
  }

  if(HTTPSConvertCertToInternalFormat(CMSTrustSignedCert,
                                      of_strlen(CMSTrustSignedCert),
                                      &pCMSTrustSignedCert,
                                      NULL) != OF_SUCCESS)
  {
  Trace(HTTPS_ID, TRACE_SEVERE,
          "HTTPSCertsInit: ZYXEL SELF-SIGNED CERT INITIALIZATION FAILED\n");
      HttpDebug("TRUST-SIGNED CERT INITIALIZATION FAILED\n");
    return OF_FAILURE;
  }

  if(HTTPSConvertCertToInternalFormat(CMSSelfSignedCert,
                                      of_strlen(CMSSelfSignedCert),
                                      &pCMSSelfSignedCert,
                                      NULL) != OF_SUCCESS)
  {
  Trace(HTTPS_ID, TRACE_SEVERE,
          "HTTPSCertsInit: ZYXEL SELF-SIGNED CERT INITIALIZATION FAILED\n");
      HttpDebug("SELF-SIGNED CERT INITIALIZATION FAILED\n");
    return OF_FAILURE;
  }
  /* Push Trusted certificate */ 
  sk_X509_push(HttpsRSATrustedCACerts, pCMSTrustSignedCert);
  
  /* Put the Private key and Self-signed certifcate */
  ii = 0;
  HttpsRSAKeyPairs[ii].bUsed = TRUE;
  of_strcpy(HttpsRSAKeyPairs[ii].aIDName, "CMS");
  HttpsRSAKeyPairs[ii].pMyCert    = pCMSSelfSignedCert;
  HttpsRSAKeyPairs[ii].pMyPrivKey = pCMSPKey;
  printf("HTTPSCertsInit: LOADED SELF AND TRUSTED CERTS\n");
#endif  /* INTOTO_CMS_SUPPORT */
  Trace(HTTPS_ID, TRACE_SEVERE,
        "HTTPSCertsInit: LOADED SELF AND TRUSTED CERTS\n");
  return OF_SUCCESS;
} /* HTTPSCertsInit() */

/***************************************************************************
 * Function Name : HttpsGetTrustedCACerts()
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  :
 ***************************************************************************/
/**/
int32_t HttpsGetTrustedCAs( uint16_t       ucEncType,
                            uint16_t       usCertType,
                            uint16_t       *pNumCAs,
                            HttpsCAName_t  *pCAName )
{
  int16_t         iIndex           = 0;
  uint16_t        ret              = 0;
  STACK_OF(X509)  *pTrustedCACerts = NULL;
  unsigned char        *pTemp           = NULL;
  HX509Cert_t     *pCert           = NULL;
  HX509Name_t     *pSubject        = NULL;

  pTrustedCACerts = HttpsRSATrustedCACerts;

  if (pTrustedCACerts == NULL)
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGetTrustedCACerts : CA Certs not loaded \n");
    return OF_FAILURE;
  }

  *pNumCAs = sk_X509_num(pTrustedCACerts);

  for (iIndex = 0; iIndex < *pNumCAs; iIndex++)
  {
    pCert     = (HX509Cert_t *) sk_X509_value(pTrustedCACerts, iIndex);
    pSubject  = (HX509Name_t *)X509_get_subject_name(pCert);
    M_ASN1_I2D_len(pSubject, i2d_X509_NAME);

    /**
     * i2d_X509_NAME internally increments the address to point to the
     * address beyond the Subject Name. Decrement the address accordingly.
     */
    pTemp = pCAName[iIndex].aCA;

    /**
     * If the I2D conversion failed then, initialize all the lengths of the
     * DER encoded lengths to ZERO. May be we can use this to know whether
     * the value in the aCA i.e. CA name is valid or not.
     */

    if (!(pCAName[iIndex].usLength = i2d_X509_NAME(pSubject, &pTemp)))
    {
      if (iIndex)
      {
        while (iIndex-- >= 0)
        {
          pCAName[iIndex].usLength = 0;
        }

        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsGetTrustedCAs(): i2d_X509NAME failed#1\n");
        return OF_FAILURE;
      }
    }
  }
  return OF_SUCCESS;
} /* HttpsGetTrustedCAs() */

/***************************************************************************
 * Function Name : HttpsGetPrivKey
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  :
 ***************************************************************************/

int32_t HttpsGetPrivateKey( HttpsCertInfo_t  *pCertInfo,
                            void           **pPrivKeyInfo )
{
  bool       bRSA              = TRUE;
  uint16_t      usIndex           = 0;
  int32_t       theError          = OF_SUCCESS;
  unsigned char      *pTemp            = NULL;
  ASN1_INTEGER  *inASN1Serial     = NULL;
  HX509Name_t   *inX509IssuerName = NULL;

  pTemp = pCertInfo->pSerialNum;

  if ((d2i_ASN1_INTEGER(&inASN1Serial, (const unsigned char **) &pTemp,
                        (int32_t) pCertInfo->usSerialNumLen)) == NULL)
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGetPrivKey: Failed to convert DER to ASN1 integer\n");
    return OF_FAILURE;
  }

  pTemp = pCertInfo->pIssuerName;

  if ((d2i_X509_NAME(&inX509IssuerName, (const unsigned char **) &pTemp,
                     pCertInfo->name_length) == NULL))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGetPrivKey: Failed to convert DER to x509 name\n");
    return OF_FAILURE;
  }

  if (HttpsFindPrivKeyIndex(HttpsRSAKeyPairs, inASN1Serial,
                            inX509IssuerName, &usIndex) != HTTPS_NO_ERROR)
  {
    bRSA = FALSE;
  }

  if (usIndex == HTTPS_MAX_SELF_CERTS)
  {
    theError = HTTPS_PRIVKEY_NOT_FOUND;
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGetPrivKey: Couldn't find private key\n");
  }
  else
  {
    theError = OF_SUCCESS;

    if (bRSA)
    {
      *pPrivKeyInfo = (void *)
                          HttpsRSAKeyPairs[usIndex].pMyPrivKey->pkey.rsa;
    }
  }

  /**
   * d2i_xxx creates the corresponding object so free this just b4 you get
   * out this function.
   */
  X509_NAME_free((X509_NAME *)inX509IssuerName);
  ASN1_INTEGER_free(inASN1Serial);
  return theError;
} /* HttpsGetPrivateKey() */

/***************************************************************************
 * Function Name : HttpsFindPrivKeyIndex()
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  :
 ***************************************************************************/

int32_t HttpsFindPrivKeyIndex( HttpsMyKeyCert_t  *aMyKeyPairs,
                               ASN1_INTEGER      *inASN1Serial,
                               HX509Name_t       *inX509IssuerName,
                               uint16_t          *pIndex )
{
  ASN1_INTEGER  *theASN1Serial     = NULL;
  HX509Name_t   *theX509IssuerName = NULL;
  bool       bFound = FALSE;

  for ((*pIndex) = 0;
       (*pIndex) < HTTPS_MAX_SELF_CERTS &&
        aMyKeyPairs[*pIndex].pMyCert != NULL;
       (*pIndex)++)
  {
    theASN1Serial     = X509_get_serialNumber(aMyKeyPairs[*pIndex].pMyCert);
    theX509IssuerName = (HX509Name_t *)
                            X509_get_issuer_name(
                                aMyKeyPairs[*pIndex].pMyCert);

    if (ASN1_INTEGER_cmp(inASN1Serial, theASN1Serial))
    {
      continue;
    }

    if (X509_NAME_cmp(inX509IssuerName, theX509IssuerName))
    {
      continue;
    }
    else
    {
      bFound = TRUE;
      break;
    }
  }

  if (bFound)
  {
    return OF_SUCCESS;
  }
  else
  {
    return OF_FAILURE;
  }
} /* HttpsFindPrivKeyIndex() */

/***************************************************************************
 * Function Name : HttpsValidateCert()
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  :
 ***************************************************************************/

int32_t HttpsValidateCert( HttpsCertIdInfo_t   *pCertIdInfo,
                           uint8_t             usInNumCerts,
                           uint8_t             ucCertEncoding,
                           HttpsCertificate_t  *pCerts,
                           uint8_t             usInNumCRLs,
                           uint8_t             ucCRLEncoding,
                           void              (**pFreeFn)(EVP_PKEY *),
                           void              **pFreeFnArg,
                           void              **pPeerPubKey )
{
  int32_t              ulKeyType      = 0;
  int32_t              theError       = 0;
  HX509Store_t         *pStore        = NULL;
  HX509Cert_t          *pCert         = NULL;
  STACK_OF(X509)       *skCerts       = NULL;
  EVP_PKEY             *pKey          = NULL;
  HX509StoreContext_t  StoreContext;
  STACK_OF(X509)       *pTrustedCerts = NULL;
  uint32_t             lNumOfCerts    = 0, ii = 0;

  *pFreeFn    = NULL;
  *pFreeFnArg = NULL;

  memset(&StoreContext, 0, sizeof(HX509StoreContext_t));

  if ((pStore = (HX509Store_t *) X509_STORE_new()) == NULL)
  {
    theError = HTTPS_MEM_ALLOC_FAILED;
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsValidateCert: Creation of X509Store failed\n");
    return OF_FAILURE;
  }

  if (HttpsGetPeerCertByID(pCerts, usInNumCerts, pCertIdInfo,
                           &skCerts, &pCert) != OF_SUCCESS)
  {
    theError = HTTPS_FAILED;
    X509_free(pCert);
    X509_STORE_free(pStore);
    sk_X509_pop_free(skCerts, X509_free);
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsValidateCert: GetPeerCertByID returned error \n");
    return theError;
  }

  if (pCert == NULL)
  {
    theError = HTTPS_FAILED;
    X509_free(pCert);
    X509_STORE_free(pStore);
    sk_X509_pop_free(skCerts, X509_free);
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsValidateCert: pCert is NULL\n");
    return theError;
  }

  ulKeyType = OBJ_obj2nid(pCert->cert_info->key->algor->algorithm);

  if ((ulKeyType == EVP_PKEY_RSA) ||
      (ulKeyType == EVP_PKEY_RSA2))
  {
    pTrustedCerts = HttpsRSATrustedCACerts;
  }
  else
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsValidateCert::Invalid SIGNATURE Type\n\r");
  }

  X509_STORE_CTX_init(&StoreContext, pStore, pCert, skCerts);

  if (pTrustedCerts != NULL)
  {
    lNumOfCerts = sk_X509_num(pTrustedCerts);

    for (ii = 0; ii < lNumOfCerts; ii++)
    {
      if (X509_STORE_add_cert(pStore,
                              sk_X509_value(pTrustedCerts, ii)) == 0)
      {
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsValidateCert:: X509_STORE_add_cert failed\n\r");
        X509_free(pCert);
        X509_STORE_free(pStore);
        sk_X509_pop_free(skCerts, X509_free);
        return OF_FAILURE;
      }
    }
  }
  else
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HTTPSValidateCert: pTrustedCerts is NULL\n\r");
    X509_free(pCert);
    X509_STORE_free(pStore);
    sk_X509_pop_free(skCerts, X509_free);
    return OF_FAILURE;
  }

  if (X509_verify_cert(&StoreContext) != 1)
  {
    theError = HTTPS_INVALID_CERTIFICATE;
    X509_free(pCert);
    X509_STORE_free(pStore);
    sk_X509_pop_free(skCerts, X509_free);
    X509_STORE_CTX_cleanup(&StoreContext);
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsValidateCert: X509_verify_cert failed\n");
    return theError;
  }

  X509_STORE_free(pStore);
  sk_X509_pop_free(skCerts, X509_free);

  /**
   * Before freeing certificate context extract the error from it and give
   * trace.
   */
  switch (StoreContext.error)
  {
  case X509_V_ERR_CERT_NOT_YET_VALID:
    Trace(HTTPS_ID, TRACE_LOG, "ERROR# CERTIFICATE NOT YET VALID\n\r");
    break;
  case X509_V_ERR_CERT_HAS_EXPIRED:
    Trace(HTTPS_ID, TRACE_LOG, "ERROR# CERTIFICATE IS EXPIRED\n\r");
    break;
  case X509_V_ERR_CERT_SIGNATURE_FAILURE:
  case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
    Trace(HTTPS_ID, TRACE_LOG, "ERROR# SIGNATURE VERIFICATION FAILURE\n\r");
    break;
  case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
    Trace(HTTPS_ID, TRACE_LOG, "ERROR# UNABLE TO GET ISSUER CERT\n\r");
    break;
  }

  X509_STORE_CTX_cleanup(&StoreContext);

  if ((pKey = X509_get_pubkey(pCert)) == NULL)
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsValidateCert: X509_get_pubkey failed \n");
    X509_free(pCert);
    return OF_FAILURE;
  }

  X509_free(pCert);
  ulKeyType = pKey->type;

  if ((ulKeyType == EVP_PKEY_RSA) ||
      (ulKeyType == EVP_PKEY_RSA2))
  {
    *pPeerPubKey = (void *) pKey->pkey.rsa;
  }
  else
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "x509_verify_cert():  Invalid PubKey type\n");
    EVP_PKEY_free(pKey);
    return OF_FAILURE;
  }

  /**
   * We don't free the certificate as the public key is used by the calling
   * function to verify the signature value. Instead set this to be the
   * free function and the certificate pointer to be the free function
   * argument.
   */
  *pFreeFn    = EVP_PKEY_free;
  *pFreeFnArg = pKey;
  return OF_SUCCESS;
} /* HttpsValidateCert() */

/***************************************************************************
 * Function Name : HttpsGetExtValue
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 ***************************************************************************/

int32_t HttpsGetExtValue( unsigned char  *pBuffer,
                          unsigned char  *value_p )
{
  int32_t   iLength   = pBuffer[1];
  uint16_t  usTempLen = 0;

  pBuffer += 2;

  while (iLength > 0)
  {
    if (!(*pBuffer & 0x80))
    {
      Trace(HTTPS_ID, TRACE_SEVERE, "HttpsGetExtValue:: Invalid context\n");
      return OF_FAILURE;
    }

    if ((*pBuffer++ & 0x7f) == 7)
    {
      if (*pBuffer++ != 4)
      {
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsGetExtValue :: Invalid IPAddr Len \n");
        return OF_FAILURE;
      }

      of_memcpy(value_p, pBuffer, 4);
      return OF_SUCCESS;
    }
    else
    {
      usTempLen  = *pBuffer++;
      pBuffer   += usTempLen;
      iLength   -= usTempLen;
    }
  }
  return OF_FAILURE;
} /* HttpsGetExtValue() */

/***************************************************************************
 * Function Name : HttpsGetPeerCertByID
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  :
 ***************************************************************************/

int32_t HttpsGetPeerCertByID( HttpsCertificate_t  *pInCerts,
                              uint8_t             usInNumCerts,
                              HttpsCertIdInfo_t   *pCertIdInfo,
                              STACK_OF(X509)      **certSK,
                              HX509Cert_t         **pOutCert )
{
  bool      bFound      = FALSE;
  unsigned char     aSubName[HTTPS_PARAMS_SUB_NAME_LEN];
  int32_t      iCertLen    = 0;
  int32_t      iIndex      = 0;
  int32_t      theError    = OF_SUCCESS;
  unsigned char     * pCertData = NULL;
  HX509Cert_t  * pX509Cert = NULL;

  /**
   * Create stack of X509Certs.
   */
  if ((*certSK = sk_X509_new(NULL)) == NULL)
  {
    theError = HTTPS_MEM_ALLOC_FAILED;
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGetPeerCertByID: sk_X509_new failed \n");
    return theError;
  }

  /**
   * Go through all the certs in the array in DER encoded form.
   *  1) convert the DER encoded certificate to internal form
   *     d2i_x509() does this but this won't convert the
   *     extensions.
   *  2) For all the  extension in the CERT call
   *     d2i_X509_EXTENSIONS() and push it onto the stack
   *     in the X509Cert
   *  3) at the end find out for the extension with the
   *     NID value NID_subject_alt_name
   *     If an extension is found then compare the ID value
   *     with the value
   */
  for (iIndex = 0; iIndex < usInNumCerts; iIndex++)
  {
    pX509Cert = NULL;
    pCertData = pInCerts[iIndex].pCert->pData;
    iCertLen  = pInCerts[iIndex].usLength;


    /**
     * Decode the certificate to internal form X509.
     */
    if ((pCertData == NULL) ||
        (d2i_X509(&pX509Cert, (const unsigned char **) &pCertData, iCertLen) == NULL))
    {
      theError = HTTPS_MEM_ALLOC_FAILED;
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsGetPeerCertByID: d2i_X509 failed \n");
    }

    /**
     * BAKEOFF : MAY 99
     * Since most people are not sending other than peer certificate,
     * we just ignore subject alt name...This was necessary because
     * the VPNet certificate didn't have a subject_alt_name extension.
     */

    /**
     * If we have not found the leaf certificate i.e. the certificate which
     * we have to verify search in the extensions for the Extension
     * subject_alt_name, then compare the values for matching
     */

    if (!bFound)
    {
      if (pCertIdInfo->usIdType == HTTPS_ID_DER_ASN1_DN)
      {
        X509_NAME_oneline(X509_get_subject_name(pX509Cert),
                          (char *) aSubName, HTTPS_PARAMS_SUB_NAME_LEN);
        if (memcmp(pCertIdInfo->aIdData, aSubName,
                   pCertIdInfo->usIdLength) == 0)
        {
          bFound = TRUE;
          *pOutCert = pX509Cert;
          continue;
        }
      }
      else
      {
        if (HttpsFindSubAltNameInCert(pCertIdInfo,
                                      pX509Cert) == OF_SUCCESS)
        {
          bFound = TRUE;
          *pOutCert = pX509Cert;
          continue;
        }
      }
    }
    sk_X509_insert(*certSK, pX509Cert, (int32_t) iIndex);
  }

  if (theError)
  {
    HttpsFreeX509Stack(*certSK);
  }

  return theError;
} /* HttpsGetPeerCertByID() */

/***************************************************************************
 * Function Name : HttpsFreeX509Stack
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  :
 ***************************************************************************/

void HttpsFreeX509Stack( STACK_OF(X509)  *skCert )
{
  uint8_t      numCerts   = sk_X509_num(skCert);
  HX509Cert_t  *pX509Cert = NULL;

  while (numCerts)
  {
    numCerts--;
    pX509Cert = (HX509Cert_t *)sk_X509_value(skCert, numCerts);
    X509_free(pX509Cert);
  }
  of_free(skCert);
} /* HttpsFreeX509Stack() */

/***************************************************************************
 * Function Name : HttpsFindInStackBySubject()
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  :
 ***************************************************************************/

HX509Cert_t *HttpsFindInStackBySubject( STACK_OF(X509)  *pStack,
                                        HX509Name_t     *pInSubName )
{
  uint8_t      usUp = 0;
  HX509Cert_t  *x   = NULL;

  if ((!pStack) ||
      (!pInSubName))
  {
    return NULL;
  }

  while (usUp < sk_X509_num(pStack))
  {
    x = (HX509Cert_t *) sk_X509_value(pStack, usUp);

    if (X509_NAME_cmp(X509_get_subject_name(x), pInSubName) == 0)
    {
      return x;
    }
    usUp++;
  }
  return NULL;
} /* HttpsFindInStackBySubject() */

/***************************************************************************
 * Function Name : HttpsTestCert()
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  :
 ***************************************************************************/

int32_t HttpsTestCert( void )
{
  uint16_t             NumCAs = 0;
  uint16_t             index, CAIndex;
  HX509Cert_t          *pCACert = NULL, *pCert = NULL;
  HttpsMyKeyCert_t     *aMyKeyPairs;
  HX509Store_t         *pStore = NULL;
  HX509StoreContext_t  StoreContext;
  STACK_OF(X509)       *pTrustedCACerts = NULL;
  uint32_t             ulKeyType = 0;
  uint32_t             lNumOfCerts = 0, ii = 0;
  STACK_OF(X509)       *pTrustedCerts = NULL;

  memset(&StoreContext, 0, sizeof(HX509StoreContext_t));
  aMyKeyPairs = HttpsRSAKeyPairs;

  for ((index) = 0;
       (index) < HTTPS_MAX_SELF_CERTS &&
        aMyKeyPairs[index].pMyCert != NULL;
       (index)++)
  {
    pCert = aMyKeyPairs[index].pMyCert;

    if ((pStore = (HX509Store_t *) X509_STORE_new()) == NULL)
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsValidateCert: Creation of X509Store failed\n");
      continue;
    }

    ulKeyType = OBJ_obj2nid(pCert->cert_info->key->algor->algorithm);
    if ((ulKeyType == EVP_PKEY_RSA) ||
        (ulKeyType == EVP_PKEY_RSA2))
    {
      pTrustedCerts = HttpsRSATrustedCACerts;
    }
    else
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsTestCert::Invalid SIGNATURE Type\n\r");
    }

    X509_STORE_CTX_init(&StoreContext, pStore, pCert, NULL);

    if (pTrustedCerts != NULL)
    {
      lNumOfCerts = sk_X509_num(pTrustedCerts);

      for (ii = 0; ii < lNumOfCerts; ii++)
      {
        if (X509_STORE_add_cert(pStore,
                                sk_X509_value(pTrustedCerts, ii)) == 0)
        {
          Trace(HTTPS_ID, TRACE_SEVERE,
                "HttpsTestCert:: X509_STORE_add_cert failed\n");
          return OF_FAILURE;
        }
      }
    }
    else
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsTestCert: pTrustedCerts is NULL\n");
      return OF_FAILURE;
    }

    if (X509_verify_cert(&StoreContext) != 1)
    {
      X509_STORE_free(pStore);
      X509_STORE_CTX_cleanup(&StoreContext);
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsTestCert: verify_cert FAILED for the self cert %s \n",
            aMyKeyPairs[index].aIDName);
      continue;
    }
    else
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsTestCert: verify_cert SUCCESS for the self cert %s \n",
            aMyKeyPairs[index].aIDName);
    }

    X509_STORE_free(pStore);
    X509_STORE_CTX_cleanup(&StoreContext);
  }

  Trace(HTTPS_ID, TRACE_SEVERE, "Now looking for RSA CA CERT\n\r");

  pTrustedCACerts = HttpsRSATrustedCACerts;

  if (pTrustedCACerts != NULL)
  {
    NumCAs = sk_X509_num(pTrustedCACerts);
  }

  for (CAIndex = 0; CAIndex < NumCAs; CAIndex++)
  {
    pCACert = (HX509Cert_t *) sk_X509_value(pTrustedCACerts, CAIndex);

    if (HttpsVerifyTrustedCert(pCACert) != OF_SUCCESS)
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsTestCert: X509_verify_cert failed for CA.\r\n");
      continue;
    }
    else
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsTestCert: X509_verify_cert SUCCESS for CA.\r\n");
    }
  }

  Trace(HTTPS_ID, TRACE_SEVERE, "HttpsTestCert :: exit\n\r");

  return OF_SUCCESS;
} /* HttpsTestCert() */

/***********************************************************************
 * Function Name : HttpsCmpLifeTimeWithCert
 * Description   :
 * Input         :
 * Output        :
 * Return value  :
 **********************************************************************/
/**/
int32_t HttpsCmpLifeTimeWithCert( unsigned char  *pData,
                                  uint16_t  usLength,
                                  int32_t   iPolicyLifeTime,
                                  uint32_t  *pCertNotAfter )
{
  X509          *pX509 = NULL;
  int32_t       iRetVal;
  time_t        CurTime;
  unsigned char      *ptr = pData;
  int32_t       year, mon, day, hr, min, sec, tmpYear;
  time_t        ExpTime;
  ASN1_UTCTIME  *pNotAfter;
  struct tm     tmInfo;

  /**
   * Input validations.
   */
  if ((pData == NULL) ||
      (usLength <= 0))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsCmpLifeTimeWithCert: Input validations failed\n");
    return OF_FAILURE;
  }

  /**
   * Get the current time in seconds and add the lifetime to it.
   */
  time(&CurTime);

  /**
   * Convert the certificate from DER form to INTERNAL form.
   */
  if ((d2i_X509(&pX509, (const unsigned char **) &ptr, usLength) == NULL))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsCmpLifeTimeWithCert: Failed to convert DER to x509 name\n");
    return OF_FAILURE;
  }

  /**
   * Calculate the time in seconds.  Remember this is the
   * time i.e.., local time converted to UTC time in seconds.
   */
  pNotAfter = X509_get_notAfter(pX509);

  sscanf((char *) pNotAfter->data, "%02d%02d%02d%02d%02d%02d", &year, &mon, &day,
         &hr, &min, &sec);

  tmpYear = 2000+year;
  year = tmpYear-1900;

  tmInfo.tm_sec   = sec;
  tmInfo.tm_min   = min;
  tmInfo.tm_hour  = hr;
  tmInfo.tm_mday  = day;
  tmInfo.tm_mon   = mon-1;
  tmInfo.tm_year  = year;
  tmInfo.tm_wday  = 0;
  tmInfo.tm_yday  = 0;
  tmInfo.tm_isdst = 0;
  ExpTime = mktime(&tmInfo);

  *pCertNotAfter = ExpTime;
  X509_free(pX509);

  /**
   * If life time in sec is not selected in policy return SUCCESS.
   */
  if (iPolicyLifeTime > 0)
  {
    if (ExpTime >= CurTime + iPolicyLifeTime)
    {
      return iPolicyLifeTime;
    }

    iRetVal = ExpTime - CurTime;

    if (iRetVal >= 300)
    {
      return iRetVal;
    }
    else
    {
      return OF_FAILURE;
    }
  }
  else
  {
    return (ExpTime - CurTime);
  }
} /* HttpsCmpLifeTimeWithCert() */

/************************************************************************
 * Function Name : HTTPSGetCACert
 * Description   : This function gives the CA certificate in X509 format.
 * Input         : pointer to CA request, request length and pointer to peer
 *                 Certificate in X509 format.
 * Output        : None
 * Return value  : Pointer to CA Certificate in X509 format.
 ***********************************************************************/

HX509Cert_t *HTTPSGetCACert( unsigned char     *pCAReq,
                             uint16_t     usLength,
                             HX509Cert_t  *pPeerCert )
{
  HX509Cert_t  *pCACert     = NULL;
  X509_NAME    *pIssuerName = NULL;

  /**
   * If pCAReq has name convert it to X509 format else find
   * issuer name from pPeerCert and look for CA certificate in Trusted
   * CA certs in X509 format.
   */
  if ((pCAReq != NULL) &&
      (pCAReq[0] != 0))
  {
    pIssuerName = d2i_X509_NAME(NULL, (const unsigned char **) &pCAReq, usLength);
  }
  else
  {
    if (pPeerCert == NULL)
    {
      return NULL;
    }
    pIssuerName = X509_get_issuer_name(pPeerCert);
  }

  if (pIssuerName == NULL)
  {
    return NULL;
  }

  pCACert = HttpsFindInStackBySubject(HttpsRSATrustedCACerts, pIssuerName);

  return pCACert;
} /* HTTPSGetCACert() */

#endif /* OF_HTTPD_SUPPORT && OF_HTTPS_SUPPORT */


