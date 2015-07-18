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
#include "hcmgrmbl.h"
#include "hcmgrgdf.h"
#include "hcmgrgif.h"
#include "hcmgrldf.h"
#include "hcmgrlif.h"
#include "htpdglb.h"

/************************************************************
 * * * *  V a r i a b l e    D e c l a r a t i o n s  * * * *
 ************************************************************/

extern uint32_t            HttpsMaxCAs;
extern HttpsKeyCertPair_t  HttpsKeyCertPair[HTTPS_MAX_NUM_OF_KEY_CERT];
extern HttpsCACertInfo_t   HttpsCACert[HTTPS_MAX_NUM_OF_CA_CERT];
extern STACK_OF(X509)      *HttpsRSATrustedCACerts;
extern HttpsMyKeyCert_t    HttpsRSAKeyPairs[HTTPS_PARAMS_RSA_KEY_PAIRS_LEN];
extern int32_t HttpsUploadTrustedCert( unsigned char  uctype,
                                unsigned char  ucrectype,
                                unsigned char  ucenctype,
                                unsigned char  ucdefcert,
                                unsigned char  ucsigntype,
                                unsigned char  *pTrustbuf,
                                int32_t   length,
                                uint8_t   *pHandle,
                                uint32_t  ulHandleLength );


unsigned char aFacDefTrustedCert[] = "-----BEGIN CERTIFICATE-----\n\
MIIDKzCCAtWgAwIBAgIQRf/hbDSj2K1OEO8fK8+3cjANBgkqhkiG9w0BAQUFADCB\n\
nTEmMCQGCSqGSIb3DQEJARYXc3lzLWFkbWluQGludG90b2luZC5jb20xCzAJBgNV\n\
BAYTAklOMQswCQYDVQQIEwJBUDESMBAGA1UEBxMJSHlkZXJhYmFkMRgwFgYDVQQK\n\
Ew9JbnRvdG8gU29mdHdhcmUxEjAQBgNVBAsTCVN5cy1BZG1pbjEXMBUGA1UEAxMO\n\
SW50b3RvIFRlc3QgQ0EwHhcNMDcwMTI1MDczODM1WhcNMzMwMTIxMDY0NTI4WjCB\n\
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
BgEEAYI3FQEEAwIBATANBgkqhkiG9w0BAQUFAANBABZUDRt/9Go1HzKNt+RDmPFZ\n\
Nn3vqdJWaaNrDfCVSSyxHZtzxJNv/m5eFQMmsFeWSm7DK09XnB1FPflnIxWgr2Y=\n\
-----END CERTIFICATE-----";


unsigned char aFacDefPrivateKey[] = "-----BEGIN RSA PRIVATE KEY-----\n\
MIICXQIBAAKBgQC6wekot9nIMoUOhxpoqoPHPi2WDUdmXHsOqmfof25RIoHCUv/+\n\
xAtiA7M+NfcbgIpQ+SQlYCgsjLgESx9FjZb50rH1blKzJVYpY3Ge1ElFG+pa5ZL/\n\
LgUM43F/zoSoTnbG2aftJeDjSj5WYlljQKw8UoNlCak8iqCmMwE6vFFn1wIDAQAB\n\
AoGAHCbU1b15B7ON2RXGhlaFzMfzqTXROH3iUuCfDy3+XB6efsNyMV4t9RVntI3/\n\
9NZYg0I7D6NCCw0fRA5q5Volkrr9EPoubYrizB7Gn/oe+C0+GEH5adGMZKVYCBoT\n\
r4wPUDpp3gIBBbznzbgb7MrYIbW1TNlZAG3xWgK6cWWwYzkCQQDp3i1yaNTfVyJC\n\
lYdWnUABnU78lzTkJUj/8Co8CfPz0JD7soZzyF3qGpctBWj9iCUzzjZKH918257Q\n\
3FNcxImVAkEAzG5ogxgUS4hr3Yr9HhQHezXrLfKzLLdB3H2x35qCm06kHECXY/3W\n\
M6hvHJ2gX8cFN8QRht4WpxsWxiH/jBNIuwJAa1cOhPSYv+m+T9FRzO9f8V3CYy0D\n\
mbODTfVtbSEkV0PjR6MpEvtkH48U1BUnmzZh3OVJBbyabDRhMHvvxyRuAQJBALUZ\n\
RdqEVU5Ibw3Wl76yjaIXtNyCeTmuqliyvQVR8ku1Eeq5SaPZ9YzTtILMHItcWoH5\n\
XnF0gurMM5OYALNNTgsCQQC5qxMM32mTC3lR2smaPHZgaqWGgI8zdEzWXBpZV3AA\n\
8wTMqRTFHk5hbkX1piZV/umiqZ8nEC4QuiyjqRaLenlR\n\
-----END RSA PRIVATE KEY-----";


unsigned char aFacDefSelfCert[] = "-----BEGIN CERTIFICATE-----\n\
MIIEhDCCBC6gAwIBAgIKAllx4QABAAANHTANBgkqhkiG9w0BAQUFADCBnTEmMCQG\n\
CSqGSIb3DQEJARYXc3lzLWFkbWluQGludG90b2luZC5jb20xCzAJBgNVBAYTAklO\n\
MQswCQYDVQQIEwJBUDESMBAGA1UEBxMJSHlkZXJhYmFkMRgwFgYDVQQKEw9JbnRv\n\
dG8gU29mdHdhcmUxEjAQBgNVBAsTCVN5cy1BZG1pbjEXMBUGA1UEAxMOSW50b3Rv\n\
IFRlc3QgQ0EwHhcNMDkwMjAyMTIzNDUzWhcNMTkwMjAyMTI0NDUzWjARMQ8wDQYD\n\
VQQDEwZpbnRvdG8wgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBALrB6Si32cgy\n\
hQ6HGmiqg8c+LZYNR2Zcew6qZ+h/blEigcJS//7EC2IDsz419xuAilD5JCVgKCyM\n\
uARLH0WNlvnSsfVuUrMlViljcZ7USUUb6lrlkv8uBQzjcX/OhKhOdsbZp+0l4ONK\n\
PlZiWWNArDxSg2UJqTyKoKYzATq8UWfXAgMBAAGjggKVMIICkTAZBgNVHREEEjAQ\n\
gg53d3cuaW50b3RvLmNvbTAdBgNVHQ4EFgQUqtl6E2iVkq/VvWqzAZun/047TAMw\n\
gdkGA1UdIwSB0TCBzoAUN9etLLc6zplF2ji4e73bMPyde26hgaOkgaAwgZ0xJjAk\n\
BgkqhkiG9w0BCQEWF3N5cy1hZG1pbkBpbnRvdG9pbmQuY29tMQswCQYDVQQGEwJJ\n\
TjELMAkGA1UECBMCQVAxEjAQBgNVBAcTCUh5ZGVyYWJhZDEYMBYGA1UEChMPSW50\n\
b3RvIFNvZnR3YXJlMRIwEAYDVQQLEwlTeXMtQWRtaW4xFzAVBgNVBAMTDkludG90\n\
byBUZXN0IENBghBF/+FsNKPYrU4Q7x8rz7dyMIGZBgNVHR8EgZEwgY4wRKBCoECG\n\
Pmh0dHA6Ly9iaGFyYXRpLmludG90b2luZC5jb20vQ2VydEVucm9sbC9JbnRvdG8l\n\
MjBUZXN0JTIwQ0EuY3JsMEagRKBChkBmaWxlOi8vXFxiaGFyYXRpLmludG90b2lu\n\
ZC5jb21cQ2VydEVucm9sbFxJbnRvdG8lMjBUZXN0JTIwQ0EuY3JsMIHcBggrBgEF\n\
BQcBAQSBzzCBzDBjBggrBgEFBQcwAoZXaHR0cDovL2JoYXJhdGkuaW50b3RvaW5k\n\
LmNvbS9DZXJ0RW5yb2xsL2JoYXJhdGkuaW50b3RvaW5kLmNvbV9JbnRvdG8lMjBU\n\
ZXN0JTIwQ0EoMSkuY3J0MGUGCCsGAQUFBzAChllmaWxlOi8vXFxiaGFyYXRpLmlu\n\
dG90b2luZC5jb21cQ2VydEVucm9sbFxiaGFyYXRpLmludG90b2luZC5jb21fSW50\n\
b3RvJTIwVGVzdCUyMENBKDEpLmNydDANBgkqhkiG9w0BAQUFAANBAAeROV4XBZRT\n\
BOcgGDWTI0O1JmdGbbx0zwXO8wvOPbLoe6zn23KRXvlRIMa2mLSOoMVqJ+mpo7g8\n\
LBPI8mVzmY8=\n\
-----END CERTIFICATE-----";

void HTTPSLoadFactoryDefaultHTTPSCerts(void);

/***************************************************************************
 * Function Name : HTTPSLoadFactoryDefaultHTTPSCerts
 * Description   : This function is responsible for loading the factory
 *                 default certificates that are hardcoded in this file.
 *                 This function is called from Ldsv after the loading of
 *                 HTTPS certificates is complete in the condition where
 *                 there are no saved certificates.
 * Input         : NONE
 * Output        : NONE
 * Return value  : NONE
 ***************************************************************************/
void HTTPSLoadFactoryDefaultHTTPSCerts(void)
{
  unsigned char aHandle[8] = {0};
  int32_t  iRetVal;

  /* Load with the Factory Default Trusted Certificate */
  iRetVal = HttpsUploadTrustedCert('t', 'c', 'p', 'n', 'r', aFacDefTrustedCert,
                                   of_strlen((char *) aFacDefTrustedCert),
                                   aHandle, sizeof(aHandle));
  if(iRetVal != OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
      "HttpsLoadFacDefaultTrustedCert :: HttpsUploadTrustedCert failed\r\n");
#endif /*HTTPD_DEBUG*/
  }
  /* Load with the Factory Default Private key */
  iRetVal =  HttpsFlashWrite('s', 'p', 'p', 'n', 'r', aFacDefPrivateKey,
                             of_strlen((char *) aFacDefPrivateKey),(unsigned char *)  "intoto");
  if(iRetVal != OF_SUCCESS)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
          "HttpsLoadFacDefaultPrivateKey :: HttpsFlashWrite  failed\n\r");
#endif /*HTTPD_DEBUG*/
  }

  /* Load with the Factory Default Self certificate */
  iRetVal =  HttpsStoreSelfCert(aFacDefSelfCert, of_strlen((char *) aFacDefSelfCert),
                                (unsigned char *) "intoto");
  if(iRetVal == OF_FAILURE)
  {
#ifdef HTTPD_DEBUG
    Trace(HTTPS_ID, TRACE_SEVERE,
      "HttpsLoadFacDefaultSelfCert :: FAILED\r\n");
#endif /*HTTPD_DEBUG*/
  }
  return;
}

#endif

