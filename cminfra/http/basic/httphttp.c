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
/*  File        : httphttp.c                                              */
/*                                                                        */
/*  Description : This file contains http callback functions related to   */
/*                the login, logout, etc.                                 */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      00.00.00    grsyam    Fixed problem when root logins with  */
/*                                   wrong password.                      */
/*    1.2      10.30.02     dram     Modified as per simplified UI        */
/*                                   requirement.                         */
/*    1.3      05.21.04     dram     Modified during the code review.     */
/*    1.4      22.02.06    Harishp   Modified during MultipleInstances	  */
/*                                   and Multithreading	                  */
/**************************************************************************/

#if defined(OF_HTTPD_SUPPORT) 
//&& defined(INTOTO_HTTPD_CBK_SUPPORT)
/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "httpdinc.h"
#include "htpdglb.h"
#include "igwrc4.h"
#include "httpgif.h"
#include "cmincld.h"
#include "dmgdef.h"
#include "cmtransgdf.h"
#include "jegdef.h"
#include "uchtldef.h"
#include "uchtlgif.h"
#include "authgdef.h"
#include "authgif.h"
#ifdef UCM_HTTP_SPUTM_MP_SUPPORT
#include <openssl/e_os2.h> /*for mgmt*/
#else
#ifdef OPENSSL_9_7
#include <openssl/e_os.h>
#else
#include <openssl/e_os2.h>
#endif /* OPENSSL_9_7*/
#endif
#include <openssl/rsa.h>
#include <openssl/crypto.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#define HTTP_MIN_SECS   300
#define HTTP_ERR_RESP_LEN   512
#define HTTP_XSL_HDR_FIL_LEN   100
#define XMLHTTP_START_TAG        "<root>" 
#define HTTP_XML_RESP_HEADER  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
#define XMLHTTP_MSG_START_TAG    "<Msg>"
#define XMLHTTP_MSG_END_TAG      "</Msg>"
#define XMLHTTP_REDIRECT_REF     "<rdpage>login.htm</rdpage>"
#define XMLHTTP_END_TAG          "</root>" 
#define HTTP_LOGINOUT_XSL "loginout.xsl"
#define HTTP_LOGINOUT_HTM "loginout.htm"

#define UCM_LOCALHOST_NAME "localhost"
#define UCM_MASTERHOST_NAME "MASTER"

/*HTTP Error messages*/
#define HTTP_SESSION_EXISTS_MSG "Session already exists." 
#define HTTP_SESSION_ERR_MSG  "Failed to create session!" 
#define HTTP_AUTH_ERR_MSG     "Invalid Username/Password Combination"

#define HTTPD_RSA_MAX_STRING_LEN  (255)
#define SetKey \
  key->n = BN_bin2bn(n, sizeof(n)-1, key->n); \
  key->e = BN_bin2bn(e, sizeof(e)-1, key->e); \
  key->d = BN_bin2bn(d, sizeof(d)-1, key->d); \
  return  0;

#define UCM_DMPATH_LEN 100

static int passwordkey(RSA *key, unsigned char *c)
{
static unsigned char n[] ="\x01\xD7\x77\x7C\x38\x86\x3A\xEC\x21\xBA\x2D\x91\xEE\x0F\xAF\x51";
static unsigned char e[] ="\x5A\xBB";
static unsigned char d[] ="\x01\x14\x6B\xD0\x7F\x0B\x74\xC0\x86\xDF\x00\xB3\x7C\x60\x2A\x0B";
SetKey;
}

extern uint16_t HttpAppID;
extern uint16_t HttpSubAppID;
extern uint32_t igwHTTPDLogUserMsg(uint32_t uiMsgId_p,uint32_t uiMesFamily,
                                   unsigned char *pUserName,ipaddr UserIpAddr,
                                   uint32_t ulLoginTime, uint32_t ulLogoutTime);
extern int32_t igwHTTPDGetUserloginTimeFromKuki(char *pUserName, uint32_t *pulLoginTime);
extern int32_t igwHTTPDUpdateUserloginTimeInKuki(char *pUserName, uint32_t ulLoginTime);

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  MAX_TAG_LENGTH               (128)
#define  IGW_LOG_MESSAGES_LEN         (256+1)
#define  IGW_RLOGOUT_MESSAGE_LEN      (256+1)
#define  IGW_IP_ADDRESS_LEN           (15+1)
#define  IGW_STATUS_MESSAGES_LEN      (255)
#define  IGW_SUBMIT_LEN               (25+1)

#define  HTTP_RSA_VAL1_LEN             (256)

#define  HTTP_HTML_MAX_MSG_LENGTH      (600)
#define  HTTP_MAX_MESSAGE_LEN          (1024)
#define  HTTP_MAX_STRING_LENGTH        (128)
#ifdef INTOTO_CMS_SUPPORT
#define ADMIN_PRIV                  0x01
#define USER_PRIV                   0x02
#endif  /* INTOTO_CMS_SUPPORT */
/************************************************************
 * * * *  V a r i a b l e    D e c l a r a t i o n s  * * * *
 ************************************************************/

/**
 * This is a part of http server.
 * So we can directly access the configuration without any wrappers!
 */
unsigned char      *ucIpVal;
/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

int32_t httpdSendErrMessage( HttpRequest *pHttpReq, char* msg);


int32_t HttpdSup_Gen_LogInfo( HttpRequest    *pRequest,
                              Httpd_Value_t  *ValAry,
                              uint32_t       nCount,
                              void         *pAppData );
 
int32_t httpdCreateUCMSessions(unsigned char* pUserName, unsigned char* pPassword,
                                         struct cm_role_info* pRoleInfo);

 int32_t httpdDeleteUCMSessions(char* pUserName);


int32_t SendStatusMessage( HttpRequest    *pRequest,
                                  Httpd_Value_t  *ValAry,
                                  uint32_t       nCount,
                                  void         *pAppData );

int32_t HttpdSup_Gen_LoginPageInfo( HttpRequest    *pRequest,
                                    Httpd_Value_t  *ValAry,
                                    uint32_t       nCount,
                                    void         *pAppData );

int32_t HttpdSup_Gen_ChPwdPage( HttpRequest    *pRequest,
                                Httpd_Value_t  *ValAry,
                                uint32_t       nCount,
                                void         *pAppData );

int32_t HttpdSup_Gen_RLogoutPage( HttpRequest    *pRequest,
                                  Httpd_Value_t  *ValAry,
                                  uint32_t       nCount,
                                  void         *pAppData );

int32_t HttpdSup_Action_PswdChng( HttpRequest    *pHttpReq,
                                  Httpd_Value_t  *ValAry,
                                  uint32_t       nCount );

int32_t HttpdSup_Action_Logout( HttpRequest    *pHttpReq,
                                Httpd_Value_t  *ValArray,
                                uint32_t       nCount );

int32_t HttpdSup_Action_Login( HttpRequest    *pHttpReq,
                               Httpd_Value_t  *ValAry,
                               uint32_t       nCount );


int32_t HttpdUCM_Action_Login( HttpRequest    *pHttpReq,
                               Httpd_Value_t  *ValAry,
                               uint32_t       nCount );

/*
int32_t HttpdSup_Gen_ErrInfo( HttpRequest    *pHttpReq,
                              Httpd_Value_t  *ValArray,
                              uint32_t       uiValCount,
                              void         *pAppData );*/

static void HttpdSup_HextToString( char  *pTmp,
                                     char  *pOutput,
                                     uint32_t *pLen );
 void HttpHex2Str(unsigned char *pTmp,unsigned char *pOutput,uint32_t *pLen);


int32_t HttpdSup_Gen_LogInDuplicate( HttpRequest    *pRequest,
                                      Httpd_Value_t  *ValAry,
                                      uint32_t       nCount);
#ifndef OF_CM_SUPPORT
                         
int32_t HttpdSup_AuthOrder( HttpRequest    *pHttpReq,
                            Httpd_Value_t  *ValAry,
                            uint32_t nCount); //FIXME UCM with PAM : replace it with appropriate PAM logic or remove


int32_t HttpdSup_AuthInfo( HttpRequest    *pHttpReq,
                           Httpd_Value_t  *ValArray,
                           uint32_t       uiValCount,
                           void         *pAppData );//FIXME UCM with PAM : replace it with appropriate PAM logic or remove
#endif /* OF_CM_SUPPORT */

/********************************************************
 * * * * * * V a r i a b l e     A r r a y s * * * * * *
 ********************************************************/

/* loginout.htm */
Httpd_Variable_t  HttpdVars_LogInfo[] =
{
  { "LOG_MESSAGES", HTTP_HTML_MAX_MSG_LENGTH }
};
#define HTTPDVARS_LOGINFO  \
            (sizeof(HttpdVars_LogInfo)/sizeof(Httpd_Variable_t))

Httpd_Variable_t  HttpdVars_StatusInfo[] =
{
  { "STATUS_MSG", IGW_STATUS_MESSAGES_LEN }
};
#define HTTPDVARS_STATUS_INFO  \
            (sizeof(HttpdVars_StatusInfo)/sizeof(Httpd_Variable_t))

/* login.htm */
Httpd_Variable_t  HttpdVars_LoginPageInfo[] =
{
  { "CHALLENGE_VALUE", HTTP_CHALLENGE_LEN + 1 }
};
#define HTTPDVARS_LOGIN_PAGEINFO  \
            (sizeof(HttpdVars_LoginPageInfo)/sizeof(Httpd_Variable_t))

Httpd_Variable_t  HttpdVars_ChPwdPage[] =
{
  { "USER_NAME1", MAX_TAG_LENGTH },
  { "USER_NAME2", MAX_TAG_LENGTH },
  { "RC4_KEY",    MAX_TAG_LENGTH }
};
#define HTTPDVARS_CHPWDPAGE  \
            (sizeof(HttpdVars_ChPwdPage)/sizeof(Httpd_Variable_t))

/* rlogout.htm */
Httpd_Variable_t  HttpdVars_RLogoutPage[] =
{
  { "RLOGOUT_MESSAGE", IGW_RLOGOUT_MESSAGE_LEN }
};
#define HTTPDVARS_RLOGOUT_PAGE  \
            (sizeof(HttpdVars_RLogoutPage)/sizeof(Httpd_Variable_t))

#define NAT_PASSWORD_LEN 33
/* pswdchng.igw */
Httpd_Variable_t  HttpdVars_PswdChng[] =
{
  { "newpassword", MAX_TAG_LENGTH       },
  { "cnfpassword", MAX_TAG_LENGTH       },
  { "submitbtn",   MAX_TAG_LENGTH       },
  { "newresponse", 2*NAT_PASSWORD_LEN+1 },
  { "cnfresponse", 2*NAT_PASSWORD_LEN+1 },
  { "challenge",   MAX_TAG_LENGTH       }
};
#define HTTPDVARS_PSWDCHNG  \
            (sizeof(HttpdVars_PswdChng)/sizeof(Httpd_Variable_t))

/**
 * Modified based on simplified UI requirements.
 */
Httpd_Variable_t  HttpdVars_Logout[] =
{
  { "remoteip", IGW_IP_ADDRESS_LEN },
  { "dmpath", UCM_DMPATH_LEN },
  { "host", IGW_IP_ADDRESS_LEN },
  { "submit",   IGW_SUBMIT_LEN     }
};
#define HTTPDVARS_LOGOUT  \
            (sizeof(HttpdVars_Logout)/sizeof(Httpd_Variable_t))

/* login.igw */
Httpd_Variable_t  HttpdVars_Login[] =
{
  { "username",  MAX_TAG_LENGTH },
  { "password",  MAX_TAG_LENGTH },
  { "challenge", MAX_TAG_LENGTH },
  { "response",  MAX_TAG_LENGTH }
};
#define HTTPDVARS_LOGIN  \
            (sizeof(HttpdVars_Login)/sizeof(Httpd_Variable_t))

Httpd_Variable_t HttpdVars_ErrorInfo[] =
{
  { "CONFIG_ERROR_MESSAGE", 256 }
};
#define HTTPDVARS_ERROR_INFO  \
            sizeof (HttpdVars_ErrorInfo)/sizeof(Httpd_Variable_t)
Httpd_Variable_t  Httpd_AuthInfoVars[] =
{
  { "HTTP_AUTH_LIST",  MAX_TAG_LENGTH }
};
#define HTTPD_AUTH_INFO_VARS  \
            (sizeof(Httpd_AuthInfoVars)/sizeof(Httpd_Variable_t))

Httpd_Variable_t  Httpd_AuthVars[] =
{
  { "finaldblist",  MAX_TAG_LENGTH }
};
#define HTTPD_AUTH_VARS  \
            (sizeof(Httpd_AuthVars)/sizeof(Httpd_Variable_t))

/*********************************************************
 * * * *   R e g i s t r a t i o n     T a b l e   * * * *
 *********************************************************/

Httpd_RegEntry_t  HttpdSup_LocalTags[] =
{
  { HTTPD_TAG_TYPE_GENERAL, "LOG_INFO", HttpdSup_Gen_LogInfo,
    HTTPDVARS_LOGINFO, HttpdVars_LogInfo,HTTP_CBKFN_PRIV_NORMAL },

  { HTTPD_TAG_TYPE_GENERAL, "STATUS_INFO", SendStatusMessage,
    HTTPDVARS_STATUS_INFO, HttpdVars_StatusInfo,HTTP_CBKFN_PRIV_NORMAL },

  { HTTPD_TAG_TYPE_GENERAL, "LOGIN_PAGE_INFO", HttpdSup_Gen_LoginPageInfo,
    HTTPDVARS_LOGIN_PAGEINFO, HttpdVars_LoginPageInfo,HTTP_CBKFN_PRIV_NORMAL },

  { HTTPD_TAG_TYPE_GENERAL, "PSWDCHNG_PAGE_INFO", HttpdSup_Gen_ChPwdPage,
    HTTPDVARS_CHPWDPAGE, HttpdVars_ChPwdPage ,HTTP_CBKFN_PRIV_NORMAL},

  { HTTPD_TAG_TYPE_GENERAL, "RLOGOUT_PAGE", HttpdSup_Gen_RLogoutPage,
    HTTPDVARS_RLOGOUT_PAGE, HttpdVars_RLogoutPage,HTTP_CBKFN_PRIV_NORMAL },

  { HTTPD_TAG_TYPE_ACTION, "pswdchng.igw", HttpdSup_Action_PswdChng,
    HTTPDVARS_PSWDCHNG, HttpdVars_PswdChng ,HTTP_CBKFN_PRIV_NORMAL},

  { HTTPD_TAG_TYPE_ACTION, "logout.igw", HttpdSup_Action_Logout,
    HTTPDVARS_LOGOUT, HttpdVars_Logout,HTTP_CBKFN_PRIV_NORMAL },

  /*{ HTTPD_TAG_TYPE_ACTION, "login.igw", HttpdSup_Action_Login,
    HTTPDVARS_LOGIN, HttpdVars_Login,HTTP_CBKFN_PRIV_NORMAL },*/

  { HTTPD_TAG_TYPE_ACTION, "login.igw", HttpdUCM_Action_Login,
    HTTPDVARS_LOGIN, HttpdVars_Login,HTTP_CBKFN_PRIV_NORMAL },
    
  { HTTPD_TAG_TYPE_GENERAL, "ERROR_INFO", HttpdSup_Gen_ErrInfo,
    HTTPDVARS_ERROR_INFO, HttpdVars_ErrorInfo,HTTP_CBKFN_PRIV_NORMAL },

  { HTTPD_TAG_TYPE_GENERAL, "closeprevious.igw", HttpdSup_Gen_LogInDuplicate,
    0,NULL,0}, 
 #ifndef OF_CM_SUPPORT 
  { HTTPD_TAG_TYPE_ACTION, "authchange.igw", HttpdSup_AuthOrder,
    HTTPD_AUTH_VARS, Httpd_AuthVars,HTTP_CBKFN_PRIV_NORMAL },
    
  { HTTPD_TAG_TYPE_GENERAL, "HTTP_AUTH_INFO", HttpdSup_AuthInfo,
    HTTPD_AUTH_INFO_VARS, Httpd_AuthInfoVars,HTTP_CBKFN_PRIV_NORMAL },

 #endif /* OF_CM_SUPPORT */
  { HTTPD_TAG_TYPE_NONE, NULL, NULL,
    0, NULL }
};

/********************************************************
 * * I n i t i a l i z a t i o n     F u n c t i o n  * *
 ********************************************************/

int32_t Httpd_RegisterLocalTags(void)
{
  return Httpd_Register_TagArray(HttpdSup_LocalTags);
} /* Httpd_RegisterLocalTags() */

/******************************************************
 * * * *  S u p p o r t      F u n c t i o n s  * * * *
 ******************************************************/

/************************************************************************
 * Function Name : HttpdSup_Gen_LogInfo                                 *
 * Description   : Sends the log information if there is any problem in *
 *                 login.                                               *
 * Input         : HTTP request structure.                              *
 * Output        : Information related to login access problems.        *
 * Return value  : OF_SUCCESS on sending all data else OF_FAILURE.        *
 ************************************************************************/

int32_t HttpdSup_Gen_LogInfo( HttpRequest    *pRequest,
                              Httpd_Value_t  *ValAry,
                              uint32_t       nCount,
                              void         *pAppData )
{
  /**
   * Verify the application data parameter.
   */
  if (!pAppData)
  {
    return OF_FAILURE;
  }

  /**
   * Copy the application data parameter to the tag field.
   */
  Httpd_Value_CopyStr(&ValAry[0], pAppData);

  return OF_SUCCESS;
} /* HttpdSup_Gen_LogInfo() */

/************************************************************************
 * Function Name : HttpdSup_Gen_LoginPageInfo                           *
 * Description   : Sends the information related to relogin.            *
 * Input         : HTTP request structure.                              *
 * Output        : Information related to relogin.                      *
 * Return value  : OF_SUCCESS on sending all data else OF_FAILURE.        *
 ************************************************************************/

int32_t HttpdSup_Gen_LoginPageInfo( HttpRequest    *pRequest,
                                    Httpd_Value_t  *ValAry,
                                    uint32_t       nCount,
                                    void         *pAppData )
{
  char  Challenge[HTTP_CHALLENGE_LEN + 1];

  if (HttpCookie_GetByVal(pRequest->pCookieVal, NULL, (unsigned char *) Challenge,
                          NULL) == OF_SUCCESS)
  {
    Httpd_Value_CopyStr(&ValAry[0], Challenge);
    return OF_SUCCESS;
  }
  else
  {
    return OF_FAILURE;
  }
} /* HttpdSup_Gen_LoginPageInfo() */

/************************************************************************
 * Function Name : HttpdSup_Gen_ChPwdPage                               *
 * Description   : Sends the information related to relogin.            *
 * Input         : HTTP request structure.                              *
 * Output        : Information related to relogin.                      *
 * Return value  : OF_SUCCESS on sending all data else OF_FAILURE.        *
 ************************************************************************/

int32_t HttpdSup_Gen_ChPwdPage( HttpRequest    *pRequest,
                                Httpd_Value_t  *ValAry,
                                uint32_t       nCount,
                                void         *pAppData )
{
  Httpd_Value_CopyStr(&ValAry[0], (char *) pRequest->pUserName);
  Httpd_Value_CopyStr(&ValAry[1], "");
  //UDBGetRc4Key(ValAry[2].value_p); // for UCM

  return OF_SUCCESS;
} /* HttpdSup_Gen_ChPwdPage() */

/************************************************************************
 * Function Name : HttpdSup_Gen_RLogoutPage                             *
 * Description   : Sends the information related to re logout.          *
 * Input         : HTTP request structure.                              *
 * Output        : Information related to re logout.                    *
 * Return value  : OF_SUCCESS on sending all data else OF_FAILURE.        *
 ************************************************************************/

int32_t HttpdSup_Gen_RLogoutPage( HttpRequest    *pRequest,
                                  Httpd_Value_t  *ValAry,
                                  uint32_t       nCount,
                                  void         *pAppData )
{
  /**
   * Verify the application data parameter.
   */
  if (!pAppData)
  {
    return OF_FAILURE;
  }

  /**
   * Copy the application data parameter to the tag field.
   */
  Httpd_Value_CopyStr(&ValAry[0], pAppData);

  return OF_SUCCESS;
} /* HttpdSup_Gen_RLogoutPage() */

/************************************************************************
 * Function Name : HttpdSup_Action_PswdChng                             *
 * Description   : Sends the information related to re logout.          *
 * Input         : HTTP request structure.                              *
 * Output        : Information related to re logout.                    *
 * Return value  : OF_SUCCESS on sending all data else OF_FAILURE.        *
 ************************************************************************/

int32_t HttpdSup_Action_PswdChng( HttpRequest    *pHttpReq,
                                  Httpd_Value_t  *ValAry,
                                  uint32_t       nCount )
{
  uint32_t      len = 0;
#ifndef OF_CM_SUPPORT
  int32_t       RetVal;
  char       aPassword[128];
  char       aKey[256];
  IGWRC4key_t   Key;
  /*UserDbTemp_t  User;*/ /* for UCM */
 bool       isMultipleSnetsEnabled = FALSE;
 uint32_t      uiSrcDirection         = 0;

 /**
   * Check whether multiple Snet feature is enabled.
   */
   
  IGWIsMultipleSnetsEnabled(&isMultipleSnetsEnabled);

  if (isMultipleSnetsEnabled == TRUE)
  {
    /**
     * Using peer ip address get the snet id or direction.
     */
    uiSrcDirection = HttpGetPktDirection(&pHttpReq->PeerIp, NULL, NULL,
                                               IGW_SOURCE_SUBSCRIBER_INFO);
  
    /**
     * Check if the source direction is valid or not.
     */
     
     if((uiSrcDirection == OF_FAILURE)||(uiSrcDirection >= IGW_MAX))
     {
         if(HttpSendHtmlFile(pHttpReq,  (unsigned char *)"loginout.htm",
     (unsigned char *) "Error in SNET direction <BR><a href=\"login.htm\">Login Again</a>")==OF_FAILURE)
         {
            HttpDebug("HttpSendHtmlFile Failed\n");
            return OF_FAILURE;
         }
         return OF_SUCCESS;
     }
   
  
 }
#endif /*OF_CM_SUPPORT*/
  /**
   * Check whether new password & confirm password are same.
   */
  if (of_strcmp(ValAry[3].value_p, ValAry[4].value_p))
  {
    HttpSendHtmlFile(pHttpReq, (unsigned char *) "rlogout.htm",
                     (unsigned char *) "Error<HR>New Password and Confirm Should Be Same");
    return OF_SUCCESS;
  }

  HttpdSup_HextToString(ValAry[3].value_p, ValAry[4].value_p, &len);

#ifndef OF_CM_SUPPORT 

   UDBGetRc4Key(aKey);
  of_memset((void*)aPassword, 0, sizeof aPassword);
  IGWRC4SetKey(&Key, of_strlen(aKey), aKey);
  IGWRC4(&Key, len, ValAry[4].value_p, aPassword);
 
  if ((of_strlen (aPassword) > UDB_PASSWORD_LENGTH14))
  {
    HttpSendHtmlFile(pHttpReq, "rlogout.htm",
        "Error<HR> Password length should not exceed 14.");
    return OF_SUCCESS;
  }
   
  if (GetExactUsrDbRec(uiSrcDirection, pHttpReq->pUserName, &User) != OF_SUCCESS)
  {
          if (isMultipleSnetsEnabled == TRUE)
   	  {
   	        uiSrcDirection         = 0;
       	if(GetExactUsrDbRec(uiSrcDirection, pHttpReq->pUserName, &User) != OF_SUCCESS)
       	{         
		    RetVal= HTTPD_ERR_NO_USER;
              }
       	else
              {
                   RetVal= ModifyUserDb(uiSrcDirection, pHttpReq->pUserName, aPassword,
                         User.uiPrivileges, User.lInactivityTimer,
                         &User.DualIPAddr, User.aPolGrname_p, User.userType);
                }
           }
           else
           {
          	 RetVal= HTTPD_ERR_NO_USER;
           }
  }
  else
  {
    RetVal= ModifyUserDb(uiSrcDirection, pHttpReq->pUserName, aPassword,
                         User.uiPrivileges, User.lInactivityTimer,
                         &User.DualIPAddr, User.aPolGrname_p, User.userType);
  }

  switch (RetVal)
  {
  case HTTPD_ERR_NO_USER:
  case UDB_USER_NOT_FOUND:
    {
      if (HttpSendHtmlFile(pHttpReq,(unsigned char *) "loginout.htm",
                          (unsigned char *) "Invalid Username/Password Combination")
                               == OF_FAILURE)
      {
        Httpd_Error("HttpSendHtmlFile Failed\n");
      }
      break;
    }
  case HTTPD_ERR_BAD_NAME:
    {
      if (HttpSendHtmlFile(pHttpReq,(unsigned char *) "loginout.htm",
                          (unsigned char *) "Username is Required!") == OF_FAILURE)
      {
        Httpd_Error("HttpSendHtmlFile Failed\n");
      }
      break;
    }
  case 0: /* Success */
    {
      if (HttpSendHtmlFile(pHttpReq,(unsigned char *) "rlogout.htm",
         (unsigned char *) "Password Changed Successfully!"))
      {
        break;
      }
    }
  }

#endif /*OF_CM_SUPPORT*/

/* Need to send rlogout.htm on success.*/
    return OF_SUCCESS;
 
} /* HttpdSup_Action_PswdChng() */

/************************************************************************
 * Function Name : HttpdSup_Action_Logout                               *
 * Description   : This function is used for logout.                    *
 * Input         : HTTP request structure.                              *
 *                 Charecter buffer containing the user data line.      *
 *                 Length of this buffer.                               *
 * Output        : Html file to send to the browser with or without     *
 *                 information based on the operation performed.        *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.        *
 ************************************************************************/

int32_t HttpdSup_Action_Logout( HttpRequest    *pHttpReq,
                                Httpd_Value_t  *ValArray,
                                uint32_t       nCount )
{
  /**
   * 0. remoteip
   * 1. dmpath
   * 2. host
   * 3. submit
   */
  bool isAuth       = FALSE;
  bool isSaveOption = FALSE;
  ipaddr IpAddress = 0;

  char SrcIp[CM_INET6_ADDRSTRLEN];
#ifdef MESGCLNT_SUPPORT
  char Message[MAX_LOG_MESSAGE_SIZE];
#endif /*MESGCLNT_SUPPORT*/

  int32_t bIsConfig = FALSE;
  char *pXmlBuf=NULL;
  uint32_t ulLogoutTime=0;
  uint32_t ulLogInTime = 0;
  UCMHttpSession_t *pUCMHttpSession = NULL;
  /**
   * Check the submit option.
   */
  if (Httpd_Value_StrCmp(&ValArray[3], "Cancel") == 0)
  {
    /**
     * Introduced System Call Function.
     */
    if (Httpd_HandleSystemCookies(pHttpReq,
                                  HTTP_SYSTEM_LOGOUT_COOKIE) != OF_SUCCESS)
    {
      Httpd_Debug("\r\nHttpd_HandleSystemCookies Failed\r\n");
    }

    if (HttpSendHtmlFile(pHttpReq, (unsigned char *) "loadwelcome.htm", NULL) == OF_FAILURE)
    {
      return OF_FAILURE;
    }
    return OF_SUCCESS;
  }
  else if (Httpd_Value_StrCmp(&ValArray[3], "Save and Logout") == 0)
  {
    isSaveOption = TRUE;

#ifdef HTTPS_ENABLE_OLD
    if (SaveHttpsCerts() != OF_SUCCESS)
    {
      if (HttpSendHtmlFile(pHttpReq,(unsigned char *) "errscrn.htm",
                          (unsigned char *) "Failed to Save certificates") != OF_SUCCESS)
      {
        return OF_FAILURE;
      }
      return OF_SUCCESS;
    }
#endif /* HTTPS_ENABLE */

#ifdef OF_CM_SUPPORT
    if(cm_val_and_conv_aton( ValArray[2].value_p, &IpAddress) == OF_FAILURE )
    {
      UCMHTTPDEBUG("  %s :: Invalid IP Address \r\n",__FUNCTION__ );
    }
    if(ucmHTTPFindSessionEntry((char *) pHttpReq->pUserName,
                               IpAddress, &pUCMHttpSession) == OF_SUCCESS) {
      ucmHttpSaveConfig(pUCMHttpSession, ValArray[1].value_p, &pXmlBuf);
      if(pXmlBuf)
         of_free(pXmlBuf);
    }
#endif /*OF_CM_SUPPORT*/

#ifndef OF_CM_SUPPORT
   if(IGWLstSendSaveRequest(TRUE, NULL) == OF_FAILURE)
   {

     HttpSendHtmlFile(pHttpReq,(unsigned char *) "errscrn.htm",
                        (unsigned char *)  "Failed to save the configuration");
   }
#endif /*OF_CM_SUPPORT*/
    
  }

  bIsConfig = HttpUserIsConfigType((char *) pHttpReq->pUserName);

  if (bIsConfig == TRUE)
  {
#ifdef INTOTO_IPV6_SUPPORT
    if(pHttpReq->PeerIp.ip_type_ul == IGW_INET_IPV6)
    {
      if(HttpdNumToPresentation(&pHttpReq->PeerIp,(unsigned char *)SrcIp)!=OF_SUCCESS)
      {		
         HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
         return OF_FAILURE;
      }
    }
    else
#endif /*INTOTO_IPV6_SUPPORT*/
    {
      cm_inet_ntoal(pHttpReq->PeerIp, SrcIp);
    }	

    of_time((time_t *)&ulLogoutTime);
    igwHTTPDGetUserloginTimeFromKuki((char *) pHttpReq->pUserName,&ulLogInTime);

#ifdef MESGCLNT_SUPPORT
    of_memset(Message, 0, sizeof(Message));
	sprintf(Message, "Web- Config user \"%s\"logged out from Ip Addr: %s",
						  pHttpReq->pUserName, SrcIp);
    /*IBsendSysLogMsg(NULL, 0, 0, Message, 0, ISEC_AUTH_LOGIN, LOGINFO,
      HTTPD_SYSMSGID_ADMIN_USR_LOGOUT_0);*/

    igwHTTPDLogUserMsg(HTTPD_SYSMSGID_ADMIN_USR_LOGOUT_0,
                       IGW_LOG_FAMILY_USERLOGIN,pHttpReq->pUserName,
                       pHttpReq->PeerIp,ulLogInTime,ulLogoutTime);
#endif /* MESGCLNT_SUPPORT    */
  }

  if (isSaveOption)
  {
    of_sleep_ms(100);
  }

  if (HttpCookie_GetByVal(pHttpReq->pCookieVal, &isAuth, NULL,
                          NULL) != OF_SUCCESS)
  {
    HttpRequest_SetPageType(pHttpReq, LOGINPAGEREQUEST);
    return HttpSendHtmlFile(pHttpReq,
                           (unsigned char *)  "loginout.htm", (unsigned char *) "Please Login Again\n");
  }

  /*Log the Remote user logout */ 
  if (bIsConfig != TRUE)
  {
     of_time((time_t *)&ulLogoutTime);
     igwHTTPDGetUserloginTimeFromKuki((char *) pHttpReq->pUserName,&ulLogInTime);
#ifdef MESGCLNT_SUPPORT
     igwHTTPDLogUserMsg(HTTPD_SYSMSGID_REMOTE_USR_LOGOUT,
           IGW_LOG_FAMILY_USERLOGIN,pHttpReq->pUserName,
           pHttpReq->PeerIp,ulLogInTime,ulLogoutTime);
#endif /* MESGCLNT_SUPPORT */
  }

  if (HttpCookie_Del(pHttpReq->pCookieVal) == OF_SUCCESS)
  {
    Httpd_Debug("Cookie Node Deletion is Successful\n");
  }
  else
  {
    Httpd_Error("Unable To Delete Cookie Node\n");
  }

#ifdef OF_CM_SUPPORT
  if(httpdDeleteUCMSessions((char *) pHttpReq->pUserName) != OF_SUCCESS)
  {
    UCMHTTPDEBUG("Unable To Delete UCM HTTP Session.\n");
  }
#endif /* OF_CM_SUPPORT */

  HttpRequest_SetPageType(pHttpReq, LOGINPAGEREQUEST);

  if (isAuth)
  {
    if (isSaveOption)
    {
      return HttpSendHtmlFile(pHttpReq, (unsigned char *) "loginout.htm",
 (unsigned char *)"Saved the configuration and Logged Out Successfully!<BR><a href=\"login.htm\">Login Again</a>");
    }
    else
    {
      return HttpSendHtmlFile(pHttpReq,(unsigned char *) "loginout.htm",
        (unsigned char *)  "Logged Out Successfully!<BR><a href=\"login.htm\">Login Again</a>");
    }
  }
  else
  {
    return HttpSendHtmlFile(pHttpReq, (unsigned char *)"loginout.htm",
                         (unsigned char *)   "First Login, Then Logout!\n");
  }

  return OF_FAILURE;
} /* HttpdSup_Action_Logout() */


/************************************************************************
 * Function Name : HttpdSup_Action_Login                                *
 * Description   : This function is used for login.                     *
 * Input         : HTTP request structure.                              *
 *                 Charecter buffer containing the user data line.      *
 *                 Length of this buffer.                               *
 * Output        : Html file to send to the browser with or without     *
 *                 information based on the operation performed.        *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.        *
 ************************************************************************/
#ifndef OF_CM_SUPPORT

int32_t HttpdSup_Action_Login( HttpRequest    *pHttpReq,
                               Httpd_Value_t  *ValAry,
                               uint32_t       nCount )
{

  bool       isAuth                 = FALSE;
  bool       isMultipleSnetsEnabled = FALSE;
  int32_t      uiSrcDirection         = 0;
  /*IGWIpAddr_t  uiDestIPAddress        = {}; Need to fix for IPv6*/
  ipaddr  uiDestIPAddress;
  int32_t      uiDestDirection        = 0;
  uint32_t      ulSrcId               = 0;
  uint32_t      ulDestId              = 0;
  uint32_t      uiResult               = 0;
  int32_t       AuthRetValue           = HTTPD_ERR_UDB;
  bool       RootOrGuest            = FALSE;
  char       Challenge[HTTP_CHALLENGE_LEN + 1];
  IGWSNetOps_t  snetOpts;
  char       Message[MAX_LOG_MESSAGE_SIZE];
  char       sSNetName[IGW_SNET_NAME_LEN];
  char       SrcIp[IGW_INET6_ADDRSTRLEN];
  UserDbTemp_t  pUserNode_p;
#ifdef INTOTO_CMS_SUPPORT
  uint32_t      uiTimeOut = 360;
#else
  uint32_t      uiTimeOut;
#endif  /* INTOTO_CMS_SUPPORT */
  unsigned char     *IpCheck;
  unsigned char     *ucCookieValue=NULL;
  unsigned char      ucRsaPasswordInVal[HTTP_RSA_VAL1_LEN];
  char	    *ucPasswordMD5=NULL,*cValOut=NULL;
  unsigned char      ucRsaPasswordOutval[HTTP_RSA_VAL1_LEN];
  uint32_t      ucPlen=0;
  RSA 		    *Rsakey;
  unsigned char      ucPtext[HTTP_RSA_VAL1_LEN],*ucStringVal=NULL;
  unsigned char      ucText[HTTP_RSA_VAL1_LEN];  
  unsigned char	    ucCtext[HTTP_RSA_VAL1_LEN],*ucS=NULL,*ucT=NULL,*ucTs=NULL;
  unsigned char      ucCT[HTTP_RSA_VAL1_LEN];
  int32_t	    iTlen=0;
  int32_t	    iNum=0,iVal=0,i=0;

  int32_t       iValidity=0;
  char       ucMsg[HTTP_MAX_MESSAGE_LEN];
  HttpRequest	 *DummyReq=NULL;
  HttpGlobals_t   *pHttpg=NULL;
  uint32_t ulLogInTime = 0;

  pHttpg=(HttpGlobals_t *)pHttpReq->pGlobalData;
  if(!pHttpg)
  {
    Httpd_Error("pHttpg is NULL\r\n");
    return OF_FAILURE;
  }

  
  /*RSA Decryption:password coming in chunks 32 characters per each chunk
   * max received chunks is 3 (32).
   * */
  of_memset(ucRsaPasswordOutval,'\0',HTTP_RSA_VAL1_LEN);
  of_memset(ucCT,'\0',HTTP_RSA_VAL1_LEN);   
  of_memset(ucPtext,'\0',sizeof(ucPtext));
  of_memset(ucText,'\0',sizeof(ucText));
  
  iTlen=of_strlen(ValAry[1].value_p);

  of_strcpy(ucRsaPasswordInVal,ValAry[1].value_p);
  HttpHex2Str(ucRsaPasswordInVal,ucRsaPasswordOutval,&ucPlen);
  Rsakey = RSA_new();
  if(Rsakey == NULL)
  {
    Httpd_Error("Memory allocation failed\r\n");
    return OF_FAILURE;
  }

  passwordkey(Rsakey, ucCtext);
  iNum=RSA_private_decrypt((int32_t)ucPlen,ucRsaPasswordOutval,ucPtext,Rsakey,RSA_NO_PADDING);
  while(iVal < iNum )
  {
	    ucText[iVal] = ucPtext[(iNum-1)-iVal];
	    iVal++;
  }
  of_strcpy(ucCT,ucText);
  
  /*If we are receving more than 1 chunk (iTlen >32) do the below extra  processing */
  
  if(iTlen >32) 
  {
   	ucTs=strtok(ValAry[1].value_p," ");      
	while(ucTs && ucTs!='\0')
   	{
	   ucTs=strtok(NULL," ");
	   if(i==0) ucS=ucTs;
	   if(i==1) ucT=ucTs;
	   i++; 	   
	}
        of_memset(ucPtext,'\0',sizeof(ucPtext));
        of_memset(ucText,'\0',sizeof(ucText));
        of_memset(ucRsaPasswordOutval,'\0',HTTP_RSA_VAL1_LEN);
        iVal=0;
        HttpHex2Str(ucS,ucRsaPasswordOutval,&ucPlen);
        iNum=RSA_private_decrypt((int32_t)ucPlen,ucRsaPasswordOutval,ucPtext,Rsakey,RSA_NO_PADDING);
        while(iVal < iNum )
        {
       		 ucText[iVal] = ucPtext[(iNum-1)-iVal];
	        iVal++;
        } 
   	of_strcat(ucCT,ucText);
        if(iTlen >65) 
        {
          iVal=0;
          of_memset(ucPtext,'\0',sizeof(ucPtext));
          of_memset(ucText,'\0',sizeof(ucText));
          of_memset(ucRsaPasswordOutval,'\0',HTTP_RSA_VAL1_LEN);
   	  HttpHex2Str(ucT,ucRsaPasswordOutval,&ucPlen);
	  iNum=RSA_private_decrypt((int32_t)ucPlen,ucRsaPasswordOutval,ucPtext,Rsakey,RSA_NO_PADDING);
   	  while(iVal < iNum )
   	  {
	     ucText[iVal] = ucPtext[(iNum-1)-iVal];
	     iVal++;
   	  }
   	  of_strcat(ucCT,ucText);
        }
  }

 #ifdef INTOTO_CMS_SUPPORT
  HttpCookie_GetByVal(pHttpReq->pCookieVal, &isAuth,Challenge,NULL);
  of_strcpy(ValAry[2].value_p,Challenge);
#endif  /* INTOTO_CMS_SUPPORT */  
  if((ucStringVal=of_calloc(1,HTTP_RSA_VAL1_LEN))==NULL)
  {
    Httpd_Error("Memory allocation failed\r\n");
    return OF_FAILURE;
  }

  if((cValOut=of_calloc(1,HTTP_RSA_VAL1_LEN))== NULL)
  {
    of_free(ucStringVal);
    Httpd_Error("Memory allocation failed\r\n");
    return OF_FAILURE;
  }

  of_strcpy(ucStringVal,ValAry[0].value_p);
  of_strcat(ucStringVal,":");
  Httpd_MD5Encrypt(ucCT,cValOut);
  of_strcat(ucStringVal,cValOut);
  of_strcat(ucStringVal,":");
  of_strcat(ucStringVal,ValAry[2].value_p);   
  if((ucPasswordMD5=of_calloc(1,HTTP_RSA_VAL1_LEN))==NULL)
  {
    of_free(ucStringVal);
    of_free(cValOut);
    Httpd_Error("Memory allocation failed\r\n");
    return OF_FAILURE;
  }  
  Httpd_MD5Encrypt(ucStringVal,ucPasswordMD5);   
#ifdef HTTPD_DEBUG
  printf("ucCT(clear text password)=%s\r\n",ucCT); 
#endif
  
  /*free the allocated memory  */

  of_free(ucStringVal);
  of_free(cValOut);
	  
#ifndef OF_CM_SUPPORT
  /**
   * Check whether multiple Snet feature is enabled.
   */
  IGWIsMultipleSnetsEnabled(&isMultipleSnetsEnabled);
#ifndef INTOTO_CMS_SUPPORT
  if (isMultipleSnetsEnabled == TRUE)
  {
    /**
     * Using peer ip address get the snet id or direction.
     */
     uiSrcDirection = HttpGetPktDirection(&pHttpReq->PeerIp, NULL, (void *)&ulSrcId,     
                                                               IGW_SOURCE_SUBSCRIBER_INFO);                                 
                                                               
 /**
     * Check if the source direction is valid or not.
     */
     if((uiSrcDirection == OF_FAILURE)||(uiSrcDirection >= IGW_MAX))
     {
         if(HttpSendHtmlFile(pHttpReq,(unsigned char *) "loginout.htm",
     (unsigned char *) "Error in SNET direction <BR><a href=\"login.htm\">Login Again</a>")==OF_FAILURE)
         {
            of_free(ucPasswordMD5);
            HttpDebug("HttpSendHtmlFile Failed\n");
            return OF_FAILURE;
         }
         of_free(ucPasswordMD5);
         return OF_SUCCESS;
     }

    /**
     * Check the sourece direction
     */
    if (uiSrcDirection != IGW_EXT)
    {
          /**
           * Initialize the snetOpts data structure.
           */
          of_memset(&snetOpts, 0, sizeof(snetOpts));
    
          /**
           * Set the SNet Id with the obtained source direction.
           */
          snetOpts.ulSNetId = ulSrcId;
    
          /**
           * Get the SNet information.
           */
          uiResult = IGWSNetSearchByIndex(&snetOpts);
    
          if (uiResult != OF_SUCCESS)
          {
            HttpSendHtmlFile(pHttpReq,(unsigned char *) "loginout.htm",
                "Error In Login<BR><a href=\"login.htm\">Login Again</a>");

            of_free(ucPasswordMD5);
            return OF_SUCCESS;
          }
          else
          {
            /**
             * Copy the SNet name.
             */
            of_strcpy(sSNetName, snetOpts.aSNetName);
          }
    }
    else
    {
      /**
       * Get Socket IP Address.
       */
      HttpdGetSockIPAddress(pHttpReq, &uiDestIPAddress);

      /**
       * Using peer ip address get the snet id or direction.
       */
      uiDestDirection = HttpGetPktDirection(&uiDestIPAddress, NULL, (void *)&ulDestId,
                                                                IGW_DEST_SUBSCRIBER_INFO);              
   /**
     * Check if the Destination direction is valid or not.
     */
     if((uiDestDirection == OF_FAILURE)||(uiDestDirection >= IGW_MAX))
     {
         if(HttpSendHtmlFile(pHttpReq,(unsigned char *) "loginout.htm",
         (unsigned char *) "Error in SNET direction <BR><a href=\"login.htm\">Login Again</a>")==OF_FAILURE)
         {
            of_free(ucPasswordMD5);
            HttpDebug("HttpSendHtmlFile Failed\n");
            return OF_FAILURE;
         }
         of_free(ucPasswordMD5);
         return OF_SUCCESS;
     }

      /**
       * Initialize the snetOpts data structure.
       */
      of_memset(&snetOpts, 0, sizeof(snetOpts));

      /**
       * Set the SNet Id with the obtained destination direction.
       */
      snetOpts.ulSNetId = ulDestId;

      /**
       * Get the SNet information.
       */
      uiResult = IGWSNetSearchByIndex(&snetOpts);

      if (uiResult != OF_SUCCESS)
      {
             HttpSendHtmlFile(pHttpReq,(unsigned char *) "loginout.htm",
              (unsigned char *) "Error In Login<BR><a href=\"login.htm\">Login Again</a>");

              of_free(ucPasswordMD5);
              return OF_SUCCESS;
      }
      else
      {
        /**
         * Copy the SNet name.
         */
        of_strcpy(sSNetName, snetOpts.aSNetName);
      }
    }
  }
  else
  {
    /**
     * Copy the SNet name.
     */
    of_strcpy(sSNetName, IGW_SNET_GENERAL_NAME);
  }
#endif  /* INTOTO_CMS_SUPPORT */
#endif  /* OF_CM_SUPPORT */

#ifdef INTOTO_CMS_SUPPORT
  of_strcpy(sSNetName, IGW_SNET_GENERAL_NAME);
  of_memset(&snetOpts, 0, sizeof(snetOpts));
  of_strcpy(snetOpts.aSNetName,sSNetName);
  if(IGWSNetSearchByName(&snetOpts) != OF_SUCCESS)
  {
    HttpDebug("Snet Search Failed\r\n");
  }
  pHttpReq->ulSnetID = snetOpts.ulSNetId;
#endif  /* INTOTO_CMS_SUPPORT */
  /**
   * Set the snet cookie.
   */
  if (Httpd_SetCookieInfo(pHttpReq, "SNET", sSNetName, NULL,
                          HTTP_MAX_CCOKIE_TIME_SECS) != OF_SUCCESS)
  {
    Httpd_Debug("\r\nSetting  of SNET cookie in login.igw failed");
  }

  /**
   * Introduced System Call Function.
   */
  if (Httpd_HandleSystemCookies(pHttpReq,
                                HTTP_SYSTEM_LOGIN_COOKIE) != OF_SUCCESS)
  {
    Httpd_Debug("\r\nHttpd_HandleSystemCookies Failed\r\n");
  }


  if (Httpd_SendCookieInfo(pHttpReq) != OF_SUCCESS)
  {
    Httpd_Debug("\r\n Sending SNET cookie failed in login.igw");
  }

  if (HttpCookie_GetByVal(pHttpReq->pCookieVal, &isAuth, Challenge,
                          NULL) == OF_SUCCESS)
  {
    /**
     * Get whether given user is with admin privelge or not.
     */
    RootOrGuest = HttpUserIsConfigType(ValAry[0].value_p);

#if defined (HTTPS_ENABLE)
    if (RootOrGuest == FALSE)
    {
      if (!Httpd_CheckUserLogin(pHttpg,pHttpReq, ValAry[0].value_p))
      {
        of_free(ucPasswordMD5);
        return OF_SUCCESS;
      }
    }
#endif /* HTTPS_ENABLE */

    AuthRetValue = HttpAuthUserLogin(pHttpReq, ValAry[0].value_p,
                                       ucPasswordMD5, Challenge,       /*ValAry[3].value_p */
					 pHttpReq->PeerIp, sSNetName);  
    of_free(ucPasswordMD5);
    ucPasswordMD5=NULL;
    if (RootOrGuest == TRUE)
    {
      if ((AuthRetValue == HTTPD_ERR_NO_USER) ||
          (AuthRetValue == HTTPD_ERR_USER_LOCKEDOUT))
      {
#ifdef HTTPD_DEBUG
        Httpd_Error("Root/Guest Authentication Failed, Message Logged\n");
#endif
        of_memset(Message, 0, sizeof(Message));
	#ifdef INTOTO_IPV6_SUPPORT
	  if(pHttpReq->PeerIp.ip_type_ul == IGW_INET_IPV6)
	  {
		if(HttpdNumToPresentation(&pHttpReq->PeerIp,(unsigned char *)SrcIp)!=OF_SUCCESS)
		{	
                 HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
                 return OF_FAILURE;
		}
	  }	  
	  else
	  #endif  /*INTOTO_IPV6_SUPPORT*/
	  {
	       cm_inet_ntoal(pHttpReq->PeerIp, SrcIp);
	  }
     
        if(AuthRetValue == HTTPD_ERR_USER_LOCKEDOUT)
        {
            sprintf(Message, "Web - config user \"%s\" login failed from ipaddr: %s"
        	          "reason: User locked out",ValAry[0].value_p, SrcIp);
        }
        else
        {
            sprintf(Message, "Web - config user \"%s\" login failed from ipaddr: %s"
        	          "reason: Password do not match",ValAry[0].value_p, SrcIp);
        }
#ifdef MESGCLNT_SUPPORT
        /*IBsendSysLogMsg(NULL, 0, 0, Message, 0, ISEC_UN_AUTH_ACCESS,
          LOGWARNING, HTTPD_SYSMSGID_ADMIN_USR_LOGIN_FAILED_1);*/

        of_time(&ulLogInTime);
        igwHTTPDUpdateUserloginTimeInKuki(ValAry[0].value_p,ulLogInTime);

        igwHTTPDLogUserMsg(HTTPD_SYSMSGID_ADMIN_USR_LOGIN_FAILED_1,
                           IGW_LOG_FAMILY_USERLOGIN,ValAry[0].value_p,
                           pHttpReq->PeerIp,ulLogInTime, 0) ;
#endif /*MESGCLNT_SUPPORT          */

        if (HttpCookie_Del(pHttpReq->pCookieVal) == OF_SUCCESS)
        {
          Httpd_Debug("Cookie Node Deletion is Successful\n");
        }
        else
        {
          Httpd_Error("Unable To Delete Cookie Node\n");
        }

        /**
         * Previously we were just displaying message.
         * We need to have link for the user to login again.
         */
        HttpRequest_SetPageType(pHttpReq, LOGINPAGEREQUEST);

        if (AuthRetValue == HTTPD_ERR_USER_LOCKEDOUT)
        {
          if (HttpSendHtmlFile(pHttpReq,(unsigned char *) "loginout.htm",
                 (unsigned char *) "User locked out! <BR> <a href=\"login.htm\"> Try again after sometime</a>")
                      == OF_FAILURE)
          {
            return OF_FAILURE;
          }
        }
        else
        {
          if (HttpSendHtmlFile(pHttpReq,(unsigned char *) "loginout.htm",
                (unsigned char *)  "Invalid Login! <BR> <a href=\"login.htm\"> Login Again</a>")
                      == OF_FAILURE)
          {
            return OF_FAILURE;
        }
        }
        return OF_SUCCESS;
      }
      else
      {
        AuthRetValue = OF_SUCCESS;
      }
    }

    if (AuthRetValue == HTTPD_ERR_NO_USER)
    { 	    
      pHttpReq->Auth=of_calloc(1,sizeof(AuthPriority_t));
      of_strcpy(pHttpReq->Auth->ucPasswd,ucCT);
      of_strcpy(pHttpReq->Auth->ucChallenge,Challenge);
      of_strcpy(pHttpReq->Auth->ucSnetName,sSNetName);
      of_strcpy(pHttpReq->pUserName,ValAry[0].value_p);
      
      DummyReq=of_calloc(1,sizeof(HttpRequest));
      if(DummyReq == NULL)
      {
           HttpDebug(("of_calloc failed for DummyReq \r\n"));
           return OF_FAILURE;
      }
      DummyReq->Auth=of_calloc(1,sizeof(AuthPriority_t));
      if(DummyReq->Auth  == NULL)
      {
           HttpDebug(("of_calloc  failed for DummyReq->Auth \r\n"));
	    of_free(DummyReq);	   
           return OF_FAILURE;
      }       	  
      of_memcpy(DummyReq,pHttpReq,sizeof(HttpRequest));
      of_memcpy(DummyReq->Auth,pHttpReq->Auth,sizeof(AuthPriority_t));	  
    }

    if (AuthRetValue != 0)
    {
         of_memset(Message, 0, sizeof(Message));		 
	  #ifdef INTOTO_IPV6_SUPPORT
	  if(pHttpReq->PeerIp.ip_type_ul == IGW_INET_IPV6)
	  {  	  
         	 if(HttpdNumToPresentation(&pHttpReq->PeerIp,(unsigned char *)SrcIp)!=OF_SUCCESS)
         	 {     
 		      if(DummyReq)
 		      {
 		        if(DummyReq->Auth)
 		        {
 		         of_free(DummyReq->Auth);	  
 		        }
 		        of_free(DummyReq);
 		      } 	 
              HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
              return OF_FAILURE;
             }
	  }	  
	  else
	  #endif  /*INTOTO_IPV6_SUPPORT*/
	  {
	       cm_inet_ntoal(pHttpReq->PeerIp, SrcIp);
	  }        
         sprintf(Message, "Unauthorized user with name %s trying to login from ipaddr: %s", ValAry[0].value_p, SrcIp);
#ifdef MESGCLNT_SUPPORT
      /*IBsendSysLogMsg(NULL, 0, 0, Message, 0, ISEC_UN_AUTH_ACCESS,
        LOGWARNING, HTTPD_SYSMSGID_UNAUTH_USR_LOGIN_3);*/

      of_time(&ulLogInTime);
      igwHTTPDUpdateUserloginTimeInKuki(ValAry[0].value_p,ulLogInTime);
      igwHTTPDLogUserMsg(HTTPD_SYSMSGID_UNAUTH_USR_LOGIN_3,
                         IGW_LOG_FAMILY_USERLOGIN,ValAry[0].value_p,
                         pHttpReq->PeerIp,ulLogInTime,0);

#endif /*MESGCLNT_SUPPORT  */
      if (AuthRetValue == HTTPD_ERR_USER_LOCKEDOUT)
      {
        if (HttpSendHtmlFile(pHttpReq,(unsigned char *) "loginout.htm",
              (unsigned char *)  "User locked out! <BR> <a href=\"login.htm\"> Try again after sometime</a>")
                    == OF_FAILURE)
        { 
          if(DummyReq)
          {
            if(DummyReq->Auth)
            {
              of_free(DummyReq->Auth);	  
            }
            of_free(DummyReq);
          }
          return OF_FAILURE;
        }
      }
      else
      {
        if (HttpSendHtmlFile(pHttpReq,(unsigned char *) "loginout.htm",
              (unsigned char *)  "Invalid Login! <BR> <a href=\"login.htm\"> Login Again</a>")
                    == OF_FAILURE)
        {
			if(DummyReq)
			{
			  if(DummyReq->Auth)
			  {
			    of_free(DummyReq->Auth);		
			  }
			  of_free(DummyReq);
			}
            return OF_FAILURE;
        }
      }
	  if(DummyReq)
	  {
		if(DummyReq->Auth)
		{
		  of_free(DummyReq->Auth);	  
		}
		of_free(DummyReq);
	  }
      return OF_SUCCESS;
    }

    Httpd_Debug("HttpUserAuthin :: One user logged in\n");

    /**
     * Get user info. if user if of remote type set time out value to the
     * 3600 secs. else if user of type config set time out value to the user
     * time out value
     */
  
   #ifdef OF_CM_SUPPORT
    if (GetExactUsrDbRec(uiSrcDirection, /*snet id*/
                         ValAry[0].value_p,/*user name*/
                         &pUserNode_p) != OF_SUCCESS)
    {
        uiTimeOut = Httpd_GetMaxSecs(pHttpg);/*pHttpg->pHttpGlobalConfig->iHttpdMaxSecs;*/
    }
    else
    {
      if (pUserNode_p.userType == UDB_CONFIG_USER)
      {
        uiTimeOut = (uint32_t)pUserNode_p.lInactivityTimer;
      }
      else
      {
        uiTimeOut = Httpd_GetMaxSecs(pHttpg);/*pHttpg->pHttpGlobalConfig->iHttpdMaxSecs;*/
       // uiTimeOut = (uint32_t)pUserNode_p.lInactivityTimer; 
        /*need to take a decition wether to use MAX value or configured value -v.balaji*/
      }
    }

    #endif
    /**
     * Check if the admin user is already logged in or not.
     */
    if (RootOrGuest == TRUE)
    {
    	#ifdef INTOTO_IPV6_SUPPORT
	  if(pHttpReq->PeerIp.ip_type_ul == IGW_INET_IPV6)
	  {	
		if(IGWIPv6NumToPresentation(&pHttpReq->PeerIp.IP.IPv6Addr,(unsigned char *)SrcIp,sizeof(SrcIp))!=OF_SUCCESS)
		{
			HttpDebug(("IGWIPv6NumToPresentation Failed\r\n"));
			return OF_FAILURE;
		}
		IpCheck=(unsigned char *)SrcIp;
		if(of_strstr(IpCheck,HTTP_IPV6_ADDR_PREFIX)!=NULL)
		{
		   IpCheck=strtok(IpCheck,HTTP_IPV6_ADDR_PREFIX);    		  
		}
	  }	  
	  else
	  #endif  /*INTOTO_IPV6_SUPPORT*/
	  {
		  IpCheck=cm_inet_ntoal(pHttpReq->PeerIp, SrcIp);
	  }
      ucCookieValue=pHttpReq->pCookieVal;
      
      if (HttpCookie_IsUserLoggedIn(ValAry[0].value_p,IpCheck,ucCookieValue) == TRUE)
      {
        /**
         * Delete the cookie as another user with the same name has been
         * logged in.
         */
         #if 0
        if (HttpCookie_Del(pHttpReq->pCookieVal) == OF_SUCCESS)
        {
          Httpd_Debug("Cookie Node Deletion is Successful\n");
        }
        else
        {
          Httpd_Error("Unable To Delete Cookie Node\n");
        }
        #endif

        /**
         * Format the message to be logged in syslog module.
         */
        of_memset(Message, 0, sizeof(Message));
        of_memset(ucMsg, 0, sizeof(ucMsg));
	
	ucIpVal=Httpd_GetSrcIpAddress(ValAry[0].value_p);	
	sprintf(ucMsg, "<table width='800'><tr><td align=center><font face='Verdana, Arial, Helvetica, sans-serif' size='2'>Admin user (%s) has already logged in from %s.<br>If you want to close the session from %s , click <b>Ok</b> button. <br>	Click Cancel button to continue previous session.<br><br></font><input type=button value='   Ok   ' onclick=window.location.href='closeprevious.igw*[%s]'>&nbsp; <input type=button value='Cancel' onclick=window.close()><br><br></td></tr></table> ",ValAry[0].value_p,ucIpVal,ucIpVal,ucIpVal);
        
#ifdef MESGCLNT_SUPPORT
        /*sprintf(Message, "Admin user with the name, %s has been already logged in",ValAry[0].value_p);
          IBsendSysLogMsg(NULL, 0, 0, Message, 0, ISEC_UN_AUTH_ACCESS,
           LOGWARNING, HTTPD_SYSMSGID_UNAUTH_USR_LOGIN_3);*/

        of_time(&ulLogInTime);
        igwHTTPDUpdateUserloginTimeInKuki(ValAry[0].value_p,ulLogInTime);
        igwHTTPDLogUserMsg(HTTPD_SYSMSGID_UNAUTH_USR_LOGIN_3,
                           IGW_LOG_FAMILY_USERLOGIN,ValAry[0].value_p,
                           pHttpReq->PeerIp,ulLogInTime,0) ;
#endif /* MESGCLNT_SUPPORT      */

        if (HttpSendHtmlFile(pHttpReq, (unsigned char *)"loginout.htm", ucMsg) == OF_FAILURE)
        {
          return OF_FAILURE;
        }
        iValidity=1;
      }
    }

    /**
     * Timeout from User Database is in seconds.
     */
    if (HttpCookie_LoggedIn(pHttpReq->pCookieVal,
                          ValAry[0].value_p, uiTimeOut) == OF_FAILURE)
    {
      Httpd_Error("HttpUserAuthin: Unable To Reset Cookie Timer\n");
    }
    else
    {
      of_strncpy(pHttpReq->pUserName, ValAry[0].value_p, HTTP_UNAME_LEN);
      pHttpReq->pUserName[HTTP_UNAME_LEN] = '\0';
#ifdef HTTPD_DEBUG
      Httpd_Print("HttpUserAuthin :: Reset the User Timeout to %ld\n", Httpd_GetMaxSecs(pHttpg));
#endif
    }

    /**
     * Add the port number to the cookie.
     */
    if (HttpCookie_AddPortInfo(pHttpReq->pCookieVal,
                               pHttpReq->SrcPort) == OF_FAILURE)
    {
      Httpd_Error("HttpCookie_AddPortInfo: Unable To Add Port Number\n");
    }

    if (RootOrGuest == TRUE)
    {
    	if(iValidity!=1)
    		{
      HttpRequest_SendWelcomePage(pHttpg,pHttpReq);
    }
    }
    else
    {
      HttpSendHtmlFile(pHttpReq,(unsigned char *) "rlogout.htm",(unsigned char *) "Login Successful");
    }

    /**
     * Generating log message indicating root/guest logged
     * in from <IP address> at <time>
     */
    if (RootOrGuest == TRUE)
    {
      of_memset(Message, 0, sizeof(Message));
	#ifdef INTOTO_IPV6_SUPPORT
	if(pHttpReq->PeerIp.ip_type_ul == IGW_INET_IPV6)
	{	
           if(HttpdNumToPresentation(&pHttpReq->PeerIp,(unsigned char *)SrcIp)!=OF_SUCCESS)
           {
                 HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
                 return OF_FAILURE;                 
           }
	}	  
	else
	#endif  /*INTOTO_IPV6_SUPPORT*/
	{
		cm_inet_ntoal(pHttpReq->PeerIp, SrcIp);
	}             
      sprintf(Message, "Web- config user \"%s\" logged in from ipaddr %s",ValAry[0].value_p, SrcIp);
#ifdef MESGCLNT_SUPPORT
      /*IBsendSysLogMsg (NULL, 0, 0, Message, 0, ISEC_AUTH_LOGIN,
        LOGINFO, HTTPD_SYSMSGID_ADMIN_USR_LOGIN_SUCCESS_2);*/

      of_time(&ulLogInTime);
      igwHTTPDUpdateUserloginTimeInKuki(ValAry[0].value_p,ulLogInTime);
      igwHTTPDLogUserMsg(HTTPD_SYSMSGID_ADMIN_USR_LOGIN_SUCCESS_2,
                         IGW_LOG_FAMILY_USERLOGIN,ValAry[0].value_p,
                         pHttpReq->PeerIp,ulLogInTime,0);
#endif /* MESGCLNT_SUPPORT      */
    }
    else  /* Generate Log msg for Non-admin user login*/ 
    {
       of_time(&ulLogInTime);
       igwHTTPDUpdateUserloginTimeInKuki(ValAry[0].value_p,ulLogInTime);
       igwHTTPDLogUserMsg(HTTPD_SYSMSGID_REMOTE_USR_LOGIN_SUCCESS,
             IGW_LOG_FAMILY_USERLOGIN,ValAry[0].value_p,
             pHttpReq->PeerIp,ulLogInTime,0);
    }
  }
  else
  {
    if (HttpCookie_Del(pHttpReq->pCookieVal) == OF_SUCCESS)
    {
      Httpd_Debug("Cookie Node Deletion is Successful\n");
    }
    else
    {
      Httpd_Error("Unable To Delete Cookie Node\n");
    }

    if (HttpSendHtmlFile(pHttpReq,(unsigned char *) "loginout.htm",
         (unsigned char *)   "Error In Login<BR><a href=\"login.htm\">Login Again</a>")
                 == OF_FAILURE)
    {
       if(ucPasswordMD5)   
       {
           of_free(ucPasswordMD5);
       	   ucPasswordMD5=NULL;
       }
       return OF_FAILURE;
    }
  }
  if(ucPasswordMD5)   
  {
       of_free(ucPasswordMD5);
       ucPasswordMD5=NULL;
  }
  return OF_SUCCESS;
} /* HttpdSup_Action_Login() */
#endif /*OF_CM_SUPPORT*/


/************************************************************************
 * Function Name : httpdSendErrMessage                                  *
 * Description   : This function is used for login.                     *
 * Input         : HTTP request structure.                              *
 *                 Charecter buffer containing the user data line.      *
 *                 Length of this buffer.                               *
 * Output        : Html file to send to the browser with or without     *
 *                 information based on the operation performed.        *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.        *
 ************************************************************************/

int32_t httpdSendErrMessage( HttpRequest *pHttpReq, char* msg)
{
  char errRespStr[HTTP_ERR_RESP_LEN + 1];
  char sXSLFileHdr[HTTP_XSL_HDR_FIL_LEN + 1];

  of_memset(sXSLFileHdr,'\0',sizeof(sXSLFileHdr));

  sprintf(sXSLFileHdr, "<?xml-stylesheet type=\"text/xsl\" href=\"%s\"?>",
            HTTP_LOGINOUT_XSL);

  of_memset(errRespStr,'\0',sizeof(errRespStr));

  of_strcat(errRespStr, HTTP_XML_RESP_HEADER);
  of_strcat(errRespStr, sXSLFileHdr);
  of_strcat(errRespStr, XMLHTTP_START_TAG);
  of_strcat(errRespStr, XMLHTTP_MSG_START_TAG);
  of_strcat(errRespStr, msg);
  of_strcat(errRespStr, XMLHTTP_MSG_END_TAG);
  of_strcat(errRespStr, XMLHTTP_REDIRECT_REF);
  of_strcat(errRespStr, XMLHTTP_END_TAG);

  Httpd_Send_PlainString(pHttpReq, errRespStr);

  return OF_SUCCESS;
}

/************************************************************************
 * Function Name : httpdCreateUCMSessions                                *
 * Description   : This function is used to create UCM sessions.        *
 * Input         : pUserName - User Name                                *
 *                 pRoleInfo - pointer to struct cm_role_info structure.      *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.        *
 ************************************************************************/
 int32_t httpdCreateUCMSessions(unsigned char* pUserName, unsigned char* pPassword,
                                         struct cm_role_info* pRoleInfo)
{
  /*unsigned char aIpAddr[CM_MAX_ADMINNAME_LEN + 1];*/
  ipaddr uiHostAddr = 0;
  int32_t iRetVal = 0;

  //of_memset(aIpAddr,'\0',sizeof(aIpAddr));
  //of_strcpy(aIpAddr, UCM_LOCALHOST_NAME);

  if(cm_get_host_by_name(UCM_LOCALHOST_NAME, &uiHostAddr) == OF_FAILURE )
  {
    UCMHTTPDEBUG("  %s :: Invalid IP Address \r\n",__FUNCTION__ );
    return OF_FAILURE;
  }
  UCMHTTPDEBUG("  %s :: IP Address by name=%u\r\n",__FUNCTION__, uiHostAddr );
   
  iRetVal = HttpdUCMCreateLoginSession(pUserName, pPassword, 
                                       pRoleInfo, uiHostAddr); 
  if(iRetVal != OF_SUCCESS)
  {
    UCMHTTPDEBUG("  %s :: Failed to create session \r\n",__FUNCTION__ );
    return iRetVal;
  }
#ifdef UCM_HTTP_SPUTM_MP_SUPPORT
  if(cm_get_host_by_name(UCM_MASTERHOST_NAME, &uiHostAddr) == OF_FAILURE )
  {
    UCMHTTPDEBUG("  %s :: Invalid IP Address \r\n",__FUNCTION__ );
    return OF_FAILURE;
  }
  iRetVal =HttpdUCMCreateLoginSession(pUserName, pPassword,
                                      pRoleInfo, uiHostAddr);
  if(iRetVal != OF_SUCCESS)
  {
    UCMHTTPDEBUG("  %s :: Failed to create session \r\n",__FUNCTION__ );
  }
#endif /*UCM_HTTP_SPUTM_MP_SUPPORT*/
  return OF_SUCCESS;
}

/************************************************************************
 * Function Name : httpdDeleteUCMSessions                               *
 * Description   : This function is used to create UCM sessions.        *
 * Input         : pUserName - User Name                                *
 *                 pRoleInfo - pointer to struct cm_role_info structure.      *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.        *
 ************************************************************************/
 int32_t httpdDeleteUCMSessions(char* pUserName)
{
   unsigned char aIpAddr[CM_MAX_ADMINNAME_LEN + 1];
  ipaddr uiHostAddr = 0;

  of_memset(aIpAddr,'\0',sizeof(aIpAddr));
  
  of_strcpy((char *) aIpAddr, UCM_LOCALHOST_NAME);

  if(cm_get_host_by_name((char *) aIpAddr, &uiHostAddr) == OF_FAILURE )
  {
    UCMHTTPDEBUG("  %s :: Invalid IP Address \r\n",__FUNCTION__ );
    return OF_FAILURE;
  }
  UCMHTTPDEBUG("  %s :: IP Address by name=%u\r\n",__FUNCTION__, uiHostAddr );
   
  if(ucmHTTPDeleteSessionEntryFromList(pUserName) != OF_SUCCESS)
  {
    UCMHTTPDEBUG("  %s :: Failed to delete session \r\n",__FUNCTION__ );
    return OF_FAILURE;
  }

  return OF_SUCCESS;
}

/************************************************************************
 * Function Name : HttpdUCM_Action_Login                                *
 * Description   : This function is used for login.                     *
 * Input         : HTTP request structure.                              *
 *                 Charecter buffer containing the user data line.      *
 *                 Length of this buffer.                               *
 * Output        : Html file to send to the browser with or without     *
 *                 information based on the operation performed.        *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.        *
 ************************************************************************/

int32_t HttpdUCM_Action_Login( HttpRequest    *pHttpReq,
                               Httpd_Value_t  *ValAry,
                               uint32_t       nCount )
{
  

  bool       isAuth                 = FALSE;
  struct cm_role_info* pRoleInfo = NULL;
  int32_t       AuthRetValue           = HTTPD_ERR_UDB;
  int32_t       iRetValue              = OF_SUCCESS;
  bool       RootOrGuest            = FALSE;
  char       Challenge[HTTP_CHALLENGE_LEN + 1];
#ifdef MESGCLNT_SUPPORT
  char       Message[MAX_LOG_MESSAGE_SIZE];
#endif /*MESGCLNT_SUPPORT*/
  char       sSNetName[100];
  char       SrcIp[CM_INET6_ADDRSTRLEN];
 /* UserDbTemp_t  pUserNode_p; */
#ifdef INTOTO_CMS_SUPPORT
  uint32_t      uiTimeOut = 360;
#else
  uint32_t      uiTimeOut;
#endif  /* INTOTO_CMS_SUPPORT */
  unsigned char     *IpCheck;
  unsigned char     *ucCookieValue=NULL;
  unsigned char      ucRsaPasswordInVal[HTTP_RSA_VAL1_LEN];
  char	    *ucPasswordMD5=NULL,*cValOut=NULL;
  unsigned char      ucRsaPasswordOutval[HTTP_RSA_VAL1_LEN];
  uint32_t      ucPlen=0;
  RSA 		    *Rsakey;
  unsigned char      ucPtext[HTTP_RSA_VAL1_LEN],*ucStringVal=NULL;
  unsigned char      ucText[HTTP_RSA_VAL1_LEN];  
  unsigned char	    ucCtext[HTTP_RSA_VAL1_LEN],*ucS=NULL,*ucT=NULL,*ucTs=NULL;
  unsigned char      ucCT[HTTP_RSA_VAL1_LEN];
  int32_t	    iTlen=0;
  int32_t	    iNum=0,iVal=0,i=0;

  int32_t       iValidity=0;
  char       ucMsg[HTTP_MAX_MESSAGE_LEN];

  HttpRequest	 *DummyReq=NULL;
  HttpGlobals_t   *pHttpg=NULL;
  uint32_t ulLogInTime = 0;

#ifdef INTOTO_UCM_AUTH_PLUGIN_SUPPORT
  UCMAuthMgmt_t Auth;
#endif /*INTOTO_UCM_AUTH_PLUGIN_SUPPORT*/

  pHttpg=(HttpGlobals_t *)pHttpReq->pGlobalData;
  if(!pHttpg)
  {
    Httpd_Error("pHttpg is NULL\r\n");
    return OF_FAILURE;
  }

  
  /*RSA Decryption:password coming in chunks 32 characters per each chunk
   * max received chunks is 3 (32).
   * */
  of_memset(ucRsaPasswordOutval,'\0',HTTP_RSA_VAL1_LEN);
  of_memset(ucCT,'\0',HTTP_RSA_VAL1_LEN);   
  of_memset(ucPtext,'\0',sizeof(ucPtext));
  of_memset(ucText,'\0',sizeof(ucText));
  
  iTlen=of_strlen(ValAry[1].value_p);

  of_strcpy((char *) ucRsaPasswordInVal,ValAry[1].value_p);
  HttpHex2Str(ucRsaPasswordInVal,ucRsaPasswordOutval,&ucPlen);
  Rsakey = RSA_new();
  if(Rsakey == NULL)
  {
    Httpd_Error("Memory allocation failed\r\n");
    return OF_FAILURE;
  }

  passwordkey(Rsakey, ucCtext);
  iNum=RSA_private_decrypt((int32_t)ucPlen,ucRsaPasswordOutval,ucPtext,Rsakey,RSA_NO_PADDING);
  while(iVal < iNum )
  {
	    ucText[iVal] = ucPtext[(iNum-1)-iVal];
	    iVal++;
  }
  of_strcpy((char *) ucCT,(char *) ucText);
  
  /*If we are receving more than 1 chunk (iTlen >32) do the below extra  processing */
  
  if(iTlen >32) 
  {
   	ucTs=(unsigned char *) strtok((char *) ValAry[1].value_p," ");      
	while(ucTs && ucTs!='\0')
   	{
	   ucTs=(unsigned char *) strtok(NULL," ");
	   if(i==0) ucS=ucTs;
	   if(i==1) ucT=ucTs;
	   i++; 	   
	}
        of_memset(ucPtext,'\0',sizeof(ucPtext));
        of_memset(ucText,'\0',sizeof(ucText));
        of_memset(ucRsaPasswordOutval,'\0',HTTP_RSA_VAL1_LEN);
        iVal=0;
        HttpHex2Str(ucS,ucRsaPasswordOutval,&ucPlen);
        iNum=RSA_private_decrypt((int32_t)ucPlen,ucRsaPasswordOutval,ucPtext,Rsakey,RSA_NO_PADDING);
        while(iVal < iNum )
        {
       		 ucText[iVal] = ucPtext[(iNum-1)-iVal];
	        iVal++;
        } 
   	of_strcat((char *) ucCT,(char *) ucText);
        if(iTlen >65) 
        {
          iVal=0;
          of_memset(ucPtext,'\0',sizeof(ucPtext));
          of_memset(ucText,'\0',sizeof(ucText));
          of_memset(ucRsaPasswordOutval,'\0',HTTP_RSA_VAL1_LEN);
   	  HttpHex2Str(ucT,ucRsaPasswordOutval,&ucPlen);
	  iNum=RSA_private_decrypt((int32_t)ucPlen,ucRsaPasswordOutval,ucPtext,Rsakey,RSA_NO_PADDING);
   	  while(iVal < iNum )
   	  {
	     ucText[iVal] = ucPtext[(iNum-1)-iVal];
	     iVal++;
   	  }
   	  of_strcat((char *) ucCT,(char *) ucText);
        }
  }

  if((ucStringVal=of_calloc(1,HTTP_RSA_VAL1_LEN))==NULL)
  {
    Httpd_Error("Memory allocation failed\r\n");
    return OF_FAILURE;
  }

  if((cValOut=of_calloc(1,HTTP_RSA_VAL1_LEN))== NULL)
  {
    of_free(ucStringVal);
    Httpd_Error("Memory allocation failed\r\n");
    return OF_FAILURE;
  }

  of_strcpy((char *) ucStringVal,(char *) ValAry[0].value_p);
  of_strcat((char *) ucStringVal,":");
  Httpd_MD5Encrypt(ucCT,(unsigned char *)cValOut);
  of_strcat((char *)ucStringVal,(char *)cValOut);
  of_strcat((char *)ucStringVal,":");
  of_strcat((char *)ucStringVal,(char *)ValAry[2].value_p);   
  if((ucPasswordMD5=of_calloc(1,HTTP_RSA_VAL1_LEN))==NULL)
  {
    of_free(ucStringVal);
    of_free(cValOut);
    Httpd_Error("Memory allocation failed\r\n");
    return OF_FAILURE;
  }  
  Httpd_MD5Encrypt(ucStringVal,(unsigned char *)ucPasswordMD5);
#ifdef HTTPD_DEBUG
  UCMHTTPDEBUG("ucCT(clear text password)=%s\r\n",ucCT); 
  UCMHTTPDEBUG("\n ****UCMAUTH::USER NAME ValAry[0].value_p  =%s\r\n", 
                                                     ValAry[0].value_p);
  UCMHTTPDEBUG("\n ****UCMAUTH::USER NAME ucStringVal  =%s\r\n", ucStringVal);
#endif
#ifdef UCM_AUTH_SUPPORT
#ifdef INTOTO_UCM_AUTH_PLUGIN_SUPPORT
  UCMHTTPDEBUG("\n ****UCMAUTH:: Using Auth Plugin Support...\r\n");
  of_memset(&Auth,0,sizeof(UCMAuthMgmt_t));
  Auth.pUsrName = (char*)ValAry[0].value_p;
  Auth.pPwd = (char *)ucCT;
  Auth.pNewPwd = NULL;
  Auth.pRoleInfo = pRoleInfo;
  if(UCMUserMgmt(UCM_AUTH_USR_AUTH_CMD, &Auth) != OF_SUCCESS)
  {
    UCMHTTPDEBUG("\n****UCMAUTH:: Authenticaion failed using Auth Plugin.\r\n");
    if(ucStringVal)
      of_free(ucStringVal);
    if(cValOut)
      of_free(cValOut);

    httpdSendErrMessage(pHttpReq, HTTP_AUTH_ERR_MSG);
    return OF_SUCCESS;
  }
  else
#else
  if((UCMPAMAuthenticateUser((char*)ValAry[0].value_p, 
                               (char *)ucCT, &pRoleInfo)) != OF_SUCCESS)
  {
    if(ucStringVal)
      of_free(ucStringVal);
    if(cValOut)
      of_free(cValOut);

    httpdSendErrMessage(pHttpReq, HTTP_AUTH_ERR_MSG);
    return OF_SUCCESS;
  }
  else
#endif /*INTOTO_UCM_AUTH_PLUGIN_SUPPORT*/
#endif /*UCM_AUTH_SUPPORT*/
  {
    AuthRetValue = OF_SUCCESS;
  }

#ifdef INTOTO_UCM_AUTH_PLUGIN_SUPPORT
  pRoleInfo = Auth.pRoleInfo;
#endif /*INTOTO_UCM_AUTH_PLUGIN_SUPPORT*/

  iRetValue = httpdCreateUCMSessions((unsigned char*)ValAry[0].value_p, 
                                      ucCT, pRoleInfo);
  if(iRetValue != OF_SUCCESS)
  {
    if(iRetValue != UCMHTTP_SESSION_EXISTS)
    {
    if(ucStringVal)
      of_free(ucStringVal);
    if(cValOut)
      of_free(cValOut);	  
    if(pRoleInfo)
      of_free(pRoleInfo);
    if (iRetValue == UCMHTTP_USER_AUTH_FAILED)
	{
    httpdSendErrMessage(pHttpReq, HTTP_AUTH_ERR_MSG);
    return OF_SUCCESS;
	}
      httpdSendErrMessage(pHttpReq, HTTP_SESSION_ERR_MSG);
      return OF_SUCCESS;
    }
  }
  /*free the allocated memory  */

  if(ucStringVal)
    of_free(ucStringVal);
  if(cValOut)
    of_free(cValOut);	  
 
  /**
   * Introduced System Call Function.
   */
  if (Httpd_HandleSystemCookies(pHttpReq,
                                HTTP_SYSTEM_LOGIN_COOKIE) != OF_SUCCESS)
  {
    Httpd_Debug("\r\nHttpd_HandleSystemCookies Failed\r\n");
  }

  if (Httpd_SendCookieInfo(pHttpReq) != OF_SUCCESS)
  {
    Httpd_Debug("\r\n Sending SNET cookie failed in login.igw");
  }

  if (HttpCookie_GetByVal(pHttpReq->pCookieVal, &isAuth, (unsigned char *) Challenge,
                          NULL) == OF_SUCCESS)
  {
    /**
     * Get whether given user is with admin privelge or not.
     */
    RootOrGuest = HttpUserIsConfigType(ValAry[0].value_p);

#ifndef OF_CM_SUPPORT
/* Need to enable this for HTTPs and verfiy*/
#if defined (HTTPS_ENABLE)
    if (RootOrGuest == FALSE)
    {
      if (!Httpd_CheckUserLogin(pHttpg,pHttpReq, ValAry[0].value_p))
      {
        of_free(ucPasswordMD5);
        return OF_SUCCESS;
      }
    }
#endif /* HTTPS_ENABLE */
#endif /*OF_CM_SUPPORT*/
 
    of_free(ucPasswordMD5);
    ucPasswordMD5=NULL;

    if (RootOrGuest == TRUE)
    {
      if ((AuthRetValue == HTTPD_ERR_NO_USER) ||
          (AuthRetValue == HTTPD_ERR_USER_LOCKEDOUT))
      {
#ifdef HTTPD_DEBUG
        Httpd_Error("Root/Guest Authentication Failed, Message Logged\n");
#endif
#ifdef INTOTO_IPV6_SUPPORT
	  if(pHttpReq->PeerIp.ip_type_ul == IGW_INET_IPV6)
	  {
		if(HttpdNumToPresentation(&pHttpReq->PeerIp,(unsigned char *)SrcIp)!=OF_SUCCESS)
		{	
                 HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
                 return OF_FAILURE;
		}
	  }	  
	  else
#endif  /*INTOTO_IPV6_SUPPORT*/
	  {
	       cm_inet_ntoal(pHttpReq->PeerIp, SrcIp);
	  }
     
        of_time((time_t *)&ulLogInTime);
        igwHTTPDUpdateUserloginTimeInKuki(ValAry[0].value_p,ulLogInTime);

#ifdef MESGCLNT_SUPPORT
        of_memset(Message, 0, sizeof(Message));
        if(AuthRetValue == HTTPD_ERR_USER_LOCKEDOUT)
        {
            sprintf(Message, "Web - config user \"%s\" login failed from ipaddr: %s"
        	          "reason: User locked out",ValAry[0].value_p, SrcIp);
        }
        else
        {
            sprintf(Message, "Web - config user \"%s\" login failed from ipaddr: %s"
        	          "reason: Password do not match",ValAry[0].value_p, SrcIp);
        }
        /*IBsendSysLogMsg(NULL, 0, 0, Message, 0, ISEC_UN_AUTH_ACCESS,
          LOGWARNING, HTTPD_SYSMSGID_ADMIN_USR_LOGIN_FAILED_1);*/

        igwHTTPDLogUserMsg(HTTPD_SYSMSGID_ADMIN_USR_LOGIN_FAILED_1,
                           IGW_LOG_FAMILY_USERLOGIN,ValAry[0].value_p,
                           pHttpReq->PeerIp,ulLogInTime, 0) ;
#endif /*MESGCLNT_SUPPORT          */

        if (HttpCookie_Del(pHttpReq->pCookieVal) == OF_SUCCESS)
        {
          Httpd_Debug("Cookie Node Deletion is Successful\n");
        }
        else
        {
          Httpd_Error("Unable To Delete Cookie Node\n");
        }

        /**
         * Previously we were just displaying message.
         * We need to have link for the user to login again.
         */
        HttpRequest_SetPageType(pHttpReq, LOGINPAGEREQUEST);

        if (AuthRetValue == HTTPD_ERR_USER_LOCKEDOUT)
        {
          if (HttpSendHtmlFile(pHttpReq,(unsigned char *) "loginout.htm",
           (unsigned char *) "User locked out! <BR> <a href=\"login.htm\"> Try again after sometime</a>")
                      == OF_FAILURE)
          {
            return OF_FAILURE;
          }
        }
        else
        {
          if (HttpSendHtmlFile(pHttpReq,(unsigned char *)  "loginout.htm",
                 (unsigned char *)  "Invalid Login! <BR> <a href=\"login.htm\"> Login Again</a>")
                      == OF_FAILURE)
          {
            return OF_FAILURE;
        }
        }
        return OF_SUCCESS;
      }
      else
      {
        AuthRetValue = OF_SUCCESS;
      }
    }

    if (AuthRetValue == HTTPD_ERR_NO_USER)
    { 	    
      pHttpReq->Auth=of_calloc(1,sizeof(AuthPriority_t));
      of_strcpy((char *) pHttpReq->Auth->ucPasswd,(char *) ucCT);
      of_strcpy((char *) pHttpReq->Auth->ucChallenge,(char *) Challenge);
      of_strcpy((char *) pHttpReq->Auth->ucSnetName,(char *) sSNetName);
      of_strcpy((char *) pHttpReq->pUserName,(char *) ValAry[0].value_p);
      
      DummyReq=of_calloc(1,sizeof(HttpRequest));
      if(DummyReq == NULL)
      {
           HttpDebug(("of_calloc failed for DummyReq \r\n"));
           return OF_FAILURE;
      }
      DummyReq->Auth=of_calloc(1,sizeof(AuthPriority_t));
      if(DummyReq->Auth  == NULL)
      {
           HttpDebug(("of_calloc  failed for DummyReq->Auth \r\n"));
	    of_free(DummyReq);	   
           return OF_FAILURE;
      }       	  
      of_memcpy(DummyReq,pHttpReq,sizeof(HttpRequest));
      of_memcpy(DummyReq->Auth,pHttpReq->Auth,sizeof(AuthPriority_t));	  
    }

    if (AuthRetValue != 0)
    {
#ifdef MESGCLNT_SUPPORT
         of_memset(Message, 0, sizeof(Message));
#endif /*MESGCLNT_SUPPORT*/

#ifdef INTOTO_IPV6_SUPPORT
	  if(pHttpReq->PeerIp.ip_type_ul == IGW_INET_IPV6)
	  {  	  
         	 if(HttpdNumToPresentation(&pHttpReq->PeerIp,(unsigned char *)SrcIp)!=OF_SUCCESS)
         	 {     
 		      if(DummyReq)
 		      {
 		        if(DummyReq->Auth)
 		        {
 		         of_free(DummyReq->Auth);	  
 		        }
 		        of_free(DummyReq);
 		      } 	 
              HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
              return OF_FAILURE;
             }
	  }	  
	  else
#endif  /*INTOTO_IPV6_SUPPORT*/
	  {
	       cm_inet_ntoal(pHttpReq->PeerIp, SrcIp);
	  }        
      of_time((time_t *)&ulLogInTime);
      igwHTTPDUpdateUserloginTimeInKuki(ValAry[0].value_p,ulLogInTime);
#ifdef MESGCLNT_SUPPORT
      sprintf(Message, 
        "Unauthorized user with name %s trying to login from ipaddr: %s", 
        ValAry[0].value_p, SrcIp);
      igwHTTPDLogUserMsg(HTTPD_SYSMSGID_UNAUTH_USR_LOGIN_3,
                         IGW_LOG_FAMILY_USERLOGIN,ValAry[0].value_p,
                         pHttpReq->PeerIp,ulLogInTime,0);
#endif /*MESGCLNT_SUPPORT  */
      if (AuthRetValue == HTTPD_ERR_USER_LOCKEDOUT)
      {
        if(HttpSendHtmlFile(pHttpReq, (unsigned char *) "loginout.htm",
        (unsigned char *)   "User locked out! <BR> <a href=\"login.htm\"> Try again after sometime</a>")
                    == OF_FAILURE)
        { 
          if(DummyReq)
          {
            if(DummyReq->Auth)
            {
              of_free(DummyReq->Auth);	  
            }
            of_free(DummyReq);
          }
          return OF_FAILURE;
        }
      }
      else
      {
        if (HttpSendHtmlFile(pHttpReq, (unsigned char *) "loginout.htm",
                (unsigned char *) "Invalid Login! <BR> <a href=\"login.htm\"> Login Again</a>")
                    == OF_FAILURE)
        {
			if(DummyReq)
			{
			  if(DummyReq->Auth)
			  {
			    of_free(DummyReq->Auth);		
			  }
			  of_free(DummyReq);
			}
            return OF_FAILURE;
        }
      }
	  if(DummyReq)
	  {
		if(DummyReq->Auth)
		{
		  of_free(DummyReq->Auth);	  
		}
		of_free(DummyReq);
	  }
      return OF_SUCCESS;
    }

    Httpd_Debug("HttpUserAuthin :: One user logged in\n");

    /**
     * Get user info. if user if of remote type set time out value to the
     * 3600 secs. else if user of type config set time out value to the user
     * time out value
     */
#ifndef OF_CM_SUPPORT
    if (GetExactUsrDbRec(uiSrcDirection, /*snet id*/
                         ValAry[0].value_p,/*user name*/
                         &pUserNode_p) != OF_SUCCESS)
    {
        uiTimeOut = Httpd_GetMaxSecs(pHttpg);/*pHttpg->pHttpGlobalConfig->iHttpdMaxSecs;*/
    }
    else
    {
      if (pUserNode_p.userType == UDB_CONFIG_USER)
      {
        uiTimeOut = (uint32_t)pUserNode_p.lInactivityTimer;
      }
      else
      {
        uiTimeOut = Httpd_GetMaxSecs(pHttpg);/*pHttpg->pHttpGlobalConfig->iHttpdMaxSecs;*/
       // uiTimeOut = (uint32_t)pUserNode_p.lInactivityTimer; 
        /*need to take a decition wether to use MAX value or configured value -v.balaji*/
      }
    }
#endif /* OF_CM_SUPPORT */
     uiTimeOut = Httpd_GetMaxSecs(pHttpg);/*pHttpg->pHttpGlobalConfig->iHttpdMaxSecs;*/
    /**
     * Check if the admin user is already logged in or not.
     */
    if (RootOrGuest == TRUE)
    {
#ifndef OF_CM_SUPPORT
    	#ifdef INTOTO_IPV6_SUPPORT
	  if(pHttpReq->PeerIp.ip_type_ul == IGW_INET_IPV6)
	  {	
		if(IGWIPv6NumToPresentation(&pHttpReq->PeerIp.IP.IPv6Addr,(unsigned char *)SrcIp,sizeof(SrcIp))!=OF_SUCCESS)
		{
			HttpDebug(("IGWIPv6NumToPresentation Failed\r\n"));
			return OF_FAILURE;
		}
		IpCheck=(unsigned char *)SrcIp;
		if(of_strstr(IpCheck,HTTP_IPV6_ADDR_PREFIX)!=NULL)
		{
		   IpCheck=strtok(IpCheck,HTTP_IPV6_ADDR_PREFIX);    		  
		}
	  }	  
	  else
	  #endif  /*INTOTO_IPV6_SUPPORT*/
#endif /* OF_CM_SUPPORT */
	  {
		  IpCheck=(unsigned char *) cm_inet_ntoal(pHttpReq->PeerIp, SrcIp);
	  }
      ucCookieValue=pHttpReq->pCookieVal;
      
      if (HttpCookie_IsUserLoggedIn((unsigned char *) ValAry[0].value_p,IpCheck,ucCookieValue) == TRUE)
      {
        /**
         * Delete the cookie as another user with the same name has been
         * logged in.
         */

        /**
         * Format the message to be logged in syslog module.
         */
        of_memset(ucMsg, 0, sizeof(ucMsg));
	
	ucIpVal=Httpd_GetSrcIpAddress((unsigned char *) ValAry[0].value_p);

#ifdef PROMPT_REQUIRED
	sprintf(ucMsg, "<table width='800'><tr><td align=center><font face='Verdana, Arial, Helvetica, sans-serif' size='2'>Admin user (%s) has already logged in from %s.<br>If you want to close the session from %s , click <b>Ok</b> button. <br>	Click Cancel button to continue previous session.<br><br></font><input type=button value='   Ok   ' onclick=window.location.href='closeprevious.igw*[%s]'>&nbsp; <input type=button value='Cancel' onclick=window.close()><br><br></td></tr></table> ",ValAry[0].value_p,ucIpVal,ucIpVal,ucIpVal);
#else
    sprintf(ucMsg, "<table width='1000'><tr><td align='center'><font face='Verdana, Arial, Helvetica, sans-serif' size='2'><b>Admin user's (%s) current session at %s is going to expire...</b><br/></td></tr></table><script>function logout(){window.location.href='closeprevious.igw*[%s]';}setTimeout('logout()',5000);setMultiLoginFlag(true);</script>",ValAry[0].value_p,ucIpVal,ucIpVal);
#endif
   
#ifdef MESGCLNT_SUPPORT
        of_memset(Message, 0, sizeof(Message));
        /*sprintf(Message, "Admin user with the name, %s has been already logged in",ValAry[0].value_p);
          IBsendSysLogMsg(NULL, 0, 0, Message, 0, ISEC_UN_AUTH_ACCESS,
           LOGWARNING, HTTPD_SYSMSGID_UNAUTH_USR_LOGIN_3);*/

        of_time(&ulLogInTime);
        igwHTTPDUpdateUserloginTimeInKuki(ValAry[0].value_p,ulLogInTime);
#ifndef OF_CM_SUPPORT
       igwHTTPDLogUserMsg(HTTPD_SYSMSGID_UNAUTH_USR_LOGIN_3,
                           IGW_LOG_FAMILY_USERLOGIN,ValAry[0].value_p,
                           pHttpReq->PeerIp,ulLogInTime,0);
#endif /* OF_CM_SUPPORT */
#endif /* MESGCLNT_SUPPORT      */

        if(HttpSendHtmlFile(pHttpReq,(unsigned char*)"loginout.htm",(unsigned char*)ucMsg) == OF_FAILURE)
        {
          return OF_FAILURE;
        }
        iValidity=1;
      }
    }

    /**
     * Timeout from User Database is in seconds.
     */
    if (HttpCookie_LoggedIn(pHttpReq->pCookieVal,
                          (unsigned char *)ValAry[0].value_p, uiTimeOut) == OF_FAILURE)
    {
      Httpd_Error("HttpUserAuthin: Unable To Reset Cookie Timer\n");
    }
    else
    {
      of_strncpy((char *)pHttpReq->pUserName, ValAry[0].value_p, HTTP_UNAME_LEN);
      pHttpReq->pUserName[HTTP_UNAME_LEN] = '\0';
#ifdef HTTPD_DEBUG
      Httpd_Print("HttpUserAuthin :: Reset the User Timeout to %ld\n", Httpd_GetMaxSecs(pHttpg));
#endif
    }

    /**
     * Add the port number to the cookie.
     */
    if (HttpCookie_AddPortInfo(pHttpReq->pCookieVal,
                               pHttpReq->SrcPort) == OF_FAILURE)
    {
      Httpd_Error("HttpCookie_AddPortInfo: Unable To Add Port Number\n");
    }

    if (RootOrGuest == TRUE)
    {
      if(iValidity!=1)
      {
        HttpRequest_SendWelcomePage(pHttpg,pHttpReq);
      }
    }
    else
    {
      HttpSendHtmlFile(pHttpReq, (unsigned char *) "rlogout.htm", (unsigned char *) "Login Successful");
    }
#ifndef OF_CM_SUPPORT

    /**
     * Generating log message indicating root/guest logged
     * in from <IP address> at <time>
     */
    if (RootOrGuest == TRUE)
    {
      of_memset(Message, 0, sizeof(Message));
	#ifdef INTOTO_IPV6_SUPPORT
	if(pHttpReq->PeerIp.ip_type_ul == IGW_INET_IPV6)
	{	
           if(HttpdNumToPresentation(&pHttpReq->PeerIp,(unsigned char *)SrcIp)!=OF_SUCCESS)
           {
                 HttpDebug((" HttpdNumToPresentation is Failed\r\n"));
                 return OF_FAILURE;                 
           }
	}	  
	else
	#endif  /*INTOTO_IPV6_SUPPORT*/
	{
		cm_inet_ntoal(pHttpReq->PeerIp, SrcIp);
	}             
      sprintf(Message, "Web- config user \"%s\" logged in from ipaddr %s",ValAry[0].value_p, SrcIp);
#ifdef MESGCLNT_SUPPORT
      /*IBsendSysLogMsg (NULL, 0, 0, Message, 0, ISEC_AUTH_LOGIN,
        LOGINFO, HTTPD_SYSMSGID_ADMIN_USR_LOGIN_SUCCESS_2);*/

      of_time(&ulLogInTime);
      igwHTTPDUpdateUserloginTimeInKuki(ValAry[0].value_p,ulLogInTime);
      igwHTTPDLogUserMsg(HTTPD_SYSMSGID_ADMIN_USR_LOGIN_SUCCESS_2,
                         IGW_LOG_FAMILY_USERLOGIN,ValAry[0].value_p,
                         pHttpReq->PeerIp,ulLogInTime,0);
#endif /* MESGCLNT_SUPPORT      */
    }
    else  /* Generate Log msg for Non-admin user login*/ 
    {
       of_time(&ulLogInTime);
       igwHTTPDUpdateUserloginTimeInKuki(ValAry[0].value_p,ulLogInTime);
       igwHTTPDLogUserMsg(HTTPD_SYSMSGID_REMOTE_USR_LOGIN_SUCCESS,
             IGW_LOG_FAMILY_USERLOGIN,ValAry[0].value_p,
             pHttpReq->PeerIp,ulLogInTime,0);
    } 
#endif /* OF_CM_SUPPORT */
  }
  else
  {
    if (HttpCookie_Del(pHttpReq->pCookieVal) == OF_SUCCESS)
    {
      Httpd_Debug("Cookie Node Deletion is Successful\n");
    }
    else
    {
      Httpd_Error("Unable To Delete Cookie Node\n");
    }

    if (HttpSendHtmlFile(pHttpReq, (unsigned char *) "loginout.htm",
          (unsigned char *)   "Error In Login<BR><a href=\"login.htm\">Login Again</a>")
                 == OF_FAILURE)
    {
       if(ucPasswordMD5)   
       {
           of_free(ucPasswordMD5);
       	   ucPasswordMD5=NULL;
       }
       return OF_FAILURE;
    }
  }
  if(ucPasswordMD5)   
  {
       of_free(ucPasswordMD5);
       ucPasswordMD5=NULL;
  }
  return OF_SUCCESS;
} /* HttpdUCM_Action_Login() */

/************************************************************************
 * Function Name : HttpdSup_Gen_ErrInfo                                 *
 * Description   : This function is used to display error information.  *
 * Input         : HTTP request structure.                              *
 *                 Charecter buffer containing the user data line.      *
 *                 Length of this buffer.                               *
 * Output        : Html file to send to the browser with or without     *
 *                 information based on the operation performed.        *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.        *
 ************************************************************************/

int32_t HttpdSup_Gen_ErrInfo( HttpRequest    *pHttpReq,
                              Httpd_Value_t  *ValArray,
                              uint32_t       uiValCount,
                              void         *pAppData )
{
  uint32_t uiTagIndex = 0;

  Httpd_Value_CopyStr(&ValArray[uiTagIndex++], pAppData);
  return OF_SUCCESS ;
} /* HttpdSup_Gen_ErrInfo() */

static void HttpdSup_HextToString( char  *pTmp,
                                     char  *pOutput,
                                     uint32_t *pLen )
{
  int32_t ii=0, tmp=0, len;
  char ucVal;

  if (!pTmp || !pOutput) return;

  len = of_strlen(pTmp);
  len /= 2;
  for (ii=0; ii < len; ii++)
  {
    ucVal = pTmp[2*ii];
    if (ucVal >= '0' && ucVal <=  '9')
      ucVal -= '0';
    else if (ucVal >= 'a' && ucVal <= 'f')
      ucVal -= ('a' - 10);
    else
      break;

    pOutput[tmp] = (ucVal<<4)&0xf0;

    ucVal = pTmp[2*ii+1];
    if (ucVal >= '0' && ucVal <=  '9')
      ucVal -= '0';
    else if (ucVal >= 'a' && ucVal <= 'f')
      ucVal -= ('a' - 10);
    else
      break;

    pOutput[tmp] |= ucVal&0xf;
    tmp++;
  }

  *pLen = tmp;
  return;
} /* HttpdSup_HextToString() */

/**Changes made by vamsi starts ***/
int32_t  SetOpaqueData(HttpRequest  *pHttpReq,uint32_t  uiIndex)
{
  char *pData;
  int32_t  iResult=0;

  pData = of_calloc(1,50);
  if(pData == NULL)
  {
    return OF_FAILURE;
  }
  of_strcpy(pData,"UPDATED SUCCESSFULLY");
  iResult =IGWHttpSetOpaqueData(pHttpReq,uiIndex,
                   pData);
  if(iResult != OF_SUCCESS)
  {
    return OF_FAILURE;
  }
  return OF_SUCCESS;
}
/***Status callback functions
 * The following callback functions are used to display the
 * updated status to the user.Let us assume that user has configured the
 * system page and submitted ,so we send the same page  for which user may
 * not be knowing whether it has updated or not  so to create an impression
 * to the user what ever he has configured is accpeted we will be
 * sending a status message after the post operation.
 * This call back function tag wil be present in most of the html files
 * of the modules which doesn't have any table page. For Eg system time,
 * Wan configuration pages
 */

int32_t SendStatusMessage( HttpRequest    *pRequest,
                           Httpd_Value_t  *ValAry,
                           uint32_t       nCount,
                           void         *pAppData )
{
  unsigned char  *pData = NULL;
  
  pData = (unsigned char *)IGWHttpGetOpaqueData(pRequest,pHttpGlobals->uiOpaqueIndex);
  if (pData == NULL)
  {
    /*Fill the null string assuming the page was requested from the
     * welcome/home.htm menu
     */
    Httpd_Value_CopyStr(&ValAry[0], "");
  }
  else
  {
    Httpd_Value_CopyStr(&ValAry[0],
                        "<div class=\"success\">Changes have been updated successfully.</div>");
  }
  return OF_SUCCESS;

}


 
void HttpHex2Str(unsigned char *pTmp,unsigned char *pOutput,uint32_t *pLen)
{
	
 int32_t ii=0,tmp=0,len;
 char ucVal;
 if(!pTmp || !pOutput)
 {
	 return;
 }
    len=of_strlen((char *) pTmp);
    len /= 2;
    for(ii=0; ii < len; ii++)
     {
         ucVal = pTmp[2*ii];
        if(ucVal >= '0' && ucVal <=  '9')
	{
           ucVal -= '0';
	}
        else if(ucVal >= 'a' && ucVal <= 'f')
	{
            ucVal -= ('a' - 10);
	}
        else 
	{ 
        	 break;
	}
        pOutput[tmp] = (ucVal<<4)&0xf0;

        ucVal = pTmp[2*ii+1];
        if(ucVal >= '0' && ucVal <=  '9')
	{
	          ucVal -= '0';
	}
        else if(ucVal >= 'a' && ucVal <= 'f')
	{
	            ucVal -= ('a' - 10);
	}
        else
	{
        	break;
	}

        pOutput[tmp] |= ucVal&0xf;
        tmp++;
  }

  *pLen = tmp;
  return;


}
/************************************************************************
 * Function Name : HttpdSup_Gen_LogInDuplicate                          *
 * Description   : This function is used to display warning 		*
 *  Message information. 						*
 * Input         : HTTP request structure, ValArray,nCount              *
 * Output        : OF_SUCCESS or OF_FAILURE  		    		*
 * Return value  : OF_SUCCESS on serving welcome page or  OF_FAILURE.     *
 ************************************************************************/

int32_t HttpdSup_Gen_LogInDuplicate( HttpRequest    *pRequest,
                             Httpd_Value_t  *ValAry,
                             uint32_t       nCount)
{
  HttpCookie_DelByIpAddress(ucIpVal);      
  ucIpVal=NULL;
  if (HttpSendHtmlFile(pRequest, (unsigned char *) "loadwelcome.htm",NULL) == OF_FAILURE)
  {
   return OF_FAILURE;
  }
  return OF_SUCCESS;
}  
#ifndef OF_CM_SUPPORT
/************************************************************************
 * Function Name : HttpdSup_AuthOrder					*
 * Description   : 							*
 * Input         :                               			*
 * Output        :                       				*
 * Return value  : OF_SUCCESS on sending all data else OF_FAILURE.        *
 ************************************************************************/

int32_t HttpdSup_AuthOrder( HttpRequest    *pHttpReq,
                            Httpd_Value_t  *ValAry,
                            uint32_t nCount)
{
 unsigned char  *ucDbList=NULL;
 unsigned char  *ucAuthFir=NULL;
 unsigned char  *ucAuthSec=NULL;
 unsigned char  *ucAuthTrd=NULL;
 unsigned char  *ucAuthFrt=NULL;
 int32_t  ii=0;

 if(!ValAry[0].value_p)
 {
   return OF_FAILURE;
 }
 ucDbList=strtok(ValAry[0].value_p,",");      
 ucAuthFir=ucDbList;
 while(ucDbList!='\0'&& ucDbList)
 {
    ucDbList=strtok(NULL,",");
    if(ii==0) ucAuthSec=ucDbList;
    if(ii==1) ucAuthTrd=ucDbList;
    if(ii==2) ucAuthFrt=ucDbList;
    ii++; 	   
 }

 if((!ucAuthFir) || (!ucAuthSec) ||(!ucAuthTrd))
 {
   if(HttpSendHtmlFile(pHttpReq, (unsigned char *)"errscrn.htm", (unsigned char *)"Error in getting the authentication order list")!=OF_SUCCESS)
   {
     Httpd_Error("HttpSendHtmlFile returned failure\r\n");
     return OF_FAILURE;
   }
   return OF_SUCCESS;
 }
 of_strcpy(pHttpGlobals->pAuthOrder[0].pAuthName,ucAuthFir);
 if(of_strcmp(ucAuthFir,HTTP_AUTH_LOCAL))
 {
   if(HttpSendHtmlFile(pHttpReq, (unsigned char *)"errscrn.htm", (unsigned char *)"Local authentication should be first")!=OF_SUCCESS)
   {
    return OF_FAILURE;
   }
  HttpAuthOrderInit(pHttpGlobals); 
  return OF_SUCCESS;
 }
 of_strcpy(pHttpGlobals->pAuthOrder[1].pAuthName,ucAuthSec);
 of_strcpy(pHttpGlobals->pAuthOrder[2].pAuthName,ucAuthTrd);

 if(HttpSendHtmlFile(pHttpReq, (unsigned char *)"uua.htm",NULL)!=OF_SUCCESS)
 {
    return OF_FAILURE;
 } 
 return OF_SUCCESS;
}

/************************************************************************
 * Function Name : HttpAuthOrderInit					*
 * Description   : 							*
 * Input         :  pHttpGlobals  - Global pointer                             			*
 * Output        :                       				*
 * Return value  : OF_SUCCESS on sending all data else OF_FAILURE.        *
 ************************************************************************/
int32_t HttpAuthOrderInit(HttpGlobals_t  *pHttpGlobals)
{
 
 int32_t iIndex=0;	

  if(!pHttpGlobals)
  {
    Httpd_Error("pHttpGlobals is NULL\r\n");	
    return OF_FAILURE;
  }
  
 pHttpGlobals->pAuthOrder=of_calloc(HTTP_AUTH_LIST_LEN,sizeof(HttpAuthOrder_t));
 if(!pHttpGlobals->pAuthOrder)
 {
     Httpd_Error("of_calloc is failed for pHttpGlobals->pAuthOrder\r\n");
     return OF_FAILURE;
 }
 of_strcpy(pHttpGlobals->pAuthOrder[iIndex++].pAuthName,HTTP_AUTH_LOCAL);
 of_strcpy(pHttpGlobals->pAuthOrder[iIndex++].pAuthName,HTTP_AUTH_RADIUS);
 of_strcpy(pHttpGlobals->pAuthOrder[iIndex++].pAuthName,HTTP_AUTH_LDAP);

 return OF_SUCCESS;
}
/************************************************************************
 * Function Name : HttpdSup_AuthInfo  	                                *
 * Description   : This function is used to display Auth  information.  *
 * Input         : HTTP request structure.                              *
 *                 Charecter buffer containing the user data line.      *
 *                 Length of this buffer.                               *
 * Output        : Html file to send to the browser with or without     *
 *                 information based on the operation performed.        *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.        *
 ************************************************************************/
 
int32_t HttpdSup_AuthInfo( HttpRequest    *pHttpReq,
                           Httpd_Value_t  *ValArray,
                           uint32_t       uiValCount,
                           void         *pAppData )
{
  int32_t uiTagIndex=0;
  char pTemp[HTTP_MAX_STRING_LENGTH];

  of_memset(pTemp,'\0',HTTP_MAX_STRING_LENGTH);
  of_strcpy(pTemp,pHttpGlobals->pAuthOrder[0].pAuthName);
  of_strcat(pTemp,",");
  of_strcat(pTemp,pHttpGlobals->pAuthOrder[1].pAuthName);
  of_strcat(pTemp,",");
  of_strcat(pTemp,pHttpGlobals->pAuthOrder[2].pAuthName);
  Httpd_Value_CopyStr(&ValArray[uiTagIndex++],pTemp);
  return OF_SUCCESS;

} /* HttpdSup_AuthInfo() */

#endif /* OF_CM_SUPPORT */
#endif /* OF_HTTPD_SUPPORT && INTOTO_HTTPD_CBK_SUPPORT */

