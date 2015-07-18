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
 * File name: routestats.c
 * Author: 
 * Date:  
 * Description: Arp lim Statistics.
 * 
 */

#ifdef OF_CM_SUPPORT

#include "cbcmninc.h"
#include "cmgutil.h"
#include "appids.h"
#include "secappl.h"
#include "cntlr_lock.h"
#include "urcu-call-rcu.h"
#include "cntlr_list.h"
#include "openflow.h"
#include "cntlr_types.h"
#include "of_utilities.h"
#include "of_msgapi.h"
#include "dprm.h"
#include "of_nem.h"
#include "netlink_listener.h"
#include "ip4_rt_api.h"

int32_t route_stats_appl_init (void);

int32_t route_stats_getexactrec (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs ** result_iv_pairs_pp);

int32_t route_stats_getparams (struct ip4_unicast_lim_stats *stats, struct cm_array_of_iv_pairs * result_iv_pairs_p);

struct cm_dm_app_callbacks route_stats_callbacks = 
{
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  route_stats_getexactrec,
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
int32_t route_stats_appl_init (void)
{
  int32_t return_value;

  return_value= cm_register_app_callbacks (CM_ON_DIRECTOR_NAMESPACE_LIM_ROUTE_STATS_ROUTESTATS_APPL_ID,
      &route_stats_callbacks);
  if(return_value != OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT("route lim stats CBK registration failed");
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
int32_t route_stats_getexactrec (struct cm_array_of_iv_pairs * pKeysArr,
    struct cm_array_of_iv_pairs ** result_iv_pairs_pp)
{
  int32_t return_value = OF_FAILURE;
  struct cm_app_result *route_stats_result = NULL;
  uint32_t  ns_id, rec_count=0;
  struct cm_array_of_iv_pairs * result_iv_pairs_p;
  struct ip4_unicast_lim_stats route_stats;

  if ((route_stats_setmandparams (pKeysArr, &ns_id, &route_stats_result)) !=
      OF_SUCCESS)
  {
    CM_CBK_DEBUG_PRINT ("Set Mandatory Parameters Failed");
    return OF_FAILURE;
  }

  return_value = get_unicast_lim_stats(0, &route_stats);
  if (return_value == OF_SUCCESS)
  {
    result_iv_pairs_p =
             (struct cm_array_of_iv_pairs *) of_calloc (1, sizeof (struct cm_array_of_iv_pairs));
    if (result_iv_pairs_p == NULL)
    {
      CM_CBK_DEBUG_PRINT ("Memory allocation failed for result_iv_pairs_p");
      return OF_FAILURE;
    }
    route_stats_getparams(&route_stats, &result_iv_pairs_p[rec_count]); 
    rec_count ++;
    *result_iv_pairs_pp = result_iv_pairs_p;
    CM_CBK_DEBUG_PRINT ("Route lim stats response");
    return OF_SUCCESS;
  }
  *result_iv_pairs_pp = NULL;
  CM_CBK_DEBUG_PRINT ("Route lim stats response failed");
  return OF_FAILURE;
}

int32_t route_stats_setmandparams (struct cm_array_of_iv_pairs *pMandParams,	
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

int32_t route_stats_getparams (struct ip4_unicast_lim_stats *stats, struct cm_array_of_iv_pairs * result_iv_pairs_p)
{
  char string[64];
  int32_t index = 0;

#define CM_ROUTESTATS_CHILD_COUNT 20

  CM_CBK_DEBUG_PRINT ("Entered");
  result_iv_pairs_p->iv_pairs = (struct cm_iv_pair*)of_calloc(CM_ROUTESTATS_CHILD_COUNT, sizeof(struct cm_iv_pair));
  if(!result_iv_pairs_p->iv_pairs)
  {
    CM_CBK_DEBUG_PRINT (" Memory allocation failed");
    return OF_FAILURE;
  }

  of_memset(string, 0, sizeof(string));
  of_itoa (stats->nl_total_events_received_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index], CM_DM_ROUTESTATS_NL_TOTAL_EVENTS_RECEIVED_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_itoa (stats->nl_error_events_received_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index], CM_DM_ROUTESTATS_NL_ERROR_EVENTS_RECEIVED_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa ((stats->nl_add_events_count), string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESTATS_NL_ADD_EVENTS_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa ((stats->nl_del_events_count), string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESTATS_NL_DEL_EVENTS_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->nl_add_events_fail_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESTATS_NL_ADD_EVENTS_FAIL_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->nl_del_events_fail_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESTATS_NL_DEL_EVENTS_FAIL_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->pkt_to_peth_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESTATS_PKT_TO_PETH_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->pkt_from_peth_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESTATS_PKT_FROM_PETH_COUNT_ID,CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->pkt_to_dp_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESTATS_PKT_TO_DP_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  of_memset(string, 0, sizeof(string));
  of_ltoa (stats->pkt_from_dp_count, string);
  FILL_CM_IV_PAIR(result_iv_pairs_p->iv_pairs[index],CM_DM_ROUTESTATS_PKT_FROM_DP_COUNT_ID, CM_DATA_TYPE_STRING, string);
  index++;

  result_iv_pairs_p->count_ui = index;

  CM_CBK_PRINT_IVPAIR_ARRAY (result_iv_pairs_p);
  return OF_SUCCESS;
}

#endif /*OF_CM_SUPPORT */
