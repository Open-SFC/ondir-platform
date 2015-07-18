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

/*****************************************************************************
 *  File         : ucmhtp.c                                                  *
 *                                                                           *
 *  Description  : This file contains functions related to handle ADD/EDIT/DELETE 
                   and GetFirst/GetNext/GetExact POST requests
 *                                                                           *
 *  Version      Date        Author         Change Description               *
 *  -------    --------      ------        ----------------------            *
 *   1.0       05/05/09                     Initial implementation           *
 ****************************************************************************/
#if defined(OF_HTTPD_SUPPORT)
#ifdef OF_CM_SUPPORT

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/
#include "cmincld.h"
#include "httpdinc.h"
#include "httpgif.h"
#include "cmdefs.h"
#include "cmutil.h"
#include "cmsock.h"
#include "cmdll.h"
#include "dmgdef.h"
#include "cmtransgdf.h"
#include "jegdef.h"
#include "jegif.h"
#include "uchtldef.h"
#include "uchtlgif.h"

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define NAME_MAX_LEN         30
#define UCM_REQ_OPCODE_LEN   10
#define UCM_REQ_RDIRECT_LEN  20 
#define UCM_REQ_MTPATH_LEN   100
#define UCM_REQ_BUFF_LEN     4096
#define UCM_REQ_HOST_LEN     30
#define UCM_REQ_DM_PATH_LEN  100
#define UCM_RESP_MAX_BUFF_LEN  400 

#define UCM_LOCALHOST "localhost"

#define UCM_GET_ADDPG_OPCODE        1
#define UCM_GET_BYKEY_OPCODE        2
#define UCM_GET_MORE_OPCODE         3
#define UCM_GET_ROLE_OPCODE         4
#define UCM_GET_ROLE_INST_OPCODE    5
#define UCM_GET_ROLE__BYKEY_OPCODE  6
#define UCM_GET_IROLE__BYKEY_OPCODE 7
#ifdef UCM_STATS_COLLECTOR_SUPPORT
#define UCM_GET_STATS_AGGR_OPCODE 8
#define UCM_GET_STATS_AVG_OPCODE 9
#define UCM_GET_STATS_PERDEV_OPCODE 10
#endif
#define UCM_GET_BYKEY_PASIV_OPCODE  11

#define UCM_MULTCMD_OPCODE "MULTCMD"

#define UCM_CMD_FACTRESET_OPCODE  "FACRES"
#define UCM_CMD_SAVE_OPCODE  "SAVE"
#define UCM_CMD_SETROL_OPCODE "SETROL"
#define UCM_CMD_SETROL_INST_OPCODE "ISETROL"
#define UCM_CMD_DELROL_OPCODE "DELROL"
#define UCM_CMD_IDELROL_OPCODE "IDELROL"
#define UCM_CMD_REBOOT_OPCODE  "REBOOT"
#define UCM_CMD_DONE_OPCODE  "DONE"
#define UCM_CMD_CANCEL_OPCODE  "CANCEL"
#define UCM_CMD_COMMIT_OPCODE  "COMMIT"
#define UCM_CMD_REVOKE_OPCODE  "REVOKE"

#define UCM_GET_REC_COUNT    "recCnt"
#define UCM_KEY_GET_PARAMS   "keys"
#define UCM_KEYPRMS_GET_PARAMS "keyprms"
#define UCM_XML_RESP_HEADER  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"

#define UCM_XML_RESP_XSL_REF "resp_meta.xsl"

#define UCM_XML_RESP_START         "<root>"
#define UCM_XML_RESP_CONFIG_START  "<Config>"
#define UCM_XML_RESP_CONFIG_END    "</Config>"
#define UCM_XML_RESP_END           "</root>"
#define UCM_XML_RES_STR_START      "<Msg>"
#define UCM_XML_RES_STR_END        "</Msg>"

#define UCM_XML_RESP_PASSIVE_RECS_START  "<root passive=\"true\">"
#define UCM_XML_RESP_SUCCESS_START  "<root status=\"ok\">"
#define UCM_XML_RESP_FAILURE_START  "<root status=\"error\">"
#define UCM_XML_RESP_RESULT_END     "</root>"


#define UCM_DM_PATH_STR      "dmpath"
#define UCM_OPCODE_STR       "opcode"
#define UCM_META_FILE_PATH   "mtpath"
#define UCM_SREDIRECT_PATH   "sredirect"
#define UCM_FREDIRECT_PATH   "fredirect"
#define UCM_REQTYPE_STR      "reqtype"
#define UCM_HOST_NAME        "host"

#define UCM_DM_PATH_PARAM         0
#define UCM_OPCODE_PARAM          1
#define UCM_META_FILE_PATH_PARAM  2
#define UCM_SREDIRECT_PATH_PARAM  3
#define UCM_FREDIRECT_PATH_PARAM  4
#define UCM_REQ_PARAM             5
#define UCM_REQTYPE_STR_PARAM     6
#define UCM_KEY_PARAM             7
#define UCM_HOST_PARAM            8
#define UCM_SUBMIT_PARAM          9

/********************************************************
 * * * * * * G l o b a l V a r i a b l e s  * * * * * * *
 ********************************************************/


/********************************************************
 * * * * * * V a r i a b l e     A r r a y s * * * * * *
 ********************************************************/

Httpd_Variable_t UcmHandlerActVars_ucmhandler[] = {
  {UCM_DM_PATH_STR, UCM_REQ_DM_PATH_LEN + 1},
  {UCM_OPCODE_STR, UCM_REQ_OPCODE_LEN + 1},
  {UCM_META_FILE_PATH, UCM_REQ_MTPATH_LEN + 1},
  {UCM_SREDIRECT_PATH, UCM_REQ_RDIRECT_LEN +1 },
  {UCM_FREDIRECT_PATH, UCM_REQ_RDIRECT_LEN +1 },
  {"ucmreq", UCM_REQ_BUFF_LEN},
  {UCM_REQTYPE_STR, UCM_REQ_OPCODE_LEN +1 },
  {"keyprms", UCM_REQ_BUFF_LEN},
  {"host", UCM_REQ_HOST_LEN + 1},
  {"Submit", 32}
};

#define UCM_ACT_VARCNT_ucm sizeof(UcmHandlerActVars_ucmhandler)/sizeof(Httpd_Variable_t)

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/


int32_t HttpdUCMConfigGetHandler(HttpRequest *pHttpReq);

 char ucmx2c( unsigned char  *what );
 void ucmUnEscapeUrl( unsigned char  *url );
 void ucmHTTPFreeUCMCmds(struct cm_command *pCmds, int32_t iCmdCount);

 int32_t
ucmHTTPCreateNewSession(char *pUserName, ipaddr IpAddress, 
                        UCMHttpSession_t **ppUCMHttpSession);

 int32_t
ucmHTTPGetReqHandler(char *dmPath, char *mtPath,  int32_t iPassive,
                     char *spgFile, char *keyStr, char *keyPrms, 
                     char *pHost, int32_t iOpcode, HttpRequest *pHttpReq);

 int32_t UcmGetRequestHandler_ucmhandler (HttpRequest * pHttpReq,
                                                    Httpd_Value_t * ValArray,
                                                    uint32_t uiValCount);
 int32_t UcmRequestHandler_ucmhandler (HttpRequest * pHttpReq,
                                                 Httpd_Value_t * ValArray,
                                                 uint32_t uiValCount);

/*********************************************************
 * * * *   R e g i s t r a t i o n     T a b l e   * * * *
 *********************************************************/

Httpd_RegEntry_t UcmHttp_LocalTags[] = {

  {HTTPD_TAG_TYPE_ACTION, "ucmreq.igw",
   UcmRequestHandler_ucmhandler,
   UCM_ACT_VARCNT_ucm, UcmHandlerActVars_ucmhandler},
  {HTTPD_TAG_TYPE_GENERAL, "ucmreq.igw", UcmGetRequestHandler_ucmhandler,
   UCM_ACT_VARCNT_ucm, UcmHandlerActVars_ucmhandler},
  {HTTPD_TAG_TYPE_NONE, NULL, NULL, 0, NULL}
};

extern char ucmDeclare[];
extern char endUcmDeclare[];

/********************************************************
 * * I n i t i a l i z a t i o n     F u n c t i o n  * *
 ********************************************************/
/***********************************************************************
 * Function Name : IGWUcmHttpInit                                      *
 * Description   : This API is used to initialize ucm session          *
 * Input         : none                                                *
 * Output        : None                                                *
 * Return value  : OF_FAILURE/OF_SUCCESS                                 *
 ***********************************************************************/
int32_t IGWUcmHttpInit (void)
{
  int32_t ii;

  UCMHTTPDEBUG (" Inside IGWUcmHttpInit... \r\n");

  ii = Httpd_Register_TagArray (UcmHttp_LocalTags);
  if (ii < 0)
  {
    UCMHTTPDEBUG ("IGWUcmHttpInit() failed\r\n");
    return OF_FAILURE;
  }
  HttpdUCMSessionInit();

  return OF_SUCCESS;
}


/***********************************************************************
 * Function Name : ucmx2c                                              *
 * Description   : This converts hex bytes to characters in URL     *
 * Input         : what - start of buffer                              *
 * Output        : None                                                *
 * Return value  : Converted character                              *
 ***********************************************************************/


char ucmx2c( unsigned char  *what )
{
  register char digit;

  digit = ((what[0] >= 'A') ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
  digit *= 16;
  digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
  return(digit);
} /* ucmx2c() */

/***********************************************************************
 * Function Name : ucmUnEscapeUrl                                      *
 * Description   : This removes escape characters(%XX stuff) from URL  *
 * Input         : url - Url String To Be Converted                    *
 * Output        : None                                                *
 * Return value  : None                                                *
 ***********************************************************************/


void ucmUnEscapeUrl( unsigned char  *url )
{
  register int32_t  x, y;

  for (x = 0, y = 0; url[y]; ++x, ++y)
  {
    if ((url[x] = url[y]) == '%')
    {
      url[x] = ucmx2c(&url[y+1]);
      y += 2;
    }
  }
  url[x] = '\0';
} /* ucmUnEscapeUrl() */


/***********************************************************************
 * Function Name : HttpdUCMConfigGetHandler                            *
 * Description   : This function is used for handling POST requests.   *
 * Input         : pHttpReq - HTTP request structure.                  *
 * Output        : Html file to send to the browser with or without    *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.       *
 ***********************************************************************/
int32_t HttpdUCMConfigGetHandler(HttpRequest *pHttpReq)
{
  UCMHTTPDEBUG("\n INSIDE:HttpdUCMConfigGetHandler... \r\n");

  int32_t ii = 0;
  int32_t iOpcode = 0;

  char *dmPath = NULL;
  char *mtPath = NULL;
  char *spgFile = NULL;
  char *keyStr = NULL;
  char *keyPrms = NULL;
  char *pHost = NULL;

  IGWGetParams_t *pParams = (IGWGetParams_t *)pHttpReq->pApplData;

  if(pParams != NULL)
  {
    while(ii!=HTTP_XML_ASSOCLIST_LEN)
    { 
       UCMHTTPDEBUG("----NAME---------VALUE-------\r\n");	
       UCMHTTPDEBUG("     %s          ",pParams[ii].ucName);
       UCMHTTPDEBUG("     %s          \r\n",pParams[ii].ucValue);

       if(of_strcmp((char *) pParams[ii].ucName, UCM_DM_PATH_STR) == 0)
       {
         dmPath = (char *) pParams[ii].ucValue;
       }
       else if(of_strcmp((char *) pParams[ii].ucName, UCM_META_FILE_PATH) == 0)
       {
         mtPath = (char *) pParams[ii].ucValue;
       }
       else if(of_strcmp((char *) pParams[ii].ucName, UCM_SREDIRECT_PATH) == 0)
       {
         spgFile = (char *) pParams[ii].ucValue;
       }
       else if(of_strcmp((char *) pParams[ii].ucName, UCM_KEY_GET_PARAMS) == 0)
       {
         keyStr = (char *) pParams[ii].ucValue;
       }
       else if(of_strcmp((char *) pParams[ii].ucName, UCM_KEYPRMS_GET_PARAMS) == 0)
       {
         keyPrms = (char *) pParams[ii].ucValue;
       }
       else if(of_strcmp((char *) pParams[ii].ucName, UCM_OPCODE_STR) == 0) {
         iOpcode = of_atoi((char *) pParams[ii].ucValue);
         UCMHTTPDEBUG("  iOpcode =   %d \r\n", iOpcode);
       }
       else if(of_strcmp((char *) pParams[ii].ucName, UCM_HOST_NAME) == 0)
       {
         pHost = (char *) pParams[ii].ucValue;
       }
       ii++;
    }
  }

  UCMHTTPDEBUG("dmPath = %s \n",dmPath);
  ucmHTTPGetReqHandler(dmPath, mtPath, FALSE, spgFile, keyStr, 
                       keyPrms, pHost, iOpcode, pHttpReq);

  return OF_SUCCESS;
}

/***********************************************************************
 * Function Name : ucmHTTPCreateNewSession                             *
 * Description   : This function is used to create a new session.      *
 * Input         : pUserName - User name.                              *
                   IpAddress - IP Address                              *
 * Output        : ppUCMHttpSession - newly created HTTP session       *
 * Return value  : OF_SUCCESS on successful session creation else OF_FAILURE.*
 ***********************************************************************/
 int32_t
ucmHTTPCreateNewSession(char *pUserName, ipaddr IpAddress, 
                        UCMHttpSession_t **ppUCMHttpSession)
{
  int32_t retVal = 0;
  ipaddr localIpAddr = 0;
  UCMHttpSession_t *pUCMLocalSession = NULL;

  cm_val_and_conv_aton(UCM_LOCALHOST, &localIpAddr);
  if(ucmHTTPFindSessionEntry(pUserName, localIpAddr, 
                             &pUCMLocalSession) == OF_SUCCESS)
  {
    retVal = HttpdUCMCreateLoginSession((unsigned char *) pUserName, (unsigned char *) "",
                            &pUCMLocalSession->RoleInfo, IpAddress);
    if(retVal != OF_SUCCESS)
    {
      UCMHTTPDEBUG("  %s :: Failed to create session \r\n",__FUNCTION__ );
      return OF_FAILURE;
    }
    if(ucmHTTPFindSessionEntry(pUserName,
                             IpAddress, ppUCMHttpSession) != OF_SUCCESS)
    {
      UCMHTTPDEBUG("  %s :: Failed to create session \r\n",__FUNCTION__ );
      return OF_FAILURE;
    }
  }

  return OF_SUCCESS;
}

/***********************************************************************
 * Function Name : ucmHTTPGetReqHandler                                *
 * Description   : This function is used for handle GetFirst/GetNext   *
                   and GetExact POST requests.                         *
 * Input         : dmPath - Data model template path
                   pHttpReq - HTTP request structure.                  *
 *                 mtPath - Meta data path
                   keyStr - Key value 
                   keyPrms - array of key value parameters             *
 *                 iOpCode - Operation code.                           *
 * Output        : Html file to send to the browser with or without    *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.       *
 ***********************************************************************/
 int32_t
ucmHTTPGetReqHandler(char *dmPath, char *mtPath, int32_t iPassive,
                     char *spgFile, char *keyStr, char *keyPrms, 
                     char *pHost, int32_t iOpcode, HttpRequest *pHttpReq)
{
  UCMHTTPDEBUG("\n INSIDE:%s... \r\n", __FUNCTION__);

  ipaddr IpAddress = 0;
  char *xmlBuf = NULL;
  UCMHttpSession_t *pUCMHttpSession = NULL;
  char *respBuf = NULL;
  char metaFile[100];
  char cDMPath[512];
  char sRedirectFile[100];
  char cKeysArray[512 + 1];
  unsigned char delims[] = "^!|";
  int32_t buffLen = 0;
  int32_t maxBuffLen = UCM_RESP_MAX_BUFF_LEN;
  int32_t recCount = UCM_HTTP_MAX_RECORDS_PER_PAGE;
  int32_t bExact = FALSE;
  int32_t retVal = 0;
  //int32_t iLen = 0;
  struct cm_array_of_nv_pair KeysArray;
  struct cm_array_of_nv_pair KeyPrmsArray;
  
  if( !dmPath)
  {
    return OF_FAILURE;
  }
  of_memset(&KeysArray,0,sizeof(struct cm_array_of_nv_pair));
  of_memset(&KeyPrmsArray,0,sizeof(struct cm_array_of_nv_pair));
  of_memset(metaFile, 0, sizeof(metaFile));
  of_memset(cDMPath, 0, sizeof(cDMPath));
  of_memset(sRedirectFile, 0, sizeof(sRedirectFile));
  of_memset(cKeysArray, 0, sizeof(cKeysArray));

  sprintf(cDMPath, "<dmpath>%s</dmpath>" , dmPath);
  sprintf(metaFile, "<MetaData>%s</MetaData>" , mtPath);
  sprintf(sRedirectFile, 
             "<?xml-stylesheet type=\"text/xsl\" href=\"%s\"?>",
             spgFile);
  if(keyStr)
  {
    UCMHTTPDEBUG("keyStr = %s \n",keyStr);
    /* Convert into key value pairs.*/
    ucmHttpConvertoNameValuePair(keyStr, &KeysArray, delims);
  }
  else
  {
    KeysArray.count_ui = 0;
    KeysArray.nv_pairs = NULL;
  }
  if(keyPrms != NULL && of_strlen(keyPrms) > 0)
  {
    UCMHTTPDEBUG("keyPrms= %s \n",keyPrms);
    /* Convert into key value pairs.*/
    ucmHttpConvertoNameValuePair(keyPrms, &KeyPrmsArray, delims);
  }
  else
  {
    KeyPrmsArray.count_ui = 0;
    KeyPrmsArray.nv_pairs = NULL;
  }
  UCMHTTPDEBUG(" \n%s:: iOpcode =   %d \r\n", __FUNCTION__, iOpcode);
  UCMHTTPDEBUG(" \n%s:: pHost =   %s \r\n", __FUNCTION__, pHost);

  if(pHost)
  {   
    if(cm_val_and_conv_aton(pHost, &IpAddress) == OF_FAILURE )
    {
      UCMHTTPDEBUG("  %s :: Invalid IP Address \r\n",__FUNCTION__ );
    }
  }
  if(ucmHTTPFindSessionEntry((char *) pHttpReq->pUserName, 
                             IpAddress, &pUCMHttpSession) != OF_SUCCESS)
  {
    /* If session not found try creating new session with same role*/
    ucmHTTPCreateNewSession((char *) pHttpReq->pUserName, IpAddress, &pUCMHttpSession);
  }
  
  if(pUCMHttpSession)
  {
    switch(iOpcode)
    {
      case UCM_GET_ADDPG_OPCODE:
        respBuf = (char*) of_calloc(1, UCM_RESP_MAX_BUFF_LEN +1);
        if(respBuf == NULL)
        {
          UCMHTTPDEBUG(" \n%s:: failed to allocate respBuf \r\n", __FUNCTION__);
          return OF_SUCCESS;
        }
        of_strcpy(respBuf, UCM_XML_RESP_HEADER);
        of_strcat(respBuf, sRedirectFile);
        of_strcat(respBuf, UCM_XML_RESP_START);
        of_strcat(respBuf, metaFile);
        of_strcat(respBuf, cDMPath);
        of_strcat(respBuf, UCM_XML_RESP_CONFIG_START);
        of_strcat(respBuf, UCM_XML_RESP_CONFIG_END);
        of_strcat(respBuf, UCM_XML_RESP_END);

        Httpd_Send_PlainString(pHttpReq,respBuf);
        of_free(respBuf);
        return OF_SUCCESS;
      case UCM_GET_BYKEY_OPCODE:
        bExact = TRUE;
        retVal = ucmHttpDisplayTableRecords(pUCMHttpSession, &KeysArray,
                                           &KeyPrmsArray, dmPath, bExact, &xmlBuf);
        break;
      case UCM_GET_MORE_OPCODE:       
        retVal = ucmHttpDisplayTableNextNRecords(pUCMHttpSession, &KeysArray,
                                      &KeyPrmsArray, dmPath, recCount, &xmlBuf);
        break;
#ifdef UCM_ROLES_PERM_SUPPORT
      case UCM_GET_ROLE_OPCODE:
        retVal = ucmHttpDisplayRolesettings(pUCMHttpSession, dmPath, &xmlBuf);
        break;
      case UCM_GET_ROLE_INST_OPCODE:
        retVal = ucmHttpDisplayInstanceRolesettings(pUCMHttpSession, 
                                                    dmPath, &xmlBuf);
        break;
      case UCM_GET_ROLE__BYKEY_OPCODE:
        retVal =  ucmHttpDisplayRolesettingsByRole(pUCMHttpSession,
                                        dmPath, &KeysArray, &xmlBuf);
        break;
      case UCM_GET_IROLE__BYKEY_OPCODE:
        retVal =  ucmHttpDisplayInstRolesettingsByRole(pUCMHttpSession, 
                                   dmPath, &KeysArray, &xmlBuf);
        break;
#endif /*UCM_ROLES_PERM_SUPPORT*/
#ifdef UCM_STATS_COLLECTOR_SUPPORT
      case UCM_GET_STATS_AGGR_OPCODE:
        retVal = ucmHttpDisplayStats(pUCMHttpSession, dmPath, &KeysArray, 
                                  ucmGetUCMCommandType("AGGR"), &xmlBuf);
          break;
      case UCM_GET_STATS_AVG_OPCODE:
         retVal = ucmHttpDisplayStats(pUCMHttpSession, dmPath, &KeysArray,
                                      ucmGetUCMCommandType("AVG"), &xmlBuf);
         break;
      case UCM_GET_STATS_PERDEV_OPCODE:
         retVal = ucmHttpDisplayStats(pUCMHttpSession, dmPath, &KeysArray, 
                                      ucmGetUCMCommandType("PERDEV"), &xmlBuf);
        break;
#endif
      default:
        retVal = ucmHttpDisplayTableRecords(pUCMHttpSession, &KeysArray,
                                  &KeyPrmsArray, dmPath, bExact, &xmlBuf);
        break;
    }
  }
  else
  {
    retVal = OF_FAILURE;
  }

  if(xmlBuf != NULL)
  {
    maxBuffLen += of_strlen(xmlBuf);
  }
 
  respBuf = (char*) of_calloc(1, maxBuffLen +1);
  if(respBuf == NULL)
  {
    UCMHTTPDEBUG(" \n%s:: failed to allocate respBuf \r\n", __FUNCTION__);
    return OF_SUCCESS;
  }
  if(retVal != OF_SUCCESS)
  {
    of_strcpy(respBuf, UCM_XML_RESP_HEADER);
    of_strcat(respBuf, sRedirectFile);
    of_strcat(respBuf, UCM_XML_RESP_START);
    of_strcat(respBuf, metaFile);
    of_strcat(respBuf, cDMPath);
    of_strcat(respBuf, UCM_XML_RESP_CONFIG_START);
    of_strcat(respBuf, UCM_XML_RESP_CONFIG_END);
    of_strcat(respBuf, UCM_XML_RESP_END);

    Httpd_Send_PlainString(pHttpReq,respBuf);
  }
  else
  {
    of_strcpy(respBuf, UCM_XML_RESP_HEADER);
    of_strcat(respBuf, sRedirectFile);
    if(iPassive == TRUE)
    {
      of_strcat(respBuf, UCM_XML_RESP_PASSIVE_RECS_START);
    }
    else
    {
      of_strcat(respBuf, UCM_XML_RESP_START);
    }
    of_strcat(respBuf, metaFile);
    of_strcat(respBuf, cDMPath);
    of_strcat(respBuf, UCM_XML_RESP_CONFIG_START);
    if(xmlBuf != NULL)
    {
      of_strcat(respBuf, xmlBuf);
      of_free(xmlBuf);
    }
    of_strcat(respBuf, UCM_XML_RESP_CONFIG_END);
    of_strcat(respBuf, UCM_XML_RESP_END);

    buffLen = of_strlen (respBuf);
    UCMHTTPDEBUG("respBuf = %s \n", respBuf);
    UCMHTTPDEBUG("\nrespBuf len = %d \n", buffLen);

    Httpd_Send_PlainString(pHttpReq,respBuf);

  }
  if(respBuf != NULL)
  {
    of_free(respBuf);
  }

  /* Free KeysArray.*/
  if(KeysArray.count_ui > 0)
    CM_FREE_NVPAIR_ARRAY(KeysArray, KeysArray.count_ui);

   return OF_SUCCESS;
}

/***********************************************************************
 * Function Name : UcmGetRequestHandler_ucmhandler                     *
 * Description   : This function is used for handle GetFirst/GetNext   *
 *                 and GetExact POST requests.                         *
 * Input         : pHttpReq - HTTP request structure.                  *
 *                 ValArray - pointer to Httpd_Value_t structure.      *
 *                 uiValCount - ValArray size.                         *
 * Output        : Html file to send to the browser with or without    *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.       *
 ***********************************************************************/
 int32_t
UcmGetRequestHandler_ucmhandler (HttpRequest * pHttpReq,
                                 Httpd_Value_t * ValArray,
                                 uint32_t uiValCount)
{
  int32_t iLen = 0;
    UCMHTTPDEBUG(" INSIDE:: %s ....\n",__FUNCTION__);

  /*Httpd_Value_CopyStr(&ValArray[0], (char*)pHttpReq->pAppData); */

  iLen = of_strlen((char *) pHttpReq->pAppData);
  ValArray[0].Size = iLen;
  ValArray[0].value_p = (char *) pHttpReq->pAppData;

  return UcmRequestHandler_ucmhandler (pHttpReq, ValArray, uiValCount);

}

/***********************************************************************
 * Function Name : UcmRequestHandler_ucmhandler                        *
 * Description   : This function is used for handle ADD/EDIT/DELETE    *
 *                 POST requests.                                      *
 * Input         : pHttpReq - HTTP request structure.                  *
 *                 ValArray - pointer to Httpd_Value_t structure.      *
 *                 uiValCount - ValArray size.                         *
 * Output        : Html file to send to the browser with or without    *
 * Return value  : OF_SUCCESS on setting all data else OF_FAILURE.       *
 ***********************************************************************/
 int32_t
UcmRequestHandler_ucmhandler (HttpRequest * pHttpReq,
                              Httpd_Value_t * ValArray, uint32_t uiValCount)
{
  int32_t i = 0;
  int32_t iPassive = FALSE;
  ipaddr IpAddress = 0;
  uint32_t count = 0;
  char *pXmlBuf = NULL;
  char respBuf[UCM_REQ_BUFF_LEN];
  char metaFile[100];
  char dMPath[512];
  char opCode[100];
  char sRedirPath[100];    /* To hold success redirect path */
  char fRedirPath[100];    /* To hold failure redirect path */
  char sRedirectFile[100]; /* For XSL reference */
  int32_t retVal = 0;
  int32_t iOpcode = 0;
  int32_t iCmdCount = 0;
  int32_t bisComitRst = TRUE;
  unsigned char delims[] = "#=|";
  unsigned char delm[] = "^";
  UCMHttpSession_t *pUCMHttpSession = NULL;

  struct cm_array_of_nv_pair nv_pair_array;
  struct cm_command *pCmds;

  of_memset(&nv_pair_array, 0, sizeof (struct cm_array_of_nv_pair));
  of_memset(respBuf, 0, sizeof(respBuf));
  of_memset(dMPath, 0, sizeof(dMPath));
  of_memset(metaFile, 0, sizeof(metaFile));
  of_memset(opCode, 0, sizeof(opCode));
  of_memset(sRedirPath, 0, sizeof(sRedirPath));
  of_memset(fRedirPath, 0, sizeof(fRedirPath));
  of_memset(sRedirectFile, 0, sizeof(sRedirectFile));

  for(i = 0; i < uiValCount; i++)
  {
    UCMHTTPDEBUG ("\n*** %s()::ValArray[%d] = %s \r\n", 
                           __FUNCTION__, i, ValArray[i].value_p);
  }

  if(of_strcmp( ValArray[UCM_REQTYPE_STR_PARAM].value_p, "GETPASSV")==0)
  {
    iPassive = TRUE;
  }

  if((of_strcmp( ValArray[UCM_REQTYPE_STR_PARAM].value_p, "GET")==0) 
       || (of_strcmp( ValArray[UCM_REQTYPE_STR_PARAM].value_p, "STATS")==0)
       || (of_strcmp( ValArray[UCM_REQTYPE_STR_PARAM].value_p, "GETPASSV")==0))
  {
    iOpcode = of_atoi(ValArray[UCM_OPCODE_PARAM].value_p);
    UCMHTTPDEBUG("  iOpcode =   %d \r\n", iOpcode);
    return ucmHTTPGetReqHandler(ValArray[UCM_DM_PATH_PARAM].value_p,
                                ValArray[UCM_META_FILE_PATH_PARAM].value_p,
                                iPassive,
                                ValArray[UCM_SREDIRECT_PATH_PARAM].value_p,
                                ValArray[UCM_REQ_PARAM].value_p,
                                ValArray[UCM_KEY_PARAM].value_p,
                                ValArray[UCM_HOST_PARAM].value_p,
                                iOpcode, pHttpReq);
  }

  sprintf(dMPath, "<dmpath>%s</dmpath>" , ValArray[UCM_DM_PATH_PARAM].value_p);
  sprintf(opCode, "<OPCode>%s</OPCode>" , ValArray[UCM_OPCODE_PARAM].value_p);
  sprintf(metaFile, "<MetaData>%s</MetaData>",
                                     ValArray[UCM_META_FILE_PATH_PARAM].value_p);
  sprintf(sRedirPath, "<Sredirect>%s</Sredirect>" , 
                                     ValArray[UCM_SREDIRECT_PATH_PARAM].value_p);
  sprintf(fRedirPath, "<Fredirect>%s</Fredirect>",
                                     ValArray[UCM_FREDIRECT_PATH_PARAM].value_p);

  if(cm_val_and_conv_aton(ValArray[UCM_HOST_PARAM].value_p, &IpAddress) == OF_FAILURE)
  {
    UCMHTTPDEBUG("  %s :: Invalid IP Address \r\n",__FUNCTION__ );
  }
  if(ucmHTTPFindSessionEntry((char *) pHttpReq->pUserName, 
                          IpAddress, &pUCMHttpSession) != OF_SUCCESS)
  {
    /* If session not found try creating new session with same role*/
    ucmHTTPCreateNewSession((char *) pHttpReq->pUserName, IpAddress, &pUCMHttpSession);
  }

  if(pUCMHttpSession)
  {
    if(of_strcmp( ValArray[UCM_OPCODE_PARAM].value_p, "ADD")==0)
    {
      ucmHttpConvertoUCMCommandsPair(ValArray[UCM_REQ_PARAM].value_p, CM_CMD_ADD_PARAMS,
                                     &iCmdCount, delm, delims, &pCmds);
      retVal = 
        ucmHttpFrameAndSendAddParamsToJE(pUCMHttpSession, iCmdCount,
                                                     pCmds, &pXmlBuf);
    }
    else if(of_strcmp( ValArray[UCM_OPCODE_PARAM].value_p, "EDIT")==0)
    {
      ucmHttpConvertoUCMCommandsPair(ValArray[UCM_REQ_PARAM].value_p, CM_CMD_SET_PARAMS,
                                     &iCmdCount, delm, delims, &pCmds);
      retVal = 
         ucmHttpFrameAndSendSetParamsToJE(pUCMHttpSession, iCmdCount,
                                                              pCmds, &pXmlBuf);
    }
    else if(of_strcmp( ValArray[UCM_OPCODE_PARAM].value_p, "DEL")==0)
    {
      ucmHttpConvertoUCMCommandsPair(ValArray[UCM_REQ_PARAM].value_p, CM_CMD_DEL_PARAMS,
                                     &iCmdCount, delm, delims, &pCmds);
      retVal = ucmHttpSendDeletParamsToJE(pUCMHttpSession, iCmdCount,
                                               pCmds, &pXmlBuf);
    }
    if(of_strcmp( ValArray[UCM_OPCODE_PARAM].value_p, UCM_MULTCMD_OPCODE)==0)
    {
      ucmHttpConvertoUCMCommandsPair(ValArray[UCM_REQ_PARAM].value_p, 0,
                                     &iCmdCount, delm, delims, &pCmds);
      retVal = 
        ucmHttpFrameAndSendMultCmdsToJE(pUCMHttpSession, iCmdCount,
                                                     pCmds, &pXmlBuf);
    }
    else if(of_strcmp( ValArray[UCM_OPCODE_PARAM].value_p, UCM_CMD_SAVE_OPCODE)==0)
    {
      bisComitRst = FALSE;
      retVal = ucmHttpSaveConfig(pUCMHttpSession, 
                                 ValArray[UCM_DM_PATH_PARAM].value_p, &pXmlBuf);
    }
    else if(of_strcmp( ValArray[UCM_OPCODE_PARAM].value_p, 
                               UCM_CMD_FACTRESET_OPCODE)==0)
    {
      bisComitRst = FALSE;
      retVal = ucmHttpSetFactoryDefaults(pUCMHttpSession, 
                                ValArray[UCM_DM_PATH_PARAM].value_p, &pXmlBuf);
    }
#ifdef UCM_ROLES_PERM_SUPPORT
    else if(of_strcmp( ValArray[UCM_OPCODE_PARAM].value_p, 
                               UCM_CMD_SETROL_OPCODE)==0)
    {
      bisComitRst = FALSE;
      ucmHttpConvertoNameValuePair(ValArray[UCM_REQ_PARAM].value_p, 
                                              &nv_pair_array, delims);
      retVal = ucmHttpSetRolePermissions(pUCMHttpSession, 
                                         ValArray[UCM_DM_PATH_PARAM].value_p, 
                                         &nv_pair_array,&pXmlBuf);
    }
    else if(of_strcmp( ValArray[UCM_OPCODE_PARAM].value_p, UCM_CMD_SETROL_INST_OPCODE)==0)
    {
      bisComitRst = FALSE;
      ucmHttpConvertoNameValuePair( ValArray[UCM_REQ_PARAM].value_p, &nv_pair_array, delims);
      retVal = ucmHttpSetInstanceRolePermissions(pUCMHttpSession,
                                         ValArray[UCM_DM_PATH_PARAM].value_p,
                                         &nv_pair_array);
    }
    else if(of_strcmp( ValArray[UCM_OPCODE_PARAM].value_p, UCM_CMD_DELROL_OPCODE)==0)
    {
      bisComitRst = FALSE;
      ucmHttpConvertoNameValuePair( ValArray[UCM_REQ_PARAM].value_p, &nv_pair_array, delims);
      retVal = ucmHttpDelRolePermissions(pUCMHttpSession, 
                                         ValArray[UCM_DM_PATH_PARAM].value_p, 
                                         &nv_pair_array);
    }
    else if(of_strcmp( ValArray[UCM_OPCODE_PARAM].value_p, UCM_CMD_IDELROL_OPCODE)==0)
    {
      bisComitRst = FALSE;
      ucmHttpConvertoNameValuePair( ValArray[UCM_REQ_PARAM].value_p, &nv_pair_array, delims);
      retVal = ucmHttpDelInstanceRolePermissions(pUCMHttpSession, 
                                         ValArray[UCM_DM_PATH_PARAM].value_p, 
                                         &nv_pair_array);
    }
#endif /*UCM_ROLES_PERM_SUPPORT*/
    else if(of_strcmp( ValArray[UCM_OPCODE_PARAM].value_p, 
                               UCM_CMD_REBOOT_OPCODE)==0)
    {

      bisComitRst = FALSE;
      retVal = ucmHttpRebootDevice(pUCMHttpSession, 
                                ValArray[UCM_DM_PATH_PARAM].value_p, &pXmlBuf);
    }
    if(of_strcmp( ValArray[UCM_REQTYPE_STR_PARAM].value_p, 
                                                 UCM_CMD_DONE_OPCODE)==0)
    {
      if(retVal == OF_SUCCESS)
      {
         bisComitRst = FALSE;
         retVal = ucmHttpDoneCmd (pUCMHttpSession,
                                ValArray[UCM_DM_PATH_PARAM].value_p, &pXmlBuf);
      }
      else 
      {
        bisComitRst = FALSE;
        ucmHttpCancelCmd(pUCMHttpSession,
                          ValArray[UCM_DM_PATH_PARAM].value_p, &pXmlBuf);
        retVal = OF_FAILURE;
      }
    }
    if(of_strcmp( ValArray[UCM_REQTYPE_STR_PARAM].value_p,
                                                 UCM_CMD_CANCEL_OPCODE)==0)
    {
       bisComitRst = FALSE;
       retVal = ucmHttpCancelCmd(pUCMHttpSession,
                                ValArray[UCM_DM_PATH_PARAM].value_p, &pXmlBuf);
    }
    if(of_strcmp( ValArray[UCM_REQTYPE_STR_PARAM].value_p, 
                                                 UCM_CMD_COMMIT_OPCODE)==0)
    {
       bisComitRst = TRUE;
    }
    if(of_strcmp( ValArray[UCM_REQTYPE_STR_PARAM].value_p, 
                                                 UCM_CMD_REVOKE_OPCODE)==0)
    {
       bisComitRst = FALSE;
       ucmHttpRevokeConfigSession(pUCMHttpSession);
    }
    of_strcat(respBuf, UCM_XML_RESP_HEADER);

    if(bisComitRst == TRUE)
    {
       /*
        * Save does not require Commit or Revoke.
        */
       if(retVal == OF_SUCCESS)
       {
         retVal = ucmHttpCommitConfigSession(pUCMHttpSession, &pXmlBuf);
         if(retVal == OF_FAILURE)
         {
           ucmHttpRevokeConfigSession(pUCMHttpSession);
         }
       }
       else
       {
         ucmHttpRevokeConfigSession(pUCMHttpSession);
       }
    }
  }
  else
  {
    retVal = OF_FAILURE;
  }
  if(retVal == OF_SUCCESS)
  {
    sprintf(sRedirectFile, "<?xml-stylesheet type=\"text/xsl\" href=\"%s\"?>",
            UCM_XML_RESP_XSL_REF);
    of_strcat(respBuf, sRedirectFile);
    of_strcat(respBuf, UCM_XML_RESP_SUCCESS_START);
  }
  else
  {
    sprintf(sRedirectFile, "<?xml-stylesheet type=\"text/xsl\" href=\"%s\"?>",
            ValArray[UCM_FREDIRECT_PATH_PARAM].value_p);
    of_strcat(respBuf, sRedirectFile);
    of_strcat(respBuf, UCM_XML_RESP_FAILURE_START);

  }

  of_strcat(respBuf, dMPath);
  of_strcat(respBuf, metaFile);
  of_strcat(respBuf, opCode);
  of_strcat(respBuf, sRedirPath);
  of_strcat(respBuf, fRedirPath);

  if(pXmlBuf)
  {
    of_strcat(respBuf, UCM_XML_RES_STR_START);

    UCMHTTPDEBUG(" %s :: pXmlBuf = %s \n", __FUNCTION__, pXmlBuf);

    of_strcat(respBuf, pXmlBuf);
    of_free(pXmlBuf);
    of_strcat(respBuf, UCM_XML_RES_STR_END);
  }
  of_strcat(respBuf, UCM_XML_RESP_RESULT_END);

  UCMHTTPDEBUG(" %s :: respBuf = %s \n", __FUNCTION__, respBuf);

  Httpd_Send_PlainString(pHttpReq,respBuf);

  if(iCmdCount > 0)
  {
    ucmHTTPFreeUCMCmds(pCmds, iCmdCount);
  }
    /* Free nv_pair_array */
  if(iCmdCount == 0 && nv_pair_array.count_ui > 0)
  {
    count = nv_pair_array.count_ui;
    CM_FREE_NVPAIR_ARRAY(nv_pair_array, count);
  }
  
  return OF_SUCCESS;
}

/***********************************************************************
 * Function Name : ucmHTTPFreeUCMCmds                                  *
 * Description   : This function is used for free allocated memory.    *
 * Input         : pCmds - Array of commands.                          *
                   iCmdCount - No of commands
 * Output        : none                                                *
 * Return value  : none                                                *
 ***********************************************************************/
void ucmHTTPFreeUCMCmds(struct cm_command *pCmds, int32_t iCmdCount)
{
  int32_t i = 0;

  for(i = 0; i < iCmdCount; i++)
  { 
    of_free(pCmds[i].dm_path_p);
    if(pCmds[i].nv_pair_array.count_ui > 0 
            && pCmds[i].nv_pair_array.nv_pairs != NULL)
    {
      CM_FREE_NVPAIR_ARRAY(pCmds[i].nv_pair_array, 
                            pCmds[i].nv_pair_array.count_ui);
    }
  }
}

#endif /*OF_CM_SUPPORT */
#endif /*OF_HTTPD_SUPPORT */
