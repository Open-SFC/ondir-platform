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
/*  File        : httpslds.c                                              */
/*                                                                        */
/*  Description : This file contains load and save functions related to   */
/*                the https certificates.                                 */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.1      05.12.04    DRAM      Modified during the code review.     */
/*    1.2      22.02.06    Harishp   Modified during MultipleInstances	  */	
/*                                   and Multithreading	                  */
/**************************************************************************/

#if defined(OF_HTTPD_SUPPORT) && defined(OF_HTTPS_SUPPORT)

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "httpdinc.h"
#include "cmutil.h"
#include "htpdglb.h"
#include "hcmgrmbl.h"
#include "hcmgrgdf.h"
#include "hcmgrgif.h"
#include "hcmgrldf.h"
#include "hcmgrlif.h"
#include "fcntl.h"

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  HTTPS_MAX_FILES              (2)

/************************************************************
 * * * *  V a r i a b l e    D e c l a r a t i o n s  * * * *
 ************************************************************/

extern uint32_t            HttpsMaxCAs;
extern HttpsKeyCertPair_t  HttpsKeyCertPair[HTTPS_MAX_NUM_OF_KEY_CERT];
extern HttpsCACertInfo_t   HttpsCACert[HTTPS_MAX_NUM_OF_CA_CERT];
extern STACK_OF(X509)      *HttpsRSATrustedCACerts;
extern HttpsMyKeyCert_t    HttpsRSAKeyPairs[HTTPS_PARAMS_RSA_KEY_PAIRS_LEN];

int32_t  iHttpsPrmFsId, iHttpsSndFsId;
bool  bHttpsNeedBkp = FALSE;

unsigned char  *pHCertPath;
unsigned char  *pHCertBackupPath;

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/


int32_t HttpsLoadCerts( void );


int32_t HttpsNewFileSystem( bool  bFormat,
                            int32_t  iFsIndex );


void HttpsSeek( int32_t  Fd,
                  int32_t  count );

extern void HTTPSLoadFactoryDefaultHTTPSCerts(void);
/***************************************************************************
 * Function Name : HttpsLdSvInit
 * Description   :
 * Input         :
 * Output        :
 * Return value  :
 ***************************************************************************/

int32_t HttpsLdSvInit( void )
{
  if ((iHttpsPrmFsId = HttpsNewFileSystem(FALSE, 0)) < 0)
  {
    HTTPSLoadFactoryDefaultHTTPSCerts();
    return OF_FAILURE;
  }

  if ((iHttpsSndFsId = HttpsNewFileSystem(FALSE, 1)) < 0)
  {
    HTTPSLoadFactoryDefaultHTTPSCerts();
    return OF_FAILURE;
  }

  if ((HttpsLoadCerts()) != HTTPS_NO_ERROR)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsLdSvInit: Loading of the certs failed\n");
#endif /*HTTPD_DEBUG*/
    HTTPSLoadFactoryDefaultHTTPSCerts();
    return OF_FAILURE;
  }

  if(!HttpsMaxCAs)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE, 
      "HttpsLdSvInit :: No certs on disk. Loading Factory defaults\r\n");
#endif /*HTTPD_DEBUG*/
    HTTPSLoadFactoryDefaultHTTPSCerts();
  }
  return OF_SUCCESS;
} /* HttpsLdSvInit() */

/***************************************************************************
 * Function Name : HttpsLoadCerts()
 * Description   : This will load both the self and trusted certificate as well
 *                 the key. It also copy the encoded record from the flash to mblk.
 * Input         : None
 * Output        : None
 * Return value  :
 ***************************************************************************/


int32_t HttpsLoadCerts( void )
{
  int32_t      theError = OF_SUCCESS;
  HX509Cert_t  *pX509Cert  = NULL;
  EVP_PKEY     *pPrivKey   = NULL;
  uint32_t     ulReclen, ulKeyType, ulTotalLen;
  int32_t      iRetVal = -1, ii;
  unsigned char     *pCertOrKey;
  unsigned char     ucEncType, ucDefCert;
  unsigned char     ucRecType, ucSignType, ucCertType;
  unsigned char     *pFlashBuffer=NULL, cTempBuf[25], *pNextAddr, *pFlash;
  bool      bReadFromCert2 = FALSE;
  uint32_t     ulCheckSum = 0, ulFlashCheckSum = 0;
  int16_t      iRSAIndex = 0;
  unsigned char     aIdName[8];
  int32_t      CertHandle = 0;

  /**
   * Certificate's primary flash handler.
   */
  uint32_t  ulCmgrPrmFlshHdl = -1;

  /**
   * Certificate's secondary flash handler.
   */
  uint32_t  ulCmgrSecFlshHdl = -1;
  bool   bPrmExist = TRUE;

  of_memset(cTempBuf, 0, sizeof(cTempBuf));

  if ((CertHandle = cm_file_open(iHttpsPrmFsId, pHCertPath,
                                UCMFILE_MODE_READ)) < 0)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsLoadCerts: Unable to open file for Primary\n\r");
#endif /*HTTPD_DEBUG*/
    bPrmExist = FALSE;
  }

  ulCmgrPrmFlshHdl = CertHandle;

  if (!bPrmExist)
  {
    if ((CertHandle = cm_file_open(iHttpsSndFsId, pHCertBackupPath,
                                  UCMFILE_MODE_READ)) <= OF_FAILURE)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsLoadCerts: Unable to open file for Backup\n\r");
#endif /*HTTPD_DEBUG*/
      return OF_FAILURE;
    }

    bReadFromCert2   = TRUE;
    ulCmgrSecFlshHdl = CertHandle;
  }
  else
  {
    cm_file_seek(iHttpsPrmFsId, ulCmgrPrmFlshHdl, UCMFILE_SEEK_BEG, 0);

    iRetVal = cm_file_read(iHttpsPrmFsId, ulCmgrPrmFlshHdl, cTempBuf, 8);

    if (iRetVal <= 0)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsLoadCerts: reading from Flash1 failed #1\n\r");
#endif /*HTTPD_DEBUG*/
      bReadFromCert2 = TRUE;
    }

    pNextAddr = cTempBuf;

    of_memcpy(&ulTotalLen, pNextAddr, 4);

    do
    {
      if (bReadFromCert2 == TRUE)
      {
        break;
      }

      if (ulTotalLen > (64*1024))
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsLoadCerts:  Flash1 seems to be corrupted\n");
#endif /*HTTPD_DEBUG*/
        bReadFromCert2 = TRUE;
        break;
      }

      if ((pFlashBuffer = (unsigned char *) of_calloc(1,
                                               (ulTotalLen +10))) == NULL)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsLoadCerts: memory allocation failed\n");
#endif /*HTTPD_DEBUG*/

        cm_file_close(iHttpsPrmFsId, ulCmgrPrmFlshHdl);

        if (ulCmgrSecFlshHdl >= 0)
        {
          cm_file_close(iHttpsSndFsId, ulCmgrSecFlshHdl);
        }

        bHttpsNeedBkp = TRUE;
        return OF_FAILURE;
      }

      cm_file_seek(iHttpsPrmFsId, ulCmgrPrmFlshHdl, UCMFILE_SEEK_BEG,
                  0);

      iRetVal = cm_file_read(iHttpsPrmFsId, ulCmgrPrmFlshHdl, pFlashBuffer,
                            (ulTotalLen+8));
      if (iRetVal <= 0)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsLoadCerts: reading from flash1 failed #2\n");
#endif /*HTTPD_DEBUG*/
        of_free(pFlashBuffer);
        bReadFromCert2 = TRUE;
        bHttpsNeedBkp = FALSE;
        break;
      }
      pFlash = pFlashBuffer  + 8;

      of_memcpy(&ulFlashCheckSum, pNextAddr+4, 4);

      for (ii = 0; ii < ulTotalLen; ii++)
      {
        ulCheckSum += pFlash[ii];
      }

      if (ulFlashCheckSum != ulCheckSum)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsLoadCerts: check sum failed for  Flash 1\n");
#endif /*HTTPD_DEBUG*/
        of_free(pFlashBuffer);
        bReadFromCert2 = TRUE;
        bHttpsNeedBkp = FALSE;
        break;
      }
      bHttpsNeedBkp = TRUE;

    } while (FALSE);

    cm_file_close(iHttpsPrmFsId, ulCmgrPrmFlshHdl);
  } /* if (!bPrmExist) */


  if ((bReadFromCert2) &&
      (ulCmgrSecFlshHdl >= 0))
  {
    iRetVal = cm_file_read(iHttpsSndFsId, ulCmgrSecFlshHdl, cTempBuf, 8);

    if (iRetVal <= 0)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsLoadCerts: reading from Flash2 failed #1\n");
#endif /*HTTPD_DEBUG*/
      cm_file_close(iHttpsSndFsId, ulCmgrSecFlshHdl);
      return OF_FAILURE;
    }

    pNextAddr = cTempBuf;
    of_memcpy(&ulTotalLen, pNextAddr, 4);

    if (ulTotalLen > (64*1024))
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsLoadCerts: reading from Flash2 failed #2\n");
#endif /*HTTPD_DEBUG*/
      cm_file_close(iHttpsSndFsId, ulCmgrSecFlshHdl);
      return OF_FAILURE;
    }

    if ((pFlashBuffer = (unsigned char *) of_calloc(1,
                                             (ulTotalLen +10))) == NULL)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsLoadCerts: memory allocation failed\n");
#endif /*HTTPD_DEBUG*/
      cm_file_close(iHttpsSndFsId, ulCmgrSecFlshHdl);
      return OF_FAILURE;
    }

    cm_file_seek(iHttpsSndFsId, ulCmgrSecFlshHdl, UCMFILE_SEEK_BEG, 0);

    iRetVal = cm_file_read(iHttpsSndFsId, ulCmgrSecFlshHdl, pFlashBuffer,
                          (ulTotalLen+8));
    if (iRetVal <= 0)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsLoadCerts: reading from flash2 failed #3\n");
#endif /*HTTPD_DEBUG*/
      of_free(pFlashBuffer);
      cm_file_close(iHttpsSndFsId, ulCmgrSecFlshHdl);
      return OF_FAILURE;
    }

    cm_file_close(iHttpsSndFsId, ulCmgrSecFlshHdl);

    pFlash = pFlashBuffer + 8;
    of_memcpy(&ulFlashCheckSum, pNextAddr+4, 4);
    ulCheckSum = 0;

    for (ii = 0; ii < ulTotalLen; ii++)
    {
      ulCheckSum += pFlash[ii];
    }

    if (ulFlashCheckSum != ulCheckSum)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsLoadCerts: check sum failed for  flash 2\n");
#endif /*HTTPD_DEBUG*/
      of_free(pFlashBuffer);
      return OF_FAILURE;
    }
  }

  pNextAddr = pFlashBuffer + 8;

  while (ulTotalLen > 0)
  {
    of_memcpy(&ulReclen, pNextAddr, 4);

    if ((pCertOrKey = (unsigned char *) of_calloc(1, (ulReclen + 2))) == NULL)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsLoadCerts: memory allocation failed\n");
#endif /*HTTPD_DEBUG*/
      of_free(pFlashBuffer);
      return OF_FAILURE;
    }
    ucRecType = *((unsigned char*)(pNextAddr+4));
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE, "\ncert or key type = %c\n",
          ucRecType);
#endif /*HTTPD_DEBUG*/

    ucSignType = *((unsigned char *)(pNextAddr+5));
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE, "Signature type = %c\n", ucSignType);
#endif /*HTTPD_DEBUG*/

    ucCertType = *((unsigned char *)(pNextAddr+8));
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE, "certificate type = %c\n", ucCertType);
#endif /*HTTPD_DEBUG*/

    ucEncType = *((unsigned char *)(pNextAddr+6));
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE, "encoding type = %c\n", ucEncType);
#endif /*HTTPD_DEBUG*/

    ucDefCert = *((unsigned char *)(pNextAddr+7));
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE, "is default = %c\n", ucDefCert);
#endif /*HTTPD_DEBUG*/

    of_memcpy(pCertOrKey, pNextAddr+ 20, ulReclen);

    switch (ucCertType)
    {
    case 't':
      of_memset(aIdName, 0, 8);

      theError =  HttpsFlashWriteTrust(ucCertType, ucRecType, ucEncType,
                                       ucDefCert, ucSignType, pCertOrKey,
                                       ulReclen, aIdName);

      if (theError != OF_SUCCESS)
      {
        theError = HTTPS_READ_FILE_FAILED;

#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsLoadCerts: Error in putting encoded cert in memory\n");
#endif /*HTTPD_DEBUG*/
        break;
      }

      theError = HttpsReadFromFile(pCertOrKey, HTTPS_CERT,
                                   (void**)&pX509Cert, ucSignType);

      if (theError != OF_SUCCESS)
      {
        theError = HTTPS_READ_FILE_FAILED;
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsLoadCerts: Error in extracting X509cert\n");
#endif /*HTTPD_DEBUG*/
        break;
      }

      if (HttpsVerifyTrustedCert(pX509Cert) != OF_SUCCESS)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsLoadCerts: Verify cert failed.\n");
#endif /*HTTPD_DEBUG*/
      }

      ulKeyType = OBJ_obj2nid(pX509Cert->cert_info->key->algor->algorithm);

      if ((ulKeyType == EVP_PKEY_RSA) ||
          (ulKeyType == EVP_PKEY_RSA2))
      {
        sk_push(/*(STACK *)*/(_STACK *) HttpsRSATrustedCACerts, (char *)pX509Cert);
#if 0
        #ifdef IGW_UCM_DP_CHANNEL_SUPPORT
        sk_push((STACK *) HttpsRSATrustedCACerts, (char *)pX509Cert);
        #else
        sk_push((_STACK *) HttpsRSATrustedCACerts, (char *)pX509Cert);
        #endif
#endif
      }
      else
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsLoadCerts: Cert Type invalid\n");
#endif /*HTTPD_DEBUG*/
        of_free(pCertOrKey);
        of_free(pFlashBuffer);
        /* return OF_FAILURE; */
        return OF_SUCCESS;
      }
      HttpsMaxCAs++;
      break;

    case 's':
      of_memcpy(aIdName, pNextAddr + 12, 8);

      theError =  HttpsFlashWrite(ucCertType, ucRecType, ucEncType,
                                  ucDefCert, ucSignType, pCertOrKey,
                                  ulReclen, aIdName);

      if (theError != OF_SUCCESS)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsLoadCerts: Copying the encoded cert in memory failed\n");
#endif /*HTTPD_DEBUG*/
        break;
      }
      switch (ucRecType)
      {
      case 'p':
        if ((*(pNextAddr + ulReclen + 24) == 'c') &&
            (!memcmp((pNextAddr + ulReclen + 32), aIdName, 8)))
        {
          theError = HttpsReadFromFile(pCertOrKey, HTTPS_PRIV_KEY,
                                       (void**)&pPrivKey, ucSignType);
          if (theError != OF_SUCCESS)
          {
            theError = HTTPS_READ_FILE_FAILED;
#ifdef HTTPD_DEBUG
            Trace(HTTPS_ID, TRACE_SEVERE,
                  "HttpsLoadCerts: HttpsReadFromFile failed #1\n");
#endif /*HTTPD_DEBUG*/
            break;
          }
          if ((ucSignType == 'r') ||
              (ucSignType == 'R'))
          {
            of_memcpy(HttpsRSAKeyPairs[iRSAIndex].aIDName, aIdName, 8);
            HttpsRSAKeyPairs[iRSAIndex].pMyPrivKey = pPrivKey;
            HttpsRSAKeyPairs[iRSAIndex].bUsed      = TRUE;
            iRSAIndex++;
          }
        }
        break;
      case 'c':
        pX509Cert = NULL;
        theError = HttpsReadFromFile(pCertOrKey, HTTPS_CERT,
                                    (void**)&pX509Cert, ucSignType);
        if (theError != OF_SUCCESS)
        {
          theError = HTTPS_READ_FILE_FAILED;
#ifdef HTTPD_DEBUG
          Trace(HTTPS_ID, TRACE_SEVERE,
                "HttpsLoadCerts: HttpsReadFromFile failed #2\n");
#endif /*HTTPD_DEBUG*/
          break;
        }

        if (HttpsVerifySelfCert(pX509Cert) != OF_SUCCESS)
        {
#ifdef HTTPD_DEBUG
          Trace(HTTPS_ID, TRACE_SEVERE,
                "HttpsLoadCerts: VerifyCert returned error\n");
#endif /*HTTPD_DEBUG*/
        }

        ulKeyType=OBJ_obj2nid(pX509Cert->cert_info->key->algor->algorithm);

        if ((ulKeyType == EVP_PKEY_RSA) ||
            (ulKeyType == EVP_PKEY_RSA2))
        {
          for (ii = 0; ii < 10; ii++)
          {
            if (!memcmp(HttpsRSAKeyPairs[ii].aIDName, aIdName, 8))
            {
              break;
            }
          }
          if (ii != 10)
          {
            HttpsRSAKeyPairs[ii].pMyCert    = pX509Cert;
          }
        }
        break;
      default:
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsLoadCerts: Strange - record type is other than key/cert\n");
#endif /*HTTPD_DEBUG*/
        break;
      }
      break;
    }

    of_free(pCertOrKey);

    if (theError != OF_SUCCESS)
    {
      of_free(pFlashBuffer);
      return theError;
    }
    pNextAddr  += (ulReclen + 20);
    ulTotalLen -= (ulReclen + 20);
  }
  of_free(pFlashBuffer);
  return OF_SUCCESS;
} /* HttpsLoadCerts() */

/***************************************************************************
 * Function Name : HttpsSaveCerts()
 * Description   :
 * Input         : None
 * Output        : None
 * Return value  :
 ***************************************************************************/

int32_t HttpsSaveCerts( void )
{
  uint32_t      ulReclen = 0, ulTotalLen = 0, ulLen = 0;
  int32_t       ii, iIndex = 0, iRetVal = 0;
  unsigned char      cTempBuf[25], *pNextAddr;
  unsigned char      *pFlash, *pFlashBuffer;
  uint32_t      ulCheckSum = 0, ulFlashCheckSum = 0;
  HTTPS_mblk_t  *pmblk;
  int32_t       CertHandle = 0;
  uint32_t      ulCmgrPrmFlshHdl = -1; /* certificate's primary flash handler */
  uint32_t      ulCmgrSecFlshHdl = -1; /* certificate's secondary flash handler */

  do
  {
    if (bHttpsNeedBkp)
    {
      if ((CertHandle = cm_file_open(iHttpsPrmFsId, pHCertPath,
                                    UCMFILE_MODE_READ)) < 0)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsSaveCerts: Unable to open file for Primary\n\r");
#endif /*HTTPD_DEBUG*/
        break;
      }

      ulCmgrPrmFlshHdl = CertHandle;
      CertHandle = 0;

#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_INFO,
            "1. Destroying snd file system\n\r");
#endif /*HTTPD_DEBUG*/

      if ((iHttpsSndFsId = HttpsNewFileSystem(TRUE, 1)) < 0)
      {
        return OF_FAILURE;
      }

      if ((CertHandle = cm_file_open(iHttpsSndFsId, pHCertBackupPath,
                                    UCMFILE_MODE_WRITE)) < 0)
      {
        if ((CertHandle = cm_file_create(iHttpsSndFsId,
                                       pHCertBackupPath)) < 0)
        {
#ifdef HTTPD_DEBUG
          Trace(HTTPS_ID, TRACE_SEVERE,
                "HttpsSaveCerts: Unable to create file for Secondry\n");
#endif /*HTTPD_DEBUG*/
          cm_file_close(iHttpsPrmFsId, ulCmgrPrmFlshHdl);
          break;
        }
      }

      ulCmgrSecFlshHdl = CertHandle;
      CertHandle       = 0;
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "\nSaving from Flash1 to Flash2.....\n");
#endif /*HTTPD_DEBUG*/

      iRetVal = cm_file_read(iHttpsPrmFsId, ulCmgrPrmFlshHdl, cTempBuf, 8);

      if (iRetVal <= 0)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsSaveCerts: reading from Flash1 failed #1\n");
#endif /*HTTPD_DEBUG*/
        break;
      }

      pNextAddr = cTempBuf;
      of_memcpy(&ulTotalLen, pNextAddr, 4);

      if (ulTotalLen > (64*1024))
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsSaveCerts: flash1 seems to be corrupted\n");
#endif /*HTTPD_DEBUG*/
        break;
      }

      if ((pFlashBuffer = (unsigned char *) of_calloc(1,
                                               (ulTotalLen +10))) == NULL)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsSaveCerts: memory allocation failed\n");
#endif /*HTTPD_DEBUG*/
        break;
      }

      cm_file_seek(iHttpsPrmFsId, ulCmgrPrmFlshHdl, UCMFILE_SEEK_BEG,
                  0);

      iRetVal = cm_file_read(iHttpsPrmFsId, ulCmgrPrmFlshHdl, pFlashBuffer,
                            (ulTotalLen+8));

      if (iRetVal <= 0)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsSaveCerts: reading from Flash1 failed #2\n");
#endif /*HTTPD_DEBUG*/
        of_free(pFlashBuffer);
        break;
      }

      pFlash = pFlashBuffer  + 8;

      of_memcpy(&ulFlashCheckSum, pNextAddr+4, 4);

      for (ii = 0; ii < ulTotalLen; ii++)
      {
        ulCheckSum += pFlash[ii];
      }

      if (ulFlashCheckSum != ulCheckSum)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsSaveCerts: check sum failed for  Flash 1\n");
#endif /*HTTPD_DEBUG*/
        of_free(pFlashBuffer);
        break;
      }

      if (cm_file_write(iHttpsSndFsId, ulCmgrSecFlshHdl, pFlashBuffer,
                       (ulTotalLen+8)) < 0)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsSaveCerts: cm_file_write failed for secondary #1\n\r");
#endif /*HTTPD_DEBUG*/
      }

#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "Saved certificate from Flash1 to Flash2\n");
#endif /*HTTPD_DEBUG*/

      of_free(pFlashBuffer);

      cm_file_flush(iHttpsSndFsId, ulCmgrSecFlshHdl);
      cm_file_close(iHttpsSndFsId, ulCmgrSecFlshHdl);
      cm_file_close(iHttpsPrmFsId, ulCmgrPrmFlshHdl);
    } /*if (bHttpsNeedBkp) */
  } while (FALSE);

  ulTotalLen = 0;
  ulCheckSum = 0;

#ifdef HTTPD_DEBUG
  Trace(HTTPS_ID, TRACE_INFO,
        "Destroying PRM file system\n\r");
#endif /*HTTPD_DEBUG*/

  if ((iHttpsPrmFsId = HttpsNewFileSystem(TRUE, 0)) < 0)
  {
    return OF_FAILURE;
  }

  if ((CertHandle = cm_file_open(iHttpsPrmFsId, pHCertPath,
                                UCMFILE_MODE_WRITE)) < 0)
  {
    if ((CertHandle = cm_file_create(iHttpsPrmFsId,
                                   pHCertPath)) == OF_FAILURE)
    {
#ifdef HTTPD_DEBUG
      Trace(HTTPS_ID, TRACE_SEVERE,
            "HttpsSaveCerts: Unable to create file for Primary\n\r");
#endif /*HTTPD_DEBUG*/

      return OF_FAILURE;
    }
  }

  ulCmgrPrmFlshHdl = CertHandle;

  cm_file_seek(iHttpsPrmFsId, ulCmgrPrmFlshHdl, UCMFILE_SEEK_BEG, 8);

  of_memset(cTempBuf, 0, 25);
  pNextAddr = cTempBuf;

#ifdef HTTPD_DEBUG
  Trace(HTTPS_ID, TRACE_SEVERE,
        "\nSaving Trusted Certificates into File...\n\r");
#endif /*HTTPD_DEBUG*/

  for (iIndex = 0; iIndex < HTTPS_MAX_NUM_OF_CA_CERT; iIndex++)
  {
    if (HttpsCACert[iIndex].bValid == TRUE)
    {
      /* *((uint32_t*)pNextAddr) = ulReclen = HttpsCACert[iIndex].uicertcnt;*/
      ulReclen = HttpsCACert[iIndex].uicertcnt;
      of_memcpy(pNextAddr, &ulReclen, 4);

      *(pNextAddr + 4) = 'c';
      *(pNextAddr + 5) = HttpsCACert[iIndex].val.ucsigntype;
      *(pNextAddr + 6) = HttpsCACert[iIndex].val.ucenctype;
      *(pNextAddr + 7) = HttpsCACert[iIndex].val.ucdefcert;
      *(pNextAddr + 8) = HttpsCACert[iIndex].val.uctype;
      of_memcpy((pNextAddr + 12), HttpsCACert[iIndex].IdName, 8);

      if (cm_file_write(iHttpsPrmFsId, ulCmgrPrmFlshHdl,
                       pNextAddr, 20) <= 0)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsSaveCerts: cm_file_write failed for primary #2\n\r");
#endif /*HTTPD_DEBUG*/
      }

      for (ii = 0; ii < 20; ii++)
      {
        ulCheckSum += pNextAddr[ii];
      }

      ulTotalLen += 20;

      HttpsSeek(ulCmgrPrmFlshHdl, 20);
      pmblk = HttpsCACert[iIndex].phead;

      while (ulReclen > 0)
      {
        if (pmblk == NULL)
        {
#ifdef HTTPD_DEBUG
          Trace(HTTPS_ID, TRACE_SEVERE,
                "Error - mblk pointer is NULL\n");
#endif /*HTTPD_DEBUG*/
          return OF_FAILURE;
        }
        ulLen = MIN(ulReclen, 1024);

        if (cm_file_write(iHttpsPrmFsId, ulCmgrPrmFlshHdl,
                         pmblk->pRptr, ulLen) <= 0)
        {
#ifdef HTTPD_DEBUG
          Trace(HTTPS_ID, TRACE_SEVERE,
                "HttpsSaveCerts: cm_file_write failed for Primary #4\n\r");
#endif /*HTTPD_DEBUG*/
        }

        ulReclen   -= ulLen;
        ulTotalLen += ulLen;

        HttpsSeek(ulCmgrPrmFlshHdl, ulLen);

        for (ii = 0; ii < ulLen; ii++)
        {
          ulCheckSum += pmblk->pRptr[ii];
        }
        pmblk = pmblk->pCont;
      }
    }
  }

#ifdef HTTPD_DEBUG
  Trace(HTTPS_ID, TRACE_SEVERE,
        "Saving Self Certificates into File...\n\r");
#endif /*HTTPD_DEBUG*/

  for (iIndex = 0; iIndex < HTTPS_MAX_NUM_OF_KEY_CERT; iIndex++)
  {
    if (HttpsKeyCertPair[iIndex].bValid == TRUE)
    {
      /* *((uint32_t*)pNextAddr) = ulReclen = HttpsKeyCertPair[iIndex].uikeycnt; */
      ulReclen = HttpsKeyCertPair[iIndex].uikeycnt;
      of_memcpy(pNextAddr, &ulReclen, 4);

      *(pNextAddr + 4) = 'p';
      *(pNextAddr + 5) = HttpsKeyCertPair[iIndex].val.ucsigntype;
      *(pNextAddr + 6) = HttpsKeyCertPair[iIndex].val.ucenctype;
      *(pNextAddr + 7) = HttpsKeyCertPair[iIndex].val.ucdefcert;
      *(pNextAddr + 8) = HttpsKeyCertPair[iIndex].val.uctype;
      of_memcpy((pNextAddr + 12), HttpsKeyCertPair[iIndex].IdName, 8);

      if (cm_file_write(iHttpsPrmFsId, ulCmgrPrmFlshHdl,
                       pNextAddr, 20) <= 0)
      {
#ifdef HTTPD_DEBUG
        Trace(HTTPS_ID, TRACE_SEVERE,
              "HttpsSaveCerts: cm_file_write failed for primary #5\n\r");
#endif /*HTTPD_DEBUG*/
      }

      for (ii = 0; ii < 20; ii++)
      {
        ulCheckSum += pNextAddr[ii];
      }

      ulTotalLen += 20;

      HttpsSeek(ulCmgrPrmFlshHdl, 20);
      pmblk = HttpsKeyCertPair[iIndex].pkey;

      while (ulReclen > 0)
      {
        if (pmblk == NULL)
        {
#ifdef HTTPD_DEBUG
          Trace(HTTPS_ID, TRACE_SEVERE, "Error - mblk pointer is NULL\n");
#endif /*HTTPD_DEBUG*/

          cm_file_close(iHttpsPrmFsId, ulCmgrPrmFlshHdl);

          return OF_SUCCESS;
        }

        ulLen = MIN(ulReclen, 1024);

        if (cm_file_write(iHttpsPrmFsId, ulCmgrPrmFlshHdl,
                         pmblk->pRptr, ulLen) <= 0)
        {
#ifdef HTTPD_DEBUG
          Trace(HTTPS_ID, TRACE_SEVERE,
                "HttpsSaveCerts: cm_file_write failed for primary #6\n\r");
#endif /*HTTPD_DEBUG*/
        }

        ulReclen   -= ulLen;
        ulTotalLen += ulLen;

        HttpsSeek(ulCmgrPrmFlshHdl, ulLen);

        for (ii = 0; ii < ulLen; ii++)
        {
          ulCheckSum += pmblk->pRptr[ii];
        }
        pmblk = pmblk->pCont;
      }

      if ((pmblk = HttpsKeyCertPair[iIndex].pcert) != NULL)
      {
        /* *((uint32_t*)pNextAddr) = ulReclen =
                          HttpsKeyCertPair[iIndex].uicertcnt; */
        ulReclen = HttpsKeyCertPair[iIndex].uicertcnt;
        of_memcpy(pNextAddr, &ulReclen, 4);
        *(pNextAddr + 4) = 'c';
        *(pNextAddr + 8) = HttpsKeyCertPair[iIndex].val.uctype;

        if (cm_file_write(iHttpsPrmFsId, ulCmgrPrmFlshHdl,
                         pNextAddr, 20) <= 0)
        {
#ifdef HTTPD_DEBUG
          Trace(HTTPS_ID, TRACE_SEVERE,
                "HttpsSaveCerts: cm_file_write failed for primary #7\n\r");
#endif /*HTTPD_DEBUG*/
        }

        for (ii = 0; ii < 20; ii++)
        {
          ulCheckSum += pNextAddr[ii];
        }

        ulTotalLen += 20;
        HttpsSeek(ulCmgrPrmFlshHdl, 20);

        while (ulReclen > 0)
        {
          if (pmblk == NULL)
          {
#ifdef HTTPD_DEBUG
            Trace(HTTPS_ID, TRACE_SEVERE, "Error - mblk pointer is NULL\n");
#endif /*HTTPD_DEBUG*/
            /* return OF_FAILURE; */
            cm_file_close(iHttpsPrmFsId, ulCmgrPrmFlshHdl);
            return OF_SUCCESS;
          }
          ulLen = MIN(ulReclen, 1024);

          if (cm_file_write(iHttpsPrmFsId, ulCmgrPrmFlshHdl,
                           pmblk->pRptr, ulLen) <= 0)
          {
#ifdef HTTPD_DEBUG
            Trace(HTTPS_ID, TRACE_SEVERE,
                  "HttpsSaveCerts: cm_file_write failed for primary #8\n\r");
#endif /*HTTPD_DEBUG*/
          }

          ulReclen   -= ulLen;
          ulTotalLen += ulLen;
          HttpsSeek(ulCmgrPrmFlshHdl, ulLen);

          for (ii = 0; ii < ulLen; ii++)
          {
            ulCheckSum += pmblk->pRptr[ii];
          }
          pmblk = pmblk->pCont;
        }
      }
    }
  }

  /**
   * Writing the total length and checksum now.
   */
  pNextAddr = cTempBuf;
  of_memset(pNextAddr, 0, sizeof(cTempBuf));
  of_memcpy(pNextAddr, &ulTotalLen, 4);
  of_memcpy(pNextAddr+4, &ulCheckSum, 4);

  cm_file_seek(iHttpsPrmFsId, ulCmgrPrmFlshHdl, UCMFILE_SEEK_BEG, 0);

  if (cm_file_write(iHttpsPrmFsId, ulCmgrPrmFlshHdl, pNextAddr, 8) <= 0)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsSaveCerts: cm_file_write failed for primary #9\n\r");
#endif /*HTTPD_DEBUG*/
  }

  cm_file_flush(iHttpsPrmFsId, ulCmgrPrmFlshHdl);
  cm_file_close(iHttpsPrmFsId, ulCmgrPrmFlshHdl);

  bHttpsNeedBkp = TRUE;
  return OF_SUCCESS;
} /* HttpsSaveCerts() */


void HttpsSeek( int32_t  Fd,
                  int32_t  count )
{
/*#ifdef INTOTO_pSOS_SUPPORT
  Fd += count;
#endif*/
} /* HttpsSeek() */


int32_t HttpsNewFileSystem( bool  bFormat,
                            int32_t  iFsIndex )
{
#ifndef OF_CM_SUPPORT
  uint32_t aFlshAddr[2]  = { FLASH_HTTPS_CERTS1_ADDR,
                             FLASH_HTTPS_CERTS2_ADDR };
  uint32_t aFlshLen[2]   = { FLASH_HTTPS_CERTS1_LEN,
                             FLASH_HTTPS_CERTS2_LEN };
#endif /* OF_CM_SUPPORT */
  char  *aFilePath[2];
  int32_t  iHttpsFsId = 0;
  HttpGlobals_t *pHttpg=NULL;
  
  pHttpg=pHTTPG;
  if(!pHttpg)
  {
    Httpd_Error("pHttpg is NULL\r\n");	
    return OF_FAILURE;
  }
 HttpsGetFileNames(pHttpg,&pHCertPath,&pHCertBackupPath);

  aFilePath[0] = (char *) pHCertPath;
  aFilePath[1] = (char *) pHCertBackupPath;

#ifndef OF_CM_SUPPORT
  if (bFormat)
  {
    UCMFileDestroyFileSystem(aFlshAddr[iFsIndex]);
  }

  if ((iHttpsFsId = UCMFileNewFileSystem(aFlshAddr[iFsIndex],
                                         aFlshLen[iFsIndex])) == OF_FAILURE)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsNewFileSystem: UCMFileNewFileSystem Failed for %d !\n\r",
          iFsIndex);
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }
#endif /*OF_CM_SUPPORT*/

  if (cm_file_is_format_needed(iHttpsFsId))
  {
    bFormat = TRUE;
  }

  if (bFormat)
  {
    if (cm_file_format_file_system(iHttpsFsId, HTTPS_MAX_FILES,
                                UCMFILE_MODE_READ_WRITE) != OF_SUCCESS)
    {
      return OF_FAILURE;
    }
  }
  if (cm_file_verify_checksum(iHttpsFsId, (unsigned char *) aFilePath[iFsIndex]) != OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "CheckSum Failed for file %s\n\r", aFilePath[iFsIndex]);
#endif /*HTTPD_DEBUG*/
  }

  return iHttpsFsId;
} /* HttpsNewFileSystem() */

#endif /* OF_HTTPD_SUPPORT && OF_HTTPS_SUPPORT */

