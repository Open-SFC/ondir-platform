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
/*  File        : httpsutl.c                                              */
/*                                                                        */
/*  Description : This file contains https utility function               */
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
#include "hcmgrmbl.h"
#include "hcmgrgdf.h"
#include "hcmgrgif.h"
#include "hcmgrlif.h"
#include "hcmgrldf.h"
#include <openssl/x509v3.h>

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

enum SubAltNameTypes
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

HttpsValCert_t  Httpsval;
unsigned char        ucHttpsrectype;
unsigned char        ucHttpssigntype;
unsigned char        ucHttpsenctype;
unsigned char        ucHttpsdefcert;
unsigned char        ucHttpstype;
uint32_t        Httpslength;

/****************************************************************
 * * * *  F u n c t i o n    I m p l e m e n t a t i o n  * * * *
 ****************************************************************/

/***************************************************************************
 * Function Name : HttpsWritetoFile
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 ***************************************************************************/

int32_t HttpsWritetoFile( unsigned char  *pbuf,
                          uint16_t  usSrcType,
                          void    *pSrc,
                          unsigned char  ucCT,
                          unsigned char  *pIdName )
{

  BIO         *pMemBio = NULL;
  int32_t     ret      = 0;
  EVP_PKEY    *pKey    = NULL;
  HX509Req_t  *pReq    = NULL;
  unsigned char    *pTemp   = NULL;

  if ((usSrcType != HTTPS_PRIV_KEY) &&
      (usSrcType != HTTPS_CERT_REQ))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsWritetoFile:: Invalid Source type\n");
    return OF_FAILURE;
  }

  if ((ucCT != 'r') &&
      (ucCT != 'd'))
  {
    Trace(HTTPS_ID, TRACE_SEVERE, "Invalid Certificate Type\n");
    return OF_FAILURE;
  }

  switch (usSrcType)
  {
  case HTTPS_PRIV_KEY:

    /**
     * Write the private key into a file in PEM Format..
     * Currently, we are not using any password to encrypt the private
     * key file...
     */
    pMemBio = BIO_new(BIO_s_mem());

    if (pMemBio == NULL)
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsWritetoFile:: BIO_new() is failed\n\r");
      return OF_FAILURE;
    }

    pKey = (EVP_PKEY *)pSrc;
    ret  = PEM_write_bio_PrivateKey(pMemBio, pKey, NULL, NULL, 0, NULL, 0);

    if (ret <= 0)
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsWritetoFile:: PEM_write_bio_PrivateKey failed\n\r");
      BIO_free(pMemBio);
      return OF_FAILURE;
    }

    pTemp = NULL;
    Httpslength = BIO_get_mem_data(pMemBio, &pTemp);
    ucHttpsrectype='p';

    if (ucCT=='r')
    {
      ucHttpssigntype='r';
    }
    else
    {
      ucHttpssigntype='d';
    }
    ucHttpsenctype='p';
    ucHttpsdefcert='n';
    ucHttpstype='s';
    pbuf = pTemp;
    HttpsFlashWrite(ucHttpstype, ucHttpsrectype, ucHttpsenctype,
                    ucHttpsdefcert, ucHttpssigntype, pbuf, Httpslength,
                    pIdName);
    break;

  case HTTPS_CERT_REQ:
    pMemBio = BIO_new(BIO_s_mem());

    if (pMemBio == NULL)
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsWritetoFile::BIO_new() is failed\n\r");
      return OF_FAILURE;
    }
    pReq = (HX509Req_t*)pSrc;

    if (!PEM_write_bio_X509_REQ(pMemBio, pReq))
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsWritetoFile:: PEM_write_bio_X509_REQ failed\n\r");
      BIO_free(pMemBio);
      return OF_FAILURE;
    }

    pTemp = NULL;
    Httpslength = BIO_get_mem_data(pMemBio, &pTemp);
    ucHttpsrectype='r';
    ucHttpsenctype='p';
    ucHttpsdefcert='n';
    ucHttpstype='s';

    if (ucCT=='r')
    {
      ucHttpssigntype='r';
    }
    else
    {
      ucHttpssigntype='d';
    }

    pbuf = pTemp;

    HttpsFlashWrite(ucHttpstype, ucHttpsrectype, ucHttpsenctype,
                    ucHttpsdefcert, ucHttpssigntype, pbuf, Httpslength,
                    pIdName);

    Httpsval.ucrectype='c';
    Httpsval.ucenctype='p';
    Httpsval.ucdefcert='n';
    Httpsval.uctype='s';
    Httpsval.ucsigntype='d';

    break;
  }
  BIO_free(pMemBio);
  return OF_SUCCESS;
} /* HttpsWritetoFile() */

/***************************************************************************
 * Function Name : HttpsReadFromFile
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 ***************************************************************************/

int32_t HttpsReadFromFile( unsigned char  *preq,
                           uint16_t  usSrcType,
                           void    **pSrc,
                           unsigned char  ucCt )
{

  BIO       *pMemBio = NULL;
  uint32_t  ulLength = 0;

  if ((usSrcType != HTTPS_PRIV_KEY) &&
      (usSrcType != HTTPS_CERT))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsReadFromFile:: Invalid Source type\n");
    return OF_FAILURE;
  }

  if ((ucCt != 'r') &&
      (ucCt != 'd'))
  {
    Trace(HTTPS_ID,TRACE_SEVERE,
          "HttpsReadFromFile:: No Value present in ucCt\n");
  }

  switch (usSrcType)
  {
  case HTTPS_PRIV_KEY:

    /**
     * Read the private key from a file in PEM Format..
     *
     * An UGLY Hack!! If the first char is 'R', assume it is RSA
     * private key and if it is 'D', assume it is DSA private key.
     * Later, we might want to have two directories RSA and DSA under
     * self directory to figure out the type of the private key!
     *        -- ANUPAMA, May 17th, 1999
     */
    /** --DKK-- **/
    ulLength = strlen((char *) preq);

    pMemBio = BIO_new_mem_buf(preq, ulLength);

    if (pMemBio == NULL)
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsReadFromFile:: BIO_new_mem_buf()is failed\n\r");
      return OF_FAILURE;
    }

    *pSrc = (void*)PEM_read_bio_PrivateKey(pMemBio, NULL, NULL, NULL);

    if (*pSrc == NULL)
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsReadFromFile:: PEM_read_bio_PrivateKey failed\n\r");
      BIO_free(pMemBio);
      return OF_FAILURE;
    }

    break;
  case HTTPS_CERT:
    /**
     * Read the Certificate from a file in PEM Format..
     */
    /* --DKK-- extra argument */

    ulLength = strlen((char *) preq);
    pMemBio = BIO_new_mem_buf(preq, ulLength);

    if (pMemBio == NULL)
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsReadFromFile:: BIO_new_mem_buf()is failed\n\r");
      return OF_FAILURE;
    }

    if ((*pSrc = (void*) PEM_read_bio_X509(pMemBio, NULL,
                                             NULL, 0)) != NULL)
    {
      Trace(HTTPS_ID, TRACE_INFO, "Loaded the certificates\n\r");
    }
    else
    {
      Trace(HTTPS_ID, TRACE_SEVERE,
            "ReadFromFile:Loading the certificates failed\n\r");
      BIO_free(pMemBio);
      return OF_FAILURE;
    }
  }

  BIO_free(pMemBio);

  return OF_SUCCESS;
} /* HttpsReadFromFile() */

/***************************************************************************
 * Function Name : HttpsFindSubAltNameInCert
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 ***************************************************************************/

int32_t HttpsFindSubAltNameInCert( HttpsCertIdInfo_t  *pCertIdInfo,
                                   X509               *pCert )
{
  X509_EXTENSION          *pExt = NULL;
  int32_t                 iItem, ii, iNum;
  STACK_OF(GENERAL_NAME)  *pSkSubAlt = NULL;
  GENERAL_NAME            *pSubAlt = NULL;
  bool                 bFlag = FALSE;
  unsigned char                ucType;

  if ((pCert == NULL) ||
      (pCertIdInfo == NULL))
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsFindSubAltNameInCert:: Input validations failed\r\n");
    return OF_FAILURE;
  }

  /**
   * Check for the Sub-Alt name extensions in the certificate.
   */
  iItem = X509v3_get_ext_by_NID(pCert->cert_info->extensions,
                                NID_subject_alt_name, -1);

  if (iItem < 0)
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsFindSubAltNameInCert:: X509v3_get_ext_by_NID failed\n");
    return OF_FAILURE;
  }

  /**
   * Get the Sub-Alt name Extension.
   */
  pExt = X509_get_ext(pCert, iItem);

  if (pExt == NULL)
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsFindSubAltNameInCert:: sk_X509_EXTENSION_value failed\n");
    return OF_FAILURE;
  }

  /**
   * Convert the Sub-Alt name Extension into Stack_of(General Name).
   */
  pSkSubAlt = X509V3_EXT_d2i(pExt);

  if (pSkSubAlt == NULL)
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsFindSubAltNameInCert:: X509_EXT_d2i() failed\r\n");
    return OF_FAILURE;
  }

  iNum = sk_GENERAL_NAME_num(pSkSubAlt);

  if (iNum < 0)
  {
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsFindSubAltNameInCert:: sk_GENERAL_NAME_num() failed\r\n");
    sk_GENERAL_NAME_pop_free(pSkSubAlt, GENERAL_NAME_free);
    return OF_FAILURE;
  }

  switch (pCertIdInfo->usIdType)
  {
  case HTTPS_ID_IPV4_ADDR:
    ucType = GEN_IPADD;
    break;
  case HTTPS_ID_FQDN:
    ucType = GEN_DNS;
    break;
  case HTTPS_ID_USER_FQDN:
    ucType = GEN_EMAIL;
    break;
  default:
   Trace(HTTPS_ID, TRACE_SEVERE,
     "Subject Alt Name is not present in the Extensions\n");
   sk_GENERAL_NAME_pop_free(pSkSubAlt, GENERAL_NAME_free);

   return OF_FAILURE;
  } /* switch */

  for (ii = 0; ii < iNum; ii++)
  {
    pSubAlt = sk_GENERAL_NAME_value(pSkSubAlt, ii);

    if (pSubAlt->type != ucType)
    {
      continue;
    }

    switch (ucType)
    {
    case GEN_IPADD:
      if ((memcmp(pCertIdInfo->aIdData,
                    pSubAlt->d.ip->data, pSubAlt->d.ip->length) != 0))
      {
        bFlag = FALSE;
      }
      else
      {
        bFlag = TRUE;
      }

      break;
    case GEN_DNS:
      if ((memcmp(pCertIdInfo->aIdData,
                    pSubAlt->d.ia5->data, pSubAlt->d.ia5->length) != 0))
      {
        bFlag = FALSE;
      }
      else
      {
        bFlag = TRUE;
      }

      break;
    case GEN_EMAIL:
      if ((memcmp(pCertIdInfo->aIdData,
                   pSubAlt->d.ia5->data, pSubAlt->d.ia5->length) != 0))
      {
        bFlag = FALSE;
      }
      else
      {
        bFlag = TRUE;
      }

      break;
    default:
      bFlag = FALSE;
    } /* switch */

    if (bFlag == TRUE)
    {
      sk_GENERAL_NAME_pop_free(pSkSubAlt, GENERAL_NAME_free);
      pSkSubAlt = NULL;
      return OF_SUCCESS;
    }
    if (bFlag == FALSE)
    {
      sk_GENERAL_NAME_pop_free(pSkSubAlt, GENERAL_NAME_free);
      pSkSubAlt = NULL;
      return OF_FAILURE;
    }
  } /* for */

  if (pSkSubAlt)
  {
    sk_GENERAL_NAME_pop_free(pSkSubAlt, GENERAL_NAME_free);
  }

  return OF_FAILURE;
} /* HttpsFindSubAltNameInCert() */

/***************************************************************************
 * Function Name : HttpsGetSubAltNameIdType
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  : 0 on success and -1 on failure
 ***************************************************************************/

uint16_t HttpsGetSubAltNameIdType( uint16_t  usIdType )
{
  switch (usIdType)
  {
  case HTTPS_ID_IPV4_ADDR :
    return HTTPS_SAN_IPADDR;

  case HTTPS_ID_FQDN :
    return HTTPS_SAN_DNSNAME;

  case HTTPS_ID_USER_FQDN :
    return HTTPS_SAN_RFC822NAME;

  default :
    return 0;
  }
} /* HttpsGetSubAltNameIdType() */

#endif /* OF_HTTPD_SUPPORT && OF_HTTPS_SUPPORT */


