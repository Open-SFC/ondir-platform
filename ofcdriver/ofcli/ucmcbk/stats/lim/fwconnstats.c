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
 * File name: fwconnstats.c
 * Author: 
 * Date:  
 * Description: Firewall lim Statistics.
 * 
 */

#ifdef OF_CM_SUPPORT

#include "cbcmninc.h"
#include "cmgutil.h"
#include "appids.h"
#include "secappl.h"
#include "cntlr_types.h"
#include "urcu-call-rcu.h"
#include "cntlr_lock.h"
#include "mchash.h"
#include "timerapi.h"
#include "fwnat_ldef.h"
#include "openflow.h"
#include "cntlr_list.h"
#include "of_utilities.h"
#include "of_msgapi.h"
#include "dprm.h"
#include "of_nem.h"
#include "fwnat_listener.h"
#include "common_nfapi.h"
#include "pktmbuf.h"
#include "fwnat_conninfo.h"

int32_t fwconn_stats_appl_init (void);

int32_t fwconn_stats_getexactrec (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs ** result_iv_pairs_pp);

int32_t fwconn_stats_getparams (struct ip4_fwnat_conninfo_lim_stats *stats, struct cm_array_of_iv_pairs * result_iv_pairs_p);

struct cm_dm_app_callbacks fwconn_stats_callbacks = 
{
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  fwconn_stats_getexactrec,
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
int32_t fwconn_stats_appl_init (void)
{
  int32_t return_value;

  return_value= cm_register_app_callbacks (CM_ON_DIRECTOR_NAMESPACE_LIM_FIREWALL_STATS_FWCONNSTATS_APPL_ID,
      &fwconn_stats_callbacks);
  if(return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT("firewall connection track lim stats CBK registration failed");
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
int32_t fwconn_stats_getexactrec (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs ** result_iv_pairs_pp)
{
  int32_t return_value = OF_FAILURE;
  struct cm_app_result *fwconn_stats_result = NULL;
  uint32_t  ns_id, rec_count=0;
  struct cm_array_of_iv_pairs * result_iv_pairs_p;
  struct ip4_fwnat_conninfo_lim_stats fwconn_stats;

  if ((fwconn_stats_setmandparams (pKeysArr, &ns_id, &fwconn_stats_result)) !=
      OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
    return OF_FAILURE;
  }

  return_value = get_fwnat_conninfo_lim_stats(&ns_id, &fwconn_stats) ;
  if (return_value == OF_SUCCESS)
  {
    result_iv_pairs_p =
             (struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));
    if (result_iv_pairs_p == NULL)
    {
      CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
      return OF_FAILURE;
    }
    fwconn_stats_getparams(&fwconn_stats, &result_iv_pairs_p[rec_count]); 
    rec_count ++;
    *result_iv_pairs_pp = result_iv_pairs_p;
    CM_CBK_DEBUG_PRINT ("Firewall lim stats response");
    return OF_SUCCESS;
  }
  *result_iv_pairs_pp = NULL;
  CM_CBK_DEBUG_PRINT ("Firewall connection limints stats response failed");
  return OF_FAILURE;
}

int32_t fwconn_stats_setmandparams (struct cm_array_of_iv_pairs *pMandParams,	
	           uint32_t *ns_id, struct cm_app_result ** result_pp)
{
   uint32_t uiMandParamCnt, nsid;
   char ns_name[5];

   CM_CBK_DEBUG_PRINT ("Entered");
   for (uiMandParamCnt = 0; uiMandParamCnt < pMandParams->count_ui;
			uiMandParamCnt++)
   {
   	switch (pMandParams->iv_pairs[uiMandParamCnt].id_ui)
   	{
          case CM_DM_NAMESPACE_NAME_ID:
           {
	      strcpy(ns_name,(char *) pMandParams->iv_pairs[uiMandParamCnt].value_p);
	      if (of_nem_get_nsid_from_namespace(ns_name, &nsid) == OF_SUCCESS)
	      {
		CM_CBK_DEBUG_PRINT("Conversion from nsid to string success");
		*ns_id = nsid;   
	      }
	   }
          //*ns_id=of_atoi((char *) pMandParams->iv_pairs[uiMandParamCnt].value_p);
          break;
	}
    }
    CM_CBK_PRINT_IVPAIR_ARRAY (pMandParams);
    return OF_SUCCESS;
}

int32_t fwconn_stats_getparams (struct ip4_fwnat_conninfo_lim_stats *stats, struct cm_array_of_iv_pairs * result_iv_pairs_p)
{
  char string[64];
  int32_t index = 0;

#define CM_FWCONNSTATS_CHILD_COUNT 20

  CM_CBK_DEBUG_PRINT ("Entered");
  result_iv_pairs_p->iv_pairs = (struct cm_iv_pair*)of_calloc(CM_FWCONNSTATS_CHILD_COUNT, sizeof(struct cm_iv_pair));
  if(!result_iv_pairs_p->iv_pairs)
  {
    CM_CBK_DEBUG_PRINT (" Memory allocation failed");
    return OF_FAILURE;
  }

  of_memset(string, 0, sizeof(string));
  of_itoa (stats->nl_total_ct_new_events_received, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index], CM_DM_FWCONNSTATS_NL_TOTAL_CT_NEW_EVENTS_RECEIVED_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_itoa (stats->nl_total_ct_update_events_received, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index], CM_DM_FWCONNSTATS_NL_TOTAL_CT_UPDATE_EVENTS_RECEIVED_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_itoa (stats->nl_total_ct_del_events_received, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index], CM_DM_FWCONNSTATS_NL_TOTAL_CT_DEL_EVENTS_RECEIVED_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_itoa (stats->nl_error_events_received_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index], CM_DM_FWCONNSTATS_NL_ERROR_EVENTS_RECEIVED_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa ((stats->entry_add_count - stats->entry_del_count), string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FWCONNSTATS_ENTRY_ACTIVE_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->entry_mod_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FWCONNSTATS_ENTRY_MOD_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->entry_add_fail_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FWCONNSTATS_ENTRY_ADD_FAIL_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->entry_mod_fail_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FWCONNSTATS_ENTRY_MOD_FAIL_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->entry_del_fail_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FWCONNSTATS_ENTRY_DEL_FAIL_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->pkt_to_peth_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FWCONNSTATS_PKT_TO_PETH_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->pkt_from_peth_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FWCONNSTATS_PKT_FROM_PETH_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->pkt_to_dp_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FWCONNSTATS_PKT_TO_DP_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->pkt_from_dp_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_FWCONNSTATS_PKT_FROM_DP_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  result_iv_pairs_p->count_ui = index;

  CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);
  return OF_SUCCESS;
}

#endif /*OF_CM_SUPPORT */
