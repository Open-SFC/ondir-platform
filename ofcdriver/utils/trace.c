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
 * File name: trace.c
 * Author: 
 * Date:  
 * Description:This file is used to set and get log messages severity for logging.
 * 
 */


#include "oflog.h"
#include "controller.h"


trace_params_t trace_params_values[]={
	//OF_LOG_MOD 
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_XPRT_PROTO 
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_NEM_LOG
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_RT_LOG
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_PETH_APP
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_SAMPLE_APP
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_ARP_APP
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_ARP_NF
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_FWD_NF
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_FWD_NF
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_MCAST_APP
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_INFRA_APP
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_IPSEC
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_NETLINK
	{TRACE_MODULE_SET,OF_LOG_ERROR},
        //OF_LOG_QOS_APP
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_QOS_NF
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_FWNAT_
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_UTIL
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_CRM
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_NSRM
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_CNTLR_TEST
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_UTIL_L3EXT
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_EXT_NICIRA
	{TRACE_MODULE_SET,OF_LOG_ERROR},
        //OF_LOG_QOS
	{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_TTP
	{TRACE_MODULE_SET,OF_LOG_ERROR},
        //OF_LOG_TSC
	{TRACE_MODULE_SET,OF_LOG_ERROR},
        //OF_LOG_BIND_OBJ,
	{TRACE_MODULE_SET,OF_LOG_ERROR},
        //OF_LOG_ARP_SUB_TTP,
	{TRACE_MODULE_SET,OF_LOG_ERROR},
        //OF_LOG_L3_SUB_TTP
	{TRACE_MODULE_SET,OF_LOG_ERROR},
        //OF_LOG_QOS_SUB_TTP
	{TRACE_MODULE_SET,OF_LOG_ERROR},


	//OF_LOG_ALL
	//{TRACE_MODULE_SET,OF_LOG_ERROR},
	//OF_LOG_NONE
	//{TRACE_MODULE_SET,OF_LOG_NONE},

};



/************************ set and get trace level********************/
/**************************************************************************
 * Function Name : of_set_trace_level                                     *
 * Description   : This function sets the trace level with given input    *
 * Input         : value                                                  *
 * Output        : NONE                                                   *
 * Return value  : NONE                                                   *
 **************************************************************************/
void of_set_trace_level(int trace_module,int level_value)
{
  int ii;
  OF_LOG_MSG(OF_LOG_MOD,OF_LOG_INFO,"Module is %d  level is %d ",trace_module,level_value);
  if(trace_module == OF_LOG_MODULE_ALL){
    for(ii=0;ii<(TOTAL_LOG_MODULES-1);ii++){
      trace_params_values[ii].is_trace_module_set = TRACE_MODULE_SET;
      if(level_value != -1)
      trace_params_values[ii].trace_level = level_value;
      }
    }
  else if(trace_module == OF_LOG_MODULE_NONE){
    for(ii=0;ii<TOTAL_LOG_MODULES;ii++){
      trace_params_values[ii].is_trace_module_set = TRACE_MODULE_NOT_SET;
    }
  }
  else {
  trace_params_values[trace_module].is_trace_module_set = TRACE_MODULE_SET;
  if(level_value != -1)
    trace_params_values[trace_module].trace_level = level_value;
  }
	
}

