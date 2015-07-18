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

/* Reviewed :: NO
   Coverity :: NO
   Ready For review:: NO
   Doxyzen :: NO */

/*
 * $Source: /cvsroot/fslrepo/sdn-of/cminfra/http/basic/ucmhtglu.c,v $
 * $Revision: 1.1.2.1.2.2 $
 * $Date: 2014/06/26 06:38:55 $
 */
/*****************************************************************************
 *  File         : ucmhtglu.c                                                 *
 *                                                                           *
 *  Description  : UCM handler for iGateway handler.                         *
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
#include "cmdefs.h"
#include "cmutil.h"
#include "cmsock.h"
#include "cmdll.h"
#include "dmgdef.h"
#include "cmtransgdf.h"
#include "jegdef.h"
#include "jegif.h"
#include "dmgif.h"
#include "ldsvgif.h"
#include "uchtldef.h"
#include "uchtlgif.h"

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/
#define UCM_XML_MAX_BUFF_LEN (12 * 1024)
#define UCM_XML_MAX_RES_BUFF_LEN 2048
#define UCM_XML_MAX_LINE_BUFF_LEN 1024
#define UCM_MAX_RECORDS 5
#define UCMHTP_ROLE_NAME "rname"
#define UCMHTP_ROLE_PERMISSION "permiss"
#define RES_STR_TAGS_LEN 17
#define UCM_HTTP_RESULT_START_TAG "<ResStr>"
#define UCM_HTTP_RESULT_END_TAG "</ResStr>"

#define XML_LESS_THAN_STR "&lt;"
#define XML_GREATER_THAN_STR "&gt;"
#define XML_AMP_STR "&amp;"
#define XML_APOS_STR "&apos;"
#define XML_QUOT_STR "&quot;"

/********************************************************
 * * * * * * G l o b a l V a r i a b l e s  * * * * * * *
 ********************************************************/

char cXMLBuff[UCM_XML_MAX_BUFF_LEN + 1];

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/

char * replaceWithXMLEscapeStr(char *pStr, char *buffer);
 char* ucmStrTok(char *s,char *ct);
 char* ucmStrpBrk(char* cs,char* ct);
 int32_t ucmGetUCMCommandType(char *pCmdStr);

void ucmHttpTokArry(char *str, char *delims,
                      char *resultArry[], int32_t *cnt);
 char  ucmGetUCMDataType(char *chStr);
 void   ucmHttpFillCommandInfo(int32_t iCmd, char *dm_path_p,
                                        int32_t iCount, struct cm_nv_pair *pNvPair, 
                                        struct cm_command *pCmd);
 int32_t ucmHttpGetPrevRecKey(struct cm_array_of_nv_pair *pOutNvPairAry,
                                        struct cm_array_of_nv_pair *pPrevRecKey, 
                                        struct cm_array_of_nv_pair *pKeysArray);
#ifdef UCM_ROLES_PERM_SUPPORT
 void ucmHttpFillRoleInfo(struct cm_array_of_nv_pair *pnv_pair_array,
                                      UCMDM_RolePermission_t *pRolePerm);
  int32_t ucmHttpConvertRoleNvPairToXML(
                                 UCMDM_ArrayOfRolePermissions_t  *pnv_pair_array,
                                 uint32_t uiArrCnt,
                                 char **pXMLOutBuff);
 uint32_t ucmGetPermission(int32_t iPermission);
 int32_t ucmHttpCheckParams(struct cm_array_of_nv_pair *pKeysArray);

#endif /*UCM_ROLES_PERM_SUPPORT*/

/******************************************************************************
 * Function Name : UCMHttpDisplayResult
 * Description   : This API is used to display the Result received from UCM
 *                 JE. 
 * Input params  : pResult - pointer to UCM Result structure
 * Output params : pResStr - Result string 
 * Return value  : NONE
 *****************************************************************************/
void UCMHttpDisplayResult(struct cm_result *pResult, char **pResStr)
{
  int32_t iBufLen = 0;
  int32_t iTotalLen = 0;
  char cResultStr[UCM_XML_MAX_RES_BUFF_LEN + 1];
  char tmpBuf[UCM_XML_MAX_LINE_BUFF_LEN + 1];
  char errBuf[UCM_XML_MAX_LINE_BUFF_LEN + 1];
  char *pResultStr = NULL;

  of_memset(cResultStr, 0, sizeof(cResultStr));

  switch(pResult->result_code)
  {
     case CM_JE_SUCCESS:
        if (!pResult->result.success.reserved)
           break;
        of_memset(tmpBuf, 0, sizeof(tmpBuf));
        cm_je_get_success_msg (pResult->result.success.reserved, tmpBuf);
        pResultStr = (char*)of_calloc(1, UCM_XML_MAX_LINE_BUFF_LEN +1);
        if(pResultStr == NULL)
        {
           UCMHTTPDEBUG("\n%s:: failed to allocate memory for pResultStr \n", 
                 __FUNCTION__);
           return;
        }
        of_strcpy(pResultStr, UCM_HTTP_RESULT_START_TAG);
        of_strcat(pResultStr, tmpBuf);
        of_strcat(pResultStr, UCM_HTTP_RESULT_END_TAG);
        iTotalLen = of_strlen(pResultStr);
        *pResStr = (char*)of_calloc(1, iTotalLen + 1);
        if(*pResStr == NULL)
        {
           // Free pResultStr and return.
           UCMHTTPDEBUG("\n%s:: failed to allocate memory for *pResStr\n", 
                 __FUNCTION__);
           if(pResultStr)
              of_free(pResultStr);
           return;
        }
        of_strcpy(*pResStr, pResultStr);
        UCMHTTPDEBUG("\n%s:: pResStr = %s  \n\t", __FUNCTION__, *pResStr);
#if 0 
        switch(pResult->CommandInfo.uiCmdID)    
        {         
           case UCM_CMD_COMPARE_PARAM:
              {
                 /* Display Name-Value pairs for element exists in Config Session and 
                  * that exists in Security Application database */                 
              }  
              break;
           case UCM_CMD_GET_CONFIG_SESSION:
              {
                 /* Display the DMPath, Command and Name-Value pairs for each DMPath
                 */            
              }
              break;
        }     
#endif      
        break;                      
    case CM_JE_FAILURE:

      of_memset(tmpBuf, 0, sizeof(tmpBuf));
      if (pResult->result.error.result_string_p)
      {
        iBufLen = of_strlen(pResult->result.error.result_string_p);
        pResultStr = (char*)of_calloc(1, iBufLen + RES_STR_TAGS_LEN +1);
        if(pResultStr == NULL)
        {
          UCMHTTPDEBUG("\n%s:: failed to allocate memory for pResultStr \n", 
                                                                 __FUNCTION__);
          return;
        }
        of_strcpy(pResultStr, UCM_HTTP_RESULT_START_TAG);
        of_strcat(pResultStr, pResult->result.error.result_string_p);
        of_strcat(pResultStr, UCM_HTTP_RESULT_END_TAG);
      }
      /* Display Error Name Value Pair */
      if (pResult->result.error.error_code)
      {
        of_memset(tmpBuf, 0, sizeof(tmpBuf));
        of_memset(errBuf, 0, sizeof(errBuf));
        cm_je_get_err_msg (pResult->result.error.error_code, errBuf);
        sprintf (tmpBuf, "<ErrMsg>%s</ErrMsg>",errBuf);
        of_strcat (cResultStr, tmpBuf);

        /* Display DM Path Length  */
        if (pResult->result.error.dm_path_p)
        {
          of_memset(tmpBuf, 0, sizeof(tmpBuf));
          sprintf (tmpBuf, " <DMPath>%s</DMPath>", 
                                      pResult->result.error.dm_path_p);
          of_strcat (cResultStr, tmpBuf);
        }

        if (pResult->result.error.nv_pair.name_p)
        {
          of_memset(tmpBuf, 0, sizeof(tmpBuf));
          sprintf (tmpBuf, "<NvPair name=\"%s\"  value=\"%s\" />",
                     pResult->result.error.nv_pair.name_p,
                     (char*)pResult->result.error.nv_pair.value_p);
          of_strcat (cResultStr, tmpBuf);
        }
      }
  
      iBufLen = of_strlen(cResultStr);
      if(pResultStr)
      {
        iTotalLen = of_strlen(pResultStr);
      }
      iTotalLen += iBufLen;

      *pResStr = (char*)of_calloc(1, iTotalLen + 1);
      if(*pResStr == NULL)
      {
        // Free pResultStr and return.
        UCMHTTPDEBUG("\n%s:: failed to allocate memory for *pResStr\n", 
                                                              __FUNCTION__);
        if(pResultStr)
          of_free(pResultStr);
        return;
      }
      
      if(pResultStr)
      {
        of_strcpy(*pResStr, pResultStr);
        of_strcat(*pResStr, cResultStr);
      }
      else 
      {
        of_strcpy(*pResStr, cResultStr);
      }
      UCMHTTPDEBUG("\n%s:: pResStr = %s  \n\t", __FUNCTION__, *pResStr);
      break;
    default:
      break;
  }
  if(pResultStr)
    of_free(pResultStr);
}
/**********************************************************************************
 * Function Name:ucmStrTok
 * 
 * Description -A strtok which returns empty strings, too
 * input: The string to be searched
 *        The characters to search for
 * output:1.)Null if not match found
 *        2.)Pointer pointing to the matching position in
 *********************************************************************************/

  char* ucmStrTok(char *s, char *ct)
{
  char *sbegin=NULL;
  char *send=NULL;
  static char *ssave = NULL;

  sbegin  = s ? s : ssave;
  if (!sbegin)
  {
     return NULL;
  }
  if (*sbegin == '\0')
  {
    ssave = NULL;
    return NULL;
  }
  send =ucmStrpBrk(sbegin, ct);
  if (send && *send != '\0')
  {
    *send++ = '\0';
  }
  ssave = send;
  return sbegin;
}
/**********************************************************************************
 * Function Name: ucmStrpbrk                                                      *
 * Description - Find the first occurrence of a set of characters              *
 * input:@cs: The string to be searched                        *      
 *       @ct: The characters to search for                                     *  
 * output:1.)Null if not match found                                              *
 *        2.)Pointer pointing to the matching position in @cs                     *
 *********************************************************************************/
 char* ucmStrpBrk(char* cs,char* ct)
{
  char *sc1=NULL;
  char *sc2=NULL;
  for( sc1 = cs; *sc1 != '\0'; ++sc1) 
  {
    for( sc2 = ct; *sc2 != '\0'; ++sc2)
    {
      if (*sc1 == *sc2)
      {    
        return (char *) sc1;
    }
    }
  }
  return NULL;
}
#ifdef UCM_ROLES_PERM_SUPPORT
/******************************************************************************
 * Function Name : ucmGetPermission
 * Description   : This API takes the input iPermission and returns the 
 *                 corresponding UCM Permission Data type.
 * Input params  : chStr - input string
 * Output params : none 
 * Return value  : UCM permission type
 *****************************************************************************/
 uint32_t ucmGetPermission(int32_t iPermission)
{
  UCMHTTPDEBUG("ucmGetPermission :: iPermission =%d\n\r", iPermission);
  switch(iPermission)
  {
    case 0: 
      return UCM_PERMISSION_NOACCESS;
    case 1:
      return UCM_PERMISSION_READ_ONLY;
    case 2:
      return UCM_PERMISSION_READ_WRITE;
    case 3: 
      return UCM_PERMISSION_READ_ROLEPERM;
    case 4: 
      return UCM_PERMISSION_READ_WRITE_ROLEPERM;
  }
}
#endif /*UCM_ROLES_PERM_SUPPORT*/
/******************************************************************************
 * Function Name : ucmGetUCMCommandType
 * Description   : This API takes the input string and returns the 
 *                 corresponding UCM Command type.
 * Input params  : pCmdStr - input string
 * Output params : none 
 * Return value  : Command type
 *****************************************************************************/
 int32_t ucmGetUCMCommandType(char *pCmdStr)
{
  if(pCmdStr == NULL)
  {
    return 0;
  } 
  if((of_strcmp(pCmdStr, "ADD")) == 0 )
  {
    return CM_CMD_ADD_PARAMS;
  }
  else if(of_strcmp(pCmdStr, "EDIT") == 0)
  {
    return CM_CMD_SET_PARAMS;
  }
  else if(of_strcmp(pCmdStr, "DEL") == 0)
  {
    return CM_CMD_DEL_PARAMS;
  }
  else if(of_strcmp(pCmdStr, "DONE") == 0)
  {
    return CM_CMD_EXEC_TRANS_CMD;
  }
  else if(of_strcmp(pCmdStr, "CANCEL") == 0)
  {
    return CM_CMD_CANCEL_TRANS_CMD;
  }
#ifdef UCM_STATS_COLLECTOR_SUPPORT
  else if(of_strcmp(pCmdStr, "AGGR") == 0)
  {
    return UCM_CMD_GET_AGGREGATE_STATS;
  }
  else if(of_strcmp(pCmdStr, "AVG") == 0)
  {
    return UCM_CMD_GET_AVERAGE_STATS;
  }
  else if(of_strcmp(pCmdStr, "PERDEV") == 0)
  {
    return UCM_CMD_GET_PER_DEVICE_STATS;
  }
#endif
  else 
  {
    return 0;
  }
}
/******************************************************************************
 * Function Name : ucmGetUCMDataType
 * Description   : This API takes the input string and returns the 
 *                 corresponding UCM Data type.
 * Input params  : chStr - input string
 * Output params : none 
 * Return value  : Data type
 *****************************************************************************/
 char ucmGetUCMDataType(char *chStr)
{
  if(chStr == NULL)
  {
    return CM_DATA_TYPE_UNKNOWN;
  } 

  /*if((of_strcmp(chStr, "STR")) == 0 ||(of_strcmp(chStr, "IPADDR") == 0))*/
  if((of_strcmp(chStr, "STR")) == 0 )
  {
    return CM_DATA_TYPE_STRING;
  }
  else if(of_strcmp(chStr, "IPADDR") == 0)
  {
    return CM_DATA_TYPE_IPADDR;
  }
  else if(of_strcmp(chStr, "INT") == 0)
  {
    return CM_DATA_TYPE_INT;
  }
  else if(of_strcmp(chStr, "UINT") == 0)
  {
    return CM_DATA_TYPE_UINT;
  }
  else if(of_strcmp(chStr, "BOOL") == 0)
  {
    return CM_DATA_TYPE_BOOLEAN;
  }
  else if(of_strcmp(chStr, "DTTM") == 0)
  {
    return CM_DATA_TYPE_DATETIME;
  }
  else if(of_strcmp(chStr, "BASE64") == 0)
  {
    return CM_DATA_TYPE_BASE64;
  }
  else
  {
    return CM_DATA_TYPE_UNKNOWN;
  }

}
#ifdef UCM_ROLES_PERM_SUPPORT
/******************************************************************************
 * Function Name : ucmHttpConvertRoleNvPairToXML
 * Description   : This API is used to convert the input Name Value Pairs to 
 *                 XML output buffer.
 * Input params  : pnv_pair_array - Pointer to struct cm_array_of_nv_pair structure 
 * Output params : pXMLOutBuff - output XML string buffer
 * Return value  : OF_SUCCESS / OF_FAILURE
 *****************************************************************************/
int32_t ucmHttpConvertRoleNvPairToXML(
                                 UCMDM_ArrayOfRolePermissions_t  *pnv_pair_array,
                                 uint32_t uiArrCnt,
                                 char **pXMLOutBuff)
{
  int32_t i=0;
  int32_t j=0;
  int32_t bufLen = 0;
  char tmpBuf[UCM_XML_MAX_LINE_BUFF_LEN + 1];

  of_memset(tmpBuf, 0, sizeof(tmpBuf));

  UCMHTTPDEBUG("\n uiArrCnt = %u \n", uiArrCnt);

  /*for (i=0; i < uiArrCnt; i++)
  {
    of_strcat(cXMLBuff, "<record>\n"); */

    for (j=0; j < pnv_pair_array[i].count_ui; j++)
    {
        of_strcat(cXMLBuff, "<record>\n");
        sprintf(tmpBuf, "<param name=\"%s\" value=\"%s\" />\n",
                  UCMHTP_ROLE_NAME,
                  pnv_pair_array[i].RolePermissions[j].ucRole);
        of_strcat(cXMLBuff,tmpBuf);
        UCMHTTPDEBUG("tmpBuf at i=%d is  = %s\n", j, tmpBuf);
        of_memset(tmpBuf, 0, sizeof(tmpBuf));

        sprintf(tmpBuf, "<param name=\"%s\" value=\"%d\" />\n",
                  UCMHTP_ROLE_PERMISSION,
                  pnv_pair_array[i].RolePermissions[j].uiPermissions);
        of_strcat(cXMLBuff,tmpBuf);
        UCMHTTPDEBUG("tmpBuf at i=%d is  = %s\n", j, tmpBuf);
        of_memset(tmpBuf, 0, sizeof(tmpBuf));
        of_strcat(cXMLBuff, "</record>\n"); 
    }
    
   /* of_strcat(cXMLBuff, "</record>\n");
  }*/
  bufLen = of_strlen(cXMLBuff);
  *pXMLOutBuff = (char*)of_calloc(1, bufLen + 1);

  of_memcpy(*pXMLOutBuff,cXMLBuff , bufLen);

  return OF_SUCCESS;
}
#endif /*UCM_ROLES_PERM_SUPPORT*/

/******************************************************************************
 * Function Name : replaceWithXMLEscapeStr
 * Description   : This API is used to convert the input Value to XML string.
 * Input params  : pStr - input string 
 *
 * Return value  : String replaced with XML escape characters.
 *****************************************************************************/
char * replaceWithXMLEscapeStr(char *pStr, char *buffer)
{
  //char buffer[4096];
  char  *buf;
  int32_t i, plen =0;
  int32_t len = 0;

  of_memset(buffer, 0, sizeof(buffer));
  buf = buffer;
  plen = strlen(pStr);

  for(i = 0; i < plen; i++)
  {
    switch(pStr[i])
    {
      case '<':
        len = strlen(XML_LESS_THAN_STR);
        memcpy(buf, XML_LESS_THAN_STR, len);
        buf+=len;
        break;
      case '>': 
        len = strlen(XML_GREATER_THAN_STR);
        memcpy(buf, XML_GREATER_THAN_STR, len);
        buf+=len;
        break;
      case '&':
        len = strlen(XML_AMP_STR);
        memcpy(buf, XML_AMP_STR, len);
        buf+=len;
        break;
      case '\'':
        len = strlen(XML_APOS_STR);
        memcpy(buf, XML_APOS_STR, len);
        buf+=len;
        break;
      case '\"':
        len = strlen(XML_QUOT_STR);
        memcpy(buf, XML_QUOT_STR, len);
        buf+=len;
        break;
      default:
        len = 1;
        memcpy(buf, &pStr[i], len);
        buf+=len;
    } /* switch */
  }
  return buffer;
} /* replaceWithXMLEscapeStr */

/******************************************************************************
 * Function Name : ucmHttpConvertNvPairToXML
 * Description   : This API is used to convert the input Name Value Pairs to 
 *                 XML output buffer.
 * Input params  : pnv_pair_array - Pointer to struct cm_array_of_nv_pair structure 
 * Output params : pXMLOutBuff - output XML string buffer
 * Return value  : OF_SUCCESS / OF_FAILURE
 *****************************************************************************/
int32_t ucmHttpConvertNvPairToXML(struct cm_array_of_nv_pair  *pnv_pair_array,
                                  uint32_t uiArrCnt,
                                  char **pXMLOutBuff,
                                  int32_t bFinal)
{
  int32_t i=0;
  int32_t j=0;
  int32_t bufLen = 0;
  char tmpBuf[UCM_XML_MAX_LINE_BUFF_LEN + 1];
  char *pTmp=NULL;
  char buffer[4096];

  of_memset(tmpBuf, 0, sizeof(tmpBuf));

  for (i=0; i < uiArrCnt; i++)
  {
    of_strcat(cXMLBuff, "<record>\n"); 

    for (j=0; j < pnv_pair_array[i].count_ui; j++)
    {
        of_memset(buffer, 0, sizeof(buffer));
        replaceWithXMLEscapeStr((char *)pnv_pair_array[i].nv_pairs[j].value_p, buffer);
        sprintf(tmpBuf, "<param name=\"%s\" value=\"%s\" />\n",
                           pnv_pair_array[i].nv_pairs[j].name_p, 
                           buffer);
        of_strcat(cXMLBuff,tmpBuf);
        of_memset(tmpBuf, 0, sizeof(tmpBuf));
    }
    
    of_strcat(cXMLBuff, "</record>\n"); 
  }

  if(bFinal == TRUE)
  {
    bufLen = of_strlen(cXMLBuff);
    pTmp = (char*)of_calloc(1, bufLen + 1);
    of_memcpy(pTmp,cXMLBuff , bufLen);
  }

    *pXMLOutBuff =pTmp;
  return OF_SUCCESS;
}

/******************************************************************************
 * Function Name : ucmHttpTokArry 
 * Description   : This API is used to tokenize the input value into chunk of tokens
 * Input params  : str - Input string to tokenize
                   delims - tokenize using the delimeters 
 * Output params : resultArry - tokenized tokens
 * Return value  : none
 *****************************************************************************/
void ucmHttpTokArry(char *str, char *delims,
                      char *resultArry[], int32_t *cnt)
{
  char *result = NULL;
  int32_t ii = 0,x =0;

  result = ucmStrTok( str, delims );

  while( result != NULL ) 
  {
      resultArry[x] = result;
      x++;
      ii++;

      result = ucmStrTok( NULL, delims );
  }

  UCMHTTPDEBUG(" %s()::ii = %d \n", __FUNCTION__, ii);
  for(x = 0; x < ii; x++)
  {
    UCMHTTPDEBUG(" %s()::resultArry = %s \n", __FUNCTION__, resultArry[x]);
  }
  *cnt = ii; 
}
/******************************************************************************
 * Function Name : ucmHttpConvertoUCMCommandsPair
 * Description   : This API is used to convert the input buffer to 
 *                 UCM name value pair.
 * Input params  : pInBuf - input string buffer
                   iCmd - Command
                   delims - delimeters to differentiate array of commands
                   subdelim - sub delimeters to differentiate commands
 * Output params : pCmds - Array of commands
                   pCount - no of commands
 * Return value  : NONE
 *****************************************************************************/
void ucmHttpConvertoUCMCommandsPair(char *pInBuf, int32_t iCmd, 
                                      int32_t *pCount, unsigned char *delims, 
                                      unsigned char *subdelim, struct cm_command **pCmds)
{
  char  *resultArry[100];
  char  *dmOptArry[100];
  char  *resArry[100];
  char  *dm_path_p = NULL;
  char  *pTmdm_path_p = NULL;
  struct cm_command *pUCMCmds = NULL;
  struct cm_array_of_nv_pair nv_pair_array;

  int32_t  ii = 0, jj = 0, cnt = 0, dmcnt = 0;
  int32_t  iDmLen = 0;
  int32_t  iCmdType = 0;
  int32_t  iSubCnt = 0;

  of_memset(resultArry,0, sizeof(resultArry));

  ucmHttpTokArry(pInBuf, (char *) delims, resultArry, &cnt);

  UCMHTTPDEBUG(" \n***Token count cnt= %d  \n\n",cnt);

  while(jj < cnt)
  {
    if(of_strstr(resultArry[jj], "ucdm_" ))
    {
      dmcnt++;
    }
    jj++;
  }

  UCMHTTPDEBUG(" \n***dmcnt= %d  \n\n",dmcnt);
  pUCMCmds = (struct cm_command*)of_calloc(dmcnt, sizeof(struct cm_command));
  if(pUCMCmds == NULL)
  {
    UCMHTTPDEBUG("\n%s:: Failed to allocate memory for pUCMCmds\n",
                                                      __FUNCTION__);
    return;
  }

  jj = 0;

  while(ii < cnt)
  {
    of_memset(resArry,0, sizeof(resArry));
    if(resultArry[ii] != NULL)
    {
      if(of_strstr(resultArry[ii], "ucdm_" ))
      {
        if(of_strchr(resultArry[ii],'!'))
        {
          of_memset(dmOptArry,0, sizeof(dmOptArry));
          ucmHttpTokArry(resultArry[ii], "!", dmOptArry, &iSubCnt);
          if(iSubCnt == 2)
          {
            pTmdm_path_p = dmOptArry[0];
            iCmdType = ucmGetUCMCommandType(dmOptArry[1]);
            UCMHTTPDEBUG("\n%s::dmOptArry[0]=%s .\n", __FUNCTION__, dmOptArry[0]);
            UCMHTTPDEBUG("\n%s::dmOptArry[1]=%s .\n", __FUNCTION__, dmOptArry[1]);
          }
        }
        else
        {
          pTmdm_path_p = resultArry[ii];
        }

        iDmLen = of_strlen(pTmdm_path_p);
        dm_path_p = (char*)of_calloc(1, iDmLen +1);
        if(dm_path_p == NULL)
        {
          UCMHTTPDEBUG("\n %s:: Failed to allocate memory for dm_path_p\n",
                                                             __FUNCTION__);
          return;
        }
        /* 5 charecters are for ucdm_*/
        of_strncpy(dm_path_p, pTmdm_path_p+5, iDmLen);
        
        ii++;
        ucmHttpConvertoNameValuePair( resultArry[ii], &nv_pair_array, subdelim);
        if(iCmd == 0)
        {
          ucmHttpFillCommandInfo(iCmdType, dm_path_p,
                         nv_pair_array.count_ui, nv_pair_array.nv_pairs,
                         &pUCMCmds[jj]);
        }
        else
        {
          ucmHttpFillCommandInfo(iCmd, dm_path_p,
                         nv_pair_array.count_ui, nv_pair_array.nv_pairs,
                         &pUCMCmds[jj]);
        }
        jj++;
      }
    }
    ii++;
    UCMHTTPDEBUG("\n%s::resultArry[%d]=%s .\n", __FUNCTION__, ii, resultArry[ii]);
  }
  *pCmds = pUCMCmds;
  *pCount =  dmcnt;
}
/******************************************************************************
 * Function Name : ucmHttpConvertoNameValuePair
 * Description   : This API is used to convert the input buffer to 
 *                 UCM name value pair.
 * Input params  : pInBuf - input string buffer
 * Output params : pnv_pair_array - Pointer to struct cm_array_of_nv_pair structure
 * Return value  : NONE
 *****************************************************************************/
void ucmHttpConvertoNameValuePair( char *pInBuf,
                                     struct cm_array_of_nv_pair *pnv_pair_array,
                                     unsigned char *delims)
{
  struct cm_nv_pair *pTmpNvPair;  
  unsigned char         *resultArry[100];
  unsigned char         *result = NULL;
  unsigned char         *name_p = NULL; // Temporary Name field
  unsigned char         *pType = NULL; // Temporary Type field
  unsigned char         *value_p = NULL; // Temporary Value field

  int32_t          ii = 0, i = 0, x = 0, j = 0;
  uint32_t count = 0;
  uint32_t count_ui = 0;

  UCMHTTPDEBUG("\n ***%s:: pInBuf = %s \r\n",__FUNCTION__, pInBuf);

  result = (unsigned char *) ucmStrTok( pInBuf, (char *) delims );

  while( result != NULL )
  {
     //printf( "value =  \"%s\"\n", result );

     if(i == 2 && (of_strlen((char *) result) > 0))
     {
        count++;
     }
     resultArry[ii] = result;

     result = (unsigned char *) ucmStrTok( NULL, (char *) delims );
     ii++;
     i++;
     if(i>2) i = 0;
  }
  UCMHTTPDEBUG("Num of toks = %d \n ", (ii-1)/3);
  UCMHTTPDEBUG("Num of SETs = %u \n ", count);

  /* Convert to Name Value Pairs */

  pnv_pair_array->count_ui = count;
  pnv_pair_array->nv_pairs = (struct cm_nv_pair *) of_calloc(pnv_pair_array->count_ui,
                                                  sizeof(struct cm_nv_pair));
  if(pnv_pair_array->nv_pairs == NULL)
  {
    UCMHTTPDEBUG("%s :: Memory allocation failed for nv_pairs\n\r", __FUNCTION__);
    return;
  }
  /*for(i = 0; i < ii; x++)
  {
     if(resultArry[i] != NULL)
       UCMHTTPDEBUG("resultArry = %s \n", resultArry[i]);
  }*/
  /* Fill Name-Value pairs */
  pTmpNvPair = pnv_pair_array->nv_pairs;

  i = 0;
  while( i < ii )
  {
     if(x ==0)
     {
        name_p = resultArry[i];
     }
     if(x==1)
     {
       pType = resultArry[i];
     }
     if(x==2)
     {
       value_p = resultArry[i];
       if (Httpd_PostString_Convert(value_p) < 0)
       {
           /* Free all the nv_pairs*/
           count_ui = j;
           CM_FREE_PTR_NVPAIR_ARRAY(pnv_pair_array, count_ui);
           UCMHTTPDEBUG("%s :: String convertion failed\n\r", __FUNCTION__);      
           return;      
       }
       if(resultArry[i] != NULL && of_strlen((char *) resultArry[i]) > 0)
       {
         UCMHTTPDEBUG( "name =  %s \n", name_p);
         UCMHTTPDEBUG( "type =  %s \n", pType);
         UCMHTTPDEBUG( "value =  %s \n\n", value_p);
 
         /* Copy Name Len */
         pTmpNvPair[j].name_length = of_strlen((char *) name_p);

         /* Copy Name */
         pTmpNvPair[j].name_p = (char*)of_calloc(1, 
                                             pTmpNvPair[j].name_length + 1);
         if(pTmpNvPair[j].name_p == NULL)    
         {
           /* Free all the nv_pairs*/
           count_ui = j;
           CM_FREE_PTR_NVPAIR_ARRAY(pnv_pair_array, count_ui);
           UCMHTTPDEBUG("%s :: Memory allocation failed\n\r", __FUNCTION__);      
           return;      
         }
         of_memcpy(pTmpNvPair[j].name_p, name_p, pTmpNvPair[j].name_length);

         /* Copy Value Type  */
         pTmpNvPair[j].value_type = ucmGetUCMDataType((char *) pType);
        
         /* Copy Value Length  */
         pTmpNvPair[j].value_length = of_strlen((char *) value_p);

         /* Copy Value */     
         pTmpNvPair[j].value_p = 
            (char *) of_calloc(1, pTmpNvPair[j].value_length +1);
         if (pTmpNvPair[j].value_p == NULL)
         {
           /* Free all the nv_pairs */
           of_free (pTmpNvPair[j].name_p);
           count_ui = j;
           CM_FREE_PTR_NVPAIR_ARRAY(pnv_pair_array, count_ui);
           UCMHTTPDEBUG("%s :: Memory allocation failed\n\r", __FUNCTION__);      
           return;
         }
         of_memcpy(pTmpNvPair[j].value_p, value_p, pTmpNvPair[j].value_length );
         j++;
       }
     }
     i++;
     x++;
     if(x>2) x = 0;
  }
}
/******************************************************************************
 * Function Name : ucmHttpFillCommandInfo
 * Description   : This API is used to fill the command structure information.
 * Input params  : iCmd - Command ID
 *                 dm_path_p - Pointer to Data model path
 *                 iCount - Name-Value pair count
 *                 pNvPair - Pointer to NvPair(s)
 * Output params : pCmd - Pointer to command structure
 * Return value  : NONE
 *****************************************************************************/
 void ucmHttpFillCommandInfo(int32_t iCmd, char *dm_path_p,
                                        int32_t iCount, struct cm_nv_pair *pNvPair, 
                                        struct cm_command *pCmd)
{
  of_memset(pCmd, 0, sizeof(struct cm_command));
  pCmd->command_id = iCmd;
  pCmd->dm_path_p = dm_path_p;
  pCmd->nv_pair_array.count_ui = iCount;   
  pCmd->nv_pair_array.nv_pairs = pNvPair;   
}
#ifdef UCM_ROLES_PERM_SUPPORT
/******************************************************************************
 * Function Name : ucmHttpFillRoleInfo
 * Description   : This API is used to fill the command structure information.
 * Input params  : pnv_pair_array - name value pairs
 * Output params : pRolePerm - Roles and permissions
 * Return value  : NONE
 *****************************************************************************/
 void ucmHttpFillRoleInfo(struct cm_array_of_nv_pair *pnv_pair_array,
                                      UCMDM_RolePermission_t *pRolePerm)
{
  uint32_t i = 0;
  int32_t iPermissions = 0;
  printf("\n %s:: pnv_pair_array->count_ui=%u\n",__FUNCTION__, pnv_pair_array->count_ui);

  for(i =0; i < pnv_pair_array->count_ui; i++)
  {
    if((pnv_pair_array->nv_pairs) && (pnv_pair_array->nv_pairs[i].name_p))
    {
      if(of_strcmp(pnv_pair_array->nv_pairs[i].name_p, UCMHTP_ROLE_NAME ) == 0)
      {
        of_memset(pRolePerm->ucRole, 0, sizeof(pRolePerm->ucRole));
        of_strcpy(pRolePerm->ucRole, (char*)pnv_pair_array->nv_pairs[i].value_p);
      }
      if(of_strcmp(pnv_pair_array->nv_pairs[i].name_p, UCMHTP_ROLE_PERMISSION) == 0)
      {
        iPermissions = cm_atoi((char*)pnv_pair_array->nv_pairs[i].value_p);
        pRolePerm->uiPermissions = ucmGetPermission(iPermissions);
      }
    }
  }
}
/***************************************************************************
 * Function Name : ucmHttpSetRolePermissions
 * Description   : This callback API is used to set the roles and permissions 
 *                 of a perticular DM node.
 * Input         : pSess - Http Session pointer.
                   aDMPath - Data model path
 * Output        : pResStr- result string
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
int32_t ucmHttpSetRolePermissions(UCMHttpSession_t *pSess,
                                  char aDMPath[],
                                  struct cm_array_of_nv_pair *pnv_pair_array,char **pResStr)
{
  UCMDM_RolePermission_t RolePerm = {};
  struct cm_result *pResult = NULL;

  if((!pSess) || (!pnv_pair_array))
  {
    UCMHTTPDEBUG("Invalid input\n\r");
    return OF_FAILURE;
  }

  ucmHttpFillRoleInfo(pnv_pair_array, &RolePerm);
  UCMHTTPDEBUG("%s:: RolePerm.ucRole=%s \n\r", 
                           __FUNCTION__, RolePerm.ucRole);
  UCMHTTPDEBUG("%s:: RolePerm.uiPermissions=%u \n\r", 
                           __FUNCTION__, RolePerm.uiPermissions);

  if(UCMDM_SetRolePermissionsByRole (pSess->pTransChannel,
                   CM_HTTP_MGMT_ENGINE_ID, aDMPath, 
                   pSess->RoleInfo.admin_role, &RolePerm,&pResult) != OF_SUCCESS)
  {
    UCMHTTPDEBUG("UCMDM_SetRolePermissionsByRole failed\n\r");
    if(pResult)
    {
      UCMHttpDisplayResult(pResult,pResStr);
      UCMFreeUCMResult(pResult);
    }
    return OF_FAILURE;
  }
  UCMHTTPDEBUG("Successfully set the Roles and Permissions.\n\r");
  return OF_SUCCESS;
}

/***************************************************************************
 * Function Name : ucmHttpDisplayRolesettings
 * Description   : This function is used to display role settings and permissions
 * Input         : pSess - pointer to UCMHttpSession_t structure.
 *                 dm_path_p - Data Model path string.
 * Output        : xmlBuf -  Table records in XML format.
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
int32_t ucmHttpDisplayRolesettings(UCMHttpSession_t *pSess,
                                   char *dm_path_p,
                                   char **xmlBuf)
{
  int32_t iRetVal;
  UCMDM_ArrayOfRolePermissions_t *pRolePermAry;
  of_memset(cXMLBuff, 0, sizeof(cXMLBuff));

  if(!pSess)
  {
    UCMHTTPDEBUG("Invalid input\n\r");
    return OF_FAILURE;
  }

  iRetVal = UCMDM_GetRolePermissions (pSess->pTransChannel,
                                  CM_HTTP_MGMT_ENGINE_ID, dm_path_p,
                                  pSess->RoleInfo.admin_role,
                                  &pRolePermAry);
  if(iRetVal == OF_FAILURE)
  {
    UCMHTTPDEBUG("No records exists\n\r");
    return OF_FAILURE;
  }

  if(pRolePermAry && pRolePermAry->count_ui >0)
  {
    ucmHttpConvertRoleNvPairToXML(pRolePermAry, pRolePermAry->count_ui, xmlBuf);
  }

  return OF_SUCCESS;
}
/***************************************************************************
 * Function Name : ucmHttpDisplayInstanceRolesettings
 * Description   : This function is used to display instance level roles and permissions
 * Input         : pSess - pointer to UCMHttpSession_t structure.
 *                 pKeysArray - pointer to struct cm_array_of_nv_pair for get keys.
 *                 dm_path_p - Data Model path string.
 * Output        : xmlBuf -  Table records in XML format.
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
int32_t ucmHttpDisplayInstanceRolesettings(UCMHttpSession_t *pSess,
                                   char *dm_path_p,
                                   char **xmlBuf)
{
  int32_t iRetVal;
  UCMDM_ArrayOfRolePermissions_t *pRolePermAry;
  of_memset(cXMLBuff, 0, sizeof(cXMLBuff));

  if(!pSess)
  {
    UCMHTTPDEBUG("Invalid input\n\r");
    return OF_FAILURE;
  }


  iRetVal = UCMDM_GetInstanceRolePermissions(pSess->pTransChannel,
                                  CM_HTTP_MGMT_ENGINE_ID, dm_path_p,
                                  pSess->RoleInfo.admin_role,
                                  &pRolePermAry);
  if(iRetVal == OF_FAILURE)
  {
    UCMHTTPDEBUG("No records exists\n\r");
    return OF_FAILURE;
  }

  if(pRolePermAry && pRolePermAry->count_ui >0)
  {
    ucmHttpConvertRoleNvPairToXML(pRolePermAry, pRolePermAry->count_ui, xmlBuf);
  }

  return OF_SUCCESS;
}

/***************************************************************************
 * Function Name : ucmHttpDisplayRolesettingsByRole
 * Description   : This function is used to display roles and permissions using a role
 * Input         : pSess - pointer to UCMHttpSession_t structure.
 *                 pKeysArray - pointer to struct cm_array_of_nv_pair for get keys.
 *                 dm_path_p - Data Model path string.
 * Output        : xmlBuf -  Table records in XML format.
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
int32_t ucmHttpDisplayRolesettingsByRole(UCMHttpSession_t *pSess,
                                   char *dm_path_p,
                                   struct cm_array_of_nv_pair *pnv_pair_array,
                                   char **xmlBuf)
{
  int32_t iRetVal;
  UCMDM_RolePermission_t RolePerm = {};
  UCMDM_ArrayOfRolePermissions_t RolePermAry;
  of_memset(cXMLBuff, 0, sizeof(cXMLBuff));

  if((!pSess) || (!pnv_pair_array))
  {
    UCMHTTPDEBUG("Invalid input\n\r");
    return OF_FAILURE;
  }
  ucmHttpFillRoleInfo(pnv_pair_array, &RolePerm);

  UCMHTTPDEBUG("%s:: RolePerm.ucRole=%s \n\r", 
                           __FUNCTION__, RolePerm.ucRole);
  UCMHTTPDEBUG("%s:: RolePerm.uiPermissions=%u \n\r", 
                           __FUNCTION__, RolePerm.uiPermissions);

  iRetVal = UCMDM_GetRolePermissionsByRole(pSess->pTransChannel,
                                  CM_HTTP_MGMT_ENGINE_ID, dm_path_p,
                                  pSess->RoleInfo.admin_role,
                                  &RolePerm);
  if(iRetVal == OF_FAILURE)
  {
    UCMHTTPDEBUG("No records exists\n\r");
    return OF_FAILURE;
  }
  RolePermAry.count_ui = 1;
  RolePermAry.RolePermissions = &RolePerm;

  ucmHttpConvertRoleNvPairToXML(&RolePermAry, RolePermAry.count_ui, xmlBuf);

  return OF_SUCCESS;
}
/***************************************************************************
 * Function Name : ucmHttpDisplayInstRolesettingsByRole
 * Description   : This function is used to display instance level roles and permissions
                  using a role
 * Input         : pSess - pointer to UCMHttpSession_t structure.
 *                 pKeysArray - pointer to struct cm_array_of_nv_pair for get keys.
 *                 dm_path_p - Data Model path string.
 * Output        : xmlBuf -  Table records in XML format.
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
int32_t ucmHttpDisplayInstRolesettingsByRole(UCMHttpSession_t *pSess,
                                   char *dm_path_p,
                                   struct cm_array_of_nv_pair *pnv_pair_array,
                                   char **xmlBuf)
{
  int32_t iRetVal;
  UCMDM_RolePermission_t RolePerm = {};
  UCMDM_ArrayOfRolePermissions_t RolePermAry = {};
  of_memset(cXMLBuff, 0, sizeof(cXMLBuff));

  if((!pSess) || (!pnv_pair_array))
  {
    UCMHTTPDEBUG("Invalid input\n\r");
    return OF_FAILURE;
  }
  ucmHttpFillRoleInfo(pnv_pair_array, &RolePerm);

  UCMHTTPDEBUG("%s:: RolePerm.ucRole=%s \n\r",
                           __FUNCTION__, RolePerm.ucRole);
  UCMHTTPDEBUG("%s:: RolePerm.uiPermissions=%u \n\r", 
                           __FUNCTION__, RolePerm.uiPermissions);

  iRetVal = UCMDM_GetInstanceRolePermissionsByRole(pSess->pTransChannel,
                                  CM_HTTP_MGMT_ENGINE_ID, dm_path_p,
                                  pSess->RoleInfo.admin_role,
                                  &RolePerm);
  if(iRetVal == OF_FAILURE)
  {
    UCMHTTPDEBUG("No records exists\n\r");
    return OF_FAILURE;
  }
  RolePermAry.count_ui = 1;
  RolePermAry.RolePermissions = &RolePerm;

  ucmHttpConvertRoleNvPairToXML(&RolePermAry, RolePermAry.count_ui, xmlBuf);

  return OF_SUCCESS;
}
/***************************************************************************
 * Function Name : ucmHttpSetInstanceRolePermissions
 * Description   : This callback API is used to set the roles and permissions 
 *                 of a perticular DM Instance.
 * Input         : pSess - Http Session pointer.
                   pnv_pair_array - Name value pairs
                   aDMPath - Data model path
 * Output        : none
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
int32_t ucmHttpSetInstanceRolePermissions(UCMHttpSession_t *pSess,
                                           char aDMPath[],
                                           struct cm_array_of_nv_pair *pnv_pair_array)
{
  UCMDM_RolePermission_t RolePerm = {};

  if((!pSess) || (!pnv_pair_array))
  {
    UCMHTTPDEBUG("Invalid input\n\r");
    return OF_FAILURE;
  }

  ucmHttpFillRoleInfo(pnv_pair_array, &RolePerm);
  UCMHTTPDEBUG("%s:: RolePerm.ucRole=%s \n\r", 
                           __FUNCTION__, RolePerm.ucRole);
  UCMHTTPDEBUG("%s:: RolePerm.uiPermissions=%u \n\r", 
                           __FUNCTION__, RolePerm.uiPermissions);

  if(UCMDM_SetInstanceRolePermissionsByRole (pSess->pTransChannel,
                     CM_HTTP_MGMT_ENGINE_ID, aDMPath, 
                           pSess->RoleInfo.admin_role, &RolePerm) != OF_SUCCESS)
  {
    UCMHTTPDEBUG("UCMDM_SetInstanceRolePermissionsByRole failed\n\r");
    return OF_FAILURE;
  }
  UCMHTTPDEBUG("Successfully set the Roles and Permissions.\n\r");
  return OF_SUCCESS;
}
/***************************************************************************
 * Function Name : ucmHttpDelInstanceRolePermissions
 * Description   : This callback API is used to delete Roles and permissions 
 *                 of a perticular DM Instance.
 * Input         : pSess - Http Session pointer.
                   pnv_pair_array - Name value pairs
                   aDMPath - Data model path
 * Output        : none 
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
int32_t ucmHttpDelInstanceRolePermissions(UCMHttpSession_t *pSess,
                                           char aDMPath[],
                                           struct cm_array_of_nv_pair *pnv_pair_array)
{
  UCMDM_RolePermission_t RolePerm = {};

  if((!pSess) || (!pnv_pair_array))
  {
    UCMHTTPDEBUG("Invalid input\n\r");
    return OF_FAILURE;
  }

  ucmHttpFillRoleInfo(pnv_pair_array, &RolePerm);
  UCMHTTPDEBUG("%s:: RolePerm.ucRole=%s \n\r", 
                           __FUNCTION__, RolePerm.ucRole);
  UCMHTTPDEBUG("%s:: RolePerm.uiPermissions=%u \n\r", 
                           __FUNCTION__, RolePerm.uiPermissions);

  if(UCMDM_DeleteInstanceRolePermissionsByRole (pSess->pTransChannel,
                     CM_HTTP_MGMT_ENGINE_ID, aDMPath, 
                           pSess->RoleInfo.admin_role, &RolePerm) != OF_SUCCESS)
  {
    UCMHTTPDEBUG("UCMDM_SetInstanceRolePermissionsByRole failed\n\r");
    return OF_FAILURE;
  }
  UCMHTTPDEBUG("Successfully Delete Roles and Permissions for instance.\n\r");
  return OF_SUCCESS;
}
/***************************************************************************
 * Function Name : ucmHttpDelRolePermissions
 * Description   : This callback API is used to delete the roles and permissions 
 *                 of a perticular DM node.
 * Input         : pSess - Http Session pointer.
                   aDMPath - Data model path
                   pnv_pair_array - Name value pairs
 * Output        : none 
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
int32_t ucmHttpDelRolePermissions(UCMHttpSession_t *pSess, 
                                   char aDMPath[],
                                   struct cm_array_of_nv_pair *pnv_pair_array)
{
  UCMDM_RolePermission_t RolePerm = {};

  if((!pSess) || (!pnv_pair_array))
  {
    UCMHTTPDEBUG("Invalid input\n\r");
    return OF_FAILURE;
  }

  ucmHttpFillRoleInfo(pnv_pair_array, &RolePerm);

  if(UCMDM_DeleteRolePermissionsByRole (pSess->pTransChannel,
                       CM_HTTP_MGMT_ENGINE_ID, aDMPath, 
                       pSess->RoleInfo.admin_role, &RolePerm) != OF_SUCCESS)
  {
    UCMHTTPDEBUG("UCMDM_DeleteRolePermissions failed\n\r");
    return OF_FAILURE;
  }
  UCMHTTPDEBUG("Successfully deleted the Roles and Permissions.\n\r");
  return OF_SUCCESS;
}
#endif /*UCM_ROLES_PERM_SUPPORT*/

/***************************************************************************
 * Function Name : ucmHttpSetFactoryDefaults
 * Description   : This callback API is used to reset the Box to Factory
 *                 defaults.
 * Input         : pSess - Http Session pointer.
                   aDMPath - Data model path
 * Output        : pResStr- result string
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
int32_t ucmHttpSetFactoryDefaults(UCMHttpSession_t *pSess, char aDMPath[], 
                                                          char **pResStr)
{
  struct cm_result *pResult = NULL;
  struct cm_command CommandInfo;

  if (!pSess)
  {
    UCMHTTPDEBUG("Invalid input\n\r");
    return OF_FAILURE;
  }

  of_memset (&CommandInfo, 0, sizeof (CommandInfo));
  CommandInfo.command_id = CM_CMD_RESET_TO_FACTORY_DEFAULTS;
  CommandInfo.dm_path_p = aDMPath;
#ifdef CM_LDSV_SUPPORT
  if (cm_ldsv_factory_reset (pSess->pTransChannel, &CommandInfo, &pResult,
                            CM_HTTP_MGMT_ENGINE_ID,
                            &pSess->RoleInfo) != OF_SUCCESS)
  {
    UCMHTTPDEBUG("cm_ldsv_factory_reset failed\n\r");
    if (pResult)
    {
      UCMHttpDisplayResult(pResult, pResStr);
      UCMFreeUCMResult (pResult);
      return OF_FAILURE;
    }
  }
#endif

  if (pResult)
  {
    UCMFreeUCMResult ( pResult);
  }

  UCMHTTPDEBUG("Successfully reset to Factory Default Configuration.\n\r");
  return OF_SUCCESS;
}

/***************************************************************************
 * Function Name : ucmHttpRebootDevice
 * Description   : This API is used to reboot the device 
 * Input         : pSess - HTTP session pointer.
                   aDMPath - Data model path
 * Output        : pResStr - Result string 
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
int32_t ucmHttpRebootDevice(UCMHttpSession_t *pSess, char aDMPath[], 
                                                   char **pResStr)
{
  struct cm_array_of_nv_pair *ppnv_pair_array = NULL; 
  if(!pSess)
  {
    UCMHTTPDEBUG("Invalid input\n\r");
    return OF_FAILURE;
  }

  if(UCMRebootDevice ( CM_HTTP_MGMT_ENGINE_ID,&pSess->RoleInfo, 
                       pSess->pTransChannel,&ppnv_pair_array)!= OF_SUCCESS)
  {
    UCMHTTPDEBUG("Failed to Reboot the Device \n\r");
    return OF_FAILURE;
  }

  UCMHTTPDEBUG("Successfully rebooted the device.\n\r");
  return OF_SUCCESS;
}

/***************************************************************************
 * Function Name : ucmHttpSaveConfig
 * Description   : This API is used to Save configuration in to a persistent store 
 * Input         : pSess - HTTP session pointer.
                   aDMPath - Data model path
 * Output        : pResStr - Result string
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
int32_t ucmHttpSaveConfig(UCMHttpSession_t *pSess, char aDMPath[], 
                                                   char **pResStr)
{
  struct cm_result *pResult = NULL;
  struct cm_command CommandInfo;
  
  if(!pSess)
  {
    UCMHTTPDEBUG("Invalid input\n\r");
    return OF_FAILURE;
  }

  of_memset(&CommandInfo, 0, sizeof(CommandInfo));
  CommandInfo.command_id =  CM_CMD_SAVE_CONFIG;
  CommandInfo.dm_path_p = aDMPath;

#ifdef CM_LDSV_SUPPORT
  if (cm_ldsv_save_config (pSess->pTransChannel, &CommandInfo, &pResult,
           CM_HTTP_MGMT_ENGINE_ID, &pSess->RoleInfo) != OF_SUCCESS)
  {
      UCMHTTPDEBUG("Failed to save the Configuration\n\r");
      if(pResult) 
      {     
      UCMHttpDisplayResult(pResult, pResStr);
        UCMFreeUCMResult(pResult);
        return OF_FAILURE;      
      }    
    }    
#endif

    if(pResult) 
    {     
      UCMFreeUCMResult(pResult);
    }
  UCMHTTPDEBUG("Successfully saved the Configuration.\n\r");
  return OF_SUCCESS;
}

/******************************************************************************
 * Function Name : ucmHttpFrameAndSendMultCmdsToJE
 * Description   : This API is used to send the Multiple commands request to JE. 
 *                 This API checks for the Config Session, If not exists, Starts 
 *                 the config session at JE and
 *                 push the Name-Value pairs for the Add request to JE.
 * Input params  : pSess - pointer to UCMHttpSession_t structure. 
                   iCount - Command count
                   pCmds - Commands with name value pairs
 * Output params : pResStr - Result string 
 * Return value  : OF_SUCCESS / OF_FAILURE
 *****************************************************************************/
int32_t ucmHttpFrameAndSendMultCmdsToJE(UCMHttpSession_t *pSess,
                                        int32_t iCount, 
                                        struct cm_command *pCmds,
                                        char **pResStr)
{
  UCMHTTPDEBUG("\n INSIDE:%s... \r\n",__FUNCTION__);

  struct cm_result *pResult = NULL;

  /* Start config session if needed */   
  if(pSess->pConfigSession == NULL)
  {
    pSess->pConfigSession = (struct cm_je_config_session *) cm_config_session_start(
                                                         CM_HTTP_MGMT_ENGINE_ID,
                                                         &pSess->RoleInfo,
                                                         pSess->pTransChannel);
    if(pSess->pConfigSession == NULL)
    {
      UCMHTTPDEBUG("%s :: cm_config_session_start failed\n\r", __FUNCTION__);       
      return OF_FAILURE;      
    }                  
  }            
  /* Send to JE using ConfigSessionUpdate */
  if(cm_config_session_update_cmds (pSess->pConfigSession, 
                                 pCmds, iCount, &pResult) != OF_SUCCESS)
  {
    UCMHTTPDEBUG("%s :: cm_config_session_update_cmds failed\n\r", __FUNCTION__);
    if(pResult)
    {
      UCMHttpDisplayResult(pResult,pResStr);
      UCMHTTPDEBUG("\n %s:: pResStr = %s  \n\t", __FUNCTION__, *pResStr);
      UCMFreeUCMResult(pResult);
    }

    return OF_FAILURE;    
  }         

  if(pResult)  
  {
    UCMFreeUCMResult(pResult);

  }              
  return OF_SUCCESS;
}
/******************************************************************************
 * Function Name : ucmHttpFrameAndSendAddParamsToJE
 * Description   : This API is used to send the Add record request to JE. This
 *                 API checks for the Config Session, If not exists, Starts 
 *                 the config session at JE and
 *                 push the Name-Value pairs for the Add request to JE.
 * Input params  : pSess - pointer to UCMHttpSession_t structure. 
                   iCount - Command count
                   pCmds - Commands with name value pairs
 * Output params : pResStr - Result string 
 * Return value  : OF_SUCCESS / OF_FAILURE
 *****************************************************************************/
int32_t ucmHttpFrameAndSendAddParamsToJE( UCMHttpSession_t *pSess,
                                          int32_t iCount, 
                                          struct cm_command *pCmds,
                                          char **pResStr)
{
  UCMHTTPDEBUG("\n INSIDE:ucmHttpFrameAndSendAddParamsToJE... \r\n");

  struct cm_result *pResult = NULL;

  /* Start config session if needed */   
  if(pSess->pConfigSession == NULL)
  {
    pSess->pConfigSession = (struct cm_je_config_session *) cm_config_session_start(
                                                         CM_HTTP_MGMT_ENGINE_ID,
                                                         &pSess->RoleInfo,
                                                         pSess->pTransChannel);
    if(pSess->pConfigSession == NULL)
    {
      UCMHTTPDEBUG("%s :: cm_config_session_start failed\n\r", __FUNCTION__);       
      return OF_FAILURE;      
    }                  
  }            
  /* Send to JE using ConfigSessionUpdate */
  if(cm_config_session_update_cmds (pSess->pConfigSession, 
                                 pCmds, iCount, &pResult) != OF_SUCCESS)
  {
    UCMHTTPDEBUG("%s :: cm_config_session_update_cmds failed\n\r", __FUNCTION__);
    if(pResult)
    {
      UCMHttpDisplayResult(pResult,pResStr);
      UCMHTTPDEBUG("\n %s:: pResStr = %s  \n\t", __FUNCTION__, *pResStr);
      UCMFreeUCMResult(pResult);
    }

    return OF_FAILURE;    
  }         

  if(pResult)  
  {
    UCMFreeUCMResult(pResult);

  }              
  return OF_SUCCESS;
}

/******************************************************************************
 * Function Name : ucmHttpFrameAndSendSetParamsToJE
 * Description   : This API is used to send the Edit record request to JE. This
 *                 API checks for the Config Session, If not exists, Starts 
 *                 the config session at JE and
 *                 push the Name-Value pairs for the Edit request to JE.
 * Input params  : pSess - pointer to UCMHttpSession_t structure. 
                   iCount - Command count
                   pCmds - Commands with name value pairs
 * Output params : pResStr - Result string 
 * Return value  : OF_SUCCESS / OF_FAILURE
 *****************************************************************************/
int32_t ucmHttpFrameAndSendSetParamsToJE( UCMHttpSession_t *pSess,
                                          int32_t iCount, 
                                          struct cm_command *pCmds,
                                          char **pResStr)
{
  UCMHTTPDEBUG("\n INSIDE:ucmHttpFrameAndSendSetParamsToJE... \r\n");

  struct cm_result *pResult;

  /* Start config session if needed */   
  if(pSess->pConfigSession == NULL)
  {
    pSess->pConfigSession = (struct cm_je_config_session *) cm_config_session_start(
                                                         CM_HTTP_MGMT_ENGINE_ID,
                                                         &pSess->RoleInfo,
                                                         pSess->pTransChannel);
    if(pSess->pConfigSession == NULL)
    {
      UCMHTTPDEBUG("%s :: cm_config_session_start failed\n\r", __FUNCTION__);
      return OF_FAILURE;      
    }                  
  }            
  /* Send to JE using ConfigSessionUpdate */
  if(cm_config_session_update_cmds (pSess->pConfigSession,
                                 pCmds, iCount, &pResult) != OF_SUCCESS)
  {
    UCMHTTPDEBUG("%s :: cm_config_session_update_cmd failed\n\r", __FUNCTION__);
    if(pResult)
    {
      UCMHttpDisplayResult(pResult, pResStr);
      UCMFreeUCMResult(pResult);
    }

    return OF_FAILURE;    
  }         

  if(pResult)  
  {
    UCMFreeUCMResult(pResult);
  }              
  return OF_SUCCESS;
}

/******************************************************************************
 * Function Name : ucmHttpSendDeletParamsToJE
 * Description   : This API is used to send the Delete record request to JE.
 *                 This API checks for the Config Session, If not exists, 
 *                 Starts the config session at JE and
 *                 push the Name-Value pairs for the Delete request to JE.
 * Input params  : pSess - pointer to UCMHttpSession_t structure. 
                   iCount - Command count
                   pCmds - Commands with name value pairs
 * Output params : pResStr - Result string 
 * Return value  : OF_SUCCESS / OF_FAILURE
 *****************************************************************************/
int32_t ucmHttpSendDeletParamsToJE( UCMHttpSession_t *pSess,
                                          int32_t iCount, 
                                          struct cm_command *pCmds,
                                          char **pResStr)
{
  UCMHTTPDEBUG("\n INSIDE:ucmHttpSendDeletParamsToJE... \r\n");

  struct cm_result *pResult;

  /* Start config session if needed */   
  if(pSess->pConfigSession == NULL)
  {
    pSess->pConfigSession = (struct cm_je_config_session *) cm_config_session_start(
                                                         CM_HTTP_MGMT_ENGINE_ID,
                                                         &pSess->RoleInfo,
                                                         pSess->pTransChannel);
    if(pSess->pConfigSession == NULL)
    {
      UCMHTTPDEBUG("%s :: cm_config_session_start failed\n\r", __FUNCTION__);       
      return OF_FAILURE;      
    }                  
  }            

  /* Send to JE using ConfigSessionUpdate */
  if(cm_config_session_update_cmds (pSess->pConfigSession,
                                 pCmds, iCount, &pResult) != OF_SUCCESS)
  {
    UCMHTTPDEBUG("%s :: cm_config_session_update_cmd failed\n\r", __FUNCTION__);
    if(pResult)
    {
      UCMHttpDisplayResult(pResult, pResStr);
      UCMFreeUCMResult(pResult);
    }

    return OF_FAILURE;    
  }         

  if(pResult)  
  {
    UCMFreeUCMResult(pResult);
  }              
  return OF_SUCCESS;
}

/***************************************************************************
 * Function Name : ucmHttpDisplayTableNextNRecords
 * Description   : This function displays the next N table records.
 * Input         : pSess - pointer to UCMHttpSession_t structure.
 *                 pKeysArray - pointer to struct cm_array_of_nv_pair for get keys.
 *                 pPrmKeysArray - pointer to struct cm_array_of_nv_pair for get keys.
 *                 dm_path_p - Data Model path string.
 *                 recCount - record count to be fetched.
 * Output        : xmlBuf -  Table records in XML format.
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
int32_t ucmHttpDisplayTableNextNRecords(UCMHttpSession_t *pSess,
                                   struct cm_array_of_nv_pair *pKeysArray,
                                   struct cm_array_of_nv_pair *pPrmKeysArray,
                                   char *dm_path_p, int32_t recCount,
                                   char **xmlBuf)
{

  UCMHttpSession_t *pTmpSess;
  struct cm_array_of_nv_pair  *pOutArrayofNvPairArr = NULL;
  struct cm_array_of_nv_pair *pKeyParmsArray = NULL;
  struct cm_array_of_nv_pair  PrevRecKey;

  uint32_t count_ui = 1;
  uint32_t iRecCnt = 0;
  int32_t iRetVal;

  pTmpSess = pSess;
  of_memset(&PrevRecKey,0,sizeof(struct cm_array_of_nv_pair));

  if(!dm_path_p || !pKeysArray || !pKeysArray->nv_pairs || !pKeysArray->nv_pairs->name_p)
  {
    UCMHTTPDEBUG("Invalid arguments: Key parameters.\n\r");
    return OF_FAILURE;
  }

  if(recCount > 0 )
  {
    count_ui = recCount;
  }
  UCMHTTPDEBUG("pKeysArray->count_ui= %d\n\r", pKeysArray->count_ui);

  of_memset(cXMLBuff, 0, sizeof(cXMLBuff));

  if((!pPrmKeysArray) || (pPrmKeysArray->count_ui == 0) 
        || (ucmHttpCheckParams(pPrmKeysArray)!=OF_SUCCESS))
  {
    //pKeyParmsArray = &TmpKeysArray;
    pKeyParmsArray = pKeysArray;
  }
  else
  {
    pKeyParmsArray = pPrmKeysArray;
  }

  iRetVal =  cm_get_next_n_records (pTmpSess->pTransChannel,
                                 CM_HTTP_MGMT_ENGINE_ID,
                                 pTmpSess->RoleInfo.admin_role,
                                 dm_path_p, pKeysArray,
                                 pKeyParmsArray, &count_ui,
                                 &pOutArrayofNvPairArr);
  if(iRetVal == OF_FAILURE)
  {
    UCMHTTPDEBUG("No records exists\n\r");
    return OF_FAILURE;
  }
  if(count_ui > 0)
  {
    ucmHttpConvertNvPairToXML(pOutArrayofNvPairArr, count_ui, xmlBuf, FALSE);
  }

  while((iRecCnt < UCM_MAX_RECORDS) && (count_ui > 0))
  {    
       if (ucmHttpGetPrevRecKey(&pOutArrayofNvPairArr[count_ui -1],    
                                  &PrevRecKey, pKeysArray) != OF_SUCCESS)
       {    
         break;    
       }
       CM_FREE_PTR_NVPAIR_ARRAY(pOutArrayofNvPairArr, count_ui);
       pOutArrayofNvPairArr = NULL;    
       
       iRetVal =  cm_get_next_n_records (pTmpSess->pTransChannel,
                                      CM_HTTP_MGMT_ENGINE_ID,
                                      pTmpSess->RoleInfo.admin_role,
                                      dm_path_p, &PrevRecKey,
                                      pKeyParmsArray, &count_ui,
                                      &pOutArrayofNvPairArr);

       if((iRetVal == OF_FAILURE) || (count_ui == 0))
       {    
         if(pOutArrayofNvPairArr && pOutArrayofNvPairArr->count_ui > 0)
               {
           CM_FREE_PTR_NVPAIR_ARRAY(pOutArrayofNvPairArr, count_ui);
           pOutArrayofNvPairArr = NULL;    //CID 574
               }
         break;
       }
       if(PrevRecKey.count_ui > 0)
           CM_FREE_NVPAIR_ARRAY (PrevRecKey, PrevRecKey.count_ui); 
       
       ucmHttpConvertNvPairToXML(pOutArrayofNvPairArr, count_ui, xmlBuf,FALSE);
       iRecCnt++;
  }
 
  ucmHttpConvertNvPairToXML(pOutArrayofNvPairArr,0, xmlBuf,TRUE);

  return OF_SUCCESS;
}

int32_t ucmHttpCheckParams(struct cm_array_of_nv_pair *pKeysArray)
{
  int32_t i = 0;

  for (i = 0; i < pKeysArray->count_ui; i++)
  {
    if(of_strcmp((char*)pKeysArray->nv_pairs[i].value_p, "nil") == 0)
    {
      return OF_FAILURE;
    }
  }
  return OF_SUCCESS;
}

#ifdef UCM_STATS_COLLECTOR_SUPPORT
/***************************************************************************
 * Function Name : ucmHttpDisplayStats
 * Description   : This function displays the statistics records. 
 * Input         : pSess - pointer to UCMHttpSession_t structure.
 *                 dm_path_p - Data Model path string.
 *                 uiSubCmd - Type of the Sub commnad (aggr/avg/perdev) 
                   pKeyArrNvPair - Name value paris
 * Output        : xmlBuf -  Statistics records in XML format.
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
int32_t ucmHttpDisplayStats (UCMHttpSession_t * pSess,
                             char *dm_path_p,  struct cm_array_of_nv_pair *pKeyArrNvPair, 
                             uint32_t uiSubCmd,
                             char **xmlBuf)
{
  struct cm_array_of_nv_pair  *pOutArrayofNvPairArr = NULL;
  uint32_t uiRecCount =0, iRetVal=OF_FAILURE, count_ui=0;
  //struct cm_array_of_nv_pair KeyArrNvPair;
  UCMHttpSession_t *pTmpSess = pSess;
  of_memset(cXMLBuff, 0, sizeof(cXMLBuff));

  //of_memset (&KeyArrNvPair,0,sizeof(struct cm_array_of_nv_pair));
  iRetVal = UCMGetStatsRecords (pTmpSess->pTransChannel,
                               CM_HTTP_MGMT_ENGINE_ID,
                               pTmpSess->RoleInfo.admin_role,
                               dm_path_p, uiSubCmd, pKeyArrNvPair,
                               &uiRecCount, &pOutArrayofNvPairArr);
  count_ui=uiRecCount;
  if(iRetVal == OF_SUCCESS)
  {
    ucmHttpConvertNvPairToXML(pOutArrayofNvPairArr, uiRecCount, xmlBuf, TRUE);
    CM_FREE_PTR_NVPAIR_ARRAY(pOutArrayofNvPairArr, count_ui);
    return OF_SUCCESS;
  }
  return OF_FAILURE;
}
#endif
/***************************************************************************
 * Function Name : ucmHttpDisplayTableRecords
 * Description   : This function displays the table records. 
 * Input         : pSess - pointer to UCMHttpSession_t structure.
 *                 pKeysArray - pointer to struct cm_array_of_nv_pair for get keys. 
 *                              For filter keys.
 *                 pDMKeysArray - pointer to struct cm_array_of_nv_pair for get keys. 
 *                              For DM keys.
 *                 dm_path_p - Data Model path string.
 *                 bExact - boolean variable to specify request type is
 *                          GetExact or not.
 * Output        : xmlBuf -  Table records in XML format.
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
int32_t ucmHttpDisplayTableRecords(UCMHttpSession_t *pSess,
                                              struct cm_array_of_nv_pair *pKeysArray,
                                              struct cm_array_of_nv_pair *pDMKeysArray,
                                              char *dm_path_p, int32_t bExact,
                                              char **xmlBuf)
{

  UCMHttpSession_t *pTmpSess = pSess;
  struct cm_array_of_nv_pair  *pOutArrayofNvPairArr = NULL;
  struct cm_array_of_nv_pair PrevRecKey;
  struct cm_array_of_nv_pair TmpKeysArray;
  struct cm_array_of_nv_pair *pKeyParmsArray = NULL;


  uint32_t count_ui = UCM_HTTP_MAX_RECORDS_PER_PAGE;
  int32_t iRetVal =0;
  int32_t iRecCnt =0;
  of_memset(&PrevRecKey,0,sizeof(struct cm_array_of_nv_pair));

  TmpKeysArray.count_ui = 0;
  TmpKeysArray.nv_pairs =NULL;


  if(dm_path_p == NULL)
  {
    UCMHTTPDEBUG("Invalid arguments: Key parameters.\n\r");
    return OF_FAILURE;
  }                 
  of_memset(cXMLBuff, 0, sizeof(cXMLBuff));

  if(bExact == TRUE)
  {
    iRetVal = cm_get_exact_record (pTmpSess->pTransChannel,
                                 CM_HTTP_MGMT_ENGINE_ID,
                                 pTmpSess->RoleInfo.admin_role,
                                 dm_path_p,pKeysArray,
                                 &pOutArrayofNvPairArr);
    if(iRetVal == OF_FAILURE)
    {
      UCMHTTPDEBUG("Failed to extract Record from Database\n\r");
      return OF_FAILURE;      
    }    
    ucmHttpConvertNvPairToXML(pOutArrayofNvPairArr,1 ,xmlBuf,TRUE);
  }
  else
  {
    if((!pKeysArray) || (pKeysArray->count_ui == 0) 
          || (ucmHttpCheckParams(pKeysArray)!=OF_SUCCESS))
    {
      pKeyParmsArray = &TmpKeysArray;
    }
    else
    {
      pKeyParmsArray = pKeysArray;
    }

    iRetVal = cm_get_first_n_records (pTmpSess->pTransChannel,
                                   CM_HTTP_MGMT_ENGINE_ID,
                                   pTmpSess->RoleInfo.admin_role,
                                   dm_path_p, pKeyParmsArray,
                                   &count_ui, &pOutArrayofNvPairArr);
    if(iRetVal == OF_FAILURE || count_ui < 1)
    {
      UCMHTTPDEBUG("No records exists\n\r");
      return OF_FAILURE;
    }

    if(count_ui > 0)
    {
      ucmHttpConvertNvPairToXML(pOutArrayofNvPairArr, count_ui, xmlBuf, FALSE);
    }
    /* Fill Previous Keys Array of nv_pairs and get Next set of records. */    
     while(iRecCnt < UCM_MAX_RECORDS)
     {    
       if (ucmHttpGetPrevRecKey(&pOutArrayofNvPairArr[count_ui -1],    
                                  &PrevRecKey, pDMKeysArray) != OF_SUCCESS)
       {
         break;    
       }
       CM_FREE_PTR_NVPAIR_ARRAY(pOutArrayofNvPairArr, count_ui);    
       pOutArrayofNvPairArr = NULL;    
                    
       iRetVal =  cm_get_next_n_records (pTmpSess->pTransChannel,
                                      CM_HTTP_MGMT_ENGINE_ID,
                                      pTmpSess->RoleInfo.admin_role,
                                      dm_path_p, &PrevRecKey,
                                      pKeysArray, &count_ui,
                                      &pOutArrayofNvPairArr);
       if((iRetVal == OF_FAILURE) || (count_ui == 0))
       {    
         if(pOutArrayofNvPairArr && pOutArrayofNvPairArr->count_ui > 0)
               { 
           CM_FREE_PTR_NVPAIR_ARRAY(pOutArrayofNvPairArr, count_ui);
           pOutArrayofNvPairArr = NULL;    //CID 575
               } 
         break;
       }
       if(PrevRecKey.count_ui > 0)
         CM_FREE_NVPAIR_ARRAY (PrevRecKey, PrevRecKey.count_ui);
       
       ucmHttpConvertNvPairToXML(pOutArrayofNvPairArr, count_ui, xmlBuf,FALSE);
       iRecCnt++;
      }
      ucmHttpConvertNvPairToXML(pOutArrayofNvPairArr,0, xmlBuf,TRUE);
  }
  return OF_SUCCESS;
}

/******************************************************************************
 * Function Name : ucmHttpDoneCmd
 * Description   : This API is used to send DONE  command to JE 
 * Input params  : pSess - Http Session
                   aDMPath - Data model path
 * Output params : pResStr - Result string
 * Return value  : OF_SUCCESS / OF_FAILURE
 *****************************************************************************/
int32_t ucmHttpDoneCmd (UCMHttpSession_t *pSess, char aDMPath[],
                                                 char **pResStr)
{
  struct cm_command CommandInfo;
  struct cm_result *pResult;

  if (!pSess)
  {
    UCMHTTPDEBUG ("Invalid input\n\r");
    return OF_FAILURE;
  }

  if (!pSess->pConfigSession)
  {
    UCMHTTPDEBUG ("No Config Session exists\n\r");
    return OF_FAILURE;
  }

  of_memset (&CommandInfo, 0, sizeof (CommandInfo));
  ucmHttpFillCommandInfo (CM_CMD_EXEC_TRANS_CMD, aDMPath,
                         0, NULL,
                         &CommandInfo);

  /* Send to JE using ConfigSessionUpdate */
  if (cm_config_session_update_cmd (pSess->pConfigSession, &CommandInfo, &pResult)
                                                                  != OF_SUCCESS)
  {
    UCMHTTPDEBUG("%s :: cm_config_session_update_cmd failed\n\r", __FUNCTION__);
    if(pResult)
    {
      UCMHttpDisplayResult(pResult, pResStr);
      UCMFreeUCMResult(pResult);
    }

    return OF_FAILURE;    
  }         
  if (pResult)
  {
    UCMFreeUCMResult (pResult);
  }
  return OF_SUCCESS;
}

/******************************************************************************
 * Function Name : ucmHttpCancelCmd
 * Description   : This callback API is used to send a Cancel command to
 *                 JE, to undo the config commands of the given context.
 * Input params  : pSess - pointer to config session
                   aDMPath - Data model path
 * Output params : pResStr - Result string
 * Return value  : OF_SUCCESS / OF_FAILURE
 *****************************************************************************/
int32_t ucmHttpCancelCmd (UCMHttpSession_t *pSess, char aDMPath[],
                                                   char **pResStr)
{
  struct cm_command CommandInfo;
  struct cm_result *pResult;

  if (!pSess)
  {
    return OF_FAILURE;
  }

  if (!pSess->pConfigSession)
  {
    UCMHTTPDEBUG ("No Config Session exists\n\r");
    return OF_FAILURE;
  }

  of_memset (&CommandInfo, 0, sizeof (CommandInfo));

  ucmHttpFillCommandInfo (CM_CMD_CANCEL_TRANS_CMD, aDMPath,
                         0, NULL,
                         &CommandInfo);

  /* Send to JE using ConfigSessionUpdate */
  if (cm_config_session_update_cmd (pSess->pConfigSession, &CommandInfo, &pResult)
                                                                  != OF_SUCCESS)
  {
    UCMHTTPDEBUG("%s :: cm_config_session_update_cmd failed\n\r", __FUNCTION__);
    if(pResult)
    {
      UCMHttpDisplayResult(pResult, pResStr);
      UCMFreeUCMResult(pResult);
    }

    return OF_FAILURE;    
  }         
  if (pResult)
  {
    UCMFreeUCMResult (pResult);
  }
  UCMHTTPDEBUG("Command is nullified\n\r");

  return OF_SUCCESS;
}

/******************************************************************************
 * Function Name : ucmHttpCommitConfigSession
 * Description   : This API is used to inform JE to COMMIT all the statements
 *                 exsits in the configuration session.
 * Input params  : pSess - Http Session
 * Output params : pResStr - Result string
 * Return value  : OF_SUCCESS / OF_FAILURE
 *****************************************************************************/
int32_t ucmHttpCommitConfigSession(UCMHttpSession_t *pSess,
                                          char **pResStr)
{
  struct cm_result *pResult = NULL;
  int32_t iRetVal;
  
  if(!pSess)
  {
    UCMHTTPDEBUG("%s :: Invalid input\n\r", __FUNCTION__);
    return OF_FAILURE;
  }

  if(pSess->pConfigSession == NULL)
  {
    UCMHTTPDEBUG("No Config Session exists\n\r");
    return OF_FAILURE;
  }

  /*Call Config Session end with OPCode as COMMIT */
  iRetVal = cm_config_session_end(pSess->pConfigSession,
                                CM_CMD_CONFIG_SESSION_COMMIT,
                                &pResult);
  if(iRetVal == OF_FAILURE)
  {
    UCMHTTPDEBUG("%s :: cm_config_session_end failed\n\r", __FUNCTION__);
    /* Free the Result structure */    
    if(pResult)
    {       
      UCMHttpDisplayResult(pResult, pResStr);
      UCMFreeUCMResult(pResult);
    }      
    return OF_FAILURE;
  }
  
  UCMHttpDisplayResult(pResult, pResStr);
  pSess->pConfigSession = NULL;
  if(pResult) 
  {     
    UCMFreeUCMResult(pResult);
  }    

  UCMHTTPDEBUG("Committed Successfully\n\r");

  return OF_SUCCESS;
}

/******************************************************************************
 * Function Name : ucmHttpRevokeConfigSession
 * Description   : This callback API is used to send a Revocation command to
 *                 JE, to revoke the config session and not to execute the
 *                 commands exists in the config session. 
 * Input params  : pSess - pointer to config session
 * Output params : NONE
 * Return value  : OF_SUCCESS / OF_FAILURE
 *****************************************************************************/
int32_t ucmHttpRevokeConfigSession(UCMHttpSession_t *pSess)
{
  struct cm_result *pResult = NULL;
  int32_t iRetVal;

  if(!pSess)
  {
    UCMHTTPDEBUG("Invalid input\n\r");
    return OF_FAILURE;
  }

  if(!pSess->pConfigSession)
  {
    UCMHTTPDEBUG("\n Config Session does not exists\n\r");
    return OF_FAILURE;
  }

  /* invoke End Config session */
  iRetVal = cm_config_session_end(pSess->pConfigSession,
                                CM_CMD_CONFIG_SESSION_REVOKE,
                                &pResult);
  if(iRetVal != OF_SUCCESS)
  {
    if(pResult)    
    {
      UCMFreeUCMResult(pResult);
    }
    return OF_FAILURE;
  }
  pSess->pConfigSession = NULL;
  
  /* Free UCM result structure */
  if(pResult)  
  {     
    UCMFreeUCMResult(pResult);
  }    

  UCMHTTPDEBUG("\t Successfully Revoked the session.\n\r");
  return OF_SUCCESS;
}

/***************************************************************************
 * Function Name : ucmHttpGetPrevRecKey
 * Description   : This function populates the previous key value pairs
 *                 to retrive the next set of records.
 * Input         : pOutNvPairAry - pointer to struct cm_array_of_nv_pair structure
 *                                 from prevous output list of records.
 *                 pKeysArray - pointer to struct cm_array_of_nv_pair structure.
 *                 
 * Output        : pPrevRecKey - pointer to previous key Name value pairs.
 * Return value  : OF_SUCCESS / OF_FAILURE
 ***************************************************************************/
 int32_t ucmHttpGetPrevRecKey(struct cm_array_of_nv_pair *pOutNvPairAry,
                                        struct cm_array_of_nv_pair *pPrevRecKey, 
                                        struct cm_array_of_nv_pair *pKeysArray)
{
  int32_t ii;
  int32_t iKeyCount = 0;
  int32_t iNvCnt;

  if(!(pKeysArray->nv_pairs && pKeysArray->nv_pairs[0].name_p))
  {
    return OF_FAILURE;
  }

  pPrevRecKey->count_ui = pKeysArray->count_ui;
  pPrevRecKey->nv_pairs= 
      (struct cm_nv_pair *) of_calloc (pKeysArray->count_ui, sizeof(struct cm_nv_pair));
  if(!pPrevRecKey->nv_pairs)
  {
    return OF_FAILURE;
  }
  for(ii =0; ii< pOutNvPairAry->count_ui; ii++)
  {
    for (iNvCnt = 0; iNvCnt < pKeysArray->count_ui; iNvCnt++)
    {
     if(pOutNvPairAry->nv_pairs[ii].name_p)
     {
      if(of_strcmp(pOutNvPairAry->nv_pairs[ii].name_p, pKeysArray->nv_pairs[iNvCnt].name_p) == 0)
      {
        pPrevRecKey->nv_pairs[iNvCnt].name_length = pOutNvPairAry->nv_pairs[ii].name_length;
        pPrevRecKey->nv_pairs[iNvCnt].name_p = (char*)of_calloc(1, pOutNvPairAry->nv_pairs[ii].name_length + 1);
        if(pPrevRecKey->nv_pairs[iNvCnt].name_p)
        {
          of_strncpy(pPrevRecKey->nv_pairs[iNvCnt].name_p, pOutNvPairAry->nv_pairs[ii].name_p,
                           pOutNvPairAry->nv_pairs[ii].name_length);
        }
        pPrevRecKey->nv_pairs[iNvCnt].value_type = pOutNvPairAry->nv_pairs[ii].value_type;
        pPrevRecKey->nv_pairs[iNvCnt].value_length = pOutNvPairAry->nv_pairs[ii].value_length;
        pPrevRecKey->nv_pairs[iNvCnt].value_p = (char*)of_calloc(1, pOutNvPairAry->nv_pairs[ii].value_length + 1);
        if(pPrevRecKey->nv_pairs[iNvCnt].value_p)
        {
          of_strncpy(pPrevRecKey->nv_pairs[iNvCnt].value_p, pOutNvPairAry->nv_pairs[ii].value_p,
                    pOutNvPairAry->nv_pairs[ii].value_length);
        }
        iKeyCount++;
      }
     }
    }
  }
  if(iKeyCount == pKeysArray->count_ui) {
    return OF_SUCCESS;
  }
  else
  {
    return OF_FAILURE;
  }
}
#endif /*OF_CM_SUPPORT */
#endif /*OF_HTTPD_SUPPORT */
