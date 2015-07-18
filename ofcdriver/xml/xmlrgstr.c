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
 *  File         : xmlrgstr.c                                                *
 *                                                                           *
 *  Description  : XML handler for iGateway handler.                         *
 *                                                                           *
 *  Version      Date        Author         Change Description               *
 *  -------    --------      ------        ----------------------            *
 *   1.0       14/06/05       sbala         Initial implementation           *
 ****************************************************************************/
#ifdef OF_XML_SUPPORT

/***********************************************************
 * * * *  I n c l u d e    H e a d e r    f i l e s  * * * *
 ***********************************************************/
 #include "cntrlxml.h"
// #include "pspdummy.h"
 //#include "pspcmn.h"

#ifdef PSP_XML_TEST_SUPPORT
 void PSPXmlNTGRPHttpInit(void);
#endif /* PSP_XML_TEST_SUPPORT */
#if 0
#ifdef PSP_XML_TEST_SUPPORT
 void PSPXmlNetworkGroupHttpInit(void);
#endif /* PSP_XML_TEST_SUPPORT */
#endif


/**************************************************************************
 * Function Name	: XMLMain_RegisterAll                                 *
 * Description		: Registers all XML callback functions.               *
 * Input			: None                                                *
 * Output		: None                                                    *
 * Return value	: None                                                    *
 **************************************************************************/

void XMLMain_RegisterAll( void )
{
	#ifdef PSP_XML_TEST_SUPPORT
	   /*PSPXmlNetworkGroupHttpInit();*/
	  /* PSPXmlNTGRPHttpInit();*/    /*TODO: move to mgmt.*/
	#endif /* PSP_XML_TEST_SUPPORT */
}

#endif /*PSP_XML_SUPPORT */


