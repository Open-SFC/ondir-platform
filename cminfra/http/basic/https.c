
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
/*  File        : https.c                                                 */
/*                                                                        */
/*  Description : This file contains https server related functions.      */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.11.04    DRAM      Modified during the code review.     */
/*    1.2      22.02.06    Harishp   Modified during MultipleInstances	  */	
/*                                   and Multithreading	                  */
/**************************************************************************/

#if defined(OF_HTTPD_SUPPORT) && defined(OF_HTTPS_SUPPORT)

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "cmincld.h"
#include "httpdinc.h"
#include "htpdglb.h"
#ifdef OF_HTTPS_SUPPORT
#include "httpswrp.h"
#endif /*OF_HTTPS_SUPPORT*/
#include "httpdtmr.h"
#include "httpd.h"
#include "httpgif.h"
#include "hcrtgif.h"

/**
 * include the header files corresponding to openssl library.
 */
#include "openssl/rsa.h"
#include "openssl/crypto.h"
#include "openssl/x509.h"
#include "openssl/pem.h"
#include "openssl/ssl.h"
#include "openssl/err.h"

#ifdef INTOTO_CMS_SUPPORT
static char aPacket1 [] = {0x29, 0xfa, 0x0c, 0x80, 0x15, 0xd5, 0xf9, 0xc7, 0xf3, 0xce, 0x54, 0xe5, 0x18, 0xf0, 0xda, 0xa0, 0x37, 0x75, 0xc4, 0xf8, 0xd8, 0x61, 0x2b, 0xcb, 0x34, 0x80, 0xf9, 0xcb, 0x4b, 0xb9, 0xce, 0x11, 0x53, 0x1a, 0xe3, 0x7a, 0x6a, 0xbb, 0x53, 0xbe, 0x31, 0x4d, 0x02, 0x1e, 0x79, 0x45, 0x57, 0xb1, 0x42, 0xbf, 0xd4, 0x6f};
#endif  /* INTOTO_CMS_SUPPORT */
/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/
#define  HTTPS_LOCAL_BUFFER_LEN       (512)
#define  HTTPS_LOCAL_IP_ADDRESS_LEN   (20)

/************************************************************
 * * * *  V a r i a b l e    D e c l a r a t i o n s  * * * *
 ************************************************************/

 char *onlyCertBasicHtmlPages[] = { "login.htm",
                                                 "loginout.htm",
                                                 "errlogin.htm",
                                                 "logout.htm",
                                                 "login.igw",
                                                 "logout.igw",
                                                 "VortiqaLogo.gif",
                                                #ifdef INTOTO_UAM_SUPPORT
                                                  "wifilog.htm",
                                                  "uamlogin.igw",
  "wifilog.igw",
  "wifilout.htm",
  "wifilout.htm",
  "wifidefa.htm",
  "wifiidtm.htm",
  "wifitmot.htm",
  "wifiintro.htm",
  "wififorg.htm",
  "wifiusr.htm",
  "wifilink.htm",
  "company.gif",
                                               #endif

                                                 "" };

/**
 * The HTML file, htpsctab.htm MUST be the first entry and used as welcome
 * page in the https enable build.
 */
 char *onlyCertHtmlPages[] = { "htpsctab.htm",
                                            "htpscreq.htm",
                                            "htpscupl.htm",
                                            "htpspvtk.htm",
                                            "htpscact.htm",
                                            "httpsselfcert.igw",
                                            "httpstrustcert.igw",
                                            "httpsprivkey.igw",
                                            "httpsmgr.igw",
                                            "httpsself.igw",
                                            "httpstrust.igw",
                                            "htpstime.htm",
                                            "htpstime.igw",
                                            ""  };

 char *ucHttpdRedirMsg = "<html><head><title>Access Denied</title></head><body><h1>You must use SSL based http(HTTPS) server.</h1></body></html>";

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

/**
 *
 */

int32_t Https_ClientAccept( HttpServer_t     *pSer,
                            HttpClientReq_t  *pReq );

/**
 *
 */

int32_t Https_ClientRead( HttpClientReq_t  *pReq,
                          unsigned char         *pBuf,
                          uint32_t         ulLen,
                          int32_t          *pBytesRead );

/**
 *
 */

int32_t Https_ClientWrite( HttpClientReq_t  *pReq,
                           unsigned char         *pBuf,
                           uint32_t         ulLen );

/**
 *
 */

int32_t Https_ClientClose( HttpClientReq_t  *pReq );

/**************************************************************************
 * Function Name : Httpd_MapHtmlFile                                      *
 * Description   : This function is maps to the actual HTML file to be    *
 *                 send to the browser.                                   *
 * Input         : pReq (I/O) - Http client request handle.               *
 * Output        : None                                                   *
 * Return value  : None                                                   *
 **************************************************************************/

void  Httpd_MapHtmlFile( HttpClientReq_t  *pReq )
{
  if(!pReq) return ;
  if ((!of_strcmp((char *) pReq->ucFileName, "hsctab.htm")) ||
      (!of_strcmp((char *) pReq->ucFileName, "logout.htm")))
  {
    of_strcpy((char *) pReq->ucFileName, "htpsctab.htm");
    return;
  }

  if (!of_strcmp((char *) pReq->ucFileName, "hscreq.htm"))
  {
    of_strcpy((char *) pReq->ucFileName, "htpscreq.htm");
    return;
  }

  if (!of_strcmp((char *) pReq->ucFileName, "hscupl.htm"))
  {
    of_strcpy((char *) pReq->ucFileName, "htpscupl.htm");
    return;
  }

  if (!of_strcmp((char *) pReq->ucFileName, "hspvtk.htm"))
  {
    of_strcpy((char *) pReq->ucFileName, "htpspvtk.htm");
    return;
  }

  if (!of_strcmp((char *) pReq->ucFileName, "hscact.htm"))
  {
    of_strcpy((char *) pReq->ucFileName, "htpscact.htm");
    return;
  }
  if (!of_strcmp((char *) pReq->ucFileName, "errscrn.htm"))
  {
    of_strcpy((char *) pReq->ucFileName, "loginout.htm");
    return;
  }
} /* Httpd_MapHtmlFile() */

/**************************************************************************
 * Function Name : Httpd_OnlyCerts_GetPermission                          *
 * Description   : This function is used to get the user privileges for   *
 *                 serving the relevant pages.                            *
 * Input         : pFileName  (I) - Filename for which privilege is to set*
 *                 pPermission(O) - privileges to be set for the page.    *
 * Output        : Privilege for the page to be served.                    *
 * Return value  : OF_SUCCESS on success, OF_FAILURE otherwise.             *
 **************************************************************************/

int32_t Httpd_OnlyCerts_GetPermission( char  *pFileName,
                                       int32_t  *pPermission )
{
  int32_t  ii = 0;

  if(!pFileName || !pPermission) return OF_FAILURE;
  /**
   *
   */
  while (of_strcmp(onlyCertHtmlPages[ii], ""))
  {
    if (!of_strcmp(onlyCertHtmlPages[ii], pFileName))
    {
      *pPermission = HTTPD_PRIV_ADMIN;
      return OF_SUCCESS;
    }
    ii++;
  }

  /**
   *
   */
  ii = 0;

  while (of_strcmp(onlyCertBasicHtmlPages[ii], ""))
  {
    if (!of_strcmp(onlyCertBasicHtmlPages[ii], pFileName))
    {
      *pPermission = HTTPD_PRIV_USER;
      return OF_SUCCESS;
    }
    ii++;
  }
  return OF_FAILURE;
} /* Httpd_OnlyCerts_GetPermission() */

/**************************************************************************
 * Function Name : Httpd_OnlyCerts_GetIndexFile                           *
 * Description   : This function returns the first file of the html pages *
 *                 list to be served.                                     *
 * Input         : None                                                   *
 * Output        : First file name of the list.                           *
 * Return value  : First file name of the list.                           *
 **************************************************************************/

char *Httpd_OnlyCerts_GetIndexFile( void )
{
  /**
   * Send the welcome page related to http certificates.
   */
  return onlyCertHtmlPages[0];
} /* Httpd_OnlyCerts_GetIndexFile() */

/**************************************************************************
 * Function Name : Httpd_Check4Certificate                                *
 * Description   : This function is used                                  *
 *                                                                        *
 * Input         : None                                                   *
 * Output        : None                                                   *
 * Return value  : equivalent enumerated value on success.                      *
 *                 hVersionBad on error                                   *
 **************************************************************************/

void Httpd_Check4Certificate( void )
{
#ifdef HTTP_REDIRECT_HTTPS
  X509      *pCert    = NULL;
  EVP_PKEY  *pPrivKey = NULL;
  HttpGlobals_t *pHttpg=NULL;

  pHttpg=pHTTPG;
  if(!pHttpg)
  {
    Httpd_Error("pHttpg is NULL\r\n");	
    return;
  }
  /**
   *
   */
  if (HttpsGetValidCert(&pCert, &pPrivKey) == OF_SUCCESS)
  {
    Httpd_Disable(pHttpg,hServerNormal);
    Httpd_Enable(pHttpg,hServerSSL);
  }
  else
  {
    Httpd_Enable(pHttpg,hServerNormal);
    Httpd_Disable(pHttpg,hServerSSL);
  }
#endif
  return;
} /* Httpd_Check4Certificate() */

/**************************************************************************
 * Function Name : Httpd_Send_Redirect                                    *
 * Description   : This function is                                       *
 *                                                                        *
 * Input         :                                                        *
 * Output        : None                                                   *
 * Return value  : equivalent enumerated value on success.                      *
 *                 hVersionBad on error                                   *
 **************************************************************************/

int32_t Httpd_Send_Redirect( HttpClientReq_t  *pReq )
{
  char   tmpBuf[HTTPS_LOCAL_BUFFER_LEN];
  char   cMyAddr[HTTPS_LOCAL_IP_ADDRESS_LEN];
  ipaddr    MyIp;
  uint16_t  usMyPort=0;
  int32_t   len=0;

  if(!pReq) return OF_FAILURE;
  /**
   *  For  all HTTPS sessions  we are supporting IPV4 address only.
   */

#ifdef INTOTO_IPV6_SUPPORT
  MyIp.ip_type_ul=IGW_INET_IPV6;
#endif /*#ifdef INTOTO_IPV6_SUPPORT*/

  if (cm_socket_get_sock_name(pReq->SockFd, &MyIp,&usMyPort) != OF_SUCCESS)
  {
    if(pReq->eVersion==hVersion09)
    {
      len = sprintf(tmpBuf,"HTTP/0.9 200 OK%s", CRLF);
    }
    else if(pReq->eVersion==hVersion10)
    {
      len = sprintf(tmpBuf,"HTTP/1.0 200 OK%s", CRLF);
    }
    else
    {
      len = sprintf(tmpBuf,"HTTP/1.1 200 OK%s", CRLF);
    }

    if (HttpClient_Send(pReq, tmpBuf, len) != OF_SUCCESS)
    {
      return OF_FAILURE;
    }

    len = sprintf(tmpBuf,"Content-type: text/html%s%s", CRLF,CRLF);

    if (HttpClient_Send(pReq, tmpBuf, len) != OF_SUCCESS)
    {
      return OF_FAILURE;
    }

    if (HttpClient_Send(pReq, ucHttpdRedirMsg,
                        of_strlen(ucHttpdRedirMsg)) != OF_SUCCESS)
    {
      return OF_FAILURE;
    }

    return OF_SUCCESS;
  }

#ifdef INTOTO_IPV6_SUPPORT
  if(HttpdNumToPresentation(&MyIp,(unsigned char *)cMyAddr)
     !=  OF_SUCCESS)
  {
    HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
    return OF_FAILURE;
  }
#else
  Httpd_ntoa(htonl(MyIp), cMyAddr);
#endif /*#ifdef INTOTO_IPV6_SUPPORT*/

  if(pReq->eVersion==hVersion09)
  {
    len = sprintf(tmpBuf,"HTTP/0.9 301 Moved Permanently %s", CRLF);
  }
  else if(pReq->eVersion==hVersion10)
  {
    len = sprintf(tmpBuf,"HTTP/1.0 302 Moved Temporarily%s", CRLF);  
  }
  else
  {
    len = sprintf(tmpBuf,"HTTP/1.1 302 Moved Temporarily%s", CRLF);  
  }


  if (HttpClient_Send(pReq, tmpBuf, len) != OF_SUCCESS)
  {
    return OF_FAILURE;
  }

  len = sprintf(tmpBuf,"Location: https://%s/%s", cMyAddr, CRLF);

  if (HttpClient_Send(pReq, tmpBuf, len) != OF_SUCCESS)
  {
    return OF_FAILURE;
  }

  len = sprintf(tmpBuf,"Content-type: text/html%s%s", CRLF,CRLF);

  if (HttpClient_Send(pReq, tmpBuf, len) != OF_SUCCESS)
  {
    return OF_FAILURE;
  }

  if (HttpClient_Send(pReq, ucHttpdRedirMsg,
                      of_strlen(ucHttpdRedirMsg)) != OF_SUCCESS)
  {
    return OF_FAILURE;
  }

  return OF_SUCCESS;
} /* Httpd_Send_Redirect() */

/**************************************************************************
 * Function Name : Https_Init                                             *
 * Description   : This function initializes https related prerequisites, *
 *                 which will be used by the http server to serve the html*
 *                 pages to the browser.                                  *
 * Input         : pSer (O) - pointer to the http server data structure.  *
 * Output        : None                                                   *
 * Return value  : equivalent enumerated value on success.                      *
 *                 hVersionBad on error                                   *
 **************************************************************************/

int32_t Https_Init( HttpServer_t  *pSer )
{
  SSL_CTX     *ctx;
  SSL_METHOD  *meth;

  if ( !pSer )
  {
    return OF_FAILURE;
  }

  pSer->ClientAccept = Https_ClientAccept;
  pSer->ClientRead   = Https_ClientRead;
  pSer->ClientWrite  = Https_ClientWrite;
  pSer->ClientClose  = Https_ClientClose;

#ifdef INTOTO_CMS_SUPPORT
  RAND_seed(aPacket1, sizeof(aPacket1));
#endif  /* INTOTO_CMS_SUPPORT */
  meth = IGWSSLv23_server_method();
  ctx  = IGWSSL_CTX_new(meth);

  if ( !ctx )
  {
    return OF_FAILURE;
  }

  pSer->pSpData = ctx;
  return OF_SUCCESS;
} /* Https_Init() */

/**************************************************************************
 * Function Name : Https_CertsInit                                        *
 * Description   : This function is used to initialize the ceritificate   *
 *                 related prerequisites.                                 *
 * Input         : None                                                   *
 * Output        : None                                                   *
 * Return value  : None                                                   *
  **************************************************************************/

void Https_CertsInit( void )
{
  /**
   * Initialize the https certificate related prerequisites.
   */
  IGWSSL_load_error_strings();
  IGWSSLeay_add_ssl_algorithms();
}

/**************************************************************************
 * Function Name : Https_ClientAccept                                     *
 * Description   : This function is used                                  *
 *                                                                        *
 * Input         :                                                        *
 * Output        : None                                                   *
 * Return value  : equivalent enumerated value on success.                      *
 *                 hVersionBad on error                                   *
 **************************************************************************/


int32_t Https_ClientAccept( HttpServer_t     *pSer,
                            HttpClientReq_t  *pReq )
{
  int32_t   iRetValue = 0;
  SSL       *ssl = NULL;
  X509      *pCert = NULL;
  EVP_PKEY  *pPrivKey = NULL;
  HttpGlobals_t *pHttpg=NULL;
  
  if(!pReq || !pSer) return OF_FAILURE;

  /**
   *
   */
  ssl = IGWSSL_new((SSL_CTX*) pSer->pSpData);

  if (!ssl)
  {
    printf(" SSL_new failed \n");
    return OF_FAILURE;
  }

  pHttpg=pHTTPG;
  if(!pHttpg)
  {
    Httpd_Error("pHttpg is NULL\r\n");	
    return OF_FAILURE;
  }
  if (HttpsGetValidCert(&pCert, &pPrivKey) != OF_SUCCESS)
  {
#ifdef HTTP_REDIRECT_HTTPS
    Httpd_Enable(pHttpg,hServerNormal);
    Httpd_Disable(pHttpg,hServerSSL);
#endif
    IGWSSL_free(ssl);
    return OF_FAILURE;
  }

  /**
   *
   */
  if (IGWSSL_use_certificate(ssl, pCert) <= 0)
  {
    HttpDebug((" IGWSSL_use_certificate failed \n"));
    IGWSSL_free(ssl);
    return OF_FAILURE;
  }

  /**
   *
   */
  if (IGWSSL_use_PrivateKey(ssl, pPrivKey) <= 0)
  {
    IGWSSL_free(ssl);
    return OF_FAILURE;
  }

  /**
   *
   */
  if (!IGWSSL_check_private_key(ssl))
  {
    IGWSSL_free(ssl);
    return OF_FAILURE;
  }

  /**
   *
   */
  pReq->pSpData = ssl;

  /**
   *
   */
  iRetValue = IGWSSL_set_fd(ssl, pReq->SockFd); /* 0 failed 1 succeeded */

  if (iRetValue == 0)
  {
    printf(" SSL_set_fd failed \n");
    IGWSSL_free(ssl);
    return OF_FAILURE;
  }

  /**
   *
   */
  iRetValue = IGWSSL_accept(ssl);

  if (iRetValue <= 0)
  {
    iRetValue = IGWSSL_get_error(ssl, iRetValue);

    if (!((iRetValue == SSL_ERROR_WANT_READ) ||
        (iRetValue == SSL_ERROR_WANT_WRITE)))
    {
      IGWSSL_free(ssl);
      return OF_FAILURE;
    }
  }

  return OF_SUCCESS;
} /* Https_ClientAccept() */

/**************************************************************************
 * Function Name : Https_ClientRead                                       *
 * Description   :                                                        *
 *                                                                        *
 * Input         :                                                        *
 * Output        : None                                                   *
 * Return value  : equivalent enumerated value on success.                      *
 *                 hVersionBad on error                                   *
 **************************************************************************/


int32_t Https_ClientRead( HttpClientReq_t  *pReq,
                          unsigned char         *pBuf,
                          uint32_t         ulLen,
                          int32_t          *pBytesRead )
{
  SSL      *ssl = NULL;
  int32_t  iRetValue = 0;
  uint32_t uiRdStatus = 0;
  if(!pReq || !pBuf)
  {    
	    HttpDebug((" pReq is NULL \n"));
	    return OF_FAILURE;
  }

  ssl = (SSL*)pReq->pSpData;
  if(!ssl)
  {
	   HttpDebug((" SSL is NULL \n"));
	   return OF_FAILURE;
  } 

  if (!IGWSSL_is_init_finished(ssl))
  {
    iRetValue = IGWSSL_accept(ssl);

    if (iRetValue <= 0)
    {
      iRetValue = IGWSSL_get_error(ssl, iRetValue);

      if (!((iRetValue == SSL_ERROR_WANT_READ) ||
           (iRetValue == SSL_ERROR_WANT_WRITE)))
      {
        return OF_FAILURE;
      }
    }

    *pBytesRead = 0;
    return OF_SUCCESS;
  }
  else
  {
    *pBytesRead = IGWSSL_read(ssl, pBuf, ulLen, &uiRdStatus);

    if (*pBytesRead <= 0)
    {
      iRetValue = IGWSSL_get_error(ssl, *pBytesRead);
      if (!((iRetValue == SSL_ERROR_WANT_READ) ||
           (iRetValue == SSL_ERROR_WANT_WRITE)))
      {
        return OF_FAILURE;
      }
      *pBytesRead = 0;
      if (iRetValue == SSL_ERROR_WANT_READ) 
        return HTTPS_SSL_READ_PENDING;
      else
        return OF_SUCCESS;
    }
    else if(uiRdStatus==1)
    {
      return HTTPS_SSL_READ_PENDING;
    }
    return OF_SUCCESS;
  }
} /* Https_ClientRead() */

/**************************************************************************
 * Function Name : Https_ClientWrite                                      *
 * Description   :                                                        *
 *                                                                        *
 * Input         :                                                        *
 * Output        : None                                                   *
 * Return value  : equivalent enumerated value on success.                      *
 *                 hVersionBad on error                                   *
 **************************************************************************/


int32_t Https_ClientWrite( HttpClientReq_t  *pReq,
                           unsigned char         *pBuf,
                           uint32_t         ulLen )
{
  int32_t  iRetValue = 0;

  if(!pReq || !pBuf) return OF_FAILURE;
  iRetValue = IGWSSL_write((SSL*)pReq->pSpData, pBuf, ulLen);

  if (iRetValue > 0)
  {
    return iRetValue;
  }
  else
  {
    iRetValue = IGWSSL_get_error((SSL*)pReq->pSpData, iRetValue);

    if (!((iRetValue == SSL_ERROR_WANT_READ) ||
         (iRetValue == SSL_ERROR_WANT_WRITE)))
    {
      return OF_FAILURE;
    }
    return 0;
  }
} /* Https_ClientWrite() */

/**************************************************************************
 * Function Name : Https_ClientClose                                      *
 * Description   :                                                        *
 *                                                                        *
 * Input         :                                                        *
 * Output        : None                                                   *
 * Return value  : equivalent enumerated value on success.                *
 *                 hVersionBad on error                                   *
 **************************************************************************/

 int32_t  Https_ClientClose( HttpClientReq_t  *pReq )
{
  int32_t iRetVal;
  if(!pReq)
  {
    return OF_FAILURE;
  }

  iRetVal = cm_socket_close(pReq->SockFd);
  IGWSSL_free((SSL *)(pReq->pSpData));

  return iRetVal;
} /* Https_ClientClose() */

#endif /* OF_HTTPD_SUPPORT && OF_HTTPS_SUPPORT */

