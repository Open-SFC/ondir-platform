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
/*  File        : httperr.c                                               */
/*                                                                        */
/*  Description : This file contains the functions that are used to give  */
/*                the error information                                   */
/*                                                                        */
/*  Version      Date          Author      Change Description             */
/*  -------    --------        ------     ---------------------           */
/*   1.0      23-07-2004     K.RamaRao    Initial Implimentation          */
/**************************************************************************/

#ifdef OF_HTTPD_SUPPORT

#include "cmincld.h"
#include "htpercod.h"
#include "httpgdef.h"
#include "httpgif.h"

unsigned char *HttpErrMsgs[ ] = 
{
  (unsigned char *) " ",
/* Error messages start from here */
(unsigned char *)   "Unable to allocate memory",
(unsigned char *)   "Port already configured",
(unsigned char *)   "Maximum servers configured for the given server type reached",
(unsigned char *)   "Inavlid command recieved",
 (unsigned char *)  "Inavlid Application Type",
  (unsigned char *) "Invalid Argument recieved",
  (unsigned char *) "Port already in enable state",
  (unsigned char *) "Port already in disable state",
  (unsigned char *) "Port not found",
  (unsigned char *) "No Port configured",
  (unsigned char *) "This is last port",
  (unsigned char *) "Unable create socket",
  (unsigned char *) "unable to bind",
  (unsigned char *) "Unable to Listen",
  (unsigned char *) "Unable to send data on loop back socket",
  (unsigned char *) "Unable to recieve data on loop back socket",
  (unsigned char *) "Invalid server type",
  (unsigned char *) "Invalid port number",
  (unsigned char *) "Unable to open pseudo driver",
  (unsigned char *) "Server type change is not allowed",
  (unsigned char *) "Port number change is not allowed",
  (unsigned char *) "Nothing to modify",
  (unsigned char *) "Cannot delete/disable all the ports, atleast one port must be enabled",
/* Error messages end here*/
  (unsigned char *) "  "
};

/***************************************************************************
 * Function Name  : HttpGetErrorMsg
 * Description    : This API will get the error message from the error
 *                  messages structure. This API will get the error message
 *                  for the given error code. Cli or Http to display the 
 *                  appropriate error message for the error code returned
 *                  by the core functions will call this API.
 * Input          : uiErrCode - Error Code
 * Output         : pMsg - The string to hold the error message corresponding
 *                  to the error code
 * Return value   : If the error code is valid and pMsg is not NULL then 
 *                  OF_SUCCESS wil be returned else OF_FAILURE.
 ***************************************************************************/
int32_t HttpGetErrorMsg (uint32_t uiErrCode, char *pErrMsg)
{
  /*
   * Check whether uiErrCode falls under the registered range,
   * if not return OF_FAILURE
   */
  if((uiErrCode <= HTTP_ERR_START) || (uiErrCode >= HTTP_ERR_MAX))
  {
#ifdef HTTPD_DEBUG
    HttpDebug(("Invalid Error index recieved. ErrCode = %ld\n\r",uiErrCode));
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }
  
  /*Check whether pErrMsg is NULL*/
  if(pErrMsg == NULL)
  {
#ifdef HTTPD_DEBUG
    HttpDebug(("pMsg is NULL\n\r"));
#endif /*HTTPD_DEBUG*/
    return OF_FAILURE;
  }

  /* Copy the error message for the given error index */
  sprintf(pErrMsg,"%s\r\n",HttpErrMsgs[uiErrCode - HTTP_ERR_START]);
  return OF_SUCCESS;
}


/***************************************************************************
 * Function Name  : HttpErrorMsgInit
 * Description    : This function will initializes the error messages. If
 *                  number of error messages defined in the HttpErrMsgs array 
 *                  and the error codes defined are same then it sets the
 *                  iSize variable to the number of error messages else to -1
 * Input          : None
 * Output         : None
 * Return value   : OF_FAILURE if the number of error messages are not same
 *                  as the number of error codes.
 ***************************************************************************/
int32_t HttpErrorMsgInit(void)
{
   int32_t iErrIndex;

   /* Count number of messages defined in the HttpErrMsgs array */
   for(iErrIndex = 0; HttpErrMsgs[iErrIndex] != NULL; iErrIndex++);

   /**
    * Check whether number of messages = no. of error codes
    */
   if((iErrIndex - 1) != (HTTP_ERR_MAX - HTTP_ERR_START) )
   {
     printf("Failed to initialize Http Error messages\n\r");
     return OF_FAILURE;
   }
   
   return OF_SUCCESS;
}

#endif
