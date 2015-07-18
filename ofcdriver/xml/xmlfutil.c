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
 *  File         : xmlfutil.c                                                *
 *                                                                           *
 *  Description  : XML File handler for iGateway handler.                    *
 *                                                                           *
 *  Version      Date        Author         Change Description               *
 *  -------    --------      ------        ----------------------            *
 *   1.0       03/09/05                Initial implementation                *
 ****************************************************************************/

#ifdef OF_XML_SUPPORT

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/
/*#include "pspdummy.h"*/
//#include <psp/psp.h>
#include <cntrlxml.h>
#include <stdio.h>
#include "xmlfutil.h"
#include "ixmlparser.h"
#include "oftype.h"
#include "cntlr_error.h"

/*********************************************************
*   CNTRLXMLReadFile
*       read a xml file or buffer contents into xml parser.
*       Internal to parser only.
*
**********************************************************/

int32_t CNTRLXMLReadFile(char *xmlFileName, char **dataBuffer)
{
    int fileSize = 0;
    int bytesRead = 0;
    FILE *xmlFilePtr = NULL;


    printf("%s:%d xml file = %s \r\n",__FUNCTION__,__LINE__,xmlFileName);

    xmlFilePtr = fopen( xmlFileName, "rb" );
    if( xmlFilePtr == NULL ) {
            perror("fopen: error - \r\n");
            printf("%s:%d \r\n",__FUNCTION__,__LINE__);
        return IXML_NO_SUCH_FILE;
    } else {
        fseek( xmlFilePtr, 0, SEEK_END );
        fileSize = ftell( xmlFilePtr );
        if( fileSize == 0 ) {
            printf("%s:%d \r\n",__FUNCTION__,__LINE__);
            fclose( xmlFilePtr );
            return IXML_SYNTAX_ERR;
        }

        *dataBuffer = ( char * )malloc( fileSize + 1 );
        if( *dataBuffer == NULL ) {
            fclose( xmlFilePtr );
            printf("%s:%d \r\n",__FUNCTION__,__LINE__);
            return IXML_INSUFFICIENT_MEMORY;
        }

        fseek( xmlFilePtr, 0, SEEK_SET );
        bytesRead =
           fread( *dataBuffer, 1, fileSize, xmlFilePtr );
        (*dataBuffer)[bytesRead] = '\0';    /* append null*/
        fclose( xmlFilePtr );
    }
	return PSP_SUCCESS;
}

#endif /*PSP_XML_SUPPORT*/

