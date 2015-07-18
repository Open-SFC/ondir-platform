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
/*  File        : httpsreq.c                                              */
/*                                                                        */
/*  Description : This file contains https request related function       */
/*                implementations.                                        */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.1      05.14.04    DRAM      Modified during the code review.     */
/**************************************************************************/

#if defined(OF_HTTPD_SUPPORT) && defined(OF_HTTPS_SUPPORT)

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "httpdinc.h"
#include "htpdglb.h"
#include <openssl/x509v3.h>
#include "hcmgrmbl.h"
#include "hcmgrgdf.h"
#include "hcmgrgif.h"
#include "hcmgrldf.h"
#include "hcmgrlif.h"

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  MIN_KEY_LENGTH               (384)

/************************************************************
 * * * *  V a r i a b l e    D e c l a r a t i o n s  * * * *
 ************************************************************/

#ifdef OPENSSL_9_7
#ifdef NID_postalCode
#undef NID_postalCode
#endif 
uint32_t  NID_postalCode;
#endif

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

static
int32_t Httpsreq_fix_data( int32_t  nid,
                           int32_t  *type,
                           int32_t  len,
                           int32_t  min,
                           int32_t  max );
/**
 *
 */

HX509Req_t *HttpsCertReqNew( void );

/**
 *
 */

void HttpsCertReqFree( HX509Req_t  *a );

/**
 * This function is used to
 */

int32_t Httpsadd_DN_object( HX509Name_t  *n,
                            char      *text,
                            char      *def,
                            char      *value,
                            int32_t      nid,
                            int32_t      min,
                            int32_t      max);

/****************************************************************
 * * * *  F u n c t i o n    I m p l e m e n t a t i o n  * * * *
 ****************************************************************/

/***************************************************************************
 * Function Name : HttpsGenCertReq
 * Description   : This function generates the certificate request.
 * Input         : None
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 ***************************************************************************/

int32_t HttpsGenCertReq( HttpsCertReqInfo_t    *pCertReqInfo,
                         unsigned char              *pPrivbuf,
                         unsigned char              *pCertbuf,
                         HttpsCertReqParams_t  *pParams )
{
  EVP_PKEY                  *pkey         = NULL;
  HX509Req_t                *preq         = NULL;
#ifdef OPENSSL_9_7
  const  EVP_MD             *pDigest;
#else
  EVP_MD                    *pDigest      = NULL;
#endif /*OPENSSL_9_7*/
  int32_t                   iRetVal;
  unsigned char                  aSubAltName[HTTPS_PARAMS_SUB_NAME_LEN];
  bool                   bFlag         = FALSE;
  STACK_OF(X509_EXTENSION)  *skExtensions = NULL;
  X509_EXTENSION            *pSubExt      = NULL;
  unsigned char                  ucCT;
  int32_t                   uileng;

  /**
   * Input Validations...
   */
  if (pCertReqInfo == NULL)
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: Invalid parameters\n");
    return OF_FAILURE;
  }

  /**
   * Allocate space for the public-private key pair..
   */

  if ((pkey = EVP_PKEY_new()) == NULL)
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: MemAlloc failure\n");
    return OF_FAILURE;
  }

  /**
   * Generate a public-private key pair..
   */
  switch (pCertReqInfo->usCertType)
  {
  case RSA_MD5:
  case RSA_SHA1:
    if ((pCertReqInfo->cr_params.rsa_params.usNumBits < MIN_KEY_LENGTH))
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsGenCertReq : Invalid RSA parameters\n");
      EVP_PKEY_free(pkey);
      return OF_FAILURE;
    }

    if (!EVP_PKEY_assign_RSA(pkey,
            RSA_generate_key(pCertReqInfo->cr_params.rsa_params.usNumBits,
            0x10001, NULL, NULL)))
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsGenCertReq: EVP_PKEY assign failed\n");
      EVP_PKEY_free(pkey);
      return OF_FAILURE;
    }

    ucCT='r';

    if (pCertReqInfo->usCertType == RSA_MD5)
    {
      pDigest = (EVP_MD *) EVP_md5();
    }
    else
    {
      pDigest = (EVP_MD *) EVP_sha1();
    }

    break;
  default:
    Trace(HTTPS_ID, TRACE_SEVERE, "HttpsGenCertReq:: Invalid certificate type\n");
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

  /**
   * Prepare the Certificate Request..
   */
  if ((preq = HttpsCertReqNew()) == NULL)
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: X509_req_new failed\n");
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

  /**
   * Version Number..
   */
  if (!ASN1_INTEGER_set(preq->req_info->version, 0L))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: ASN1_INTEGER_set failed\n");
    HttpsCertReqFree(preq);
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

  /**
   * Subject Name...Filled with two RDNs...
   */

  /**
   * RDN # 1 : Country Name...
   */

  if (!Httpsadd_DN_object((HX509Name_t *) preq->req_info->subject, NULL,
                          NULL, (char *) pParams->ucCn, NID_countryName, 2, 2))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: Httpsadd_DN_object failed\n");
    HttpsCertReqFree(preq);
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

#ifdef OPENSSL_9_7
  if (NID_postalCode == 0)
  {
    NID_postalCode = OBJ_create("2.5.4.17", "2.5.4.17",
                                "postal code attribute");
  }
#endif

  /**
   * RDN #New::2: Postal code...
   */
  if (!Httpsadd_DN_object((HX509Name_t *) preq->req_info->subject, NULL,
                          NULL, (char *) pParams->aPostalCode, NID_postalCode, 0,
                          10))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: Httpsadd_DN_object failed\n");
    HttpsCertReqFree(preq);
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

  /**
   * RDN #3: State..
   */
  if (!Httpsadd_DN_object((HX509Name_t *) preq->req_info->subject, NULL,
                          NULL, (char *) pParams->aState, NID_stateOrProvinceName,
                          0, 50))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: Httpsadd_DN_object failed\n");
    HttpsCertReqFree(preq);
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

  /**
   * RDN #New::4: Locality Name...
   */
  if (!Httpsadd_DN_object((HX509Name_t *) preq->req_info->subject, NULL,
                          NULL, (char *) pParams->aCity, NID_localityName, 0, 50))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: Httpsadd_DN_object failed\n");
    HttpsCertReqFree(preq);
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

  /**
   * RDN #5 : Organization ..
   */
  if (!Httpsadd_DN_object((HX509Name_t *) preq->req_info->subject, NULL,
                          NULL, (char *) pParams->aOrg, NID_organizationName,
                          0, 50))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: Httpsadd_DN_object failed\n");
    HttpsCertReqFree(preq);
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

  /**
   * RDN #6 : Department ..
   */
  if (!Httpsadd_DN_object((HX509Name_t *) preq->req_info->subject, NULL,
                          NULL, (char *) pParams->aDept, NID_organizationalUnitName,
                          0, 50))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: Httpsadd_DN_object failed\n");
    HttpsCertReqFree(preq);
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

  /**
   * RDN #7 : Common Name (Subject)...
   */
  if (!Httpsadd_DN_object((HX509Name_t *) preq->req_info->subject, NULL,
                          NULL, (char *) pParams->ucSub, NID_commonName, 0, 50))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: Httpsadd_DN_object failed\n");
    HttpsCertReqFree(preq);
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

  of_memset( aSubAltName, 0, HTTPS_PARAMS_SUB_NAME_LEN);

  if (strlen((char *) pParams->aIpAddr))
  {
    of_strcat((char *) aSubAltName, "IP:");
    of_strcat((char *) aSubAltName, (char *) pParams->aIpAddr);
    bFlag = TRUE;
  }

  if (strlen((char *) pParams->aEmailId))
  {
    if (bFlag)
    {
      of_strcat((char *) aSubAltName, ",email:");
    }
    else
    {
      of_strcat((char *) aSubAltName, "email:");
    }

    of_strcat((char *) aSubAltName, (char *) pParams->aEmailId);
    bFlag = TRUE;
  }

  if (strlen((char *) pParams->aDomain))
  {
    if (bFlag)
    {
      of_strcat((char *) aSubAltName, ",DNS:");
    }
    else
    {
      of_strcat((char *) aSubAltName, "DNS:");
    }

    of_strcat((char *) aSubAltName, (char *) pParams->aDomain);
    bFlag = TRUE;
  }

  /**
   * Adding subject alt name extension to the request.
   */
  if ( of_strlen((char *) aSubAltName))
  {
    skExtensions = sk_X509_EXTENSION_new_null();

    if (skExtensions == NULL)
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsGenCertReq:: sk_X509_EXTENSION_new_null failed\n");
      HttpsCertReqFree(preq);
      EVP_PKEY_free(pkey);
      return OF_FAILURE;
    }
    pSubExt = X509V3_EXT_conf_nid(NULL, NULL, NID_subject_alt_name,
                                  (char *) aSubAltName);

    if (pSubExt)
    {
      sk_X509_EXTENSION_push(skExtensions, pSubExt);
    }

    iRetVal = X509_REQ_add_extensions(preq, skExtensions);

    if (iRetVal == 0)
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsGenCertReq:: X509_REQ_add_extensions failed\n");
      HttpsCertReqFree(preq);
      EVP_PKEY_free(pkey);
      return OF_FAILURE;
    }

    sk_X509_EXTENSION_pop_free(skExtensions, X509_EXTENSION_free);
  } /* strlen sub alt */

  /**
   * Set the public key..
   */
  if (!X509_REQ_set_pubkey(preq, pkey))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: X509_REQ_sign failed\n");
    HttpsCertReqFree(preq);
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

  /**
   * Sign the CertReq Info..
   */
  if (!X509_REQ_sign(preq, pkey, pDigest))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: X509_REQ_sign failed\n");
    HttpsCertReqFree(preq);
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

  /**
   * Write the Private Key into the file...
   */
  if (HttpsWritetoFile(pPrivbuf, HTTPS_PRIV_KEY, pkey, ucCT,
                       pParams->aIdName))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: HttpsWritetoFile failed #1\n");
    HttpsCertReqFree(preq);
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

  /**
   * Write the Certificate Request into the file...
   */
  if (HttpsWritetoFile(pCertbuf, HTTPS_CERT_REQ, preq, ucCT,
                       pParams->aIdName))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsGenCertReq:: HttpsWritetoFile failed #2\n");
    HttpsDeleteCerts(pParams->aIdName, 's');
    HttpsCertReqFree(preq);
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

  uileng=strlen((char *) pCertbuf);

  if (uileng > 2047)
  {
    Trace(HTTPS_ID, TRACE_SEVERE, "pCertbuf length exceeds 2k\n");
    HttpsDeleteCerts(pParams->aIdName, 's');
    HttpsCertReqFree(preq);
    EVP_PKEY_free(pkey);
    return OF_FAILURE;
  }

  Trace(HTTPS_ID, TRACE_INFO, "Generation of Certificate Request done\n");
  HttpsCertReqFree(preq);
  EVP_PKEY_free(pkey);
  return OF_SUCCESS;
} /* HttpsGenCertReq() */

/***************************************************************************
 * Function Name : Httpsadd_DN_object
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 ***************************************************************************/

int32_t Httpsadd_DN_object( HX509Name_t  *n,
                            char      *text,
                            char      *def,
                            char      *value,
                            int32_t      nid,
                            int32_t      min,
                            int32_t      max )
{
  int32_t           i, j, ret = 0;
  HX509NameEntry_t  *ne = NULL;
  char           buf[HTTPS_PARAMS_SUB_NAME_LEN];
  
  if(!value)
  {
       Trace(HTTPS_ID, TRACE_SEVERE, "value is NULL\r\n");
       return OF_FAILURE;
  }
  of_memset(buf, 0, HTTPS_PARAMS_SUB_NAME_LEN);

  if ((value != NULL) &&
      (strlen(value)))
  {
    strcpy(buf, value);
    strcat(buf, "\n");
  }

  if (strlen(value) == 0)
  {
    return (1);
  }

  if (buf[0] == '\0')
  {
    return(0);
  }
  else if (buf[0] == '\n')
  {
    if ((def == NULL) ||
        (def[0] == '\0'))
    {
      return(1);
    }

    strcpy(buf, def);
    strcat(buf, "\n");
  }
  else if ((buf[0] == '.') &&
           (buf[1] == '\n'))
  {
    return(1);
  }

  i = strlen(buf);

  if (buf[i-1] != '\n')
  {
    Trace(HTTPS_ID, TRACE_SEVERE, "Weird input\n");
    return(0);
  }

  buf[--i] = '\0';

  j = ASN1_PRINTABLE_type((unsigned char *) buf, -1);

  if (Httpsreq_fix_data(nid, &j, i, min, max) == 0)
  {
    goto err;
  }

  if ((ne = X509_NAME_ENTRY_create_by_NID(NULL, nid, j, (unsigned char *) buf,
                                           strlen(buf))) == NULL)
  {
    goto err;
  }

  if (!X509_NAME_add_entry(n, ne, X509_NAME_entry_count(n), 0))
  {
    goto err;
  }

  ret = 1;

err:
  if (ne != NULL)
  {
    X509_NAME_ENTRY_free(ne);
  }

  return(ret);
} /* Httpsadd_DN_object() */

/***************************************************************************
 * Function Name : HttpsCertReqNew
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 ***************************************************************************/

HX509Req_t *HttpsCertReqNew( void )
{
  return (X509_REQ_new());
} /* HttpsCertReqNew() */

/***************************************************************************
 * Function Name : HttpsCertReqFree
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 ***************************************************************************/

void HttpsCertReqFree( HX509Req_t  *a )
{
  X509_REQ_free(a);
} /* HttpsCertReqFree() */

/***************************************************************************
 * Function Name : Httpsreq_fix_data
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 ***************************************************************************/
static
int32_t Httpsreq_fix_data( int32_t  nid,
                           int32_t  *type,
                           int32_t  len,
                           int32_t  min,
                           int32_t  max )
{
  if (nid == NID_pkcs9_emailAddress)
  {
    *type=V_ASN1_IA5STRING;
  }

  if ((nid == NID_commonName) &&
      (*type == V_ASN1_IA5STRING))
  {
    *type = V_ASN1_T61STRING;
  }

  if ((nid == NID_pkcs9_challengePassword) &&
      (*type == V_ASN1_IA5STRING))
  {
    *type=V_ASN1_T61STRING;
  }

  if ((nid == NID_pkcs9_unstructuredName) &&
      (*type == V_ASN1_T61STRING))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "Invalid characters in string, please re-enter the string\n");
    return(0);
  }

  if (nid == NID_pkcs9_unstructuredName)
  {
    *type = V_ASN1_IA5STRING;
  }

  if (len < min)
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "string is too short, it needs to be at least %d bytes long\n",
          min);
    return(0);
  }

  if ((max != 0) &&
      (len > max))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "string is too long, it needs to be less than  %d bytes long\n",
          max);
    return(0);
  }

  return(1);
} /* Httpsreq_fix_data() */

#endif /* OF_HTTPD_SUPPORT && OF_HTTPS_SUPPORT */


