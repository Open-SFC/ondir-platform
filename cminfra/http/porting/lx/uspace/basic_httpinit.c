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
/*  File        : httpmain.c                                              */
/*                                                                        */
/*  Description : This file contains lxpc system specific functions       */
/*                related to the http server.                             */
/*                                                                        */
/*  Version      Date      Author      Change Description                 */
/*  -------    --------    ------    ----------------------               */
/*    1.0      00.00.00    kmdhar    Initial implementation               */
/*    1.1      05.20.04     dram     Modified during the code review.     */
/**************************************************************************/

#ifdef OF_HTTPD_SUPPORT

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/

#include "cmincld.h"
#include "htpdglb.h"
#include <signal.h>
#include <stdio.h>
#include "dirent.h"
#include "httpwrp.h"


#ifndef OF_CM_SUPPORT
#include "algcrgif.h"
#include "hcmgrmbl.h"
#include "hcmgrgdf.h"
#include "hcrtgif.h"
#include "hcmgrgif.h"
#include "snetgif.h"
#include "iprshttp.h"
#include "lxoswrp.h"
#endif /*OF_CM_SUPPORT*/

#define HTTPD_USE_INBUILT_LOGO 1 
bool bIsLogoFile = TRUE;
#if defined(INTOTO_DIAG_HTTP_SUPPORT) && defined(INTOTO_DIAG_SUPPORT)
#include "diaggdef.h"  
#include "diaggif.h"
#endif

#ifdef MESGCLNT_SUPPORT
extern void IGWHTTPDMessageIdsInit(void);
#endif
#if defined INTOTO_UAM_SUPPORT && defined INTOTO_UAM_HTTP_SUPPORT
extern void IGWUAMErrInit();
#endif

#ifdef IGWLIM_XMLAPI_SUPPORT
extern int32_t  IGWLimXMLInit(void );
#endif /*IGWLIM_XMLAPI_SUPPORT*/

HttpdListenPorts_t HttpdListenPorts;

/****************************************************
 * * * *  M a c r o    D e f i n i t i o n s  * * * *
 ****************************************************/

#define  PG_DRV                       "/dev/pgdrv"

#ifdef INTOTO_UCM_HTTP_PLUGIN_SUPPORT

#define UCM_HTTPPLUGIN_LIB_PATH "/igateway/ucm/lib/htplugin"
#define UCM_PAUTHPLUGIN_LIB_PATH "/igateway/ucm/lib/authplugin"

#endif /* INTOTO_UCM_HTTP_PLUGIN_SUPPORT */

/************************************************************
 * * * *  V a r i a b l e    D e c l a r a t i o n s  * * * *
 ************************************************************/

/********************************************************
 * * * *  F u n c t i o n    P r o t o t y p e s  * * * *
 ********************************************************/


extern
/*int32_t CreateHttpServer( void );*/

void HttpMain_SigHandler( int32_t  lSigNum );

void HttpMain_RegSigHandler( void );

#ifdef INTOTO_UCM_HTTP_PLUGIN_SUPPORT
int32_t UCMHttpPluginInit(void);
#endif /* INTOTO_UCM_HTTP_PLUGIN_SUPPORT */

#ifdef INTOTO_UCM_AUTH_PLUGIN_SUPPORT
int32_t UCMHttpAuthPluginInit(void);
#endif


void IGWSNetHttpInit( void );

void IGWNetworkGroupHttpInit( void );

int32_t HttpMain_Init( void );

#ifdef INTOTO_XML_SUPPORT
  void IGWXmlHttpInit( void );
#endif /* INTOTO_XML_SUPPORT */

void IGWAppsHttpInit( void );


extern
int32_t StaticIPTagsInit( void );


#if defined(INTOTO_8021X_SUPPORT) && defined(INTOTO_8021X_HTTP_SUPPORT)
extern
int32_t IGW8021xHttp_RegLocalTags( void );
#endif /*INTOTO_8021X_SUPPORT && INTOTO_8021X_HTTP_SUPPORT*/

extern
int32_t AliasTagsInit( void );


extern int32_t IGWDlOpen(char *ppath, char *fname);

 int32_t  HttpdValidateArguments (
                                                int32_t   argc,
                                                char*  argv[],
                                                bool*  pbDaemonize
                                               );

uint32_t uiOpaqueIndex;


/******************************************************************
 * * * *  F u n c t i o n s    I m p l e m e n t a t i o n  * * * *
 ******************************************************************/

/**************************************************************************
 * Function Name : HttpMain_Init                                          *
 * Description   : Initializing functions before http server starts.      *
 * Input         : None                                                   *
 * Output        : None                                                   *
 * Return value  : OF_SUCCESS on success                                   *
 *                 OF_FAILURE on failure                                   *
 **************************************************************************/

int32_t HttpMain_Init( void )
{

/**
   By setting the following variable to false we are serving logoicon.gif from 
   C array and not from file.
  */
  bIsLogoFile=TRUE;

  return OF_SUCCESS;
} /* HttpMain_Init() */

/*****************************************************************************/
/* Function        : HttpdValidateArguments                              */
/* Description     : This API is provided to validate the arguments passed to*/
/*                   HTTP Server                                       */
/* Input Arguments :                                                         */
/*    argc           - number of arguments                                   */
/*    argv[]         - array of arguments                                    */
/* Output Arguments:                                                         */
/*    pbDaemonize    - pointer to value stored for bDaemonize                */
/* Return Value    : OF_SUCCESS/OF_FAILURE                                     */
/*****************************************************************************/
 int32_t  HttpdValidateArguments (
                                            int32_t   argc,
                                            char*  argv[],
                                            bool*  pbDaemonize
                                           )
{
  int32_t   ii = 0;
  bool   bClearPortSupplied = FALSE, bSecurePortSupplied = FALSE;
  bool   bDaemonize = TRUE;

  if (argc > 6)
  {
#ifdef IGW_HTTP_DEBUG
    printf ("Excess parameters passed from command line\n");
#endif
    return (OF_FAILURE);
  }

  /*
   *Read the arguments and set the respective variable.
   */

  for (ii = 1; ii < argc; ii++)
  {
    if (bClearPortSupplied == TRUE)
    {
#ifndef OF_CM_SUPPORT
      if (ValidateNumber(argv[ii],IGW_UNSIGNED_INT16) != OF_SUCCESS)
      {
#ifdef IGW_HTTP_DEBUG
        printf ("Invalid parameters passed from command line\n");
#endif
        return (OF_FAILURE);
      }
#endif /*OF_CM_SUPPORT*/
      HttpdListenPorts.usClearPort = of_atoi (argv[ii]);
      bClearPortSupplied = FALSE;
    }
    else if (bSecurePortSupplied == TRUE)
    {
#ifndef OF_CM_SUPPORT
      if (ValidateNumber(argv[ii],IGW_UNSIGNED_INT16) != OF_SUCCESS)
      {
#ifdef IGW_HTTP_DEBUG
        printf ("Invalid parameters passed from command line\n");
#endif
        return (OF_FAILURE);
      }
#endif /*OF_CM_SUPPORT*/

      HttpdListenPorts.usSecurePort = of_atoi (argv[ii]);
      bSecurePortSupplied = FALSE;
    }
    else if (of_strcmp(argv[ii], "-f") == 0)
    {
      bDaemonize = FALSE;
    }
    else if (of_strcmp(argv[ii], "-cp") == 0)
    {
      bClearPortSupplied = TRUE;
    }
    else if (of_strcmp(argv[ii], "-sp") == 0)
    {
      bSecurePortSupplied = TRUE;
    }
  }

  *pbDaemonize = bDaemonize;
  return (OF_SUCCESS);
}

/**************************************************************************
 * Function Name : main                                                   *
 * Description   : Main Function Of Http Daemon                           *
 * Input         : argc    Command-line arg count                         *
 *                 argv    Array of command-line args                     *
 * Output        : None                                                   *
 * Return value  : 0 on success                                           *
 *                 A positive value ( 1 through 5 ) on error              *
 **************************************************************************/

int32_t main( int32_t  argc,
              char  **argv )
{
  bool   bDaemonInit = TRUE;
  HttpGlobals_t *pHttpg=NULL;
  /**
   * Validate the number of arguments.
   */
  if (argc > 6)
  {
    Httpd_Print("Excess parameters passed from command line\r\n");
    return -1;
  }

  HttpdListenPorts.iClearSocketFd = 0;
  HttpdListenPorts.iSecureSocketFd = 0;
  HttpdListenPorts.usClearPort = 0;
  HttpdListenPorts.usSecurePort = 0;
   
  if (HttpdValidateArguments(argc, argv, &bDaemonInit) != OF_SUCCESS)
  {
#ifdef IGW_HTTP_DEBUG
    printf ("Invalid parameters passed from command line\n");
#endif
    return -1;
  }


  /**
   * Based on the arguments given call the daemon_init function.
   */
  if (bDaemonInit == TRUE)
  {
    daemon_init();
  }

  /**
   * Open the pseudo driver.
   */
/*  if (CmnWrp_Init() != OF_SUCCESS)
  {
    Httpd_Print("http: unable to open pseudo driver\n");
    exit(1);
  }*/

  /**
   * Register the various signals with Signal Handler.
   */
  HttpMain_RegSigHandler();

  /**
   * Initialize some of the prerequisites.
   */
  if (HttpMain_Init() != OF_SUCCESS)
  {
    Httpd_Print("HttpMain_Init Failed\n");
    exit(1);
  }
  /*printf("%s:HttpMain_Init success\n",__FUNCTION__);*/
  /**
   * Initialize the http server resources.
   */
  if (Httpd_Init() != OF_SUCCESS)
  {
    Httpd_Error("Unable To Initialize Http Server. Exiting...\n");
    exit(2);
  }

#ifdef OF_HTTPS_SUPPORT
  /**
   * Initialize the https certificates.
   */
  if (HTTPSCertsInit() != OF_SUCCESS)
  {
    Httpd_Print("http: unable to HTTPS Certs Initialisation\n");
    exit(1);
  }
#endif /* OF_HTTPS_SUPPORT */
  pHttpg=pHTTPG;
  if(!pHttpg)
  {
    Httpd_Print("pHttpg is NULL\r\n");	
    exit(1);
  }

  if(HttpReqOpaqueInit(pHttpg) != OF_SUCCESS)
  {
     Httpd_Print("http: HttpReqOpaqueInit\n");
     exit(1);
  }

  /***Register for the Opaque data in the Http Request ***/
  if(IGWHttpRegisterforOpaqueIndex(pHttpg,&uiOpaqueIndex,of_free) != OF_SUCCESS)
  {
     Httpd_Print("http: IGWHttpRegisterforOpaqueIndex Failed \n");
     exit(1);
  }


#ifdef HTTP_REDIRECT_HTTPS
  /**
   * Check for the https certificates.
   */
  Httpd_Check4Certificate();
#endif /* HTTP_REDIRECT_HTTPS */

  HttpsLdSvInit();

#ifdef INTOTO_UCM_AUTH_PLUGIN_SUPPORT
  UCMHttpAuthPluginInit();
#endif



#ifdef INTOTO_UCM_HTTP_PLUGIN_SUPPORT
  UCMHttpPluginInit();
#endif /* INTOTO_UCM_HTTP_PLUGIN_SUPPORT */
  /**
   * Start the http server.
   */
  if (Httpd_Start(pHttpg) != OF_SUCCESS)
  {
    Httpd_Print("Unable To Start Http Server\n");
    exit(3);
  }

  return OF_SUCCESS;
} /* main() */

/**************************************************************************
* Description   :  					  		  *
*             .                                                           *
* Input         : None.                                                   *
* Output        : 				                          *             
*Return value  : 							  *	
***************************************************************************/
uint32_t  Httpd_GetInBuitLogoStatus( HttpGlobals_t  *pHttpGlobals)
{
    return HTTPD_USE_INBUILT_LOGO;
}/*Httpd_GetInBuitLogoStatus*/


/**************************************************************************
 * Function Name : HttpMain_SigHandler                                    *
 * Description   : Signal Handler. This ignores all ignorable signals.    *
 * Input         : lSigNum (I) - Signal Number                            *
 * Output        : None                                                   *
 * Return value  : None                                                   *
 **************************************************************************/

void HttpMain_SigHandler( int32_t  lSigNum )
{
  switch (lSigNum)
  {
  case SIGHUP:
  case SIGINT:
  case SIGTERM:
  case SIGPIPE:
    signal(lSigNum,HttpMain_SigHandler);
    return; /* Ignore */

  case SIGBUS:
  case SIGQUIT:
  case SIGABRT:
  case SIGSEGV:
    sleep(1);
    exit(5);
  }
} /* HttpMain_SigHandler() */

/**************************************************************************
 * Function Name : HttpMain_RegSigHandler                                 *
 * Description   : Registers signal handler for various signals.          *
 * Input         : None                                                   *
 * Output        : None                                                   *
 * Return value  : None                                                   *
 **************************************************************************/

void HttpMain_RegSigHandler( void )
{

  signal(SIGBUS,  HttpMain_SigHandler);
  signal(SIGINT,  HttpMain_SigHandler);
  signal(SIGQUIT, HttpMain_SigHandler);
  signal(SIGILL,  HttpMain_SigHandler);
  signal(SIGABRT, HttpMain_SigHandler);
  signal(SIGSEGV, HttpMain_SigHandler);
  signal(SIGCHLD, HttpMain_SigHandler);
  signal(SIGHUP,  HttpMain_SigHandler);
  signal(SIGTERM, HttpMain_SigHandler);
  signal(SIGPIPE, HttpMain_SigHandler);
} /* HttpMain_RegSigHandler() */

#ifdef INTOTO_UCM_AUTH_PLUGIN_SUPPORT

/**************************************************************************
 * Function Name : UCMHttpAuthPluginInit                                  *
 * Description   : This API is used to initialize user authentication     *
                   plugins.
 * Input         : None                                                   *
 * Output        : None                                                   *
 * Return value  : OF_FAILURE/OF_SUCCESS                                    *
 **************************************************************************/
int32_t UCMHttpAuthPluginInit(void)
{
   int32_t lRetVal=OF_FAILURE;
   DIR *pDir =NULL;
   struct dirent *dp;
   char  path[64]={'\0'};

   of_strcpy(path,UCM_PAUTHPLUGIN_LIB_PATH);
  
   pDir = opendir(path);
   if(!pDir)
   {
#ifdef HTTPD_DEBUG
     printf("%s():diropen for %s failed\n\r",__FUNCTION__,path);
#endif
     return OF_FAILURE;
   }

   while((dp = readdir(pDir))!= NULL)
   { 
     printf("%s: filename=%s\n",__FUNCTION__,dp->d_name);
#ifdef HTTPD_DEBUG
     printf("%s: filename=%s\n",__FUNCTION__,dp->d_name);
#endif
     if(!(strcmp(dp->d_name,"."))||!(strcmp(dp->d_name,"..")))
     {
       continue;
     }

     sprintf (path,
             "%s/%s",UCM_PAUTHPLUGIN_LIB_PATH,dp->d_name);

#ifdef HTTPD_DEBUG
     printf("%s(): path:%s\n\r",__FUNCTION__,path);
#endif
   
     if((lRetVal =  UCMDlOpen(path,"UCMAuthInit"))!=OF_SUCCESS)
     {
#ifdef HTTPD_DEBUG
       printf("%s:failed \n",__FUNCTION__);
#endif
       closedir(pDir);
       exit(1);
     }  
   }
   closedir(pDir);
   return OF_SUCCESS;
}
#endif /* INTOTO_UCM_AUTH_PLUGIN_SUPPORT */

#ifdef INTOTO_UCM_HTTP_PLUGIN_SUPPORT

/**************************************************************************
 * Function Name : UCMHttpPluginInit                                      *
 * Description   : This API is used to initialize ucm HTTP plugins        *
 * Input         : None                                                   *
 * Output        : None                                                   *
 * Return value  : OF_FAILURE/OF_SUCCESS                                    *
 **************************************************************************/
int32_t UCMHttpPluginInit(void)
{
   int32_t lRetVal=OF_FAILURE;
   DIR *pDir =NULL;
   struct dirent *dp;
   char  path[64]={'\0'};

   of_strcpy(path,UCM_HTTPPLUGIN_LIB_PATH);
  
   pDir = opendir(path);
   if(!pDir)
   {
#ifdef HTTPD_DEBUG
     printf("%s():diropen for %s failed\n\r",__FUNCTION__,path);
#endif
     return OF_FAILURE;
   }

   while((dp = readdir(pDir))!= NULL)
   { 
#ifdef HTTPD_DEBUG
     printf("%s: filename=%s\n",__FUNCTION__,dp->d_name);
#endif
     if(!(strcmp(dp->d_name,"."))||!(strcmp(dp->d_name,"..")))
     {
       continue;
     }

     sprintf (path,
             "%s/%s",UCM_HTTPPLUGIN_LIB_PATH,dp->d_name);

#ifdef HTTPD_DEBUG
     printf("%s(): path:%s\n\r",__FUNCTION__,path);
#endif
   
     if((lRetVal =  UCMDlOpen(path,"UCMHttp_Init"))!=OF_SUCCESS)
     {
#ifdef HTTPD_DEBUG
       printf("%s:failed \n",__FUNCTION__);
#endif
       closedir(pDir);
       exit(1);
     }  
   }
   closedir(pDir);
   return OF_SUCCESS;
}
#endif /* INTOTO_UCM_HTTP_PLUGIN_SUPPORT */

#else

int32_t main( void )
{
  return OF_SUCCESS;
}

#endif /* OF_HTTPD_SUPPORT */
