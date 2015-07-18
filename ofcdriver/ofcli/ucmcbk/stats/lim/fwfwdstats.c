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
 * File name: fwfwdstats.c
 * Author: 
 * Date:  
 * Description: Firewall forward Statistics.
 * 
 */

#ifdef OF_CM_SUPPORT

#include "cbcmninc.h"
#include "cmgutil.h"
#include "appids.h"
#include "secappl.h"
#include "cmdefs.h"
#include "urcu-call-rcu.h"
#include "cntlr_lock.h"
#include "cntlr_list.h"
#include "controller.h"
#include "cntlr_epoll.h"
#include "common_nfapi.h"
#include "fw4_nfapi.h"
#include "common_nfapi.h"
#include "of_utilities.h"
#include "dprm.h"
#include "of_nem.h"
#include "fwnat_listener.h"
#include "common_nfapi.h"
#include "pktmbuf.h"
#include "of_msgapi.h"
#include "mchash.h"
#include "timerapi.h"
#include "fwnat_ldef.h"
#include "fwnat_acl.h"
#include "ipfwd_db.h"

int32_t fw_fwdstats_appl_init (void);

int32_t fw_fwdstats_getexactrec (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs ** result_iv_pairs_pp);

int32_t fw_fwdstats_getparams (struct ip4_fwnat_acl_stats *stats,
    struct cm_array_of_iv_pairs * result_iv_pairs_p);

struct cm_dm_app_callbacks fw_fwdstats_callbacks = 
{
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  fw_fwdstats_getexactrec,
  NULL,
  NULL
};

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Function Name:
 * Description:
 * Input:
 * Output:
 * Result:
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t fw_fwdstats_appl_init (void)
{
  int32_t return_value;

  return_value= cm_register_app_callbacks (CM_ON_DIRECTOR_NAMESPACE_LIM_FIREWALL_STATS_FWFWD_RULESTATS_APPL_ID,
      &fw_fwdstats_callbacks);
  if(return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT("Firewall Forward stats Objects CBK registration failed");
    return OF_FAILURE;
  }

  return OF_SUCCESS;
}
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Function Name:
 * Description:
 * Input:
 * Output:
 * Result:
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32_t fw_fwdstats_getexactrec (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs ** result_iv_pairs_pp)
{
  int32_t return_value = OF_FAILURE;
  struct cm_app_result *fwfwd_stats_result = NULL;
  uint32_t  ii, ns_id, rec_count=0;
  struct cm_array_of_iv_pairs * result_iv_pairs_p;
  struct ip4_fwnat_acl_stats fwfwd_stats={};

  if ((fw_fwdstats_setmandparams (pKeysArr, &ns_id, &fwfwd_stats_result)) !=
      OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
    return OF_FAILURE;
  }

  //return_value = get_fwnat_acl_lim_stats(0, &fwfwd_stats) ;
  //return_value = get_fwnat_conninfo_lim_stats(0, &fwfwd_stats) ;
  if (return_value == OF_SUCCESS)
  {
    result_iv_pairs_p =
             (struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));
    if (result_iv_pairs_p == NULL)
    {
      CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
      return OF_FAILURE;
    }
    fw_fwdstats_getparams(&fwfwd_stats, &result_iv_pairs_p[rec_count]); 
    rec_count ++;
    *result_iv_pairs_pp = result_iv_pairs_p;
    CM_CBK_DEBUG_PRINT ("Firewall Forward stats response");
    return OF_SUCCESS;
  }
  *result_iv_pairs_pp = NULL;
  CM_CBK_DEBUG_PRINT ("Firewall Forward stats response failed");
  return OF_FAILURE;
}

int32_t fw_fwdstats_setmandparams (struct cm_array_of_iv_pairs *pMandParams,	
	           uint32_t *ns_id, struct cm_app_result ** result_pp)
{
	 uint32_t uiMandParamCnt;

	 CM_CBK_DEBUG_PRINT ("Entered");
	 for (uiMandParamCnt = 0; uiMandParamCnt < pMandParams->count_ui;
				 uiMandParamCnt++)
	 {
			switch (pMandParams->iv_pairs[uiMandParamCnt].id_ui)
			{

				 case CM_DM_NAMESPACE_NAME_ID:
                                        *ns_id=of_atoi((char *) pMandParams->iv_pairs[uiMandParamCnt].value_p);
				        break;

			}
	 }

	 CM_CBK_PRINT_IVPAIR_ARRAY (pMandParams);
	 return OF_SUCCESS;
}

int32_t fw_fwdstats_getparams (struct ip4_fwnat_acl_stats *stats,
                     struct cm_array_of_iv_pairs *result_iv_pairs_p)
{
  char string[64];
  int32_t index = 0;

#define CM_FWFWDSTATS_CHILD_COUNT 20

  CM_CBK_DEBUG_PRINT ("Entered");
  result_iv_pairs_p->iv_pairs = (struct cm_iv_pair*)of_calloc(CM_FWFWDSTATS_CHILD_COUNT, sizeof(struct cm_iv_pair));
  if(!result_iv_pairs_p->iv_pairs)
  {
    CM_CBK_DEBUG_PRINT (" Memory allocation failed");
    return OF_FAILURE;
  }

  of_memset(string, 0, sizeof(string));
  of_itoa (stats->nl_events_received_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index], CM_DM_FW_FWDSTATS_EVENTS_RECEIVED_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa ((stats->entry_add_count - stats->entry_del_count), string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FW_FWDSTATS_ENTRY_ACTIVE_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->entry_mod_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FW_FWDSTATS_ENTRY_MOD_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->entry_add_fail_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FW_FWDSTATS_ENTRY_ADD_FAIL_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->entry_del_fail_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FW_FWDSTATS_ENTRY_DEL_FAIL_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  result_iv_pairs_p->count_ui = index;

  CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);
  return OF_SUCCESS;
}

#endif /*OF_CM_SUPPORT */

