/* 
 *
 * Copyright  2012, 2013  Freescale Semiconductor
 *
 *
 * Licensed under the Apache License, version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
*/

/*
 *
 * File name: mccfg.c
 * Author: G Atmaram <B38856@freescale.com>
 * Date:   01/23/2013
 * Description: 
*/

#ifdef CM_MESGCLNT_SUPPORT
#include "cmincld.h"
//#include "basic_tunable.h"


/*
 *  * Maximum Threshold Q length.
 *   */
#define CM_MSGS_MAX_Q_THRESHOLD     75

/*
 *  * Maximum Threshold time in minutes.
 *   */
#define CM_MSGS_MAX_TIME_THRESHOLD  60 

/****************************************************************************
 * FunctionName  : UCMMSGSGetMaxThresQandTime
 * Description   : Maximum Threshold Q length and Threshold time.
 * Input         : NONE
 * Output        : Threshold Q and time
 * Return value  : NONE
 ***************************************************************************/
void UCMMSGSGetMaxThresQandTime(uint16_t *pusMaxThresQLen, uint16_t *pusMaxThresTime);
void UCMMSGSGetMaxThresQandTime(uint16_t *pusMaxThresQLen, uint16_t *pusMaxThresTime)
{
  if(pusMaxThresQLen)
  {
    *pusMaxThresQLen   = CM_MSGS_MAX_Q_THRESHOLD;
  }
  
  if(pusMaxThresTime)
  {
    *pusMaxThresTime = CM_MSGS_MAX_TIME_THRESHOLD;
  }

  return;
}
#endif /*CM_MESGCLNT_SUPPORT*/
