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

#ifdef OF_CM_DP_CHANNEL_SUPPORT

#include"cbcmninc.h"
#include "dpcbk.h"


void UMDPInit(void);

struct cm_dm_channel_callbacks UCMDPCallbakFunctions = {
	 UCMDP_BeginConfigTransaction,
	 UCMDP_AddRecord,
	 UCMDP_SetRecord,
	 UCMDP_DelRecord,
	 UCMDP_EndConfigTransaction,
	 UCMDP_GetFirstNRecs,
	 UCMDP_GetNextNRecs,
	 UCMDP_GetExactRec,
	 UCMDP_VerifyConfig
};


/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Function Name:
 * Description:
 * Input:
 * Output:
 * Result:
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void UCMDPInit(void)
{
	 int32_t return_value;

	 CM_CBK_DEBUG_PRINT ("Entering");

	 return_value=cm_register_channel(CM_DP_CHANNEL_ID,&UCMDPCallbakFunctions);
	 if (return_value != OF_SUCCESS)
	 {
			CM_CBK_DEBUG_PRINT (" RegisterDataElements : ofpsdnDataElements, Error : %d", return_value);
			return;
	 }

	 CM_CBK_DEBUG_PRINT ("Registered DP Channel successfully");
}
#endif
